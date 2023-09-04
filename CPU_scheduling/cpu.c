#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include "oslabs.h"

struct PCB NULLPCB = {0,0,0,0,0,0,0};

//     queue_cnt = 3;
//     struct PCB p1 = {1,1,4,0,0,4,23};
//     struct PCB p2 = {2,1,4,0,0,4,22};
//     struct PCB p3 = {3,1,4,0,0,4,24};
//     ready_queue[0] = p1;
//     ready_queue[1] = p2;
//     ready_queue[2] = p3;

struct PCB handle_process_arrival_pp(
    struct PCB ready_queue[QUEUEMAX], 
    int *queue_cnt, 
    struct PCB current_process, 
    struct PCB new_process, 
    int timestamp
    ) {
    if (current_process.process_id == 0) {
        new_process.execution_starttime = timestamp;
        new_process.execution_endtime = timestamp + new_process.total_bursttime;
        new_process.remaining_bursttime = new_process.total_bursttime;
        return new_process;
    }
    else {
        if (new_process.process_priority >= current_process.process_priority) { // new process has lower priority, just queue
            ready_queue[(*queue_cnt)++] = new_process;
            new_process.execution_starttime = 0;
            new_process.execution_endtime = 0;
            new_process.remaining_bursttime = new_process.total_bursttime;
            return current_process;
        }
        else { // if new proc has higher priority, replace cur proc
            current_process.execution_endtime = 0;
            current_process.remaining_bursttime = current_process.total_bursttime - (timestamp - current_process.execution_starttime);
            ready_queue[(*queue_cnt)++] = current_process;
            new_process.execution_starttime = timestamp;
            new_process.execution_endtime = timestamp + new_process.total_bursttime;
            new_process.remaining_bursttime = new_process.total_bursttime;
            return new_process;
        }
    }
}


struct PCB handle_process_completion_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
    
    if (*queue_cnt == 0) {
        return NULLPCB;
    }

    int highest_priority_index = 0;
    for (int i = 1; i < *queue_cnt; i++) {
        if (ready_queue[i].process_priority < ready_queue[highest_priority_index].process_priority) {
            highest_priority_index = i;
        }
    }

    struct PCB next_process = ready_queue[highest_priority_index];
    for (int i = highest_priority_index; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    *queue_cnt -= 1;

    next_process.execution_starttime = timestamp;
    next_process.execution_endtime = timestamp + next_process.remaining_bursttime;
    return next_process;
}


struct PCB handle_process_arrival_srtp(        
    struct PCB ready_queue[QUEUEMAX], 
    int *queue_cnt, 
    struct PCB current_process, 
    struct PCB new_process,
    int time_stamp
) {
    if (current_process.total_bursttime == 0) { // if no current proc, run new
        new_process.execution_starttime = time_stamp;
        new_process.execution_endtime = time_stamp + new_process.total_bursttime;
        new_process.remaining_bursttime = new_process.total_bursttime;
        current_process = new_process;
    } else if (new_process.total_bursttime < current_process.remaining_bursttime) { // if new proc time shorter, run new
        current_process.execution_starttime = 0;
        current_process.execution_endtime = 0;
        current_process.remaining_bursttime -= (time_stamp - current_process.execution_starttime)  - 1;
        ready_queue[(*queue_cnt)++] = current_process;
        new_process.execution_starttime = time_stamp;
        new_process.execution_endtime = time_stamp + new_process.total_bursttime;
        new_process.remaining_bursttime = new_process.total_bursttime;
        current_process = new_process;
    } else { // if new proc is longer, save to queue, keep current
        new_process.execution_starttime = 0;
        new_process.execution_endtime = 0;
        new_process.remaining_bursttime = new_process.total_bursttime;
        ready_queue[(*queue_cnt)++] = new_process;
    }
    return current_process;
}

struct PCB handle_process_completion_srtp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
    int i, min_index = -1;
    struct PCB next_process = NULLPCB; 

    if (*queue_cnt == 0) {
        return NULLPCB;
    }
    
    for (i = 0; i < *queue_cnt; i++) {
        if (min_index == -1 || ready_queue[i].remaining_bursttime < ready_queue[min_index].remaining_bursttime) {
            min_index = i;
        }
    }
    
    next_process = ready_queue[min_index];
    for (i = min_index; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_cnt)--;
    
    next_process.execution_starttime = timestamp;
    next_process.execution_endtime = timestamp + next_process.remaining_bursttime;
    
    return next_process;
}


