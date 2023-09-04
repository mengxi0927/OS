#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include "oslabs.h"

#define BLOCK_SIZE 1024
struct MEMORY_BLOCK NULLBLOCK = {0, 0, 0, 0};

// struct MEMORY_BLOCK {
//         int start_address;
//         int end_address;
//         int segment_size;
//         int process_id; //0 indicates a free block
// };

struct MEMORY_BLOCK best_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[], int *map_cnt, int process_id) {
    struct MEMORY_BLOCK best_block = {0, 0, 0, 0}; // best_block block
    int min_difference = -1;
    int i;
    int index = -1;

    // loop through memory_map to find the best fit block 
    for (i = 0; i < *map_cnt; i++) {
        // printf("start: %d, end: %d, size: %d, pid: %d\n\n", memory_map[i].start_address, memory_map[i].end_address, memory_map[i].segment_size, memory_map[i].process_id);
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            int difference = memory_map[i].segment_size - request_size;
            if (min_difference == -1 || difference < min_difference) {
                best_block = memory_map[i];
                min_difference = difference;
                index = i;
            }
        }
    }

    // printf("best_block: start: %d, end: %d, size: %d, pid: %d\n\n", best_block.start_address, best_block.end_address, best_block.segment_size, best_block.process_id);

    if (min_difference == -1) {
        // no free block found
        return NULLBLOCK;
    } else {
        // update best_block block
        best_block.process_id = process_id;
        int seg_size = best_block.segment_size;
        int last_address = 0;
        int left_size = seg_size - request_size;

        // split block
        memory_map[index].start_address = best_block.start_address;
        memory_map[index].end_address = best_block.start_address + request_size - 1;
        memory_map[index].segment_size = request_size;
        memory_map[index].process_id = process_id;
        best_block = memory_map[index];
        last_address = memory_map[index].end_address;

        // add new block to memory_map
        if (left_size > 0) {
            memory_map[*map_cnt].start_address = last_address + 1;
            memory_map[*map_cnt].end_address = last_address + left_size;
            memory_map[*map_cnt].segment_size = left_size;
            memory_map[*map_cnt].process_id = 0;
            (*map_cnt)++;
        }

        return best_block;
    }
}


struct MEMORY_BLOCK first_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX],int *map_cnt, int process_id) {
    int i;
    struct MEMORY_BLOCK first_block = NULLBLOCK;
    int left_size = 0;
    int start_address = 0;
    int end_address = 0;
    int index = -1;

    // Loop through the memory map
    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            first_block = memory_map[i];
            start_address = first_block.start_address;
            end_address = start_address + request_size - 1;
            index = i;
            break;
        }
    }

    // If no suitable block was found, return the NULLBLOCK
    if (first_block.start_address == 0 && first_block.end_address == 0) {
        return NULLBLOCK;
    }                                          // split the memory block

    left_size = first_block.segment_size - request_size;
    memory_map[index].start_address = start_address;
    memory_map[index].end_address = end_address;
    memory_map[index].segment_size = request_size;
    memory_map[index].process_id = process_id;
    first_block = memory_map[index];
    
    if (left_size > 0) {
        memory_map[*map_cnt].start_address = first_block.end_address + 1;
        memory_map[*map_cnt].end_address = first_block.end_address + left_size;
        memory_map[*map_cnt].segment_size = left_size;
        memory_map[*map_cnt].process_id = 0;
        (*map_cnt)++;
    }

    return first_block;
}


