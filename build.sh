#!/bin/bash
set -x
g++ -c common/recvService.cpp
g++ -c common/sendService.cpp -lpthread
g++ -o client_exe -O3 client/client.cpp ./common/fileTool.h recvService.o -l crypto
g++ -o server_exe -O3 server/server.cpp sendService.o -l crypto -lpthread
