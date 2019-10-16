#include<stdio.h>

#include "translation.h"

void *table_root = NULL;

int initialize_table(entry* table){
	for(int i = 0; i < 512; i++){
		table[i].flags = 0;
	}

	return 1;
}
//assume you have a 36 bit number
uint64_t get_bits(uint64_t page, int level){

	uint64_t chunk = (page >>(36 - 9 *(level))) & 0x1ff;

	return chunk;
}

//make sure to link parent to child table
//page is a 36 bit number already 
//mapping 36 bit page to 48 bit frame
int vm_map(uint64_t page, uint64_t frame, int number, uint16_t flags) {
	uint64_t frame_number;
	uint64_t frame_address;
	if(table_root == NULL){
		frame_number = allocate_frame(1);

		if(frame_number == -1){
			return 0;
		}

		frame_address = (uint64_t)FRAME_ADDRESS(frame_number);
		table_root = (entry * ) frame_address;
		initialize_table(table_root);
	}
	//last flag is not reading as zero? 
	for(int i = 0; i < number; i ++){
		entry* current_table = table_root;
		int level = 1;
		while(1){
			uint64_t table_entry = get_bits(page, level);
			// printf("map bits: %llu\n", table_entry);
			//table for that path hasn't been created yet 
			if(current_table[table_entry].flags == 0){
				//store frame in entry of last page number 
				if(level == 4){
					frame_address = (uint64_t)FRAME_ADDRESS(frame);
					current_table[table_entry].flags = 1;
					current_table[table_entry].address = frame_address;
					break;
				}
				//need to create next table level down
				else{
					frame_number = allocate_frame(1);
					//if no more room
					if(frame_number == -1){
						return 0;
					}

					frame_address = (uint64_t)FRAME_ADDRESS(frame_number);
					current_table[table_entry].address = frame_address;
					current_table[table_entry].flags = 1;
					current_table = (entry*)current_table[table_entry].address;
				}
			}
			else{
				//store frame in entry of last page number 
				if(level == 4){
					frame_address = (uint64_t)FRAME_ADDRESS(frame);
					current_table[table_entry].address= frame_address;
					current_table[table_entry].flags = 1;
					break;
				}
				//entry to next level has been already created, just need to traverse down
				else{
					current_table = (entry*)current_table[table_entry].address;
				}
			}
			level++;
		}
		page += 1;
		frame += 1;
	}
	//put a comment somewhere in the code where you would actually would change the flags 
	
	return 1;
}

int vm_unmap(uint64_t page, int number) {
	for(int i = 0; i<number; i++){
		entry* current_entry = table_root;
		page+=i;
		int level = 1;
		if(current_entry == NULL){
			return 0;
		}
		while(level<5){
			uint64_t table_entry = get_bits(page, level);
			//table for that path hasn't been created yet, can't unmap
			if(current_entry->flags == 0){
				return 0;
			}
			current_entry = (entry*)current_entry[table_entry].address;
			level++;
		}
		current_entry->flags = 0;
	}
	return 1;
}

uint64_t vm_locate(int number) {
	int unmapped = 0;

	for(int i = 0; i<512; i ++){
		if(number == unmapped){
			return (i-number);
		} 
		if(vm_translate(i) == -1){
			unmapped++;
		}
		else{
			unmapped = 0;
		}
	}
	return -1;
}

uint64_t vm_translate(uint64_t virtual_address) {
	entry* current_table = table_root;
	uint64_t page_number = (virtual_address/4096);
	if(current_table == NULL){
		return -1;
	}

	for(int i = 1; i < 5 ; i++){
		uint64_t entry_bits = get_bits(page_number,i);
		// printf("translate bits: %llu\n", entry_bits);
		//or if you find an page that is unmapped
		if(current_table[entry_bits].flags == 0){
			return -1;
		}	
		if(i == 4){
			//look at virtual offset
			//get address of that entry 
			uint64_t offset = virtual_address%4096;
			
			return current_table[entry_bits].address + offset;
		}
		else{
			current_table = (entry*)current_table[entry_bits].address;
		}
	}

	//if you don't find an allocated address
	
	//have to have two functions
	//offset is taking virtual address 
	//mod of virtual offset by 4069
	return -1;
}
