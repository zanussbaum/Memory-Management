#include "kmalloc.h"
#include<string.h>

uint64_t* free_list_head = NULL;

uint64_t* create_frame_node(int size){
	//getting pointer to node
	uint64_t frames = (size/4096) + 1;
	uint64_t *address = (uint64_t*)palloc(frames);
	//putting size into node
	*(address) = (frames * 4096) - 2;
	//putting next pointer into node
	*(address + 1) = 0;
	return address;
}

//creating a needed node
uint64_t* create_used_node(uint64_t* free_node, uint64_t size_needed){
	//size of memory in free node
	uint64_t size_free = *(free_node);
	//free node address of next, don't know ifyou need it 
	uint64_t* next = (uint64_t*)(free_node + 1);
	//decrementing size from free node to
	//make size of free node correct
	size_free -= size_needed;

	//if free node is now filled
	//need to update pointer to head 
	//make node needed
	//return address of free node, which is now needed
	if(size_free == 0){
		free_list_head = next;
		*(free_node) = size_needed;
		*(free_node + 1) = 0xc0dec0de;
		return (free_node + 2);
	}

	else{
		//getting address of needed node
		//starting from end of free node and working back
		//casting as char to make sure that you are moving correct amount of bits
		//adding 2 64bit ints so that you can have room for first two ints
		uint64_t *used_node = (free_node + (size_free) + 2*sizeof(uint64_t));
		//setting size
		*(used_node) = size_needed;
		//magic number
		*(used_node + 1) = 0xc0dec0de;
		
		//address pointing to first location of needed memory 
		return (used_node + 2);
	}
}

uint64_t* create_free_chunk(uint64_t *address, uint64_t size){
	*(address) = size;
	add_to_free_list(address);
	return address;
}

void add_to_free_list(uint64_t* new_node){
	uint64_t *next_free = free_list_head;
	uint64_t *last_free = NULL;

	while(next_free < new_node){
		//keep track of last address
		last_free = next_free;
		next_free = (uint64_t*)(next_free + 1);
	}
	*(new_node +1) = (uint64_t)next_free;
	*(last_free + 1) = (uint64_t)new_node;
}

uint64_t find_free_node(uint64_t*check_address){
	uint64_t *next_free = free_list_head;

	while(next_free != check_address){
		//keep track of last address
		//if next node is not in address, return 0
		if(next_free == 0){
			return 0;
		}
		next_free = (uint64_t*)(next_free + 1);
	}

	//if found node, return 1;
	return 1;
}

void coalesce_nodes(uint64_t* new_free_node){
	uint64_t *next_free = free_list_head;
	uint64_t *last_free = NULL;
	int coalesced = 0;

	while(next_free < new_free_node){
		//keep track of last address
		last_free = next_free;
		next_free = (uint64_t*)(next_free + 1);
	}
	//checking to see if coalesced between last and new node
	uint64_t* next_address = (uint64_t*)(last_free + 2 + *(last_free));
	if(next_address == new_free_node){
		*(last_free) += *(new_free_node) + 2;
		*(last_free + 1) = (uint64_t)next_free;
		coalesced++;
	}
	//checking to see if coalesced between new node and next free node
	next_address = (uint64_t*)(new_free_node + 2 + *(new_free_node));
	if(next_address == next_free){
		if(coalesced){
			*(last_free) += *(next_address) + 2;
			next_free = (uint64_t*)(next_free + 1);
			*(last_free + 1) = (uint64_t)next_free;
		}
		else{
			*(new_free_node) += *(next_address) + 2;
			next_free = (uint64_t*)(next_free + 1);
			*(new_free_node + 1) = (uint64_t)next_free;
		}
	}
}

void *palloc(uint64_t number) {
	// TODO:
	// 1) Allocate <parameter:number> frames of memory
	// frame_number = allocate_frame(...)
	uint64_t frame_number = allocate_frame(number);
	//error
	if(frame_number == -1){
		return (void*)-1;
	}
	// 2) Find the first page number (virtual) that is not mapped to a frame (physical), and <parameter:number> of them are consecutive
	// page_number = vm_locate(...)
	uint64_t page_number = vm_locate(number);
	//error
	if(page_number == -1){
		return (void*)-1;
	}
	// 3) IGNORE the result of the previous call, and map the frame number to itself
	// page_number = FRAME_NUMBER (overwrites previous value, for the sake of simulation. See notes below.)
	// vm_map(page_number, frame_number, number, /* use flags = 0 */);
	page_number = frame_number;

	if(!vm_map(page_number,frame_number,number,0)){
		return (void*)-1;
	}

	// 4) Return the address of the first byte of the allocated page [see note below]
	// return PAGE_GET_ADDRESS(page_number);
	
	
	// NOTE:
	// - You are simulating the hardware automatic address translation in software.
	// - The palloc() function would return the address of the first virtual (mapped) address, based on the page number of step (2),
	//   but your application would not understand this since it only understands its own application addressing.
	// - Therefore, you are going to SIMULATE that there is another layer of automatic virtual addressing by
	//   always mapping page number X to the address of a frame number X
	// - Inside a real kernel, you wouldn't perform the overwrite of the page number in step (3),
	//   but the hardware would be performing the automatic translation between page_number to frame_number

	// Make sure, that in the end of this function:
	// vm_translate(page) is FRAME_ADDRESS(frame) 
	if(vm_translate(page_number) != FRAME_ADDRESS(frame_number)){
		return (void*)-1;
	}

	//Every frame/page is size 4k so that's why we multipyly by 4096.
	//We know where the memory address starts so we sum from the beginning
	//to get where the frame/page starts
	//A real kernel would not sume the base memory address because it would overwrite 
	//memory that we don't want to overwrite? 
	//ask about part 3 
	return PAGE_ADDRESS(page_number);
}

