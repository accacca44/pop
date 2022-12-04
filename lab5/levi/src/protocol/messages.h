#pragma once

#include <mqueue.h>

const char* SERVER_MSQ_NAME = "/slim2181_server_msq";
const char* CLIENT_MSQ_SUFFIX = "_client_msq";
const mode_t MSQ_MODE  = 0666;                      //jogok beállítása

const int MAX_ROW_LENGTH = 1000;
const int MAX_FILE_NAME = 255;

struct RequestDto {
    pid_t client_pid;
    char file_name[MAX_FILE_NAME + 1];
};

struct ResponseDto {
    bool last;
    char row[MAX_ROW_LENGTH];
};