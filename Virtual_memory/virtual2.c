#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define INT_MAX __INT_MAX__
#include "oslabs.h"


int process_page_access_fifo(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    int i, min_arriv_ts, victim_page, fault_count = 0;
    int timestamp = 1;
    
    for (i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid == 1 && page_table[i].frame_number == page_number) {
            // Page is already in memory
            page_table[i].last_access_timestamp = current_timestamp;
            page_table[i].reference_count++;
            return page_table[i].frame_number;
        }
    }
    
    // Page is not in memory
    if (*frame_cnt > 0) {
        // There are free frames in the process frame pool
        page_table[*table_cnt].frame_number = frame_pool[0];
        page_table[*table_cnt].is_valid = 1;
        page_table[*table_cnt].arrival_timestamp = current_timestamp;
        page_table[*table_cnt].last_access_timestamp = current_timestamp;
        page_table[*table_cnt].reference_count = 1;
        (*table_cnt)++;
        (*frame_cnt)--;
        // Shift frame pool left
        for (i = 0; i < *frame_cnt; i++) {
            frame_pool[i] = frame_pool[i+1];
        }
        return page_table[*table_cnt-1].frame_number;
    } else {
        // No free frames, page replacement is required
        min_arriv_ts = -1;
        victim_page = -1;
        for (i = 0; i < *table_cnt; i++) {
            if (page_table[i].is_valid == 1 && (min_arriv_ts == -1 || page_table[i].arrival_timestamp < min_arriv_ts)) {
                min_arriv_ts = page_table[i].arrival_timestamp;
                victim_page = i;
            }
        }
        page_table[victim_page].is_valid = 0;
        frame_pool[(*frame_cnt)++] = page_table[victim_page].frame_number;
        page_table[victim_page].frame_number = -1;
        page_table[victim_page].arrival_timestamp = -1;
        page_table[victim_page].last_access_timestamp = -1;
        page_table[victim_page].reference_count = -1;
        // Add new page to table
        page_table[victim_page].frame_number = frame_pool[0];
        page_table[victim_page].is_valid = 1;
        page_table[victim_page].arrival_timestamp = current_timestamp;
        page_table[victim_page].last_access_timestamp = current_timestamp;
        page_table[victim_page].reference_count = 1;
        // Shift frame pool left
        for (i = 0; i < *frame_cnt; i++) {
            frame_pool[i] = frame_pool[i+1];
        }
        return page_table[victim_page].frame_number;
    }
}




int count_page_faults_fifo(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int i, j, timestamp = 1, page_faults = 0;

    for (i = 0; i < reference_cnt; i++) {
        int page_num = reference_string[i];

        if (!page_table[page_num].is_valid) {
            if (frame_cnt > 0) { // There are free frames
                int frame_num = frame_pool[--frame_cnt];
                page_table[page_num].frame_number = frame_num;
                page_table[page_num].is_valid = 1;
                page_table[page_num].arrival_timestamp = timestamp++;
                page_table[page_num].last_access_timestamp = timestamp++;
                page_table[page_num].reference_count = 1;
                page_faults++;
            }
            else { 
                int first_in_index = -1;
                int first_in_timestamp = INT_MAX;

                for (j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid && page_table[j].arrival_timestamp < first_in_timestamp) {
                        first_in_index = j;
                        first_in_timestamp = page_table[j].arrival_timestamp;
                    }
                }

                // Replace page
                page_table[first_in_index].is_valid = 0;
                frame_pool[frame_cnt++] = page_table[first_in_index].frame_number;
                page_table[first_in_index].frame_number = -1;
                page_table[first_in_index].arrival_timestamp = -1;
                page_table[first_in_index].last_access_timestamp = -1;
                page_table[first_in_index].reference_count = -1;

                // Add new page to table
                page_table[page_num].frame_number = frame_pool[--frame_cnt];
                page_table[page_num].is_valid = 1;
                page_table[page_num].arrival_timestamp = timestamp++;
                page_table[page_num].last_access_timestamp = timestamp++;
                page_table[page_num].reference_count = 1;
                page_faults++;
            }
        }
    }
    return page_faults;
}



