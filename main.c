#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include "common/ipc.h"
#include "utility.h"
#include "logger.h"

int main(int argc, char **argv) {
    open_logfiles();

    if (argc != 3 || strcmp(argv[1], "-p") != 0 || atoi(argv[2]) <= 1) {
        return 1;
    }
    const int proc_num = atoi(argv[2]);

    struct my_process *proc = malloc(sizeof(struct my_process));
    proc->this_pid = getpid();
    proc->this_id = PARENT_ID;
    proc->proc_num = proc_num;
    proc->write_fd = malloc(sizeof(int) * (proc_num + 1));
    proc->read_fd = malloc(sizeof(int) * (proc_num + 1));

    int pipe_pool[proc_num + 1][proc_num + 1][2];
    create_pipe_pool(proc_num+1, pipe_pool);

    proc = split(proc);
    destroy_unused_pipes(proc, proc_num+1, pipe_pool);
    get_fds(proc_num + 1, pipe_pool, proc);


    if (proc->this_id == 0) {

        char *buf = malloc(sizeof(char) * MAX_MESSAGE_LEN);
        Message m = create_message(STOP, buf);
        int received_num = 0;
        while (received_num != proc_num) {
            for (local_id i = 1 ; i < proc_num + 1 ; i++) {
                receive(proc, i, &m);
                if (m.s_header.s_type == STARTED) {
                    received_num++;
                }
            }
        }

        log_received_all_started(proc);

        received_num = 0;
        while (received_num != proc_num) {
            for (local_id i = 1 ; i < proc_num + 1 ; i++) {
                receive(proc, i, &m);
                if (m.s_header.s_type == DONE) {
                    received_num++;
                }
            }
        }

        log_received_all_done(proc);
        for (int i = 0 ; i < proc_num ; i++) {
            wait(NULL);
        }

        destroy_pipes(proc_num+1, pipe_pool);
        close_logfiles();
    } else {
        char *buffer = malloc(sizeof(char) * MAX_MESSAGE_LEN);
        snprintf(buffer, MAX_MESSAGE_LEN, log_started_fmt, proc->this_id, proc->this_pid, proc->parent_pid);
        Message m = create_message(STARTED, buffer);

        send_multicast(proc, &m);
        log_started(proc);

        int received_num = 0;
        while (received_num != proc_num - 1) {
            for (local_id i = 1 ; i < proc_num + 1 ; i++) {
                if (i != proc->this_id) {
                    receive(proc, i, &m);
                    if (m.s_header.s_type == STARTED) {
                        received_num++;
                    }
                }
            }
        }

        log_received_all_started(proc);

        snprintf(buffer, MAX_MESSAGE_LEN, log_done_fmt, proc->this_id);
        m = create_message(DONE, buffer);
        free(buffer);

        send_multicast(proc, &m);
        log_done(proc);

        received_num = 0;
        while (received_num != proc_num - 1) {
            for (local_id i = 1 ; i < proc_num + 1 ; i++) {
                if (i != proc->this_id) {
                    receive(proc, i, &m);
                    if (m.s_header.s_type == DONE) {
                        received_num++;
                    }
                }
            }
        }

        log_received_all_done(proc);
        exit(0);
    }
}
