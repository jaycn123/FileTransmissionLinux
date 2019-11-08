#ifndef FILETOOL_H
#define FILETOOL_H

#include "common.h"

static std::string get_file_md5(const std::string &file_name)
{

	std::ifstream file(file_name.c_str(), std::ifstream::binary);
	if (!file)
	{
		return "";
	}

	MD5_CTX md5Context;
	MD5_Init(&md5Context);

	char buf[1024 * 16];
	while (file.good()) {
		file.read(buf, sizeof(buf));
		MD5_Update(&md5Context, buf, file.gcount());
	}

	unsigned char result[MD5_DIGEST_LENGTH];
	MD5_Final(result, &md5Context);

	char hex[35];
	memset(hex, 0, sizeof(hex));
	for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
	{
		sprintf(hex + i * 2, "%02x", result[i]);
	}
	hex[32] = '\0';

	return std::string(hex);
}


static long long getFileSize(char* path)
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

static void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}


static bool get_all_files(const std::string& dir_in, std::vector<std::string>& files) 
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

static std::string GetCurrentExeDir()
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


#endif