int process_page_access_lru(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    int i, frame_number;

    // Check if page is in memory
    for (i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid && page_table[i].frame_number == page_number) {
            page_table[i].last_access_timestamp = current_timestamp;
            page_table[i].reference_count++;
            return page_number;
        }
    }

    // Check if there are any free frames
    if (*frame_cnt > 0) {
        frame_number = frame_pool[*frame_cnt - 1];
        (*frame_cnt)--;
    } else {
        // Find page with smallest last_access_timestamp
        int min_index = -1;
        int min_timestamp = current_timestamp + 1;
        for (i = 0; i < *table_cnt; i++) {
            if (page_table[i].is_valid && page_table[i].last_access_timestamp < min_timestamp) {
                min_index = i;
                min_timestamp = page_table[i].last_access_timestamp;
            }
        }
        frame_number = page_table[min_index].frame_number;
        page_table[min_index].is_valid = 0;
        page_table[min_index].frame_number = -1;
        page_table[min_index].arrival_timestamp = -1;
        page_table[min_index].last_access_timestamp = -1;
        page_table[min_index].reference_count = -1;
    }

    // Add page to memory
    page_table[*table_cnt].is_valid = 1;
    page_table[*table_cnt].frame_number = frame_number;
    page_table[*table_cnt].arrival_timestamp = current_timestamp;
    page_table[*table_cnt].last_access_timestamp = current_timestamp;
    page_table[*table_cnt].reference_count = 1;
    (*table_cnt)++;

    return frame_number;
}

int count_page_faults_lru(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int i, j, lru_index = 0, page_faults = 0, current_timestamp = 1;

    for (i = 0; i < reference_cnt; i++) {
        int page_num = reference_string[i];
        
        if (page_table[page_num].is_valid) { // Page is already in memory
            page_table[page_num].last_access_timestamp = current_timestamp++;
            page_table[page_num].reference_count++;
        }
        else {
            if (frame_cnt > 0) { // There are free frames in the process frame pool
                page_table[page_num].frame_number = frame_pool[frame_cnt - 1];
                page_table[page_num].is_valid = 1;
                page_table[page_num].arrival_timestamp = current_timestamp++;
                page_table[page_num].last_access_timestamp = current_timestamp++;
                page_table[page_num].reference_count = 1;
                frame_cnt--;
                page_faults++;
            }
            else { // No free frames, page replacement is required
                int last_access_timestamp = INT_MAX;
                int last_access_index = -1;
                for (j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid && page_table[j].last_access_timestamp < last_access_timestamp) {
                        last_access_timestamp = page_table[j].last_access_timestamp;
                        last_access_index = j;
                    }
                }
                page_table[last_access_index].is_valid = 0;
                frame_pool[frame_cnt++] = page_table[last_access_index].frame_number;
                page_table[last_access_index].frame_number = -1;
                page_table[last_access_index].arrival_timestamp = -1;
                page_table[last_access_index].last_access_timestamp = -1;
                page_table[last_access_index].reference_count = -1;
                page_table[page_num].frame_number = frame_pool[frame_cnt - 1];
                page_table[page_num].is_valid = 1;
                page_table[page_num].arrival_timestamp = current_timestamp++;
                page_table[page_num].last_access_timestamp = current_timestamp++;
                page_table[page_num].reference_count = 1;
                frame_cnt--;
                page_faults++;
            }
        }
    }
    
    return page_faults;
}

