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

using namespace std;

#define DEBUG 1
int THREAD_NUM=8;
//线程状态探测间隔
#define f_detect_gap 30
#define MAX_THREAD_NUM 8
//主要参数声明
extern long slicelenth;
extern long long default_timeout;
extern int max_retrytimes;

long long tftp_timeout = default_timeout;  //发生超时*2,接收成功恢复

//Opcode枚举定义
enum msg_kind
{
	msg_RRQ = (__int16)1,
	msg_WRQ = (__int16)2,
	msg_DATA = (__int16)3,
	msg_ACK = (__int16)4,
	msg_ERROR = (__int16)5
};
//TFTP返回值枚举定义
enum t_status
{
	sendwrong = -1,
	recvwrong = -1,
	wrongstep = -1,
	ok = 0
};

//TFTP Client传输类
class TFTP_msg
{
public:
	//TFTP报文数据结构
	struct pkt 
	{
		struct
		{
			__int16 Opcode;
			union
			{
				struct { char filename_tsmode[1024]; }RQ;
				struct { __int16 pktnum; }ACK;
				struct { __int16 pktnum; char data[1024]; }Data;
				struct { __int16 ErrorCode; char detail[1024]; }Error;
			}payload;
		} content;	//实际报文内容
		int lenth;	//报文长度
	};
	//TFTP报文定义
#ifdef DEBUG
	struct pkt sendpkt[MAX_THREAD_NUM];
	struct pkt recvpkt[MAX_THREAD_NUM];
#else
	struct pkt sendpkt;
	struct pkt recvpkt;
#endif
	//Client套接字与服务器地址结构定义
	SOCKET client;
	struct sockaddr_in* server_addr;
	int server_addr_length;
	msg_kind Opcode;
	//TFTP_msg类构造函数
	//将下面的函数格式化为类的构造函数，方便使用
	TFTP_msg(SOCKET c, struct sockaddr_in* s, msg_kind opcode)
	{
		//类初始化
		memset(&sendpkt, 0, sizeof(sendpkt));
		memset(&recvpkt, 0, sizeof(recvpkt));
		Opcode = opcode;
		client = c;
		server_addr = s;
		server_addr_length = sizeof(*server_addr);
		std::ios::sync_with_stdio(false);		//解除输出线程绑定
	}

#ifdef DEBUG
	//用于Client向服务器读文件
	t_status TFTP_readfile(const char* file_name, const char* ts_mode, ofstream& file)
	{
		//int filelength = 0;
		//__int16 pktnum = 1;
		//bool done = 0;
		//__int64 timer;

		//int transLenth = 0;
		//clock_t transTime = clock();

		//future<void> f_recvfrom;
		//future_status f_recvfrom_status;

		////RRQ报文发，ACK报文收

		////构建发送RRQ报文
		//Create_Pkt(msg_RRQ, file_name, ts_mode);
		//trysendto(spktlenth);

		////从序号1开始收DATA
		//while (1)
		//{
		//	//尝试接收DATA
		//		//启动定时器与接收线程
		//	timer = timeGetTime();
		//	f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });
		//	//等待接收与重传
		//	for (int retrytimes = 0; retrytimes <= max_retrytimes;)
		//	{
		//		if (timeGetTime() - timer >= tftp_timeout)
		//		{
		//			//超时重传
		//			tftp_timeout = tftp_timeout * 2;
		//			TFTP_INFO("ACK包发生重传，如果重传成功请注意超时时间的设置", TFTP_INFO::TIMEOUT, __LINE__, __func__);
		//			if (++retrytimes > max_retrytimes)
		//			{
		//				cout << "LINE:" << __LINE__ << " 达到最大重传次数" << endl;
		//				TFTP_INFO("ACK达到最大重传次数", TFTP_INFO::ACK_REACH_MAX_RETRYTIMES, __LINE__, __func__);
		//			}
		//			trysendto(spktlenth);
		//			timer = timeGetTime();
		//		}

		//		//探测recvfrom线程状态
		//		f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(f_detect_gap));
		//		if (f_recvfrom_status == future_status::ready)
		//		{
		//			//检查收包
		//			if (recvpkt.lenth != -1 && check_recvpkt(msg_DATA, pktnum) == ok)
		//			{
		//				//tryrecvfrom线程结束,收包正确
		//				tftp_timeout = default_timeout;
		//				break;
		//			}
		//			else
		//			{
		//				//tryrecvfrom线程结束,收包错误，重启接收
		//				f_recvfrom.wait();
		//				f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });
		//			}
		//		}
		//	}
		//	f_recvfrom.wait();
		//	transLenth += recvpkt.lenth;
		//	if (recvpkt.lenth - 4 < 512)
		//		done = 1;
		//	printf("成功！\t\rpktnum=%d确认接收:%d字节数据....", (uint16_t)pktnum, recvpkt.lenth - 4);

		//	file.write(recvpkt.content.payload.Data.data, recvpkt.lenth - 4);

		//	//构造发送ACK
		//	Create_Pkt(msg_ACK, pktnum);
		//	trysendto(spktlenth); pktnum++;

		//	if (done)
		//		break;
		//}
		////向用户报告传输详情
		//string user_msg("已完成一个文件的获取,共耗时:");
		//user_msg += to_string((double)(clock() - transTime) / CLOCKS_PER_SEC);
		//user_msg += "s获取了:" + to_string(transLenth);
		//user_msg += "bytes";
		//user_msg += "平均吞吐量为:";
		//user_msg += cal_rate(transLenth, (double)(clock() - transTime) / CLOCKS_PER_SEC);
		//cout << endl << user_msg << endl;
		//TFTP_INFO(user_msg.c_str(), (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
		return ok;
	}
	//用于Client向服务器写文件
	t_status TFTP_writefile(const char* file_name, const char* ts_mode, ifstream& file)
	{
		long filelenth = 0;
		__int16 pktnum = 0;
		__int16 pktnum_base = 0;
		bool file_done = 0;
		bool pkt_done[MAX_THREAD_NUM];
		bool cro_done = 0;
		__int64 timer[MAX_THREAD_NUM];
		int retrytimes[MAX_THREAD_NUM];
		struct pkt* recvpkt_ptr[MAX_THREAD_NUM];

		int transLenth = 0;
		clock_t transTime = clock();

		future<void> f_recvfrom[MAX_THREAD_NUM];
		future_status f_recvfrom_status[MAX_THREAD_NUM];

		memset(pkt_done, 0, sizeof(pkt_done));

		for (int thread = 0; thread < THREAD_NUM; thread++)
		{
			recvpkt_ptr[thread]= &recvpkt[thread];
		}
		//计算文件总大小
		file.seekg(0, std::ios::end);
		filelenth = file.tellg();
		file.seekg(0, std::ios::beg);

		//WRQ报文发，ACK报文收

		//构建WRQ报文
		Create_Pkt(&sendpkt[0],msg_WRQ, file_name, ts_mode);
		trysendto(&sendpkt[0]);

		//启动定时器与接收线程
		timer[0] = timeGetTime();
		f_recvfrom[0] = async(launch::async, [&]() {tryrecvfrom(&recvpkt[0]); });
		memset(retrytimes, 0, sizeof(retrytimes));
		//等待接收与重传
		while(1)
		{
			if (timeGetTime() - timer[0] >= tftp_timeout)
			{
				tftp_timeout = tftp_timeout * 2;
				TFTP_INFO("WRQ发生重传，如果重传成功请注意超时时间的设置", TFTP_INFO::TIMEOUT, __LINE__, __func__);
				if (retrytimes[0] > max_retrytimes)
				{
					cout << "LINE:" << __LINE__ << " 达到最大重传次数" << endl;
					TFTP_INFO("WRQ达到最大重传次数", TFTP_INFO::REQUEST_REACH_MAX_RETRYTIMES, __LINE__, __func__);
				}
				trysendto(&sendpkt[0]);
				timer[0] = timeGetTime();
			}

			//探测recvfrom线程状态
			f_recvfrom_status[0] = f_recvfrom[0].wait_for(chrono::microseconds(0));
			if (f_recvfrom_status[0] == future_status::ready)
			{
				if (recvpkt[0].lenth != -1 && check_recvpkt(recvpkt[0], msg_ACK, pktnum) == ok)
				{
					//tryrecvfrom线程结束,收包正确
					tftp_timeout = default_timeout;
					break;
				}
				else
				{
					//tryrecvfrom线程结束,收包错误，重启接收
					f_recvfrom[0].wait();
					f_recvfrom[0] = async(launch::async, [&]() {tryrecvfrom(&recvpkt[0]); });
				}
			}
		}
		f_recvfrom[0].wait();
		pktnum++;
		pktnum_base++;

		//从序号1开始传DATA
		while (1)
		{
			//构造发送DATA包
			for (int thread = 0; thread < THREAD_NUM; thread++)
			{
				if (file_done = Create_Pkt(&sendpkt[thread], msg_DATA, pktnum, file, &filelenth))
					THREAD_NUM = thread;
				transLenth += sendpkt[thread].lenth;
				trysendto(&sendpkt[thread]);
				pktnum++;
				printf("成功! %s\t\rpktnum=%d正在发送:%d字节数据....", cal_process(transLenth, filelenth).c_str(), (uint16_t)pktnum, sendpkt[thread].lenth - 4);
			}

			//尝试接收ACK
			for (int thread = 0; thread < THREAD_NUM; thread++)
			{
				timer[THREAD_NUM] = timeGetTime();	//启动定时器
				f_recvfrom[THREAD_NUM] = async(launch::async, [thread,recvpkt_ptr]() {tryrecvfrom(recvpkt_ptr[thread]); });
			}
			memset(pkt_done, 0, sizeof(pkt_done));
			memset(retrytimes, 0, sizeof(retrytimes));
			
			//等待接收与重传
			while (1)
			{
				//如果所有pkt_done都为1，说明所有包都已经确认，跳出循环
				cro_done = 1;
				for (int thread = 0; thread < THREAD_NUM; thread++)
				{
					if (pkt_done[thread]==0)
					{
						cro_done = 0;
						break;
					}
				}
				if (cro_done)
					break;

				//探测recvfrom线程状态
				for (int thread = 0; thread < THREAD_NUM; thread++)
				{
					f_recvfrom_status[thread] = f_recvfrom[thread].wait_for(chrono::microseconds(f_detect_gap));
					if (f_recvfrom_status[thread] == future_status::ready)
					{
						//tryrecvfrom线程结束,收包正确
						if (recvpkt[thread].lenth != -1 && check_recvpkt(recvpkt[thread], msg_ACK, pktnum) == ok)
						{
							tftp_timeout = default_timeout;		//可能会产生问题
							pkt_done[thread] = 1;
						}
						else
						{
							//tryrecvfrom线程结束,收包错误，重启接收
							f_recvfrom[thread].wait();
							f_recvfrom[thread] = async(launch::async, [&]() {tryrecvfrom(&recvpkt[thread]); });
						}
					}
				}
				
				for (int thread = 0; thread < THREAD_NUM; thread++)
				{
					if(pkt_done[thread])		//跳过已经确认的包
						continue;
					if (timeGetTime() - timer[thread] >= tftp_timeout)
					{
						tftp_timeout = tftp_timeout * 2;
						TFTP_INFO("DATA发生重传，如果重传成功请注意超时时间的设置", TFTP_INFO::TIMEOUT, __LINE__, __func__);
						if (retrytimes[thread] > max_retrytimes)
						{
							cout << "LINE:" << __LINE__ << " 达到最大重传次数" << endl;
							TFTP_INFO("DATA达到最大重传次数", TFTP_INFO::DATA_REACH_MAX_RETRYTIMES, __LINE__, __func__);
						}
						trysendto(&sendpkt[thread]);
						timer[thread] = timeGetTime();
					}
				}
			}

			/*for (int thread = 0; thread < THREAD_NUM; thread++)
				f_recvfrom[thread].wait();*/		//似乎无需等待，能到这一步的都是已经确认的包
			pktnum_base += THREAD_NUM;
			pktnum = pktnum_base;
			if (file_done)
				break;
		}

		//向用户报告传输详情
		string user_msg("已完成一个文件的发送,共耗时:");
		user_msg += to_string((double)(clock() - transTime) / CLOCKS_PER_SEC);
		user_msg += "s发送了:" + to_string(transLenth);
		user_msg += "bytes";
		user_msg += "平均吞吐量为:";
		user_msg += cal_rate(transLenth, (double)(clock() - transTime) / CLOCKS_PER_SEC);
		cout << endl << user_msg << endl;
		TFTP_INFO(user_msg.c_str(), (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
		return ok;
	}
#else
		//用于Client向服务器读文件
	t_status TFTP_readfile(const char* file_name, const char* ts_mode, ofstream& file)
	{
		int filelength = 0;
		__int16 pktnum = 1;
		bool done = 0;
		__int64 timer;

		int transLenth = 0;
		clock_t transTime = clock();

		future<void> f_recvfrom;
		future_status f_recvfrom_status;

		//RRQ报文发，ACK报文收

		//构建发送RRQ报文
		Create_Pkt(msg_RRQ, file_name, ts_mode);
		trysendto(spktlenth);

		//从序号1开始收DATA
		while (1)
		{
		//尝试接收DATA
			//启动定时器与接收线程
			timer = timeGetTime();	
			f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });
			//等待接收与重传
			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					//超时重传
					tftp_timeout = tftp_timeout * 2;
					TFTP_INFO("ACK包发生重传，如果重传成功请注意超时时间的设置", TFTP_INFO::TIMEOUT, __LINE__, __func__);
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " 达到最大重传次数" << endl;
						TFTP_INFO("ACK达到最大重传次数", TFTP_INFO::ACK_REACH_MAX_RETRYTIMES, __LINE__, __func__);
					}
					trysendto(spktlenth);
					timer = timeGetTime();
				}

