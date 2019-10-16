#include<stdio.h>
#include<stdlib.h>
#include<string.h> 
#include<string.h>
#include<pthread.h>

#include "frame.h"


int main(void){
    frame_init();

    printf("%s\n", "..initializing the frames...");

    int index = allocate_frame(3);

    printf("%s\n", "..allocating 3 frames...");

    printf("%s%d\n", "Allocated starting at positon: ", index);

    index = allocate_frame(5);

    printf("%s\n", "..allocating 5 frames...");

    printf("%s%d\n", "Allocated starting at positon: ", index);

    index = allocate_frame(50);

    printf("%s\n", "..allocating 50 frames...");

    printf("%s%d\n", "Allocated starting at positon: ", index);

    index = deallocate_frame(28, 10);

    printf("%s\n", "..deallocating 10 frames...");

    printf("%s%d\n", "dellocated frames starting at positon: ", index);

    return 0;
}