struct MEMORY_BLOCK worst_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    struct MEMORY_BLOCK worst_block = {0, 0, 0, 0}; // worst_block block
    int max_diff = -1;
    int i;
    int index = -1;

    // loop through memory_map to find the best fit block 
    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            int diff = memory_map[i].segment_size - request_size;
            if (max_diff == -1 || diff > max_diff) {
                worst_block = memory_map[i];
                max_diff = diff;
                index = i;
            }
        }
    }

    if (max_diff == -1) {
        return NULLBLOCK;
    } else {
        // update worst_block block
        worst_block.process_id = process_id;
        int seg_size = worst_block.segment_size;
        int last_address = 0;
        int left_size = seg_size - request_size;

        // split block
        memory_map[index].start_address = worst_block.start_address;
        memory_map[index].end_address = worst_block.start_address + request_size - 1;
        memory_map[index].segment_size = request_size;
        memory_map[index].process_id = process_id;
        worst_block = memory_map[index];
        last_address = memory_map[index].end_address;

        // add new block to memory_map
        if (left_size > 0) {
            memory_map[*map_cnt].start_address = last_address + 1;
            memory_map[*map_cnt].end_address = last_address + left_size;
            memory_map[*map_cnt].segment_size = left_size;
            memory_map[*map_cnt].process_id = 0;
            (*map_cnt)++;
        }

        return worst_block;
    }
}


struct MEMORY_BLOCK next_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id, int last_address) {
    int i;
    struct MEMORY_BLOCK next_block = NULLBLOCK;
    int left_size = 0;
    int start_address = 0;
    int end_address = 0;
    int index = -1;

    // Loop through the memory map
    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].start_address < last_address) {
            continue;
        }

        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            next_block = memory_map[i];
            start_address = next_block.start_address;
            end_address = start_address + request_size - 1;
            index = i;
            break;
        }
    }

    // If no suitable block was found, return the NULLBLOCK
    if (next_block.start_address == 0 && next_block.end_address == 0) {
        return NULLBLOCK;
    }                                          // split the memory block

    left_size = next_block.segment_size - request_size;
    memory_map[index].start_address = start_address;
    memory_map[index].end_address = end_address;
    memory_map[index].segment_size = request_size;
    memory_map[index].process_id = process_id;
    next_block = memory_map[index];
    
    if (left_size > 0) {
        memory_map[*map_cnt].start_address = next_block.end_address + 1;
        memory_map[*map_cnt].end_address = next_block.end_address + left_size;
        memory_map[*map_cnt].segment_size = left_size;
        memory_map[*map_cnt].process_id = 0;
        (*map_cnt)++;
    }

    return next_block;
}


void release_memory(struct MEMORY_BLOCK freed_block, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt) {
    // printf("%d - %d, %d, %d \n", freed_block.start_address, freed_block.end_address, freed_block.segment_size, freed_block.process_id);
    // for (int i = 0; i < *map_cnt; i++) {
    //     printf("%d %d %d %d\n", memory_map[i].start_address, memory_map[i].end_address, memory_map[i].segment_size, memory_map[i].process_id);
    // }

    int i;
    int index = -1;
    int start_address = freed_block.start_address;
    int end_address = freed_block.end_address;
    int start = 0;
    int end = 0;

    // find the index of the freed block
    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].start_address == start_address && memory_map[i].end_address == end_address) {
            index = i;
            start = memory_map[i].start_address;
            end = memory_map[i].end_address;
            break;
        }
    }

    // if the freed block is not found, return
    if (index == -1) {
        // printf("The block to be released is not found in the memory map. \n");
        return;
    }

    // update the freed block
    memory_map[index].process_id = 0;

    // merge the freed block with the previous block
    if (index > 0 && memory_map[index - 1].process_id == 0) {
        memory_map[index - 1].end_address = memory_map[index].end_address;
        memory_map[index - 1].segment_size = memory_map[index - 1].segment_size + memory_map[index].segment_size;
        memory_map[index] = memory_map[index - 1];
        index--;

        // remove the merged block
        for (i = index + 1; i < *map_cnt; i++) {
            memory_map[i - 1] = memory_map[i];
        }

        (*map_cnt)--;
    }

    // merge the freed block with the next block
    if (index < *map_cnt - 1 && memory_map[index + 1].process_id == 0) {
        memory_map[index].end_address = memory_map[index + 1].end_address;
        memory_map[index].segment_size = memory_map[index].segment_size + memory_map[index + 1].segment_size;
        memory_map[index + 1] = memory_map[index];

        for (i = index + 1; i < *map_cnt; i++) {
            memory_map[i - 1] = memory_map[i];
        }

        (*map_cnt)--;
    }
}

