#include "banking.h"
#include <stdlib.h>
#include <string.h>
#include "utility.h"
#include "logger.h"
#include <sys/wait.h>

void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount) {
    struct my_process *proc = parent_data;

    TransferOrder order = {
            .s_src = src,
            .s_dst = dst,
            .s_amount = amount
    };

    Message msg_send = create_message(TRANSFER, &order, sizeof(order));

    send(proc, dst, &msg_send);

    log_transfer_out(&order);

    Message msg_rec;
    msg_rec.s_header.s_type = STOP; //stub

    while(msg_rec.s_header.s_type != ACK) {
        receive(proc, dst, &msg_rec);
    }

    log_transfer_out(&order);
}

void parent_function(struct my_process *proc, int proc_num, int pipe_pool[proc_num+1][proc_num+1][2]) {
    char *buf = malloc(sizeof(char) * MAX_MESSAGE_LEN);
    Message m = create_message(STOP, buf, (int) strlen(buf));
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

    bank_robbery(proc, (local_id) proc_num);

    //send stop
    char *buffer = malloc(sizeof(char) * MAX_MESSAGE_LEN);
    snprintf(buffer, MAX_MESSAGE_LEN, log_started_fmt, get_physical_time(), proc->this_id, proc->this_pid, proc->parent_pid, proc->balance_state.s_balance);
    Message m_stop = create_message(STOP, buffer, (int) strlen(buffer));

    send_multicast(proc, &m_stop);

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

    destroy_all_pipes(proc_num+1, pipe_pool);
    close_logfiles();
}

void child_function(struct my_process *proc, int proc_num) {
    char *buffer = malloc(sizeof(char) * MAX_MESSAGE_LEN);
    snprintf(buffer, MAX_MESSAGE_LEN, log_started_fmt, get_physical_time(), proc->this_id, proc->this_pid, proc->parent_pid, proc->balance_state.s_balance);
    Message m = create_message(STARTED, buffer, (int) strlen(buffer));

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

    //work from there



    //work to here

    snprintf(buffer, MAX_MESSAGE_LEN, log_done_fmt, get_physical_time(), proc->this_id, proc->balance_state.s_balance);
    m = create_message(DONE, buffer, (int) strlen(buffer));
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

int main(int argc, char * argv[]) {
    open_logfiles();

    if (argc <= 3 || strcmp(argv[1], "-p") != 0 || atoi(argv[2]) < 2) {
        return 1;
    }
    const int proc_num = atoi(argv[2]);
    balance_t balance[proc_num];
    for (int i = 0 ; i < proc_num ; i++) {
        balance[i] = atoi(argv[i + 3]);
    }

    struct my_process *proc = malloc(sizeof(struct my_process));
    proc->this_pid = getpid();
    proc->this_id = PARENT_ID;
    proc->proc_num = proc_num;
    proc->write_fd = malloc(sizeof(int) * (proc_num + 1));
    proc->read_fd = malloc(sizeof(int) * (proc_num + 1));

    int pipe_pool[proc_num + 1][proc_num + 1][2];
    create_pipe_pool(proc_num+1, pipe_pool);

    proc = split(proc, balance);
    destroy_unused_pipes(proc, proc_num+1, pipe_pool);
    get_fds(proc_num + 1, pipe_pool, proc);

    // here starts the thing

    if (proc->this_id == 0) {
        parent_function(proc, proc_num, pipe_pool);
    } else {
        child_function(proc, proc_num);
    }

    return 0;
}
