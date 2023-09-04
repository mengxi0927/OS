#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define INT_MAX __INT_MAX__
#include "oslabs.h"


int process_page_access_fifo(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    int i, min_arriv_ts, victim_page = 0;

    // check if page number in the table is valid
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }
    
    // Page is not in memory
    if (*frame_cnt > 0) {
        // There are free frames, insert page into table corresponding to page number
        page_table[page_number].frame_number = frame_pool[(*frame_cnt) - 1];
        page_table[page_number].is_valid = 1;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        (*frame_cnt)--;
        return page_table[page_number].frame_number;

    } else {
        // No free frames, find victim page that first arrived
        min_arriv_ts = -1;
        victim_page = -1;
        for (i = 0; i < *table_cnt; i++) {
            if (page_table[i].is_valid && (min_arriv_ts == -1 || page_table[i].arrival_timestamp < min_arriv_ts)) {
                min_arriv_ts = page_table[i].arrival_timestamp;
                victim_page = i;
            }
        }
        // Replace page and insert new page into table corresponding to page number with current timestamp
        page_table[victim_page].is_valid = 0;
        frame_pool[(*frame_cnt)++] = page_table[victim_page].frame_number;
        page_table[victim_page].frame_number = -1;
        page_table[victim_page].arrival_timestamp = -1;
        page_table[victim_page].last_access_timestamp = -1;
        page_table[victim_page].reference_count = -1;

        page_table[page_number].frame_number = frame_pool[(*frame_cnt) - 1];
        page_table[page_number].is_valid = 1;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        (*frame_cnt)--;
        return page_table[page_number].frame_number;
    }
}


int count_page_faults_fifo(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int page_faults = 0;
    int timestamp = 1;

    // print frame pool
    // printf("F");
    // for (int i = 0; i < frame_cnt; i++) {
    //     printf("%d", frame_pool[i]);
    // }

    for (int i = 0; i < reference_cnt; i++) {
        timestamp++;
        int current_page = reference_string[i];
        if (page_table[current_page].is_valid) { // Page is in memory
            page_table[current_page].last_access_timestamp = timestamp;
            page_table[current_page].reference_count++;
        } else {
            if (frame_cnt > 0) {
                // There are free frames, insert page into table corresponding to page number
                page_table[current_page].frame_number = frame_pool[--frame_cnt];
                page_table[current_page].is_valid = true;
                page_table[current_page].arrival_timestamp = timestamp;
                page_table[current_page].last_access_timestamp = timestamp;
            } else {
                // Find victim page that first arrived
                int oldest_arrival = -1;
                int oldest_page = -1;
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid && (oldest_arrival == -1 || page_table[j].arrival_timestamp < oldest_arrival)) {
                        oldest_arrival = page_table[j].arrival_timestamp;
                        oldest_page = j;
                    }
                }
                // Replace page
                page_table[oldest_page].is_valid = false;
                page_table[oldest_page].arrival_timestamp = -1;
                page_table[oldest_page].last_access_timestamp = -1;
                page_table[oldest_page].reference_count = -1;
                page_table[current_page].frame_number = page_table[oldest_page].frame_number;
            }

            // Insert new page
            page_table[current_page].arrival_timestamp = timestamp;
            page_table[current_page].last_access_timestamp = timestamp;
            page_table[current_page].reference_count = 1;
            page_table[current_page].is_valid = true;

            page_faults++;
        }
    }

    return page_faults;
}


