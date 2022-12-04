#include <signal.h>
#include <mqueue.h>
#include <cstring>
#include <cstdio>

#include <iostream>
#include <vector>
#include <queue>


#include "messages.h"

using namespace std;

const int POOL_SIZE = 15;

mqd_t server_qid = -1; 


vector<pthread_t> threads;

queue<RequestDto> task_queue;
pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t new_task = PTHREAD_COND_INITIALIZER;


// --------------------------------------------------------------------

volatile sig_atomic_t carry_on = 1;
void stop_signal_handler(sig_atomic_t s);

void startup();
void mainloop();
void shutdown();

void* client_thread(void* arg);
void handle_client(RequestDto request, int tid);
void test() {
 
}

int main() {
    signal(SIGINT, stop_signal_handler); // Register signal handler for STOP signal (Ctrl+C)
 
    startup();

    mainloop();

    shutdown();
}

void startup() {
    printf("Starting up...\n");

    // CREATE SERVER_QUEUE
    struct mq_attr attr;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof(RequestDto);

    server_qid = mq_open(SERVER_MSQ_NAME, 
                            O_RDONLY | O_CREAT, MSQ_MODE, &attr); //Üzenetsor létrehozása

    if (server_qid == -1) {
        perror("mq_open");
        exit(1);
    }

    // CREATING THREADS (threadpool)
    threads.resize(POOL_SIZE);
    int thread_index = 1;
    for (pthread_t& thread : threads) {
        int* cur_index = new int(thread_index);
        pthread_create(&thread, NULL, client_thread, (void*)cur_index);
        thread_index++;
    }
}

void mainloop() {
    while(carry_on) {
        RequestDto request;

        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 300000;

        if ((mq_timedreceive(server_qid, (char*)&request, sizeof(RequestDto), NULL, &ts)) > 0) {
            printf("Received message: %d\n", request.client_pid);

            pthread_mutex_lock(&task_mutex);
            task_queue.push(request);
            pthread_cond_broadcast(&new_task);
            pthread_mutex_unlock(&task_mutex);
        }
    }
}

void shutdown() {


    printf("\n\n-------------------------------\n"
            "Waiting threads to finish.\n");

    for (pthread_t& thread : threads) {
        pthread_join(thread, NULL);
    }

    printf("\nStopping server..\n");

    // CLOSE AND DELETE SERVER_QUEUE
    mq_close(server_qid);
    mq_unlink(SERVER_MSQ_NAME);
}

void stop_signal_handler(int s) {
    carry_on = 0;
}


// =============================================================

void* client_thread(void* arg) {
    int* tid_ptr = (int*)arg; 
    int tid = *tid_ptr;
    delete tid_ptr;

    // printf("Thread %d started.\n", tid);

    while(carry_on) {
        pthread_mutex_lock(&task_mutex);
        while(carry_on && task_queue.empty()) {
            timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 300000;
            pthread_cond_timedwait(&new_task, &task_mutex, &ts);
        }
        if (!task_queue.empty()) {
            RequestDto request = task_queue.front();
            task_queue.pop();
            pthread_mutex_unlock(&task_mutex);

            handle_client(request, tid);
        } else {
            pthread_mutex_unlock(&task_mutex);
        }
    }

    return NULL;
}

void handle_client(RequestDto request, int tid) {
    printf("<t %d> Handling client: %d [%s]\n", tid, request.client_pid, request.file_name);

    // OPEN CLIENT_QUEUE
    mqd_t client_qid = -1;
    string client_msq_name_str = "/" + to_string(request.client_pid) + CLIENT_MSQ_SUFFIX;
    client_qid = mq_open(client_msq_name_str.c_str(), O_WRONLY);

    ResponseDto response;
    response.last = false;
    // =================================
    string command_str = "find ~ -type f -name '" + string(request.file_name) + "' -printf '%h\n'";
    FILE* fp = popen(command_str.c_str(), "r");

    char* line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) > 0) {
        // SEND RESPONSE
        strncpy(response.row, line, MAX_ROW_LENGTH + 1);
        mq_send(client_qid, (char*)&response, sizeof(ResponseDto), 1);
    }

    pclose(fp);
    // =================================
    response.last = true;
    strcpy(response.row, "Goodbye!"); // just for fun
    mq_send(client_qid, (char*)&response, sizeof(ResponseDto), 1);
}
