#include <fstream>

#include <unistd.h>
#include <mqueue.h>
#include <cstring>

#include "messages.h"

using namespace std;

mqd_t server_qid = -1; 
mqd_t client_qid = -1; 
// --------------------------------------------------------------------

void create_client_msq();
void join_server_msq(const char* file_name);
void receive_and_process_responses();
void close_msqs();

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    const char* file_name = argv[1];

    create_client_msq();

    // send message to server
    join_server_msq(file_name);

    // receive and process responses from server
    receive_and_process_responses();

    close_msqs();
}

void create_client_msq() {
    // CREATE CLIENT_QUEUE
    struct mq_attr attr;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof(ResponseDto);

    string client_msq_name_str = "/" + to_string(getpid()) + CLIENT_MSQ_SUFFIX;

    client_qid = mq_open(client_msq_name_str.c_str(), 
                            O_RDONLY | O_CREAT, MSQ_MODE, &attr);
}

void join_server_msq(const char* file_name) {
    // open server_msq
    server_qid = mq_open(SERVER_MSQ_NAME, O_WRONLY);
    if (server_qid == -1) {
        fprintf(stderr, "!\nThe server is not running!\n\n");
        exit(1);
    }

    // build up the request
    RequestDto request;
    request.client_pid = getpid();
    strncpy(request.file_name, file_name, MAX_FILE_NAME+1);

    mq_send(server_qid, (char*)&request, sizeof(RequestDto), 1);
}

void receive_and_process_responses() {

    ofstream fout("results/" + to_string(getpid()) + "_out.dat");

    if (!fout) {
        fprintf(stderr, "\nCannot open the output file!\n"
                    "Maybe the ./results/ folder does not exist.\n\n");
        exit(2);
    }

    ResponseDto response;
    while (true) {
        mq_receive(client_qid, (char*)&response, sizeof(ResponseDto), NULL);
        if (response.last) {
            break;
        } else {
            fout << response.row;
        }
    }
}

void close_msqs() {
    mq_close(server_qid);
    mq_close(client_qid);
    mq_unlink(("/" + to_string(getpid()) + CLIENT_MSQ_SUFFIX).c_str());
}