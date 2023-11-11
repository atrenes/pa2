/**
 * @file utility.h
 * @Author Danil Khanalainen and Anna Ershova
 * @date October, 2023
 */

#ifndef RASPRED1_UTILITY_H
#define RASPRED1_UTILITY_H
#include "ipc.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "banking.h"

#define PARENT_ID 0

struct my_process {
    int this_pid;
    int parent_pid;
    local_id this_id;
    int proc_num;
    int *read_fd;
    int *write_fd;
    BalanceHistory balance_history;
    BalanceState balance_state;
};

int create_pipe_pool(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]);
void get_fds(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2], struct my_process *proc);
struct my_process *split(struct my_process *proc);
Message create_message(MessageType type, char* message);
void destroy_pipes(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]);
void destroy_unused_pipes(struct my_process *proc, int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]);
void print_history(const AllHistory * history);

#endif //RASPRED1_UTILITY_H
