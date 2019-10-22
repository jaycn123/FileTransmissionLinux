#pragma once

#define NET_PACKET_DATA_SIZE 1024
#define NET_CODE 0x2766
#define MAXNAME 512

enum NetMessage
{
	SENDDATA = 1,

};

struct  NetPacketHeader
{
	unsigned short      wDataSize;  ///< 数据包大小，包含封包头和封包数据大小
	NetMessage          wOpcode;    ///< 操作码
	unsigned short      wCode = NET_CODE;
	long long           wOrderindex = 0;
	char                filename[MAXNAME] = { 0 };
	int                 tail = 0;
	long long           filesize = 0;
};
/// 网络数据包
struct NetPacket
{
	NetPacketHeader     Header;                         ///< 包头
	char       Data[NET_PACKET_DATA_SIZE];     ///< 数据
};

