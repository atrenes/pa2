/**
 * @file ipc.c
 * @Author Danil Khanalainen and Anna Ershova
 * @date October, 2023
 */

#include "ipc.h"
#include "utility.h"

int send(void * self, local_id dst, const Message * msg) {
    struct my_process *proc = self;
    write(proc->write_fd[dst], msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
    return 0;
}

int send_multicast(void * self, const Message * msg) {
    struct my_process *proc = self;
    for (int i = 0 ; i < proc->proc_num+1 ; i++) {
        if (i != proc->this_id) {
            write(proc->write_fd[i], msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
        }
    }
    return 0;
}

int receive(void * self, local_id from, Message * msg) {
    struct my_process *proc = self;
    ssize_t header_read = read(proc->read_fd[from], &(msg->s_header), sizeof(MessageHeader));
    if (msg->s_header.s_payload_len != 0 && header_read == sizeof(MessageHeader)) {
        read(proc->read_fd[from], msg->s_payload, msg->s_header.s_payload_len);
    }
    return 0;
}

int receive_any(void * self, Message * msg) {
    struct my_process *proc = self;
    for (int i = 0 ; i < proc->proc_num ; i++) {
        if (i != proc->this_id) {
            ssize_t bytes_read = read(proc->read_fd[i], &(msg->s_header), sizeof(MessageHeader));
            if (bytes_read == sizeof(MessageHeader) && msg->s_header.s_payload_len != 0) {
                read(proc->read_fd[i], msg->s_payload, msg->s_header.s_payload_len);
                return 0;
            }
        }
    }
    return 1;
}
