/**
 * @file ipc.c
 * @Author Danil Khanalainen and Anna Ershova
 * @date October, 2023
 */

#include "ipc.h"
#include "utility.h"

int send(void * self, local_id dst, const Message * msg) {
    struct my_process *proc = self;
    if (proc->this_id == 0) {
        return 1;
    }
    write(proc->write_fd[dst], msg, sizeof(Message));
    return 0;
}

int send_multicast(void * self, const Message * msg) {
    struct my_process *proc = self;
    if (proc->this_id == 0) {
        return 1;
    }
    for (int i = 0 ; i < proc->proc_num+1 ; i++) {
        if (i != proc->this_id) {
            write(proc->write_fd[i], msg, sizeof(Message));
        }
    }
    return 0;
}

int receive(void * self, local_id from, Message * msg) {
    struct my_process *proc = self;
    read(proc->read_fd[from], msg, sizeof(Message));
    return 0;
}

int receive_any(void * self, Message * msg) {
    struct my_process *proc = self;
    for (int i = 0 ; i < proc->proc_num ; i++) {
        read(proc->read_fd[i], msg, sizeof(Message));
        if (msg != NULL) {
            return 0;
        }
    }
    return 1;
}
