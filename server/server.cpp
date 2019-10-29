// server.cpp: 定义控制台应用程序的入口点。
//

#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>
#include<thread>
#include <vector>
#include "netpack.h"
#include <string>
#include "ThreadPool.h"
#include <functional>
#include "MyMd5.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#include <dirent.h>
#include <sys/stat.h>


#define PORT 8087
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

typedef std::string xstring;

typedef std::function<void()>ThreadTask;
fivestar::ThreadPool m_ThreadPool;

int StartUp()
{
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1) {
		std::cout << "Error: socket" << std::endl;
		return 0;
	}

	if (bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) 
	{
		std::cout << "Error: bind" << std::endl;
		return 0;
	}

	if (listen(listenfd, 5) == -1) 
	{
		std::cout << "Error: listen" << std::endl;
		return 0;
	}

	return listenfd;
}

std::vector<int>ClientScoketVec;

void SplitString(const xstring& s, std::vector<xstring>& v, const xstring& c);
uint64_t getFileSize(char* path);
bool get_all_files(const std::string& dir_in, std::vector<std::string>& files);

void DoSendData(int m_socket, std::vector<std::string>&filepath)
{
	//printf("now pid is %d", GetCurrentProcessId());
	//printf("now tid is %d \n", GetCurrentThreadId());
	for (auto it : filepath)
	{
		uint64_t filesize = getFileSize((char*)it.c_str());
		std::cout << "file size :" << filesize<<" K " << std::endl;
		uint64_t filesize2 = filesize;

		char buffer[BUFFER_SIZE];
		std::cout << it << std::endl;
		FILE *fp = fopen(it.c_str(), "rb");  //以只读，二进制的方式打开一个文件
		if (NULL == fp)
		{
			std::cout << "open error " << std::endl;
			return;
		}
		memset(buffer, 0, BUFFER_SIZE);
		int length = 0;
		int index = 0;
		
		std::vector<xstring> v;
		SplitString(it, v, "/");
		if (v.size() != 2)
		{
			std::cout << "Error PathName : " << it << std::endl;
			return;
		}

		std::cout << "start send : " << it << std::endl;

		while ((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
		{
			filesize -= length;

			NetPacket sendPack;
			sendPack.Header.wOpcode = SENDDATA;
			sendPack.Header.wDataSize = length + sizeof(NetPacketHeader);
			sendPack.Header.wOrderindex = index;
			sendPack.Header.filesize = filesize2;
			memcpy(sendPack.Header.filename, v[1].c_str(), MAXNAME);

			memcpy(sendPack.Data, buffer, length);
			if (filesize == 0)
			{
				std::cout << "send last char  " << std::endl;

				sendPack.Header.tail = 1;
			}

			int sendlen = send(m_socket, (const char*)&sendPack, sizeof(sendPack), 0);

			if (sendlen == -1)
			{
				fclose(fp);
				close(m_socket);
				std::cout << "socket close " << std::endl;
				return;
			}

			while (sendlen < length)
			{
				sendlen = send(m_socket, ((const char*)&sendPack) + sendlen, sizeof(sendPack) - sendlen, 0);
				length -= sendlen;
				std::cout << "buff -- " << std::endl;
			}

			memset(buffer, 0, BUFFER_SIZE);
			index++;
		}

		std::cout <<"MD5 : "<< get_file_md5(it) << std::endl;
		fclose(fp);
		std::cout << it << " Transfer Successful !" << std::endl;
	}

	close(m_socket);
}

std::vector<std::thread *>m_thradVec;

void Doaccept(int m_socket,std::vector<std::string>&filepath)
{
	while (true)
	{
		std::cout << "Listening To Client ..." << std::endl;
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int m_new_socket = accept(m_socket, (struct sockaddr *)&client_addr, &client_addr_len);
		if (m_new_socket < 0)
		{
			std::cout << "SOCKET_ERROR ..." << std::endl;
			continue;
		}
		ClientScoketVec.push_back(m_new_socket);
		std::cout << "New Client Connection" << std::endl;
		std::thread T(std::bind(DoSendData, m_new_socket, filepath));
		T.detach();
		//std::thread *Tsend = new std::thread(DoSendData, m_new_socket);
		//m_thradVec.push_back(Tsend);
	}
}

std::string GetCurrentExeDir()
{
	char szPath[1024] = { 0 }, szLink[1024] = { 0 };
#ifdef WIN32
	ZeroMemory(szPath, 1024);
	GetModuleFileNameA(NULL, szPath, 1024);
	char* p = strrchr(szPath, '\\');
	*p = 0;
#else
	snprintf(szLink, 1024, "/proc/%d/exe", getpid());/////////////
	readlink(szLink, szPath, sizeof(szPath));//////////////
#endif
	return std::string(szPath);
}


void SplitString(const xstring& s, std::vector<xstring>& v, const xstring& c)
{
	xstring::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (xstring::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

int main()
{
	signal(SIGPIPE, SIG_IGN);
	std::vector<std::string>m_vtFileList;
	//m_vtFileList.push_back("src/runtime.rar");
	get_all_files("src",m_vtFileList);
	
	for(auto it : m_vtFileList)
	{
		std::cout<<it<<std::endl;
	}
	
	int m_socket = StartUp();
	if (m_socket < 0)
	{
		std::cout << "SOCKET_ERROR " << std::endl;
		return -1;
	}


	std::thread taccept(std::bind(Doaccept, m_socket, m_vtFileList));
	taccept.join();
	std::cout << "ThreadPool over" << std::endl;
	close(m_socket);

	getchar();
	return 0;
}

uint64_t getFileSize(char* path)
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


bool get_all_files(const std::string& dir_in, std::vector<std::string>& files) 
{
    if (dir_in.empty()) {
        return false;
    }
    struct stat s;
    stat(dir_in.c_str(), &s);
    if (!S_ISDIR(s.st_mode)) {
		std::string tpatch = "mkdir " + dir_in;
		system(tpatch.c_str());
        return false;
    }
    DIR* open_dir = opendir(dir_in.c_str());
    if (NULL == open_dir) {
        std::exit(EXIT_FAILURE);
    }
    dirent* p = nullptr;
    while( (p = readdir(open_dir)) != nullptr) {
        struct stat st;
        if (p->d_name[0] != '.') {
            //因为是使用devC++ 获取windows下的文件，所以使用了 "\" ,linux下要换成"/"
           // std::string name = dir_in + std::string("\\") + std::string(p->d_name);
			std::string name = dir_in + std::string("/") + std::string(p->d_name);
            stat(name.c_str(), &st);
            if (S_ISDIR(st.st_mode)) {
                get_all_files(name, files);
            }
            else if (S_ISREG(st.st_mode)) {
                files.push_back(name);
            }
        }
    }
    closedir(open_dir);
    return true;
}

