#include<stdio.h>
#include<stdlib.h>
#include<string.h> 
#include<string.h>
#include<pthread.h>

#include "translation.h"


int main(void){
    vm_map(0,3,100,0);
    printf("Mapping 100 pages from 0 to frame 3\n");

    for(int i = 0; i < 100; i ++){
        uint64_t address = vm_translate(3000 + i);
        
        printf("address: %llu is  %s\n", address, (address == FRAME_ADDRESS(3) + 3000 + i) ? "mapped correctly" : "mapped incorrectly");
        //printf("%llu\n", address);
    }

    vm_map(10,0,10,0);

    printf("mapping 10 pages from page 10 to frame 0\n");
    for(int i = 0; i < 100; i ++){
        uint64_t address = vm_translate(3000 + i);
        
        printf("address: %llu is  %s\n", address,address == FRAME_ADDRESS(3) + 3000 + i ? "mapped correctly" : "mapped incorrectly");
        //printf("%llu\n", address);
    }

    
   
    
}



