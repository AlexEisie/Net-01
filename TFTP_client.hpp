#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <future>
#include <fstream>
#include <ctime>
#include "TFTP_INFO.hpp"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma warning(disable : 4996)

extern int slicelenth;
extern long long default_timeout;
extern int max_retrytimes;

long long tftp_timeout = default_timeout;  //发生超时*2,接收成功恢复

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

enum tftp_status
{
	tftp_err_max_retrytimes = -1,
	tftp_err_not_expected_pkt = -1,
	tftp_err_get_error_Opcode = -1,
	tftp_pkt_ok = 0
};

class TFTP_msg
{
public:
	struct pkt {
		char data[1200];
		int lenth;
	};
	struct pkt sendpkt;
	struct pkt recvpkt;
	SOCKET client;
	struct sockaddr_in* server_addr;
	int server_addr_length;
	TFTP_msg(SOCKET c, struct sockaddr_in* s, msg_kind Opcode)
	{
		memset(&sendpkt, 0, sizeof(sendpkt));
		memset(&recvpkt, 0, sizeof(recvpkt));
		*(__int8*)(sendpkt.data + 1) = (__int8)Opcode;
		client = c;
		server_addr = s;
		server_addr_length = sizeof(*server_addr);
	}

	t_status TFTP_readfile(const char* file_name, const char* ts_mode, ofstream& file)
	{
		int filelength = 0;
		__int16 pktnum = 1;
		bool done = 0;
		__int64 timer;
		
		int transLenth = 0;
		clock_t transTime = clock();

		int* spktlenth = &sendpkt.lenth;
		int* rpktlenth = &recvpkt.lenth;
		future<void> f_sendto;
		future<void> f_recvfrom;
		future_status f_sendto_status;
		future_status f_recvfrom_status;

		//RRQ报文发，ACK报文收

		//构建RRQ报文
		strcpy(sendpkt.data + 2, file_name);
		strcpy(sendpkt.data + 2 + strlen(file_name) + 1, ts_mode);
		sendpkt.lenth = 2 + strlen(file_name) + strlen(ts_mode) + 2;
		
		f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
		f_sendto.wait();
		
		//从序号1开始收DATA
		while (1)
		{

			//尝试接收DATA
			timer = timeGetTime();	//启动定时器
			f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });

			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					tftp_timeout = tftp_timeout * 2;
					TFTP_INFO("ACK包发生重传，如果重传成功请注意超时时间的设置", TFTP_INFO::TIMEOUT, __LINE__, __func__);
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " 达到最大重传次数" << endl;
						TFTP_INFO("ACK达到最大重传次数", TFTP_INFO::ACK_REACH_MAX_RETRYTIMES, __LINE__, __func__);
					}
					f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
					f_sendto.wait();
					timer = timeGetTime();
				}

				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
				if (f_recvfrom_status == future_status::ready)
				{
					if (recvpkt.lenth != -1 && check_recvptk(msg_DATA, pktnum) == tftp_pkt_ok)		//tryrecvfrom线程结束,收包正确
					{
						tftp_timeout =default_timeout;
						break;
					}
					else
					{
						f_recvfrom.wait();
						f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });		//tryrecvfrom线程结束,收包错误，重启接收
					}
				}
			}
			f_recvfrom.wait();
			transLenth += recvpkt.lenth;
			if (recvpkt.lenth - 4 < 512)
				done = 1;
			cout << "\rpktnum=" << (uint16_t)pktnum << "确认接收:" << recvpkt.lenth - 4 << "字节数据...."<<flush;
			file.write(recvpkt.data+ 4, recvpkt.lenth - 4);
			
			//构造发送ACK
			memset(&sendpkt, 0, sizeof(sendpkt));
			*((__int8*)sendpkt.data + 1) = (__int8)msg_ACK;
			*(__int8*)(sendpkt.data + 2) = *((__int8*)&pktnum + 1);		//高低端转换
			*(__int8*)(sendpkt.data + 3) = *(__int8*)&pktnum;
			sendpkt.lenth = 4;

			f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
			f_sendto.wait();
			cout << "成功！        " << flush;

			pktnum++;
			if (done)
				break;
		}
		string user_msg("已完成一个文件的获取,共耗时:");
				user_msg += to_string((double)(clock() - transTime) / CLOCKS_PER_SEC);
				user_msg += "s获取了:" + to_string(transLenth);
				user_msg += "bytes";
				user_msg += "平均吞吐量为:";
				user_msg += cal_rate(transLenth, (double)(clock() - transTime) / CLOCKS_PER_SEC);
		cout << endl<<user_msg << endl;
		TFTP_INFO(user_msg.c_str(), (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
		return ok;
	}

	t_status TFTP_writefile(const char* file_name, const char* ts_mode, ifstream& file)
	{
		int filelength = 0;
		__int16 pktnum = 0;
		bool done = 0;
		__int64 timer;

		int transLenth = 0;
		clock_t transTime = clock();

		int* spktlenth = &sendpkt.lenth;
		int* rpktlenth = &recvpkt.lenth;
		future<void> f_sendto;
		future<void> f_recvfrom;
		future_status f_sendto_status;
		future_status f_recvfrom_status;

		//string proced;
		//future<void> f_proced;

		//计算文件总大小
		file.seekg(0, std::ios::end);
		int filelenth = file.tellg();
		file.seekg(0, std::ios::beg);

		//WRQ报文发，ACK报文收

		//构建WRQ报文
		strcpy(sendpkt.data + 2, file_name);
		strcpy(sendpkt.data + 2 + strlen(file_name) + 1, ts_mode);
		sendpkt.lenth = 2 + strlen(file_name) + strlen(ts_mode) + 2;
		
		f_sendto = async(launch::async, [spktlenth,this]() {trysendto(spktlenth);});
		f_sendto.wait();
		
		timer = timeGetTime();	//启动定时器
		f_recvfrom = async(launch::async, [rpktlenth,this]() {tryrecvfrom(rpktlenth);});

		for(int retrytimes=0;retrytimes<=max_retrytimes;)
		{
			if (timeGetTime() - timer >= tftp_timeout)
			{
				tftp_timeout = tftp_timeout * 2;
				TFTP_INFO("WRQ发生重传，如果重传成功请注意超时时间的设置", TFTP_INFO::TIMEOUT, __LINE__, __func__);
				if (++retrytimes > max_retrytimes)
				{
					cout <<"LINE:" << __LINE__ << " 达到最大重传次数" << endl;
					TFTP_INFO("WRQ达到最大重传次数", TFTP_INFO::REQUEST_REACH_MAX_RETRYTIMES, __LINE__, __func__);
				}
				f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
				f_sendto.wait();
				timer = timeGetTime();
			}
			f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
			if (f_recvfrom_status == future_status::ready)
			{
				if (recvpkt.lenth != -1 && check_recvptk(msg_ACK, pktnum) == tftp_pkt_ok)		//tryrecvfrom线程结束,收包正确
				{
					tftp_timeout = default_timeout;
					break;
				}
				else
				{
					f_recvfrom.wait();
					f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });		//tryrecvfrom线程结束,收包错误，重启接收
				}
			}
;		}
		f_recvfrom.wait();
		pktnum++;

		//从序号1开始传DATA
		while (1)
		{
			//构造发送DATA包
			memset(&sendpkt, 0, sizeof(sendpkt));
			*(__int8*)(sendpkt.data + 1) = (__int8)msg_DATA;
			*(__int8*)(sendpkt.data + 2) = *((__int8*)&pktnum + 1);		//高低端转换
			*(__int8*)(sendpkt.data + 3) = *(__int8*)&pktnum;
			file.read(sendpkt.data + 4, slicelenth);
			if (filelenth >= slicelenth)		//更新filelenth,计算pktlenth
			{
				filelenth -= slicelenth;
				sendpkt.lenth = 4 + slicelenth;
			}
			else
			{
				done = 1;
				sendpkt.lenth = 4 + filelenth;
			}
			transLenth += sendpkt.lenth;

			//f_proced = async(launch::async, [transLenth, filelenth, &proced, this]() {cal_process(transLenth, filelenth, proced); });

			f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
			f_sendto.wait();
			cout << "\rpktnum=" << (uint16_t)pktnum<< "正在发送:" << sendpkt.lenth - 4 << "字节数据...."<<flush;

			//尝试接收ACK
			timer = timeGetTime();	//启动定时器
			f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });

			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					tftp_timeout = tftp_timeout * 2;
					TFTP_INFO("DATA发生重传，如果重传成功请注意超时时间的设置", TFTP_INFO::TIMEOUT, __LINE__, __func__);
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " 达到最大重传次数" << endl;
						TFTP_INFO("DATA达到最大重传次数", TFTP_INFO::DATA_REACH_MAX_RETRYTIMES, __LINE__, __func__);
					}
					f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
					f_sendto.wait();
					timer = timeGetTime();
				}

				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
				if (f_recvfrom_status == future_status::ready)
				{
					if (recvpkt.lenth != -1 && check_recvptk(msg_ACK, pktnum) == tftp_pkt_ok)		//tryrecvfrom线程结束,收包正确
					{
						tftp_timeout = default_timeout;
						break;
					}
					else
					{
						f_recvfrom.wait();
						f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });		//tryrecvfrom线程结束,收包错误，重启接收
					}
				}
			}
			f_recvfrom.wait();
			cout << "成功！             " << cal_process(transLenth,filelenth) <<flush;
			pktnum++;

			if (done)
				break;
		}
		string user_msg("已完成一个文件的发送,共耗时:");
				user_msg += to_string((double)(clock() - transTime) / CLOCKS_PER_SEC);
				user_msg += "s发送了:" + to_string(transLenth);
				user_msg += "bytes";
				user_msg += "平均吞吐量为:";
				user_msg += cal_rate(transLenth, (double)(clock() - transTime) / CLOCKS_PER_SEC);
		cout << endl <<user_msg<<endl;
		TFTP_INFO(user_msg.c_str(),(TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
		return ok;
	}
private:
	//尝试进行sendto
	void trysendto(int *pktlenth)
	{
		if (sendto(client, sendpkt.data, *pktlenth, 0, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
			TFTP_INFO("socket sendto函数返回-1", TFTP_INFO::SENDTO_FAILED, __LINE__, __func__);
	}
	
	//尝试进行recvfrom，线程1s后自动结束
	void tryrecvfrom(int* pktlenth)
	{
		timeval tv = { 1, 0};
		setsockopt(client, SOL_SOCKET,SO_RCVTIMEO, (char*)&tv, sizeof(timeval));	//设置socket行为，如果在tv超时时间后没有收到结果，则recvfrom退出阻塞状态返回10062
		if ((*pktlenth = recvfrom(client, recvpkt.data, sizeof(recvpkt.data), 0, (struct sockaddr*)server_addr, &server_addr_length)) == -1)
			TFTP_INFO("socket recvfrom函数返回-1", TFTP_INFO::RECVFROM_FAILED, __LINE__, __func__);
	}

	//检查分组Opcode及pktnum
	tftp_status check_recvptk(msg_kind Opcode, __int16 pktnum)
	{
		if (*(__int8*)(recvpkt.data + 1) == (__int8)Opcode
			&& *(__int8*)(recvpkt.data + 2) == *((__int8*)&pktnum + 1)
			&&*(__int8*)(recvpkt.data + 3) == *(__int8*)&pktnum)
			return tftp_pkt_ok;
		else if (*(__int8*)(recvpkt.data + 1) == (__int8)msg_ERROR)
		{
			string user_msg("收到Opcode=ERROR:");
			user_msg += recvpkt.data + 4;
			cout << user_msg << endl;
			TFTP_INFO(user_msg.c_str(), TFTP_INFO::RECVED_ERROR_PACKET, __LINE__, __func__);
			return tftp_err_get_error_Opcode;
		}
		else
		{
			cout <<"收到意料之外的分组"<<endl;
			TFTP_INFO("收到意料之外的分组", TFTP_INFO::RECVED_UNEXPECTED_PACKET, __LINE__, __func__);
			return tftp_err_not_expected_pkt;
		}
	}
	//进度计算与进度条生成
	string cal_process(int transed,int left)
	{
		float perc = (float)transed / (transed + left);
		int filled_num = perc * 20+1;
		string processBar(20, '-');

		for (int i = 0; i < filled_num; i++)
		{
			processBar[i] = '#';
		}

		return  "[" + processBar + "] " + to_string((int)(perc*100+1)) +"%      ";
	}
	//总吞吐量计算
	string cal_rate(int transLenth,double cost_time)
	{
		char units[3][8] = { "B/s","KB/s","MB/s" };
		int unit = 0;
		double speed = (double)transLenth / cost_time;
		if (speed >= 1024)
		{
			unit++;		//->KB
			speed = speed / 1024;
			if (speed >= 1024)
			{
				unit++;		//->MB
				speed = speed / 1024;
			}
		}
		return to_string(speed) + (string)units[unit];
	}
};
