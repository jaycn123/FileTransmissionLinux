#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <functional>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <mutex>
#include <map>
#include <openssl/md5.h>
#include <fstream>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 8087
#define BUFFER_SIZE 1576 * 10
#define CACHE_SIZE 1576 * 100
#define NowTime  time(NULL)
#define MILLION 1000000
#define SBUFFER_SIZE 1024

#endif