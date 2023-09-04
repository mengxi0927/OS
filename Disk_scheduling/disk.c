#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "oslabs.h"

struct RCB handle_request_arrival_fcfs(struct RCB request_queue[QUEUEMAX], int *queue_cnt, struct RCB current_request, struct RCB new_request, int timestamp) {
    // If the disk is free, return the newly-arriving request
    if (current_request.request_id == 0 && current_request.arrival_timestamp == 0 && current_request.cylinder == 0 && current_request.address == 0 && current_request.process_id == 0) {
        return new_request;
    }

    // Otherwise, add the newly-arriving request to the request queue and return the currently-serviced request
    if (*queue_cnt < QUEUEMAX) {
        request_queue[*queue_cnt] = new_request;
        (*queue_cnt)++;
    } else {
        printf("Error: Request queue is full!\n");
    }

    return current_request;
}


struct RCB handle_request_completion_fcfs(struct RCB request_queue[QUEUEMAX], int *queue_cnt) {
    // If the request queue is empty, return a NULLRCB
    if (*queue_cnt == 0) {
        struct RCB null_rcb = {0, 0, 0, 0, 0};
        return null_rcb;
    }

    // Find the RCB in the request queue with the earliest arrival time
    int earliest_arrival_idx = 0;
    int earliest_arrival_time = INT_MAX;
    for (int i = 0; i < *queue_cnt; i++) {
        if (request_queue[i].arrival_timestamp < earliest_arrival_time) {
            earliest_arrival_time = request_queue[i].arrival_timestamp;
            earliest_arrival_idx = i;
        }
    }

    // Remove the RCB from the request queue and update the queue count
    struct RCB next_request = request_queue[earliest_arrival_idx];
    for (int i = earliest_arrival_idx; i < *queue_cnt - 1; i++) {
        request_queue[i] = request_queue[i + 1];
    }
    (*queue_cnt)--;

    // Return the RCB with the earliest arrival time
    return next_request;
}

struct RCB handle_request_arrival_sstf(struct RCB request_queue[QUEUEMAX], int *queue_cnt, struct RCB current_request, struct RCB new_request, int timestamp) {
    // If the disk is free, return the newly-arriving request
    if (current_request.request_id == 0 && current_request.arrival_timestamp == 0 && current_request.cylinder == 0 && current_request.address == 0 && current_request.process_id == 0) {
        return new_request;
    }

    // Otherwise, add the newly-arriving request to the request queue and return the currently-serviced request
    if (*queue_cnt < QUEUEMAX) {
        request_queue[*queue_cnt] = new_request;
        (*queue_cnt)++;
    } else {
        printf("Error: Request queue is full!\n");
    }

    return current_request;
}



struct RCB handle_request_completion_sstf(struct RCB request_queue[QUEUEMAX], int *queue_cnt, int current_cylinder) {
    // If the request queue is empty, return a NULLRCB
    if (*queue_cnt == 0) {
        struct RCB null_rcb = {0, 0, 0, 0, 0};
        return null_rcb;
    }

    // Find the RCB in the request queue with the closest cylinder and earliest arrival timestamp
    int closest_cylinder_idx = 0;
    int min_seek_distance = INT_MAX;
    int earliest_arrival_time = INT_MAX;
    for (int i = 0; i < *queue_cnt; i++) {
        int seek_distance = abs(request_queue[i].cylinder - current_cylinder);
        if (seek_distance < min_seek_distance || (seek_distance == min_seek_distance && request_queue[i].arrival_timestamp < earliest_arrival_time)) {
            min_seek_distance = seek_distance;
            earliest_arrival_time = request_queue[i].arrival_timestamp;
            closest_cylinder_idx = i;
        }
    }

    // Remove the RCB from the request queue and update the queue count
    struct RCB next_request = request_queue[closest_cylinder_idx];
    for (int i = closest_cylinder_idx; i < *queue_cnt - 1; i++) {
        request_queue[i] = request_queue[i + 1];
    }
    (*queue_cnt)--;

    // Return the RCB with the closest cylinder and earliest arrival timestamp
    return next_request;
}



struct RCB handle_request_arrival_look(struct RCB request_queue[QUEUEMAX], int *queue_cnt, struct RCB current_request, struct RCB new_request, int timestamp) {
    // If the disk is free, return the newly-arriving request
    if (current_request.request_id == 0 && current_request.arrival_timestamp == 0 && current_request.cylinder == 0 && current_request.address == 0 && current_request.process_id == 0) {
        return new_request;
    }

    // Otherwise, add the newly-arriving request to the request queue and return the currently-serviced request
    if (*queue_cnt < QUEUEMAX) {
        request_queue[*queue_cnt] = new_request;
        (*queue_cnt)++;
    } else {
        printf("Error: Request queue is full!\n");
    }

    return current_request;
}


struct RCB handle_request_completion_look(struct RCB request_queue[QUEUEMAX], int *queue_cnt, int current_cylinder, int scan_direction) {
    // If the request queue is empty, return a NULLRCB
    if (*queue_cnt == 0) {
        struct RCB null_rcb = {0, 0, 0, 0, 0};
        return null_rcb;
    }

    int closest_cylinder_idx = -1;
    int min_seek_distance = INT_MAX;
    int earliest_arrival_time = INT_MAX;

    // Iterate through the request queue to find the next request to service
    for (int i = 0; i < *queue_cnt; i++) {
        int seek_distance = abs(request_queue[i].cylinder - current_cylinder);
        int same_cylinder = (seek_distance == 0);
        int valid_direction;

        if (scan_direction == 1) {
            valid_direction = request_queue[i].cylinder >= current_cylinder;
        } else {
            valid_direction = request_queue[i].cylinder <= current_cylinder;
        }

        if (valid_direction && (seek_distance < min_seek_distance || (seek_distance == min_seek_distance && request_queue[i].arrival_timestamp < earliest_arrival_time) || closest_cylinder_idx == -1)) {
            closest_cylinder_idx = i;
            min_seek_distance = seek_distance;
            earliest_arrival_time = request_queue[i].arrival_timestamp;
        }
    }

    if (closest_cylinder_idx == -1) {
        if (scan_direction == 1) {
            scan_direction = 0;
        } else {
            scan_direction = 1;
        }

        min_seek_distance = INT_MAX;
        earliest_arrival_time = INT_MAX;

        for (int i = 0; i < *queue_cnt; i++) {
            int seek_distance = abs(request_queue[i].cylinder - current_cylinder);
            int valid_direction;

            if (scan_direction == 1) {
                valid_direction = request_queue[i].cylinder >= current_cylinder;
            } else {
                valid_direction = request_queue[i].cylinder <= current_cylinder;
            }

            if (valid_direction && (seek_distance < min_seek_distance || (seek_distance == min_seek_distance && request_queue[i].arrival_timestamp < earliest_arrival_time) || closest_cylinder_idx == -1)) {
                closest_cylinder_idx = i;
                min_seek_distance = seek_distance;
                earliest_arrival_time = request_queue[i].arrival_timestamp;
            }
        }
    }

    // Remove the RCB from the request queue and update the queue count
    struct RCB next_request = request_queue[closest_cylinder_idx];
    for (int i = closest_cylinder_idx; i < *queue_cnt - 1; i++) {
        request_queue[i] = request_queue[i + 1];
    }
    (*queue_cnt)--;

    // Return the RCB of the next request to service
    return next_request;
}
















