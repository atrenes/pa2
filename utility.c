/**
 * @file utility.c
 * @Author Danil Khanalainen and Anna Ershova
 * @date October, 2023
 */

#include "utility.h"
#include "logger.h"

int create_pipe_pool(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]) {
    for (int i = 0 ; i < all_proc_num ; i++) {
        for (int j = 0 ; j < all_proc_num ; j++) {
            if (i != j) {
                int p[2];
                if (pipe(p) < 0) return 1;
                fcntl(p[0], F_SETFL, O_NONBLOCK); // non-blocking read
                fcntl(p[1], F_SETFL, O_NONBLOCK); // non-blocking write
                pipe_pool[i][j][0] = p[0];
                pipe_pool[j][i][1] = p[1];
                log_created_pipe(p[0], p[1]);
            }
        }
    }
    return 0;
}

void get_fds(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2], struct my_process *proc) {
    for (int j = 0 ; j < all_proc_num ; j++) {
        if (proc->this_id != j) {
            proc->read_fd[j] = pipe_pool[proc->this_id][j][0];
            proc->write_fd[j] = pipe_pool[proc->this_id][j][1];
        }
    }
}

struct my_process *split(struct my_process *proc) {
    int cur_pid;
    for (local_id i = 1 ; i < (local_id) proc->proc_num + 1 ; i++) {
        cur_pid = fork();
        if (cur_pid != 0) {
            continue;
        } else {
            struct my_process *child = malloc(sizeof(struct my_process));

            child->this_pid = getpid();
            child->parent_pid = proc->this_pid;
            child->this_id = i;
            child->proc_num = proc->proc_num;
            child->read_fd = malloc(sizeof(int) * (proc->proc_num + 1));
            child->write_fd = malloc(sizeof(int) * (proc->proc_num + 1));
            return child;
        }
    }
    return proc;
}

Message create_message(MessageType type, char* message) {
    Message m;
    m.s_header.s_magic = MESSAGE_MAGIC;
    m.s_header.s_type = type;
    m.s_header.s_payload_len = strlen(message);
    strcpy(m.s_payload, message);
    return m;
}

void destroy_pipes(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]) {
    for (int i = 0 ; i < all_proc_num ; i++) {
        for (int j = 0 ; j < all_proc_num ; j++) {
            if (i != j) {
                close(pipe_pool[i][j][0]);
                close(pipe_pool[i][j][1]);
                log_closed_fd(pipe_pool[i][j][0]);
                log_closed_fd(pipe_pool[i][j][1]);
            }
        }
    }
}

void destroy_unused_pipes(struct my_process *proc, int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]) {
    for (int i = 0 ; i < all_proc_num ; i++) {
        for (int j = 0 ; j < all_proc_num ; j++) {
            if (i != j && i != proc->this_id) {
                close(pipe_pool[i][j][0]);
                close(pipe_pool[i][j][1]);
                log_closed_fd(pipe_pool[i][j][0]);
                log_closed_fd(pipe_pool[i][j][1]);
            }
        }
    }
}
