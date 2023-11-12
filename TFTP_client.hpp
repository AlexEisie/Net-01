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

//�ͻ��˲���
constexpr int f_detect_gap = 30;
extern long slicelenth;
extern long long default_timeout;
extern int max_retrytimes;

long long tftp_timeout = default_timeout; //������ʱ*2,���ճɹ��ָ�

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
	ok = 0
};

//TFTP������
class TFTP_msg
{
public:
	//TFTP���Ľṹ��
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
	//���캯��
	TFTP_msg(SOCKET c, struct sockaddr_in* s, msg_kind opcode)
	{
		memset(&sendpkt, 0, sizeof(sendpkt));
		memset(&recvpkt, 0, sizeof(recvpkt));
		Opcode = opcode;
		client = c;
		server_addr = s;
		server_addr_length = sizeof(*server_addr);
		//std::ios::sync_with_stdio(false);		//�������̰߳�
		tftp_timeout = default_timeout;
	}

	//����Client�ӷ��������ļ�
	t_status TFTP_readfile(const char* file_name, const char* ts_mode, ofstream& file);

	t_status TFTP_writefile(const char* file_name, const char* ts_mode, ifstream& file);

private:
	//��ת�ߵ�λ
	__int16 reverse_HL(__int16 num)
	{
		return (num >> 8 & 0xff) + (num << 8 & 0xff00);
	}

	//���Խ���sendto
	void trysendto();

	//���Խ���recvfrom���߳�2s���Զ�����
	void tryrecvfrom();

	//�������ͱ���_����1_RQ
	void Create_Pkt(__int16 _Opcode, const char* _file_name, const char* _ts_mode);

	//�������ͱ���_����2_ACK
	void Create_Pkt(__int16 _Opcode, __int16 _pktnum)
	{
		memset(&sendpkt, 0, sizeof(sendpkt));
		sendpkt.content.Opcode = reverse_HL(_Opcode);
		sendpkt.content.payload.ACK.pktnum = reverse_HL(_pktnum);
		sendpkt.lenth = 4;
	}

	//�������ͱ���_����3_DATA �ļ���ȡ�������һ���η���1
	bool Create_Pkt(__int16 _Opcode, __int16 _pktnum, ifstream& _file, long* filelenth)
	{
		memset(&sendpkt, 0, sizeof(sendpkt));
		sendpkt.content.Opcode = reverse_HL(_Opcode);
		sendpkt.content.payload.Data.pktnum = reverse_HL(_pktnum);
		_file.read(sendpkt.content.payload.Data.data, slicelenth);
		if (*filelenth >= slicelenth) //����filelenth,����pktlenth
		{
			*filelenth -= slicelenth;
			sendpkt.lenth = 4 + slicelenth;
			return false;
		}
		sendpkt.lenth = 4 + *filelenth;
		return true;
	}

	//�������ձ���
	t_status check_recvpkt(msg_kind Opcode, __int16 pktnum);

	//���ȼ��������������
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

	//������������
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

//TFTP �ļ���
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
		//RRQ���ķ���ACK������

		//��������RRQ����
		Create_Pkt(msg_RRQ, file_name, ts_mode);

		trysendto();

