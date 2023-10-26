#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <future>
#include <fstream>
#include <ctime>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "winmm.lib ")
#pragma warning(disable : 4996)

#define server_ip "127.0.0.1"
#define server_port 69
#define slicelenth 512	//文件分片大小必须是512，不然不会接收
#define tftp_timeout 500

using namespace std;
enum msg_kind
{
	msg_RRQ = 1,
	msg_WRQ = 2,
	msg_DATA = 3,
	msg_ACK = 4,
	msg_ERROR = 5
};

enum t_status
{
	sendwrong = -1,
	recvwrong = -1,
	wrongstep = -1,
	ok = 0
};

class TFTP_msg
{
public:
	char sendbuf[1200];
	char recvbuf[1200];
	SOCKET client;
	struct sockaddr_in* server_addr;
	int server_addr_length;

	TFTP_msg(SOCKET c, struct sockaddr_in* s, msg_kind Opcode)
	{
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
		*(__int8*)(sendbuf + 1) = (__int8)Opcode;
		client = c;
		server_addr = s;
		server_addr_length = sizeof(*server_addr);
	}

	t_status TFTP_readfile(const char* file_name, const char* ts_mode, ofstream& file)
	{
		int pktlenth = 0;
		__int16 pktnum = 0;
		bool done = 0;
		strcpy(sendbuf + 2, file_name);
		strcpy(sendbuf + 2 + strlen(file_name) + 1, ts_mode);
		pktlenth = 2 + strlen(file_name) + strlen(ts_mode) + 2;
		if (sendto(client, sendbuf, pktlenth, 0, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
			return sendwrong;
		
		while (1)
		{
			if ((pktlenth = recvfrom(client, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)server_addr, &server_addr_length)) == -1)
				return recvwrong;
			else
				if (*(__int8*)(recvbuf + 1) != (__int8)msg_DATA )
					return wrongstep;
			*(__int8*)&pktnum = *(__int8*)(recvbuf + 3);
			*((__int8*)&pktnum+1) = *(__int8*)(recvbuf + 2);
			file.write(recvbuf + 4, pktlenth - 4);

			if (pktlenth-4 < 512)
				done = 1;
			cout << "pkt=" << pktnum << "尝试接收:" << pktlenth - 4 << "字节....";


			memset(sendbuf, 0, sizeof(sendbuf));
			*((__int8*)sendbuf + 1) = (__int8)msg_ACK;
			*(__int8*)(sendbuf + 2) = *((__int8*)&pktnum + 1);		//高低端转换
			*(__int8*)(sendbuf + 3) = *(__int8*)&pktnum;
			pktlenth = 4;
			if (sendto(client, sendbuf, pktlenth, 0, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
				return sendwrong;
			cout << "成功！" << endl;

			if (done)
				break;
		}
		return ok;
	}

	t_status TFTP_writefile(const char* file_name, const char* ts_mode, ifstream& file)
	{
		int pktlenth = 0;
		int filelength = 0;
		__int16 pktnum = 1;
		bool done = 0;
		__int64 pkt_time;
		future_status f_sendto_status;
		future_status f_recvfrom_status;
		//计算文件总大小
		file.seekg(0, std::ios::end);
		int filelenth = file.tellg();
		file.seekg(0, std::ios::beg);

		//WRQ报文发，ACK报文收

		//构建WRQ报文
		strcpy(sendbuf + 2, file_name);
		strcpy(sendbuf + 2 + strlen(file_name) + 1, ts_mode);
		pktlenth = 2 + strlen(file_name) + strlen(ts_mode) + 2;

		future<void> f_sendto = async(launch::async, [&pktlenth,this]() {trysendto(&pktlenth);});
		f_sendto.wait();

		pkt_time = timeGetTime();
		future<void> f_recvfrom = async(launch::async, [&pktlenth, this]() {tryrecvfrom(&pktlenth);});
		for(int retrytimes=0;retrytimes<3;)
		{
			if (timeGetTime() - pkt_time >= tftp_timeout)
			{
				future<void> f_sendto = async(launch::async, [&pktlenth, this]() {trysendto(&pktlenth); });
				f_sendto.wait();
				pkt_time = timeGetTime();
				retrytimes++;
			}
			f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
			if (f_recvfrom_status == future_status::ready)
				break;
			if (retrytimes == 3)
			{
				exit(1);
			}
;		}
		f_recvfrom.wait();
		cout << "OK";
		//else
		//	if (*(__int8*)(recvbuf + 1) != (__int8)msg_ACK)
		//		return wrongstep;
		// 
		//从序号1开始传数据
		while (1)
		{
			memset(sendbuf, 0, sizeof(sendbuf));
			*(__int8*)(sendbuf + 1) = (__int8)msg_DATA;
			*(__int8*)(sendbuf + 2) = *((__int8*)&pktnum + 1);		//高低端转换
			*(__int8*)(sendbuf + 3) = *(__int8*)&pktnum;
			file.read(sendbuf + 4, slicelenth);
			if (filelenth >= slicelenth)		//更新filelenth,计算pktlenth
			{
				filelenth -= slicelenth;
				pktlenth = 4 + slicelenth;
			}
			else
			{
				done = 1;
				pktlenth = 4 + filelenth;
			}
			
			future<void> f_sendto = async(launch::async, [&pktlenth, this]() {trysendto(&pktlenth); });
			f_sendto.wait();
			cout << "pktnum=" <<pktnum<< "尝试发送:" << pktlenth - 4 << "字节....";

			future<void> f_recvfrom = async(launch::async, [&pktlenth, this]() {tryrecvfrom(&pktlenth); });
			f_recvfrom.wait();
			cout << "成功！" << endl;
			pktnum++;

			if (done)
				break;
		}
		return ok;
	}
private:
	t_status trysendto(int *pktlenth)
	{
		if (sendto(client, sendbuf, *pktlenth, 0, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
			return sendwrong;
		return ok;
	}

	t_status tryrecvfrom(int* pktlenth)
	{
		if ((*pktlenth = recvfrom(client, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)server_addr, &server_addr_length)) == -1)
			return recvwrong;
		return ok;
	}
};
