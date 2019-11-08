#include "recvService.h"

RecvService::RecvService(std::string& ip)
{
	m_serverip = ip;
}

RecvService::~RecvService()
{
	
}

bool RecvService::StartUp()
{
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0)
    {
    	m_socket = -1;
        return false;
    }

    //指定服务端的地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(m_serverip.c_str());
    server_addr.sin_port = htons(PORT);
    if(connect(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
    	m_socket = -1;
        return false;
    }
    return true;
}

int RecvService::Start()
{
	StartUp();
    while (m_socket <= 0)
	{
	    std::cout << "Waiting for a server connection ......" << std::endl; 
	    sleep(1);
	    StartUp();
	}
	
    std::cout << "server connection !!! " << std::endl;
 	memset(buffer, 0, BUFFER_SIZE);
    while ((length = recv(m_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        alldatalen+=length;
        if ((m_nRecvSize + length) > CACHE_SIZE)
        {
            if ((m_nRecvSize - offindex) > 0)
            {
                memmove(m_cbRecvBuf, m_cbRecvBuf + offindex, m_nRecvSize - offindex);
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

					 clock_gettime(CLOCK_MONOTONIC, &tpstart);

                    //std::thread Tprint(std::bind(printProgress, pHeader->filesize, filename));
                    //Tprint.detach();

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
                    std::cout<<"pHeader->wOrderindex != pfileData->index" <<std::endl;

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
                            PrintSpeed(getFileSize((char*)pfileData->filename.c_str()));
                            std::cout << "Receive File : " << pfileData->filename.c_str() << " From Server Successful !" << std::endl;

                            fclose(pfileData->fp);
                            std::cout << get_file_md5(pfileData->filename) << std::endl;
                            break;
                        }
                        tempindex++;
                    }

                    if (pHeader->tail)
                    {
                        PrintSpeed(getFileSize((char*)pfileData->filename.c_str()));
                        std::cout << "Receive File : " << pfileData->filename.c_str() << " From Server Successful !" << std::endl;
	
                        fclose(pfileData->fp);
                        std::cout << get_file_md5(pfileData->filename) << std::endl;
                        break;
                    }
                }
            }
        }
    }

    close(m_socket);
}

 void RecvService::PrintSpeed(uint64_t filesize)
 {
	clock_gettime(CLOCK_MONOTONIC, &tpend);
    double alltime = (double)(MILLION*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000)/1000/ (double)1000;
    if((filesize/1048576) < alltime)
    {
        std::cout << "usetime : " << alltime << " 速度 : " << filesize/1024 / alltime << " k/s" << std::endl;
    }
    else
    {
        std::cout << "usetime : " << alltime << " 速度 : " << filesize/1048576 / alltime << " M/s" << std::endl;
    }
 }