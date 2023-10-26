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
#define slicelenth 512	//�ļ���Ƭ��С������512����Ȼ�������
#define tftp_timeout 500
#define max_retrytimes 3
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

					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " �ﵽ����ش�����" << endl;
						exit(tftp_err_max_retrytimes);
					}
					f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
					f_sendto.wait();
					timer = timeGetTime();
				}

				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
				if (f_recvfrom_status == future_status::ready)
				{
					if (recvpkt.lenth != -1 && check_recvptk(msg_DATA, pktnum) == tftp_pkt_ok)		//tryrecvfrom�߳̽���,�հ���ȷ
						break;
					else
					{
						cout << "wrong packet!" << endl;
						f_recvfrom.wait();
						f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });		//tryrecvfrom�߳̽���,�հ�������������
					}
				}
			}
			f_recvfrom.wait();
			if (recvpkt.lenth - 4 < 512)
				done = 1;
			cout << "pkt=" << pktnum << "ȷ�Ͻ���:" << recvpkt.lenth - 4 << "�ֽ�����....";
			file.write(recvpkt.data+ 4, recvpkt.lenth - 4);
			
			//���췢��ACK
			memset(&sendpkt, 0, sizeof(sendpkt));
			*((__int8*)sendpkt.data + 1) = (__int8)msg_ACK;
			*(__int8*)(sendpkt.data + 2) = *((__int8*)&pktnum + 1);		//�ߵͶ�ת��
			*(__int8*)(sendpkt.data + 3) = *(__int8*)&pktnum;
			sendpkt.lenth = 4;

			f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
			f_sendto.wait();
			cout << "�ɹ���" << endl;

			pktnum++;
			if (done)
				break;
		}

		return ok;
	}

	t_status TFTP_writefile(const char* file_name, const char* ts_mode, ifstream& file)
	{
		int filelength = 0;
		__int16 pktnum = 0;
		bool done = 0;
		__int64 timer;

		int* spktlenth = &sendpkt.lenth;
		int* rpktlenth = &recvpkt.lenth;
		future<void> f_sendto;
		future<void> f_recvfrom;
		future_status f_sendto_status;
		future_status f_recvfrom_status;

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

				if (++retrytimes > max_retrytimes)
				{
					cout <<"LINE:" << __LINE__ << " �ﵽ����ش�����" << endl;
					exit(tftp_err_max_retrytimes);
				}
				f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
				f_sendto.wait();
				timer = timeGetTime();
			}
			f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
			if (f_recvfrom_status == future_status::ready)
			{
				if (recvpkt.lenth != -1 && check_recvptk(msg_ACK, pktnum) == tftp_pkt_ok)		//tryrecvfrom�߳̽���,�հ���ȷ
					break;
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
			
			f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
			f_sendto.wait();
			cout << "pktnum=" <<pktnum<< "���ڷ���:" << sendpkt.lenth - 4 << "�ֽ�����....";

			//���Խ���ACK
			timer = timeGetTime();	//������ʱ��
			f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });

			for (int retrytimes = 0; retrytimes <= max_retrytimes;)
			{
				if (timeGetTime() - timer >= tftp_timeout)
				{
					if (++retrytimes > max_retrytimes)
					{
						cout << "LINE:" << __LINE__ << " �ﵽ����ش�����" << endl;
						exit(tftp_err_max_retrytimes);
					}
					f_sendto = async(launch::async, [spktlenth, this]() {trysendto(spktlenth); });
					f_sendto.wait();
					timer = timeGetTime();
				}

				f_recvfrom_status = f_recvfrom.wait_for(chrono::microseconds(0));
				if (f_recvfrom_status == future_status::ready)
				{
					if (recvpkt.lenth != -1 && check_recvptk(msg_ACK, pktnum) == tftp_pkt_ok)		//tryrecvfrom�߳̽���,�հ���ȷ
						break;
					else
					{
						f_recvfrom.wait();
						f_recvfrom = async(launch::async, [rpktlenth, this]() {tryrecvfrom(rpktlenth); });		//tryrecvfrom�߳̽���,�հ�������������
					}
				}
			}
			f_recvfrom.wait();
			cout << "�ɹ���" << endl;
			pktnum++;

			if (done)
				break;
		}
		return ok;
	}
private:
	t_status trysendto(int *pktlenth)
	{
		if (sendto(client, sendpkt.data, *pktlenth, 0, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
			return sendwrong;
		return ok;
	}

	t_status tryrecvfrom(int* pktlenth)
	{
		if ((*pktlenth = recvfrom(client, recvpkt.data, sizeof(recvpkt.data), 0, (struct sockaddr*)server_addr, &server_addr_length)) == -1)
			return recvwrong;
		return ok;
	}

	tftp_status check_recvptk(msg_kind Opcode, __int16 pktnum)
	{
		if (*(__int8*)(recvpkt.data + 1) == (__int8)Opcode
			&& *(__int8*)(recvpkt.data + 2) == *((__int8*)&pktnum + 1)
			&&*(__int8*)(recvpkt.data + 3) == *(__int8*)&pktnum)
			return tftp_pkt_ok;
		else if (*(__int8*)(recvpkt.data + 1) == (__int8)msg_ERROR)
		{
			cout << "�յ�Opcode=ERROR:" << recvpkt.data + 4 << endl;
			exit(tftp_err_get_error_Opcode);
			return tftp_err_get_error_Opcode;
		}
		else
			return tftp_err_not_expected_pkt;
	}
};