int process_page_access_lfu(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    int i, min_ref_count = INT_MAX, min_arr_time = INT_MAX, min_index = -1, free_frame = -1;

    // Check if the page is already in memory
    for (i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid && page_table[i].frame_number != -1 && page_table[i].frame_number < *frame_cnt && page_table[i].frame_number >= 0 && page_table[i].frame_number < TABLEMAX) {
            if (i == page_number) {
                page_table[i].last_access_timestamp = current_timestamp;
                page_table[i].reference_count++;
                return page_table[i].frame_number;
            }
        }
    }

    // Check if there are any free frames
    for (i = 0; i < *frame_cnt; i++) {
        if (frame_pool[i] == 0) {
            free_frame = i;
            break;
        }
    }

    // If there is a free frame, assign it to the page
    if (free_frame != -1) {
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = free_frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        frame_pool[free_frame] = 1;
        return free_frame;
    }

    // Select a page to be replaced
    for (i = 0; i < *table_cnt; i++) {
        if (page_table[i].is_valid && page_table[i].frame_number != -1 && page_table[i].frame_number < *frame_cnt && page_table[i].frame_number >= 0 && page_table[i].frame_number < TABLEMAX) {
            if (page_table[i].reference_count < min_ref_count) {
                min_ref_count = page_table[i].reference_count;
                min_arr_time = page_table[i].arrival_timestamp;
                min_index = i;
            }
            else if (page_table[i].reference_count == min_ref_count && page_table[i].arrival_timestamp < min_arr_time) {
                min_arr_time = page_table[i].arrival_timestamp;
                min_index = i;
            }
        }
    }

    // Replace the selected page
    if (min_index != -1) {
        frame_pool[page_table[min_index].frame_number] = 0;
        page_table[min_index].is_valid = 0;
        page_table[min_index].frame_number = -1;
        page_table[min_index].arrival_timestamp = -1;
        page_table[min_index].last_access_timestamp = -1;
        page_table[min_index].reference_count = -1;
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = page_table[min_index].frame_number;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        frame_pool[page_table[min_index].frame_number] = 1;
        return page_table[min_index].frame_number;
    }

    return -1;
}


int count_page_faults_lfu(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int i, j, k, min_ref_cnt, min_arriv_ts, victim_page, fault_count = 0, timestamp = 1;

    for (i = 0; i < reference_cnt; i++) {
        int page_num = reference_string[i];

        if (page_table[page_num].is_valid) { // page hit
            page_table[page_num].last_access_timestamp = timestamp++;
            page_table[page_num].reference_count++;
        } else { // page fault
            if (frame_cnt > 0) { // free frames
                int frame_num = frame_pool[--frame_cnt];
                page_table[page_num].frame_number = frame_num;
                page_table[page_num].is_valid = 1;
                page_table[page_num].reference_count = 1;
                page_table[page_num].arrival_timestamp = page_table[page_num].last_access_timestamp = timestamp++;
                fault_count++;
            } else { // no free frames
                min_ref_cnt = INT_MAX;
                min_arriv_ts = INT_MAX;
                victim_page = -1;
                for (j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid && page_table[j].reference_count < min_ref_cnt) { // find the page with the minimum reference count
                        min_ref_cnt = page_table[j].reference_count;
                        min_arriv_ts = page_table[j].arrival_timestamp;
                        victim_page = j;
                    } else if (page_table[j].is_valid && page_table[j].reference_count == min_ref_cnt && page_table[j].arrival_timestamp < min_arriv_ts) { // if there are multiple pages with the same reference count, find the page with the minimum arrival timestamp
                        min_arriv_ts = page_table[j].arrival_timestamp;
                        victim_page = j;
                    }
                }
                page_table[victim_page].is_valid = 0;
                page_table[victim_page].arrival_timestamp = page_table[victim_page].last_access_timestamp = page_table[victim_page].reference_count = 0;
                page_table[page_num].frame_number = page_table[victim_page].frame_number;
                page_table[page_num].is_valid = 1;
                page_table[page_num].reference_count = 1;
                page_table[page_num].arrival_timestamp = page_table[page_num].last_access_timestamp = timestamp++;
                fault_count++;
            }
        }
    }

    return fault_count;
}

