		//�����1��ʼ��DATA
		while (true)
		{
			//���Խ���DATA
			timer = timeGetTime(); //������ʱ��
			f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); });

			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					//��ʱ�ش�
					tftp_timeout = tftp_timeout * 2;
					TFTP_INFO("ACK�������ش�������ش��ɹ���ע�ⳬʱʱ�������", TFTP_INFO::TIMEOUT, __LINE__, __func__);
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " �ﵽ����ش�����" << endl;
						TFTP_INFO("ACK�ﵽ����ش�����", TFTP_INFO::ACK_REACH_MAX_RETRYTIMES, __LINE__, __func__);
					}
					trysendto();
					timer = timeGetTime();
				}

				//̽��recvfrom�߳�״̬
				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(f_detect_gap));
				if (f_recvfrom_status == future_status::ready)
				{
					//�����հ�
					if (recvpkt.lenth != -1 && check_recvpkt(msg_DATA, pktnum) == ok) //tryrecvfrom�߳̽���,�հ���ȷ
					{
						tftp_timeout = default_timeout;
						break;
					}
					f_recvfrom.wait();
					f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); }); //tryrecvfrom�߳̽���,�հ�������������
				}
			}
			f_recvfrom.wait();
			transLenth += recvpkt.lenth;
			if (recvpkt.lenth - 4 < 512)
				done = true;

			//��ӡ������Ϣ
			print_buf = "�ɹ�!\t\t\t\t\rpktnum=" + to_string(static_cast<uint16_t>(pktnum)) + "ȷ�Ͻ���:" + to_string(
				recvpkt.lenth - 4) + "�ֽ�����....";
			print_thread.set_msg(print_buf);
			//printf("�ɹ���\t\rpktnum=%dȷ�Ͻ���:%d�ֽ�����....", (uint16_t)pktnum, recvpkt.lenth - 4);

			file.write(recvpkt.content.payload.Data.data, recvpkt.lenth - 4);

			//���췢��ACK
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

	string user_msg("�����һ���ļ��Ļ�ȡ,����ʱ:");
	user_msg += to_string(static_cast<double>(clock() - transTime) / CLOCKS_PER_SEC);
	user_msg += "s��ȡ��:" + to_string(transLenth);
	user_msg += "bytes";
	user_msg += "ƽ��������Ϊ:";
	user_msg += cal_rate(transLenth, static_cast<double>(clock() - transTime) / CLOCKS_PER_SEC);
	cout << endl << user_msg << endl;
	TFTP_INFO(user_msg.c_str(), static_cast<TFTP_INFO::TFTP_INFO_TYPE>(0), __LINE__, __func__);
	//print_thread.~TFTP_Print();		��������������⣬��֪��Ϊʲô
	return ok;
}

//TFTP �ļ�д
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

	//�����ļ��ܴ�С
	file.seekg(0, std::ios::end);
	filelenth = file.tellg();
	file.seekg(0, std::ios::beg);
	try
	{
		//WRQ���ķ���ACK������

		//����WRQ����
		Create_Pkt(msg_WRQ, file_name, ts_mode);
		trysendto();

		timer = timeGetTime(); //������ʱ��
		f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); });

		//���Խ���ACK
		for (int retrytimes = 0; retrytimes <= max_retrytimes;)
		{
			if (timeGetTime() - timer >= tftp_timeout)
			{
				tftp_timeout = tftp_timeout * 2;
				TFTP_INFO("WRQ�����ش�������ش��ɹ���ע�ⳬʱʱ�������", TFTP_INFO::TIMEOUT, __LINE__, __func__);
				if (++retrytimes > max_retrytimes)
				{
					cout << "LINE:" << __LINE__ << " �ﵽ����ش�����" << endl;
					TFTP_INFO("WRQ�ﵽ����ش�����", TFTP_INFO::REQUEST_REACH_MAX_RETRYTIMES, __LINE__, __func__);
				}
				trysendto();
				timer = timeGetTime();
			}

			//̽��recvfrom�߳�״̬
			f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
			if (f_recvfrom_status == future_status::ready)
			{
				if (recvpkt.lenth != -1 && check_recvpkt(msg_ACK, pktnum) == ok) //tryrecvfrom�߳̽���,�հ���ȷ
				{
					tftp_timeout = default_timeout;
					break;
				}
				f_recvfrom.wait();
				f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); }); //tryrecvfrom�߳̽���,�հ�������������
			}
		}
		f_recvfrom.wait();
		pktnum++;

		//�����1��ʼ��DATA
		while (true)
		{
			//���췢��DATA��
			done = Create_Pkt(msg_DATA, pktnum, file, &filelenth);
			transLenth += sendpkt.lenth;

			trysendto();

			//��ӡ������Ϣ
			print_buf = "�ɹ�!" + cal_process(transLenth, filelenth) + "\t\t\t\t\rpktnum=" +
				to_string(static_cast<uint16_t>(pktnum)) + "���ڷ���:" + to_string(sendpkt.lenth - 4) + "�ֽ�����....";
			print_thread.set_msg(print_buf);
			//printf("�ɹ�! %s\t\rpktnum=%d���ڷ���:%d�ֽ�����....", cal_process(transLenth, filelenth).c_str(), (uint16_t)pktnum, sendpkt.lenth - 4);

			//���Խ���ACK
			timer = timeGetTime(); //������ʱ��
			f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); });

			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					//��ʱ�ش�
					tftp_timeout = tftp_timeout * 2;
					TFTP_INFO("DATA�����ش�������ش��ɹ���ע�ⳬʱʱ�������", TFTP_INFO::TIMEOUT, __LINE__, __func__);
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " �ﵽ����ش�����" << endl;
						TFTP_INFO("DATA�ﵽ����ش�����", TFTP_INFO::DATA_REACH_MAX_RETRYTIMES, __LINE__, __func__);
					}
					trysendto();
					timer = timeGetTime();
				}

				//̽��recvfrom�߳�״̬
				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(f_detect_gap));
				if (f_recvfrom_status == future_status::ready)
				{
					//�����հ�
					if (recvpkt.lenth != -1 && check_recvpkt(msg_ACK, pktnum) == ok) //tryrecvfrom�߳̽���,�հ���ȷ
					{
						tftp_timeout = default_timeout;
						break;
					}
					f_recvfrom.wait();
					f_recvfrom = async(launch::async, [&]() { tryrecvfrom(); }); //tryrecvfrom�߳̽���,�հ�������������
				}
			}
			//�յ�ACK
			f_recvfrom.wait();
			pktnum++;
			if (done)
				break;
		}
	}
	catch (TFTP_INFO& Error) //�ӹ��쳣
	{
		//print_thread.~TFTP_Print();
		throw;
	}

	string user_msg("�����һ���ļ��ķ���,����ʱ:");
	user_msg += to_string(static_cast<double>(clock() - transTime) / CLOCKS_PER_SEC);
	user_msg += "s������:" + to_string(transLenth);
	user_msg += "bytes";
	user_msg += "ƽ��������Ϊ:";
	user_msg += cal_rate(transLenth, static_cast<double>(clock() - transTime) / CLOCKS_PER_SEC);
	cout << endl << user_msg << endl;
	TFTP_INFO(user_msg.c_str(), static_cast<TFTP_INFO::TFTP_INFO_TYPE>(0), __LINE__, __func__);
	//print_thread.~TFTP_Print();		��������������⣬��֪��Ϊʲô
	return ok;
}

