#ifndef TFTP_CLIENT_HPP
#define TFTP_CLIENT_HPP
#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <future>
#include <fstream>
#include <ctime>
#include"TFTP_Print.hpp"
#include "TFTP_INFO.hpp"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma warning(disable : 4996)

//客户端参数
constexpr int f_detect_gap = 30;
extern long slicelenth;
extern long long default_timeout;
extern int max_retrytimes;

long long tftp_timeout = default_timeout; //发生超时*2,接收成功恢复

using namespace std;

enum msg_kind
{
	msg_RRQ = static_cast<__int16>(1),
	msg_WRQ = static_cast<__int16>(2),
	msg_DATA = static_cast<__int16>(3),
	msg_ACK = static_cast<__int16>(4),
	msg_ERROR = static_cast<__int16>(5)
};

enum t_status
{
	sendwrong = -1,
	recvwrong = -1,
	wrongstep = -1,
	ok = 0,
	dupstep = 1
};

//TFTP报文类
class TFTP_msg
{
public:
	//TFTP报文结构体
	struct pkt
	{
		struct
		{
			__int16 Opcode;

			union
			{
				struct
				{
					char filename_tsmode[1024];
				} RQ;

				struct
				{
					__int16 pktnum;
				} ACK;

				struct
				{
					__int16 pktnum;
					char data[1024];
				} Data;

				struct
				{
					__int16 ErrorCode;
					char detail[1024];
				} Error;
			} payload;
		} content;

		int lenth;
	};

	struct pkt sendpkt;
	struct pkt recvpkt;
	SOCKET client;
	struct sockaddr_in* server_addr;
	int server_addr_length;
	msg_kind Opcode;
	//构造函数
	TFTP_msg(SOCKET c, struct sockaddr_in* s, msg_kind opcode)
	{
		memset(&sendpkt, 0, sizeof(sendpkt));
		memset(&recvpkt, 0, sizeof(recvpkt));
		Opcode = opcode;
		client = c;
		server_addr = s;
		server_addr_length = sizeof(*server_addr);
		//std::ios::sync_with_stdio(false);		//解除输出线程绑定
		tftp_timeout = default_timeout;
	}

	//用于Client从服务器读文件
	t_status TFTP_readfile(const char* file_name, const char* ts_mode, ofstream& file);
	//用于Client向服务器写文件
	t_status TFTP_writefile(const char* file_name, const char* ts_mode, ifstream& file);

	//文件netascii化
	string do_netascii(ifstream& originfile)
	{
		string newfilename("netascii_");
		ofstream newfile;
		char proc_buf[1024];
		int proc_lenth = 0;
		long file_lenth = 0;

		originfile.seekg(0, std::ios::end);
		file_lenth = originfile.tellg();
		originfile.seekg(0, std::ios::beg);
		//清除原临时文件
		ifstream check_file(newfilename, ios::in | ios::binary);
		if (check_file.good())
		{
			check_file.close();
			remove(newfilename.c_str());
		}

		newfile.open(newfilename, ios::out | ios::binary);
		while (file_lenth >0)
		{
			memset(proc_buf, 0, sizeof(proc_buf));
			originfile.read(proc_buf, 1024);
			if (file_lenth >= 1024)
			{
				proc_lenth = 1024;
				file_lenth -= 1024;
			}
			else if (file_lenth < 1024)
			{
				proc_lenth = file_lenth;
				file_lenth = 0;
			}

			for (int i = 0; i < proc_lenth; i++)
			{
				if (proc_buf[i] == '\n')	//LF前加CR
				{
					newfile << '\r';
					newfile << '\n';
				}
				else if (proc_buf[i] == '\r')
				{
					newfile << '\r';		//CR后加NULL
					newfile << '\0';
				}
				else 
					newfile<< proc_buf[i];
			}
		}
		newfile.close();
		return newfilename;
	}

private:
	//反转高低位
	__int16 reverse_HL(__int16 num)
	{
		return (num >> 8 & 0xff) + (num << 8 & 0xff00);
	}

	//尝试进行sendto
	void trysendto();

	//尝试进行recvfrom，线程2s后自动结束
	void tryrecvfrom();

	//构建发送报文_重载1_RQ
	void Create_Pkt(__int16 _Opcode, const char* _file_name, const char* _ts_mode);

