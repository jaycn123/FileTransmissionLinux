// client.cpp: 定义控制台应用程序的入口点。
//

//#define  _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS

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
#include "../common/netpack.h"
#include "../common/MyMd5.h"


#define PORT 8087
std::string SERVER_IP = "127.0.0.1";
//#define SERVER_IP "192.168.0.96"
#define BUFFER_SIZE 1576 * 10

#define CACHE_SIZE 1576 * 100
#define NowTime  time(NULL)


long long getFileSize(char* path);

int StartUp()
{
    //创建socket
    int c_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (c_socket < 0)
    {
        return -1;
    }

    //指定服务端的地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP.c_str());
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
    server_addr.sin_port = htons(PORT);
    if (-1 == connect(c_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        return -1;
    }
    return c_socket;
}

char                m_cbDataBuf[10240];


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


std::mutex mtx;
volatile long long curlength = 0;

void printProgress(long long alllength,std::string filename)
{
    mtx.lock();
    std::cout << "\n\n开始接收文件 : " << filename.c_str() << " 文件大小 : " << alllength / 1024 / 1024 << " M" << std::endl;
    std::cout << "正在获取数据...... ";
    while (curlength < alllength)
    {
        std::cout.width(3);//i的输出为3位宽
        std::cout << (int)((float)curlength / (float)alllength *100) << "%";
        usleep(2000);
        std::cout << "\b\b\b\b";//回删三个字符，使数字在原地变化
    }

    std::cout.width(3);
    std::cout << (int)((float)curlength / (float)alllength * 100) << "%";
    std::cout << std::endl;
    mtx.unlock();
    curlength = 0;
}


int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        SERVER_IP = argv[1];
    }

    //system("mkdir des");

    int c_socket = StartUp();
    while (c_socket == -1)
    {
        c_socket = StartUp();
        std::cout << "Waiting for a server connection ......" << std::endl;
        sleep(1);
    }

    std::cout << "server connection !!! " << std::endl;

    std::vector<WriteFileData>fileNameVec;

    long long length = 0;
    long long m_nRecvSize = 0;
    long long offindex = 0;

    char buffer[BUFFER_SIZE];
    char m_cbRecvBuf[CACHE_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    uint64_t  begintime =0 , endtime = 0;
    uint64_t  alldatalen = 0;
    while ((length = recv(c_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        alldatalen+=length;
        if ((m_nRecvSize + length) > CACHE_SIZE)
        {
            if ((m_nRecvSize - offindex) > 0)
            {
                /*
                   char temp[CACHE_SIZE] = { 0 };
                   memcpy(temp, m_cbRecvBuf + offindex, m_nRecvSize - offindex);
                   */
                memmove(m_cbRecvBuf, m_cbRecvBuf + offindex, m_nRecvSize - offindex);
                /*
                   memcpy(m_cbRecvBuf, temp, m_nRecvSize - offindex);
                   */


                m_nRecvSize = m_nRecvSize - offindex;
                offindex = 0;
            }
            else
            {
                memset(m_cbRecvBuf, 0, CACHE_SIZE);
                offindex = 0;
                m_nRecvSize = 0;
            }
        }

        memcpy(m_cbRecvBuf + m_nRecvSize, buffer, length);
        m_nRecvSize += length;

        while ((m_nRecvSize - offindex) >= sizeof(NetPacketHeader))
        {
            NetPacketHeader* pHeader = (NetPacketHeader*)(m_cbRecvBuf + offindex);
            if (pHeader == nullptr)
            {
                continue;
            }

            if (pHeader->wCode != NET_CODE)
            {
                break;
            }
            if (pHeader->wOpcode == SENDDATA)
            {
                if (pHeader->wDataSize > (m_nRecvSize - offindex))
                {
                    break;
                }

                std::string filename = "des/";
                filename.append(pHeader->filename);
                bool isNew = true;
                WriteFileData *pfileData = NULL;

                for (auto it = fileNameVec.begin(); it != fileNameVec.end(); it++)
                {
                    if ((*it).filename == filename)
                    {
                        isNew = false;
                        pfileData = &(*it);
                        break;
                    }
                }

                if (isNew)
                {
                    begintime = NowTime;

                    std::thread Tprint(std::bind(printProgress, pHeader->filesize, filename));
                    Tprint.detach();

                    WriteFileData filedata;
                    filedata.fp = fopen(filename.c_str(), "wb");
                    if (filedata.fp == nullptr)
                    {
                        std::cout << "open file Error" << std::endl;
                        return -1;
                    }

                    filedata.filename = filename;
                    fileNameVec.push_back(filedata);

                    for (auto it = fileNameVec.begin(); it != fileNameVec.end(); it++)
                    {
                        if ((*it).filename == filename)
                        {
                            pfileData = &(*it);
                            break;
                        }
                    }
                }

                NetPacket* msg = (NetPacket*)(m_cbRecvBuf + offindex);

                if (pHeader->wOrderindex != pfileData->index + 1)
                {
                    FileData data;
                    data.size = pHeader->wDataSize - sizeof(NetPacketHeader);
                    data.data = msg->Data;
                    data.isLast = pHeader->tail;

                    curlength += data.size;

                    pfileData->m_cache[pHeader->wOrderindex] = data;

                    offindex += sizeof(NetPacket);
                }
                else
                {
                    pfileData->index = pHeader->wOrderindex;

                    int templength = pHeader->wDataSize - sizeof(NetPacketHeader);

                    curlength += templength;

                    if (fwrite(msg->Data, sizeof(char), templength, pfileData->fp) < templength)
                    {
                        std::cout << "write error " << std::endl;
                        return -1;
                    }

                    offindex += sizeof(NetPacket);
                    memset(buffer, 0, BUFFER_SIZE);

                    int tempindex = pHeader->wOrderindex + 1;
                    while (pfileData->m_cache.find(tempindex) != pfileData->m_cache.end())
                    {
                        pfileData->index = tempindex;
                        if (fwrite(pfileData->m_cache[tempindex].data.c_str(), sizeof(char), pfileData->m_cache[tempindex].size, pfileData->fp) < pfileData->m_cache[tempindex].size)
                        {
                            std::cout << "write error " << std::endl;
                            return -1;
                        }

                        if (pfileData->m_cache[tempindex].isLast)
                        {
                            long long mb = getFileSize((char*)pfileData->filename.c_str());// / 1048576;

                            endtime = NowTime;

                            mtx.lock();
                            uint64_t alltime = endtime - begintime;
                            std::cout << "begintime : " << begintime << " endtime  " << endtime  << std::endl;
                            if((mb/1048576) < alltime)
                            {
                                std::cout << "usetime : " << alltime << " 速度 : " << mb/1024 / alltime << " k/s" << std::endl;
                            }
                            else
                            {
                                std::cout << "usetime : " << alltime << " 速度 : " << mb/1048576 / alltime << " M/s" << std::endl;
                            }
                            std::cout << "Receive File : " << pfileData->filename.c_str() << " From Server Successful !" << std::endl;
                            std::cout<<"alldatalen : "<<alldatalen<<std::endl;
                            fclose(pfileData->fp);
                            std::cout << get_file_md5(pfileData->filename) << std::endl;
                            mtx.unlock();
                            break;
                        }
                        tempindex++;
                    }

                    if (pHeader->tail)
                    {
                        long long mb = getFileSize((char*)pfileData->filename.c_str());// / 1048576;

                        endtime = NowTime;

                        mtx.lock();

                        double alltime = endtime - begintime;
                        std::cout << "begintime : " << begintime << " endtime  " << endtime  << std::endl;
                        if((mb/1048576) < alltime)
                        {
                            std::cout << "usetime : " << alltime << " 速度 : " << mb/1024 / alltime << " k/s" << std::endl;
                        }
                        else
                        {
                            std::cout << "usetime : " << alltime << " 速度 : " << mb/1048576 / alltime << " M/s" << std::endl;
                        }
                        std::cout << "Receive File : " << pfileData->filename.c_str() << " From Server Successful !" << std::endl;
                        std::cout<<"alldatalen : "<<alldatalen<<std::endl;
                        fclose(pfileData->fp);
                        std::cout << get_file_md5(pfileData->filename) << std::endl;
                        mtx.unlock();
                        break;
                    }
                }
            }
        }
    }

    close(c_socket);
    //释放winsock 库	
}

long long getFileSize(char* path)
{
    FILE * pFile;
    long long size;

    pFile = fopen(path, "rb");
    if (pFile == NULL)
        perror("Error opening file");
    else
    {
        fseek(pFile, 0, SEEK_END);   ///将文件指针移动文件结尾
        size = ftello64(pFile); ///求出当前文件指针距离文件开始的字节数
        fclose(pFile);
        //printf("Size of file.cpp: %lld bytes.\n", size);
        return size;
    }

    return 0;
}