//TFTP_msg���ͺ���ʵ��
inline void TFTP_msg::trysendto()
{
	if (sendto(client, (const char*)&sendpkt.content, sendpkt.lenth, 0, (struct sockaddr*)server_addr,
	           sizeof(*server_addr)) < 0)
		TFTP_INFO("socket sendto��������-1", TFTP_INFO::SENDTO_FAILED, __LINE__, __func__);
}

//TFTP_msg���պ���ʵ��
inline void TFTP_msg::tryrecvfrom()
{
	timeval tv = { 2, 0 };
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(timeval));
	//����socket��Ϊ�������tv��ʱʱ���û���յ��������recvfrom�˳�����״̬����10062
	if ((recvpkt.lenth = recvfrom(client, (char*)&recvpkt.content, sizeof(recvpkt.content), 0,
		reinterpret_cast<struct sockaddr*>(server_addr), &server_addr_length)) == -1)
		TFTP_INFO("socket recvfrom��������-1", TFTP_INFO::RECVFROM_FAILED, __LINE__, __func__);
}

//TFTP_msg������ʵ��
inline void TFTP_msg::Create_Pkt(short _Opcode, const char* _file_name, const char* _ts_mode)
{
	memset(&sendpkt, 0, sizeof(sendpkt));
	sendpkt.content.Opcode = reverse_HL(_Opcode);
	strcpy(sendpkt.content.payload.RQ.filename_tsmode, _file_name);
	strcpy(sendpkt.content.payload.RQ.filename_tsmode + strlen(_file_name) + 1, _ts_mode);
	sendpkt.lenth = 2 + strlen(_file_name) + strlen(_ts_mode) + 2;
}

//TFTP_msg���ռ�麯��ʵ��
inline t_status TFTP_msg::check_recvpkt(msg_kind Opcode, short pktnum)
{
	if (recvpkt.content.Opcode == reverse_HL(Opcode) &&
		recvpkt.content.payload.ACK.pktnum == reverse_HL(pktnum)) //�˴�ACK��DATA��pktnum�ȼ�
		return ok;
	if (recvpkt.content.Opcode == reverse_HL(msg_ERROR))
	{
		string user_msg("�յ�Opcode=ERROR:");
		user_msg += recvpkt.content.payload.Error.detail;
		cout << user_msg << endl;
		TFTP_INFO(user_msg.c_str(), TFTP_INFO::RECVED_ERROR_PACKET, __LINE__, __func__);
		return wrongstep;
	}
	cout << "�յ�����֮��ķ���!";
	TFTP_INFO("�յ�����֮��ķ���", TFTP_INFO::RECVED_UNEXPECTED_PACKET, __LINE__, __func__);
	return wrongstep;
}
#endif // TFTP_CLIENT_HPP