	//构建发送报文_重载2_ACK
	void Create_Pkt(__int16 _Opcode, __int16 _pktnum)
	{
		memset(&sendpkt, 0, sizeof(sendpkt));
		sendpkt.content.Opcode = reverse_HL(_Opcode);
		sendpkt.content.payload.ACK.pktnum = reverse_HL(_pktnum);
		sendpkt.lenth = 4;
	}

	//构建发送报文_重载3_DATA 文件读取进入最后一个段返回1
	bool Create_Pkt(__int16 _Opcode, __int16 _pktnum, ifstream& _file, long* filelenth)
	{
		memset(&sendpkt, 0, sizeof(sendpkt));
		sendpkt.content.Opcode = reverse_HL(_Opcode);
		sendpkt.content.payload.Data.pktnum = reverse_HL(_pktnum);
		_file.read(sendpkt.content.payload.Data.data, slicelenth);
		if (*filelenth >= slicelenth) //更新filelenth,计算pktlenth
		{
			*filelenth -= slicelenth;
			sendpkt.lenth = 4 + slicelenth;
			return false;
		}
		sendpkt.lenth = 4 + *filelenth;
		return true;
	}

	//解析接收报文
	t_status check_recvpkt(msg_kind Opcode, __int16 pktnum);

	//进度计算与进度条生成
	string cal_process(int transed, long left)
	{
		float perc = static_cast<float>(transed) / (transed + left);
		int filled_num = perc * 20 + 1;
		string processBar(20, '-');

		for (int i = 0; i < filled_num; i++)
		{
			processBar[i] = '#';
		}

		return "[" + processBar + "] " + to_string(static_cast<int>(perc * 100 + 1)) + "%      ";
	}

	//总吞吐量计算
	string cal_rate(int transLenth, double cost_time)
	{
		char units[3][8] = {"B/s", "KB/s", "MB/s"};
		int unit = 0;
		double speed = static_cast<double>(transLenth) / cost_time;
		if (speed >= 1024)
		{
			unit++; //->KB
			speed = speed / 1024;
			if (speed >= 1024)
			{
				unit++; //->MB
				speed = speed / 1024;
			}
		}
		return to_string(speed) + static_cast<string>(units[unit]);
	}
};

//TFTP 文件读
inline t_status TFTP_msg::TFTP_readfile(const char* file_name, const char* ts_mode, ofstream& file)
{
	__int16 pktnum = 1;
	bool done = false;
	__int64 timer;

	int transLenth = 0;
	clock_t transTime = clock();

	future<void> f_recvfrom;
	future_status f_recvfrom_status;

	TFTP_Print print_thread;
	string print_buf;
	try
	{
		//RRQ报文发，ACK报文收

		//构建发送RRQ报文
		Create_Pkt(msg_RRQ, file_name, ts_mode);

		trysendto();

		//从序号1开始收DATA
		while (true)
		{
			//尝试接收DATA
			timer = timeGetTime(); //启动定时器
			f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); });

			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					//超时重传
					if (transLenth == 0)		//仅重传RRQ
						trysendto();
					string user_msg = "DATA超时 pktnum=" + to_string(static_cast<uint16_t>(pktnum));
					TFTP_INFO(user_msg.c_str(), TFTP_INFO::TIMEOUT, __LINE__, __func__);
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " 达到最大超时次数" << endl;
						TFTP_INFO("DATA超时达到最大次数", TFTP_INFO::DATA_REACH_MAX_TIMEOUTTIMES, __LINE__, __func__);
					}
					timer = timeGetTime();
				}

				//探测recvfrom线程状态
				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(f_detect_gap));
				if (f_recvfrom_status == future_status::ready)
				{
					//检查接收包
					if (recvpkt.lenth != -1)
						if (check_recvpkt(msg_DATA, pktnum) == ok)
							break;
						else if (check_recvpkt(msg_DATA, pktnum) == dupstep)
						{
							timer = timeGetTime();
							trysendto();
						}
					//f_recvfrom.wait();	//无需等待，可能会导致线程不安全
					f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); }); //tryrecvfrom线程结束,收包错误，重启接收
				}
			}
			//f_recvfrom.wait();
			transLenth += recvpkt.lenth;
			if (recvpkt.lenth - 4 < 512)
				done = true;

			//打印进度信息
			print_buf = "成功!\t\t\t\t\rpktnum=" + to_string(static_cast<uint16_t>(pktnum)) + "确认接收:" + to_string(
				recvpkt.lenth - 4) + "字节数据....";
			print_thread.set_msg(print_buf);

			file.write(recvpkt.content.payload.Data.data, recvpkt.lenth - 4);

			//构造发送ACK
			Create_Pkt(msg_ACK, pktnum);

			trysendto();
			pktnum++;
			if (done)
				break;
		}
	}
	catch (TFTP_Log& Error)
	{
		//print_thread.~TFTP_Print();
		throw;
	}

	string user_msg("已完成一个文件的获取,共耗时:");
	user_msg += to_string(static_cast<double>(clock() - transTime) / CLOCKS_PER_SEC);
	user_msg += "s获取了:" + to_string(transLenth);
	user_msg += "bytes";
	user_msg += "平均吞吐量为:";
	user_msg += cal_rate(transLenth, static_cast<double>(clock() - transTime) / CLOCKS_PER_SEC);
	cout << endl << user_msg << endl;
	TFTP_INFO(user_msg.c_str(), static_cast<TFTP_INFO::TFTP_INFO_TYPE>(0), __LINE__, __func__);
	//print_thread.~TFTP_Print();		主动析构会出问题，不知道为什么
	return ok;
}