				//探测recvfrom线程状态
				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(f_detect_gap));
				if (f_recvfrom_status == future_status::ready)
				{
					//检查收包
					if (recvpkt.lenth != -1 && check_recvpkt(msg_DATA, pktnum) == ok)		
					{
						//tryrecvfrom线程结束,收包正确
						tftp_timeout = default_timeout;
						break;
					}
					else
					{
						//tryrecvfrom线程结束,收包错误，重启接收
						f_recvfrom.wait();
						f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });		
					}
				}
			}
			f_recvfrom.wait();
			transLenth += recvpkt.lenth;
			if (recvpkt.lenth - 4 < 512)
				done = 1;
			printf("成功！\t\rpktnum=%d确认接收:%d字节数据....", (uint16_t)pktnum, recvpkt.lenth - 4);

			file.write(recvpkt.content.payload.Data.data, recvpkt.lenth - 4);

			//构造发送ACK
			Create_Pkt(msg_ACK, pktnum);
			trysendto(spktlenth);pktnum++;

			if (done)
				break;
		}
		//向用户报告传输详情
		string user_msg("已完成一个文件的获取,共耗时:");
		user_msg += to_string((double)(clock() - transTime) / CLOCKS_PER_SEC);
		user_msg += "s获取了:" + to_string(transLenth);
		user_msg += "bytes";
		user_msg += "平均吞吐量为:";
		user_msg += cal_rate(transLenth, (double)(clock() - transTime) / CLOCKS_PER_SEC);
		cout << endl << user_msg << endl;
		TFTP_INFO(user_msg.c_str(), (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
		return ok;
	}
	//用于Client向服务器写文件
	t_status TFTP_writefile(const char* file_name, const char* ts_mode, ifstream& file)
	{
		long filelenth = 0;
		__int16 pktnum = 0;
		bool done = 0;
		__int64 timer;

		int transLenth = 0;
		clock_t transTime = clock();
		
		future<void> f_recvfrom;
		future_status f_recvfrom_status;

		//计算文件总大小
		file.seekg(0, std::ios::end);
		filelenth = file.tellg();
		file.seekg(0, std::ios::beg);

		//WRQ报文发，ACK报文收

		//构建WRQ报文
		Create_Pkt(msg_WRQ, file_name, ts_mode);
		trysendto(&sendpkt.lenth);

		//启动定时器与接收线程
		timer = timeGetTime();	
		f_recvfrom = async(launch::async, [&]() {tryrecvfrom(&sendpkt.lenth); });

		//等待接收与重传
		for (int retrytimes = 0; retrytimes <= max_retrytimes;)
		{
			if (timeGetTime() - timer >= tftp_timeout)
			{
				tftp_timeout = tftp_timeout * 2;
				TFTP_INFO("WRQ发生重传，如果重传成功请注意超时时间的设置", TFTP_INFO::TIMEOUT, __LINE__, __func__);
				if (++retrytimes > max_retrytimes)
				{
					cout << "LINE:" << __LINE__ << " 达到最大重传次数" << endl;
					TFTP_INFO("WRQ达到最大重传次数", TFTP_INFO::REQUEST_REACH_MAX_RETRYTIMES, __LINE__, __func__);
				}
				trysendto(spktlenth);
				timer = timeGetTime();
			}

			//探测recvfrom线程状态
			f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
			if (f_recvfrom_status == future_status::ready)
			{
				if (recvpkt.lenth != -1 && check_recvpkt(msg_ACK, pktnum) == ok)
				{
					//tryrecvfrom线程结束,收包正确
					tftp_timeout = default_timeout;
					break;
				}
				else
				{
					//tryrecvfrom线程结束,收包错误，重启接收
					f_recvfrom.wait();
					f_recvfrom = async(launch::async, [&]() {tryrecvfrom(&sendpkt.lenth); });
				}
			}
		}
		f_recvfrom.wait();
		pktnum++;

		//从序号1开始传DATA
		while (1)
		{
		//构造发送DATA包
			done=Create_Pkt(msg_DATA, pktnum, file, &filelenth);
			transLenth += sendpkt.lenth;
			trysendto(&sendpkt.lenth);

			printf("成功! %s\t\rpktnum=%d正在发送:%d字节数据....", cal_process(transLenth, filelenth).c_str(), (uint16_t)pktnum, sendpkt.lenth - 4);
		//尝试接收ACK
			timer = timeGetTime();	//启动定时器
			f_recvfrom = async(launch::async, [&]() {tryrecvfrom(&sendpkt.lenth); });

			//等待接收与重传
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
					trysendto(&sendpkt.lenth);
					timer = timeGetTime();
				}

				//探测recvfrom线程状态
				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(f_detect_gap));
				if (f_recvfrom_status == future_status::ready)
				{
					//tryrecvfrom线程结束,收包正确
					if (recvpkt.lenth != -1 && check_recvpkt(msg_ACK, pktnum) == ok)
					{
						tftp_timeout = default_timeout;
						break;
					}
					else
					{
						//tryrecvfrom线程结束,收包错误，重启接收
						f_recvfrom.wait();
						f_recvfrom = async(launch::async, [&]() {tryrecvfrom(&sendpkt.lenth); });
					}
				}
			}
			f_recvfrom.wait();
			pktnum++;
			if (done)
				break;
		}

		//向用户报告传输详情
		string user_msg("已完成一个文件的发送,共耗时:");
		user_msg += to_string((double)(clock() - transTime) / CLOCKS_PER_SEC);
		user_msg += "s发送了:" + to_string(transLenth);
		user_msg += "bytes";
		user_msg += "平均吞吐量为:";
		user_msg += cal_rate(transLenth, (double)(clock() - transTime) / CLOCKS_PER_SEC);
		cout << endl << user_msg << endl;
		TFTP_INFO(user_msg.c_str(), (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
		return ok;
	}
#endif
private:
	//转换__int16高低字节
	__int16 reverse_HL(__int16 num)
	{
		return (num >> 8 & 0xff) + (num << 8 & 0xff00);
	}
	//尝试进行sendto
	void trysendto(struct pkt *sendpkt)
	{
		if (sendto(client, (char*) &sendpkt->content, sendpkt->lenth, 0, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
			TFTP_INFO("socket sendto函数返回-1", TFTP_INFO::SENDTO_FAILED, __LINE__, __func__);
	}
	
	//尝试进行recvfrom，线程4s后自动结束
	void tryrecvfrom(struct pkt *recvpkt)
	{
		timeval tv = { 4, 0};
		setsockopt(client, SOL_SOCKET,SO_RCVTIMEO, (char*)&tv, sizeof(timeval));	//设置socket行为，如果在tv超时时间后没有收到结果，则recvfrom退出阻塞状态返回10062
		if ((recvpkt->lenth = recvfrom(client, (char*) &recvpkt->content, sizeof(recvpkt->content), 0, (struct sockaddr*)server_addr, &server_addr_length)) == -1)
			TFTP_INFO("socket recvfrom函数返回-1", TFTP_INFO::RECVFROM_FAILED, __LINE__, __func__);
	}

	//构建发送报文_重载1_RQ
	void Create_Pkt(struct pkt *sendpkt,__int16 _Opcode,const  char* _file_name,const char* _ts_mode)
	{
		memset(sendpkt, 0, sizeof(sendpkt));
		sendpkt->content.Opcode = reverse_HL(_Opcode);
		strcpy(sendpkt->content.payload.RQ.filename_tsmode, _file_name);
		strcpy(sendpkt->content.payload.RQ.filename_tsmode + strlen(_file_name) + 1, _ts_mode);
		sendpkt->lenth = 2 + strlen(_file_name) + strlen(_ts_mode) + 2;
	}
	//构建发送报文_重载2_ACK
	void Create_Pkt(struct pkt *sendpkt, __int16 _Opcode, __int16 _pktnum)
	{
		memset(sendpkt, 0, sizeof(sendpkt));
		sendpkt->content.Opcode = reverse_HL(_Opcode);
		sendpkt->content.payload.ACK.pktnum = reverse_HL(_pktnum);
		sendpkt->lenth = 4;

	}
	//构建发送报文_重载3_DATA 文件读取进入最后一个段返回1
	bool Create_Pkt(struct pkt *sendpkt, __int16 _Opcode, __int16 _pktnum,ifstream& _file,long * filelenth)
	{
		memset(sendpkt, 0, sizeof(sendpkt));
		sendpkt->content.Opcode = reverse_HL(_Opcode);
		sendpkt->content.payload.Data.pktnum = reverse_HL(_pktnum);
		_file.read(sendpkt->content.payload.Data.data, slicelenth);
		if (*filelenth >= slicelenth)		//更新filelenth,计算pktlenth
		{
			*filelenth -= slicelenth;
			sendpkt->lenth = 4 + slicelenth;
			return 0;
		}
		else
		{
			sendpkt->lenth = 4 + *filelenth;
			return 1;
		}
	}
	//解析接收报文
	t_status check_recvpkt(struct pkt recvpkt, msg_kind Opcode, __int16 pktnum)
	{
		if (recvpkt.content.Opcode == reverse_HL(Opcode)&&
			recvpkt.content.payload.ACK.pktnum == reverse_HL(pktnum))	//此处ACK与DATA的pktnum等价
			return ok;
		else if (recvpkt.content.Opcode == reverse_HL(msg_ERROR))
		{
			string user_msg("收到Opcode=ERROR:");
			user_msg += recvpkt.content.payload.Error.detail;
			cout << user_msg << endl;
			TFTP_INFO(user_msg.c_str(), TFTP_INFO::RECVED_ERROR_PACKET, __LINE__, __func__);
			return wrongstep;
		}
		else
		{
			cout <<"收到意料之外的分组"<<endl;
			TFTP_INFO("收到意料之外的分组", TFTP_INFO::RECVED_UNEXPECTED_PACKET, __LINE__, __func__);
			return wrongstep;
		}
	}
	//进度计算与进度条生成
	string cal_process(int transed,long left)
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