void pfree(void* address, uint64_t number) {
	// TODO:
	// 1) Unmap the page number starting at the provided address (the address is the first byte of a page)
	//    This means: 
	uint64_t page_address = ((uint64_t)address/4096);

	uint64_t physical_address = vm_translate(page_address);

	if(physical_address == -1){
		return;
	}
	// 2) Do that for the number of times provided in the parameter number

	// page = PAGE_NUMBER(address)
	// frame = vm_translate(page)
	//
	// vm_unmap(page)
	// deallocate_frame(frame)
	if(!vm_unmap(page_address,number)){
		return;
	}

	if(deallocate_frame(physical_address,number) == -1){
		return;
	}
}

void *kmalloc(uint64_t size) {
	//if list is empty, initialize 
	if(free_list_head == NULL){
		free_list_head = create_frame_node(size);
	}
	
	//current address
	uint64_t* current_address = free_list_head;
	//size of free memory
	uint64_t size_free = *(current_address);
	//pointer to next node
	uint64_t next_free = *(current_address + 1);
	

	uint64_t size_needed = size + 2*sizeof(uint64_t);

	//check if you have enough space in this node
	//if not, need to change to next node
	while(size_free < size_needed){
		//allocating new frame to add more memory if there isn't another frame
		if(next_free == 0){
			//new free node

			//need to pass size 
			uint64_t* new_address = create_frame_node(size);
			*(current_address + 1) = (uint64_t)new_address;
		}
		current_address = (uint64_t*)(current_address + 1);
		next_free = *(current_address + 1);
		size_free = *(current_address);
	}
	//return data part of needed node 
	uint64_t* used_address = create_used_node(current_address,size_needed);

	return used_address+2;
}

void *krealloc(void *address, uint64_t size) {
	uint64_t* current_address = (uint64_t*)(address - 2*sizeof(uint64_t));
	uint64_t current_size = *(current_address);

	if(current_size > size){
		uint64_t num_unmaped = current_size - size;
		uint64_t page_number = (uint64_t)current_address/4096;

		current_size -= size;
		if(!vm_unmap(page_number,num_unmaped)){
			return (void*)-1;
		}

		uint64_t frame_number = vm_translate((uint64_t)address);

		if(frame_number == 0){
			return (void*)-1;
		}

		if(deallocate_frame(frame_number+current_size,num_unmaped) == -1){
			return (void*)-1;
		}

		//create new free node from new address
		uint64_t *new_address = (current_address + 2 + current_size);

		uint64_t *new_free_node = create_free_chunk(new_address, num_unmaped);
		return (current_address + 2);
	}

	else{
		//dont think this needs to be plus 2
		uint64_t* next_address = (uint64_t*)(current_address + current_size + 2*sizeof(uint64_t));

		//if you have another free node next in chunk
		if(find_free_node(next_address)){
			uint64_t size_of_next = *(next_address);
			//if you have enough memory in that free node
			if(size_of_next >=size + 2){
				current_size += size;
				size_of_next-= size;
				if(size_of_next){
					uint64_t* smaller_free_chunk = create_free_chunk((uint64_t*)(next_address + 2 + size),size_of_next);

					return current_address + 2;
				}
			}
			//need to malloc and allocate new memory and copy old memory into new chunk
			else{
				uint64_t* new_frame_address = kmalloc(current_size + size + 2*sizeof(uint64_t));
				memcpy(new_frame_address-2,current_address, current_size + 2*sizeof(uint64_t));
				create_free_chunk(current_address,current_size + 2*sizeof(uint64_t));
				return new_frame_address;
			}
		}
		//if you don't have another free node in the chunk, automatically 
		//need to create new frame 
		//make sure address you return is pointing to the data not the node 
		uint64_t* new_frame_address = kmalloc(current_size + size + 2*sizeof(uint64_t));
		memcpy(new_frame_address-2,current_address, current_size + 2*sizeof(uint64_t));
		create_free_chunk(current_address,current_size + 2*sizeof(uint64_t));
		
		return new_frame_address;
	}
}

void kfree(void *address) {
	uint64_t *node_address = (uint64_t*) (address -2*sizeof(uint64_t));
	uint64_t node_size = *(node_address);

	create_free_chunk(node_address, node_size);

	coalesce_nodes(node_address);
}