//TFTP 文件写
inline t_status TFTP_msg::TFTP_writefile(const char* file_name, const char* ts_mode, ifstream& file)
{
	long filelenth = 0;
	__int16 pktnum = 0;
	bool done = false;
	__int64 timer;

	int transLenth = 0;
	clock_t transTime = clock();

	future<void> f_recvfrom;
	future_status f_recvfrom_status;

	TFTP_Print print_thread;
	string print_buf;

	//计算文件总大小
	file.seekg(0, std::ios::end);
	filelenth = file.tellg();
	file.seekg(0, std::ios::beg);
	try
	{
		//WRQ报文发，ACK报文收

		//构建WRQ报文
		Create_Pkt(msg_WRQ, file_name, ts_mode);
		trysendto();

		timer = timeGetTime(); //启动定时器
		f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); });

		//尝试接收ACK
		for (int retrytimes = 0; retrytimes <= max_retrytimes;)
		{
			if (timeGetTime() - timer >= tftp_timeout)
			{
				TFTP_INFO("WRQ发生重传，如果重传成功请注意超时时间的设置", TFTP_INFO::TIMEOUT, __LINE__, __func__);
				if (++retrytimes > max_retrytimes)
				{
					cout << "LINE:" << __LINE__ << " 达到最大重传次数" << endl;
					TFTP_INFO("WRQ达到最大重传次数", TFTP_INFO::REQUEST_REACH_MAX_RETRYTIMES, __LINE__, __func__);
				}
				trysendto();
				timer = timeGetTime();
			}

			//探测recvfrom线程状态
			f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
			if (f_recvfrom_status == future_status::ready)
			{
				if (recvpkt.lenth != -1 && check_recvpkt(msg_ACK, pktnum) == ok) //tryrecvfrom线程结束,收包正确
					break;
				f_recvfrom.wait();
				f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); }); //tryrecvfrom线程结束,收包错误，重启接收
			}
		}
		f_recvfrom.wait();
		pktnum++;

		//从序号1开始传DATA
		while (true)
		{
			//构造发送DATA包
			done = Create_Pkt(msg_DATA, pktnum, file, &filelenth);
			transLenth += sendpkt.lenth;

			trysendto();

			//打印进度信息
			print_buf = "成功!" + cal_process(transLenth, filelenth) + "\t\t\t\t\rpktnum=" +
				to_string(static_cast<uint16_t>(pktnum)) + "正在发送:" + to_string(sendpkt.lenth - 4) + "字节数据....";
			print_thread.set_msg(print_buf);

			//尝试接收ACK
			timer = timeGetTime(); //启动定时器
			f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); });

			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					//超时重传
					string user_msg = "DATA重传 pktnum=" + to_string(static_cast<uint16_t>(pktnum));
					TFTP_INFO(user_msg.c_str(), TFTP_INFO::TIMEOUT, __LINE__, __func__);
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " 达到最大重传次数" << endl;
						TFTP_INFO("DATA达到最大重传次数", TFTP_INFO::DATA_REACH_MAX_RETRYTIMES, __LINE__, __func__);
					}
					trysendto();
					timer = timeGetTime();
				}

				//探测recvfrom线程状态
				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(f_detect_gap));
				if (f_recvfrom_status == future_status::ready)
				{
					//检查接收包
					if (recvpkt.lenth != -1 && check_recvpkt(msg_ACK, pktnum) == ok) //tryrecvfrom线程结束,收包正确
					{
						break;
					}
					//f_recvfrom.wait();	//无需等待，可能会导致线程不安全
					f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); }); //tryrecvfrom线程结束,收包错误，重启接收
				}
			}
			//收到ACK
			f_recvfrom.wait();
			pktnum++;
			if (done)
				break;
		}
	}
	catch (TFTP_INFO& Error) //接管异常
	{
		//print_thread.~TFTP_Print();
		throw;
	}

	string user_msg("已完成一个文件的发送,共耗时:");
	user_msg += to_string(static_cast<double>(clock() - transTime) / CLOCKS_PER_SEC);
	user_msg += "s发送了:" + to_string(transLenth);
	user_msg += "bytes";
	user_msg += "平均吞吐量为:";
	user_msg += cal_rate(transLenth, static_cast<double>(clock() - transTime) / CLOCKS_PER_SEC);
	cout << endl << user_msg << endl;
	TFTP_INFO(user_msg.c_str(), static_cast<TFTP_INFO::TFTP_INFO_TYPE>(0), __LINE__, __func__);
	//print_thread.~TFTP_Print();		主动析构会出问题，不知道为什么
	return ok;
}

