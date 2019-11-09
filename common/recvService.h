#ifndef RECVSERVICE_H
#define RECVSERVICE_H

#include "common.h"
#include "netpack.h"
#include "tool.h"

struct FileData
{
    int size = 0;
    bool isLast = false;
    std::string data = "";
};

struct WriteFileData
{
    std::string filename = "";
    FILE *fp = nullptr;
    long long index = -1;
    long long filesize = 0;
    std::map<long long, FileData> m_cache;
};


class RecvService
{

public:
    RecvService(std::string& ip);
    ~RecvService();
    
    bool StartUp();
    int Start();
    void PrintSpeed(uint64_t filesize);

    struct timespec time1 = {0, 0};
    char m_cbDataBuf[10240];
    std::mutex mtx;
    volatile long long curlength = 0;
    long long length = 0;
    long long m_nRecvSize = 0;
    long long offindex = 0;

    char buffer[BUFFER_SIZE];
    char m_cbRecvBuf[CACHE_SIZE];
   
    std::vector<WriteFileData>fileNameVec;
    uint64_t  begintime =0 , endtime = 0;
    uint64_t  alldatalen = 0;

    std::string m_serverip ="";
    int m_socket;

    struct timespec tpstart;
    struct timespec tpend;
};

#endif