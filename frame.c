#include "frame.h"
#define SIZE_MAP 32
#define FRAMES 1024
#include <stdio.h>


uint64_t frames_allocated;
uint64_t frames_available;

// No need to change or initialize this. This is "empty" space that simulate your physical memory.
_Alignas(4096) char memory[MEMORY_SIZE];

// You should define a bitmap to keep track of the allocated frames here.
// Look at the handout for details on size

int32_t bitmap[SIZE_MAP]; //initialized the bitmap


void frame_init() {
	frames_allocated = 0;
	frames_available = 1024;

	for (int i = 0; i < FRAMES; i++){
		bitmap[i] = 0;
	}
}

void change_bit(int i){
	int frame_array = i / 32;
	int frame_index = i % 32;
	if (get_bit(i)){ //if 1
		bitmap[frame_array] = bitmap[frame_array] ^ (1u << (31-frame_index)); //change it to 0
	} else { //if 0
		bitmap[frame_array] = bitmap[frame_array] | (1u << (31-frame_index)); //change it to 1
	}
}

int get_bit(int i){
	int frame_array = i / 32;
	int frame_index = i % 32;
	if (bitmap[frame_array] & (1u << (31 - frame_index))){
		return 1;
	} else {
		return 0;
	}
}

int64_t allocate_frame(int number_frames) {
	int empty_frames = 0;
	for (int i = 0; i < FRAMES; i++){ //loop through every frame
		if (!get_bit(i)){
			empty_frames++;
			if (empty_frames == number_frames){
				int start = i - empty_frames + 1;
				while(empty_frames > 0){
					empty_frames--;
					frames_available--;
					frames_allocated++;
					change_bit(i-empty_frames);
				}
				return start;
			}
		} 
		else {
			empty_frames = 0;
		}
	}
	return -1; //return -1 if couldn't find it 
}

int64_t deallocate_frame(uint64_t frame_number, int number_frames) {
	int occupied_frames = 0;
	int i = frame_number;
	while (i < FRAMES){
		if (get_bit(i)){
			occupied_frames ++;
		} else {
			occupied_frames = 0;
		}
		if (occupied_frames == number_frames){
			int start = i - occupied_frames + 1;
			while(occupied_frames > 0){
				occupied_frames--;
				frames_available++;
				frames_allocated--;
				change_bit(i-occupied_frames);
			}
			return start; 
		}
		i++;
	}
	return -1; 
}