struct PCB handle_process_arrival_rr(
    struct PCB ready_queue[QUEUEMAX], 
    int *queue_cnt, 
    struct PCB current_process, 
    struct PCB new_process, 
    int timestamp, 
    int time_quantum
    ) {
    if (current_process.process_id == 0) {
        new_process.execution_starttime = timestamp;
        new_process.execution_endtime = timestamp + (time_quantum > new_process.total_bursttime ? new_process.total_bursttime : time_quantum);
        // new_process.remaining_bursttime = new_process.total_bursttime - (new_process.execution_endtime - new_process.execution_starttime);
        return new_process;
    } else {
        new_process.execution_starttime = 0;
        new_process.execution_endtime = 0;
        new_process.remaining_bursttime = new_process.total_bursttime;
        ready_queue[(*queue_cnt)++] = new_process;
        return current_process;
    }
}

struct PCB handle_process_completion_rr(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp, int time_quantum)
{
    if (*queue_cnt == 0) {
        return NULLPCB;
    }

    int earliest_arrival_index = 0;
    for (int i = 1; i < *queue_cnt; i++) {
        if (ready_queue[i].arrival_timestamp < ready_queue[earliest_arrival_index].arrival_timestamp) {
            earliest_arrival_index = i;
        }
    }

    struct PCB next_process = ready_queue[earliest_arrival_index];
    *queue_cnt -= 1;

    for (int i = earliest_arrival_index; i < *queue_cnt; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }

    next_process.execution_starttime = timestamp;
    next_process.execution_endtime = timestamp + MIN(time_quantum, next_process.remaining_bursttime);
    // next_process.remaining_bursttime -= MIN(time_quantum, next_process.remaining_bursttime);

    return next_process;
}


void printPCB(struct PCB proc) {
    printf("\n");
    printf("Proc id: %d\n", proc.process_id);
    printf("Arrival time: %d\n", proc.arrival_timestamp);
    printf("Total burst time: %d\n", proc.total_bursttime);
    printf("Exec start time: %d\n", proc.execution_starttime);
    printf("Exec end time: %d\n", proc.execution_endtime);
    printf("Remaining burst time: %d\n", proc.remaining_bursttime);
    printf("Priority: %d\n", proc.process_priority);
    printf("\n");
}

int countQueue(struct PCB queue[QUEUEMAX]) {
    int count = 0;

    for (int i = 0; i < QUEUEMAX; i++)
    {
        if (queue[i].process_id != 0) {
            count++;
        }
        else return count;
    }
    
    return count;
}

// int main(void) {
//     struct PCB ready_queue[QUEUEMAX];
//     int queue_cnt = 0;
//     struct PCB current_process = {1, 1, 4, 1, 5, 4, 8};
//     struct PCB new_process = {2, 2, 3, 0, 0, 3, 6};
//     int timestamp = 2;

//     for (int i = 0; i < QUEUEMAX; i++) {
//         ready_queue[i].process_id = 0;
//         ready_queue[i].arrival_timestamp = 0;
//         ready_queue[i].execution_starttime = 0;
//         ready_queue[i].execution_endtime = 0;
//         ready_queue[i].total_bursttime = 0;
//         ready_queue[i].remaining_bursttime = 0;
//         ready_queue[i].process_priority = 0;
//     }

//     struct PCB next_process = handle_process_arrival_pp(
//         ready_queue, 
//         &queue_cnt, 
//         current_process, 
//         new_process, 
//         timestamp
//     );

//     // printf("First elem in queue:\n");
//     // printPCB(ready_queue[0]);

//     // int qcount = countQueue(ready_queue);
//     // printf("There are %d procs waiting.\n\n", qcount);

//     // printf("Next proc:\n");
//     // printPCB(next_process);



//     // printf("In queue:\n");
//     // for(int i = 0; i < queue_cnt; i++){
//     //     printPCB(ready_queue[i]);
//     // }

//     struct PCB next_process_complete = handle_process_completion_pp(
//         ready_queue,
//         &queue_cnt,
//         timestamp
//     );

//     // printf("Next proc:\n");
//     // printPCB(next_process_complete);


//     // printf("In queue:\n");
//     // for(int i = 0; i < queue_cnt; i++){
//     //     printPCB(ready_queue[i]);
//     // }




//     return 0;
// }