//TFTP_msg发送函数实现
inline void TFTP_msg::trysendto()
{
	if (sendto(client, (const char*)&sendpkt.content, sendpkt.lenth, 0, (struct sockaddr*)server_addr,
	           sizeof(*server_addr)) < 0)
		TFTP_INFO("socket sendto函数返回-1", TFTP_INFO::SENDTO_FAILED, __LINE__, __func__);
}

//TFTP_msg接收函数实现
inline void TFTP_msg::tryrecvfrom()
{
	timeval tv = { 2, 0 };
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(timeval));
	//设置socket行为，如果在tv超时时间后没有收到结果，则recvfrom退出阻塞状态返回10062
	if ((recvpkt.lenth = recvfrom(client, (char*)&recvpkt.content, sizeof(recvpkt.content), 0,
		reinterpret_cast<struct sockaddr*>(server_addr), &server_addr_length)) == -1)
		TFTP_INFO("socket recvfrom函数返回-1", TFTP_INFO::RECVFROM_FAILED, __LINE__, __func__);
}

//TFTP_msg包构造实现
inline void TFTP_msg::Create_Pkt(short _Opcode, const char* _file_name, const char* _ts_mode)
{
	memset(&sendpkt, 0, sizeof(sendpkt));
	sendpkt.content.Opcode = reverse_HL(_Opcode);
	strcpy(sendpkt.content.payload.RQ.filename_tsmode, _file_name);
	strcpy(sendpkt.content.payload.RQ.filename_tsmode + strlen(_file_name) + 1, _ts_mode);
	sendpkt.lenth = 2 + strlen(_file_name) + strlen(_ts_mode) + 2;
}

//TFTP_msg接收检查函数实现
inline t_status TFTP_msg::check_recvpkt(msg_kind Opcode, short pktnum)
{
	if (recvpkt.content.Opcode == reverse_HL(Opcode))
		if (recvpkt.content.payload.ACK.pktnum == reverse_HL(pktnum))//此处ACK与DATA的pktnum等价
			return ok;
		else if (recvpkt.content.payload.ACK.pktnum < reverse_HL(pktnum))
			return dupstep;
		else
		{
			cout << "收到意料之外的分组!";
			TFTP_INFO("收到意料之外的分组", TFTP_INFO::RECVED_UNEXPECTED_PACKET, __LINE__, __func__);
			return wrongstep;
		}
	if (recvpkt.content.Opcode == reverse_HL(msg_ERROR))
	{
		string user_msg("收到Opcode=ERROR:");
		user_msg += recvpkt.content.payload.Error.detail;
		cout << user_msg << endl;
		TFTP_INFO(user_msg.c_str(), TFTP_INFO::RECVED_ERROR_PACKET, __LINE__, __func__);
		return wrongstep;
	}
}
#endif // TFTP_CLIENT_HPP