int process_page_access_lru(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    int i, least_recent_used, victim_page = 0;

    // check if page number in the table is valid
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }
    
    // Page is not in memory
    if (*frame_cnt > 0) {
        // There are free frames, insert page into table corresponding to page number
        page_table[page_number].frame_number = frame_pool[(*frame_cnt) - 1];
        page_table[page_number].is_valid = 1;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        (*frame_cnt)--;
        return page_table[page_number].frame_number;

    } else {
        // No free frames, find least recently used page and replace it
        least_recent_used = INT_MAX;
        for (i = 0; i < *table_cnt; i++) {
            if (page_table[i].is_valid && (page_table[i].last_access_timestamp < least_recent_used)) {
                least_recent_used = page_table[i].last_access_timestamp;
                victim_page = i;
            }
        }

        // Replace page and insert new page into table corresponding to page number with current timestamp
        page_table[victim_page].is_valid = 0;
        frame_pool[(*frame_cnt)++] = page_table[victim_page].frame_number;
        page_table[victim_page].frame_number = -1;
        page_table[victim_page].arrival_timestamp = -1;
        page_table[victim_page].last_access_timestamp = -1;
        page_table[victim_page].reference_count = -1;

        page_table[page_number].frame_number = frame_pool[(*frame_cnt) - 1];
        page_table[page_number].is_valid = 1;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        (*frame_cnt)--;
        return page_table[page_number].frame_number;
    }
}


int count_page_faults_lru(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int page_faults = 0;
    int timestamp = 1;

    for (int i = 0; i < reference_cnt; i++) {
        timestamp++;
        int current_page = reference_string[i];
        if (page_table[current_page].is_valid) {
            page_table[current_page].last_access_timestamp = timestamp;
            page_table[current_page].reference_count++;
        } else {
            if (frame_cnt > 0) {
                page_table[current_page].frame_number = frame_pool[--frame_cnt];
            } else {
                int least_recent_timestamp = -1;
                int least_recent_page = -1;
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid && (least_recent_timestamp == -1 || page_table[j].last_access_timestamp < least_recent_timestamp)) {
                        least_recent_timestamp = page_table[j].last_access_timestamp;
                        least_recent_page = j;
                    }
                }
                page_table[least_recent_page].is_valid = false;
                page_table[least_recent_page].arrival_timestamp = 0;
                page_table[least_recent_page].last_access_timestamp = 0;
                page_table[least_recent_page].reference_count = 0;

                page_table[current_page].frame_number = page_table[least_recent_page].frame_number;
            }
            page_table[current_page].arrival_timestamp = timestamp;
            page_table[current_page].last_access_timestamp = timestamp;
            page_table[current_page].reference_count = 1;
            page_table[current_page].is_valid = true;

            page_faults++;
        }

    }

    return page_faults;
}

int process_page_access_lfu(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    int i, least_freq_used, victim_page = 0;

    // check if page number in the table is valid
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }
    
    // Page is not in memory
    if (*frame_cnt > 0) {
        // There are free frames, insert page into table corresponding to page number
        page_table[page_number].frame_number = frame_pool[(*frame_cnt) - 1];
        page_table[page_number].is_valid = 1;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        (*frame_cnt)--;
        return page_table[page_number].frame_number;

    } else {
        // No free frames, if process is valid, find the smallest reference count and replace it
        least_freq_used = INT_MAX;
        for (i = 0; i < *table_cnt; i++) {
            if (page_table[i].is_valid && page_table[i].reference_count < least_freq_used) {
                least_freq_used = page_table[i].reference_count;
                victim_page = i;
            }
            // if the reference count is the same, find the oldest page
            else if (page_table[i].is_valid && page_table[i].reference_count == least_freq_used) {
                if (page_table[i].arrival_timestamp < page_table[victim_page].arrival_timestamp) {
                    victim_page = i;
                }
            }
        }

        // Replace page and insert new page into table corresponding to page number with current timestamp
        page_table[victim_page].is_valid = 0;
        frame_pool[(*frame_cnt)++] = page_table[victim_page].frame_number;
        page_table[victim_page].frame_number = -1;
        page_table[victim_page].arrival_timestamp = -1;
        page_table[victim_page].last_access_timestamp = -1;
        page_table[victim_page].reference_count = -1;

        page_table[page_number].frame_number = frame_pool[(*frame_cnt) - 1];
        page_table[page_number].is_valid = 1;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        (*frame_cnt)--;
        return page_table[page_number].frame_number;
    }
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

