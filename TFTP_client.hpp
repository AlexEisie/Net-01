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

long long tftp_timeout = default_timeout;  //������ʱ*2,���ճɹ��ָ�

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

		//RRQ���ķ���ACK������

		//����RRQ����
		strcpy(sendpkt.data + 2, file_name);
		strcpy(sendpkt.data + 2 + strlen(file_name) + 1, ts_mode);
		sendpkt.lenth = 2 + strlen(file_name) + strlen(ts_mode) + 2;
		
		f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
		f_sendto.wait();
		
		//�����1��ʼ��DATA
		while (1)
		{

			//���Խ���DATA
			timer = timeGetTime();	//������ʱ��
			f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });

			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					tftp_timeout = tftp_timeout * 2;
					TFTP_INFO("ACK�������ش�������ش��ɹ���ע�ⳬʱʱ�������", TFTP_INFO::TIMEOUT, __LINE__, __func__);
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " �ﵽ����ش�����" << endl;
						TFTP_INFO("ACK�ﵽ����ش�����", TFTP_INFO::ACK_REACH_MAX_RETRYTIMES, __LINE__, __func__);
					}
					f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
					f_sendto.wait();
					timer = timeGetTime();
				}

				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
				if (f_recvfrom_status == future_status::ready)
				{
					if (recvpkt.lenth != -1 && check_recvptk(msg_DATA, pktnum) == tftp_pkt_ok)		//tryrecvfrom�߳̽���,�հ���ȷ
					{
						tftp_timeout =default_timeout;
						break;
					}
					else
					{
						f_recvfrom.wait();
						f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });		//tryrecvfrom�߳̽���,�հ�������������
					}
				}
			}
			f_recvfrom.wait();
			transLenth += recvpkt.lenth;
			if (recvpkt.lenth - 4 < 512)
				done = 1;
			cout << "\rpktnum=" << (uint16_t)pktnum << "ȷ�Ͻ���:" << recvpkt.lenth - 4 << "�ֽ�����...."<<flush;
			file.write(recvpkt.data+ 4, recvpkt.lenth - 4);
			
			//���췢��ACK
			memset(&sendpkt, 0, sizeof(sendpkt));
			*((__int8*)sendpkt.data + 1) = (__int8)msg_ACK;
			*(__int8*)(sendpkt.data + 2) = *((__int8*)&pktnum + 1);		//�ߵͶ�ת��
			*(__int8*)(sendpkt.data + 3) = *(__int8*)&pktnum;
			sendpkt.lenth = 4;

			f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
			f_sendto.wait();
			cout << "�ɹ���        " << flush;

			pktnum++;
			if (done)
				break;
		}
		string user_msg("�����һ���ļ��Ļ�ȡ,����ʱ:");
				user_msg += to_string((double)(clock() - transTime) / CLOCKS_PER_SEC);
				user_msg += "s��ȡ��:" + to_string(transLenth);
				user_msg += "bytes";
				user_msg += "ƽ��������Ϊ:";
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

		//�����ļ��ܴ�С
		file.seekg(0, std::ios::end);
		int filelenth = file.tellg();
		file.seekg(0, std::ios::beg);

		//WRQ���ķ���ACK������

		//����WRQ����
		strcpy(sendpkt.data + 2, file_name);
		strcpy(sendpkt.data + 2 + strlen(file_name) + 1, ts_mode);
		sendpkt.lenth = 2 + strlen(file_name) + strlen(ts_mode) + 2;
		
		f_sendto = async(launch::async, [spktlenth,this]() {trysendto(spktlenth);});
		f_sendto.wait();
		
		timer = timeGetTime();	//������ʱ��
		f_recvfrom = async(launch::async, [rpktlenth,this]() {tryrecvfrom(rpktlenth);});

		for(int retrytimes=0;retrytimes<=max_retrytimes;)
		{
			if (timeGetTime() - timer >= tftp_timeout)
			{
				tftp_timeout = tftp_timeout * 2;
				TFTP_INFO("WRQ�����ش�������ش��ɹ���ע�ⳬʱʱ�������", TFTP_INFO::TIMEOUT, __LINE__, __func__);
				if (++retrytimes > max_retrytimes)
				{
					cout <<"LINE:" << __LINE__ << " �ﵽ����ش�����" << endl;
					TFTP_INFO("WRQ�ﵽ����ش�����", TFTP_INFO::REQUEST_REACH_MAX_RETRYTIMES, __LINE__, __func__);
				}
				f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
				f_sendto.wait();
				timer = timeGetTime();
			}
			f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
			if (f_recvfrom_status == future_status::ready)
			{
				if (recvpkt.lenth != -1 && check_recvptk(msg_ACK, pktnum) == tftp_pkt_ok)		//tryrecvfrom�߳̽���,�հ���ȷ
				{
					tftp_timeout = default_timeout;
					break;
				}
				else
				{
					f_recvfrom.wait();
					f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });		//tryrecvfrom�߳̽���,�հ�������������
				}
			}
;		}
		f_recvfrom.wait();
		pktnum++;

		//�����1��ʼ��DATA
		while (1)
		{
			//���췢��DATA��
			memset(&sendpkt, 0, sizeof(sendpkt));
			*(__int8*)(sendpkt.data + 1) = (__int8)msg_DATA;
			*(__int8*)(sendpkt.data + 2) = *((__int8*)&pktnum + 1);		//�ߵͶ�ת��
			*(__int8*)(sendpkt.data + 3) = *(__int8*)&pktnum;
			file.read(sendpkt.data + 4, slicelenth);
			if (filelenth >= slicelenth)		//����filelenth,����pktlenth
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
			cout << "\rpktnum=" << (uint16_t)pktnum<< "���ڷ���:" << sendpkt.lenth - 4 << "�ֽ�����...."<<flush;

			//���Խ���ACK
			timer = timeGetTime();	//������ʱ��
			f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });

			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					tftp_timeout = tftp_timeout * 2;
					TFTP_INFO("DATA�����ش�������ش��ɹ���ע�ⳬʱʱ�������", TFTP_INFO::TIMEOUT, __LINE__, __func__);
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " �ﵽ����ش�����" << endl;
						TFTP_INFO("DATA�ﵽ����ش�����", TFTP_INFO::DATA_REACH_MAX_RETRYTIMES, __LINE__, __func__);
					}
					f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
					f_sendto.wait();
					timer = timeGetTime();
				}

				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
				if (f_recvfrom_status == future_status::ready)
				{
					if (recvpkt.lenth != -1 && check_recvptk(msg_ACK, pktnum) == tftp_pkt_ok)		//tryrecvfrom�߳̽���,�հ���ȷ
					{
						tftp_timeout = default_timeout;
						break;
					}
					else
					{
						f_recvfrom.wait();
						f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });		//tryrecvfrom�߳̽���,�հ�������������
					}
				}
			}
			f_recvfrom.wait();
			cout << "�ɹ���             " << cal_process(transLenth,filelenth) <<flush;
			pktnum++;

			if (done)
				break;
		}
		string user_msg("�����һ���ļ��ķ���,����ʱ:");
				user_msg += to_string((double)(clock() - transTime) / CLOCKS_PER_SEC);
				user_msg += "s������:" + to_string(transLenth);
				user_msg += "bytes";
				user_msg += "ƽ��������Ϊ:";
				user_msg += cal_rate(transLenth, (double)(clock() - transTime) / CLOCKS_PER_SEC);
		cout << endl <<user_msg<<endl;
		TFTP_INFO(user_msg.c_str(),(TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
		return ok;
	}
private:
	//���Խ���sendto
	void trysendto(int *pktlenth)
	{
		if (sendto(client, sendpkt.data, *pktlenth, 0, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
			TFTP_INFO("socket sendto��������-1", TFTP_INFO::SENDTO_FAILED, __LINE__, __func__);
	}
	
	//���Խ���recvfrom���߳�1s���Զ�����
	void tryrecvfrom(int* pktlenth)
	{
		timeval tv = { 1, 0};
		setsockopt(client, SOL_SOCKET,SO_RCVTIMEO, (char*)&tv, sizeof(timeval));	//����socket��Ϊ�������tv��ʱʱ���û���յ��������recvfrom�˳�����״̬����10062
		if ((*pktlenth = recvfrom(client, recvpkt.data, sizeof(recvpkt.data), 0, (struct sockaddr*)server_addr, &server_addr_length)) == -1)
			TFTP_INFO("socket recvfrom��������-1", TFTP_INFO::RECVFROM_FAILED, __LINE__, __func__);
	}

	//������Opcode��pktnum
	tftp_status check_recvptk(msg_kind Opcode, __int16 pktnum)
	{
		if (*(__int8*)(recvpkt.data + 1) == (__int8)Opcode
			&& *(__int8*)(recvpkt.data + 2) == *((__int8*)&pktnum + 1)
			&&*(__int8*)(recvpkt.data + 3) == *(__int8*)&pktnum)
			return tftp_pkt_ok;
		else if (*(__int8*)(recvpkt.data + 1) == (__int8)msg_ERROR)
		{
			string user_msg("�յ�Opcode=ERROR:");
			user_msg += recvpkt.data + 4;
			cout << user_msg << endl;
			TFTP_INFO(user_msg.c_str(), TFTP_INFO::RECVED_ERROR_PACKET, __LINE__, __func__);
			return tftp_err_get_error_Opcode;
		}
		else
		{
			cout <<"�յ�����֮��ķ���"<<endl;
			TFTP_INFO("�յ�����֮��ķ���", TFTP_INFO::RECVED_UNEXPECTED_PACKET, __LINE__, __func__);
			return tftp_err_not_expected_pkt;
		}
	}
	//���ȼ��������������
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
	//������������
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
