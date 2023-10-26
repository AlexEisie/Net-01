//#include<iostream>
//#include<cstring>
//#include<winsock2.h>
//#include<Windows.h>
//#pragma comment(lib,"ws2_32.lib")
//#pragma warning(disable : 4996)
//
//#define server_ip "127.0.0.1"
//#define server_port 69
//using namespace std;
//
//enum msg_kind
//{
//	msg_RRQ = 1,
//	msg_WRQ = 2,
//	msg_DATA = 3,
//	msg_ACK = 4,
//	msg_ERROR = 5
//};
//
//enum t_status
//{
//	sendwrong = -1,
//	recvwrong = -1,
//	wrongstep = -1,
//	ok=0
//	
//};
//
//class TFTP_msg
//{
//public:
//	char sendbuf[1200];
//	char recvbuf[1200];
//	SOCKET client;
//	struct sockaddr_in *server_addr;
//	int server_addr_length = sizeof(server_addr);
//
//	TFTP_msg(SOCKET c, struct sockaddr_in *s,msg_kind Opcode)
//	{
//		memset(sendbuf, 0, sizeof(sendbuf));
//		memset(recvbuf, 0, sizeof(recvbuf));
//		*(__int16 *)sendbuf = Opcode;
//		client = c;
//		server_addr = s;
//	}
//
//	t_status TFTP_readfile(const char *file_name,const char *ts_mode)
//	{
//		int sendlen = 0;
//		int recvlen = 0;
//		__int16 block_num = 0;
//		strcpy(sendbuf + 2, file_name);
//		strcpy(sendbuf + 2 + strlen(file_name) + 1, ts_mode);
//		if (sendto(client, sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
//			return sendwrong;
//
//		while (1)
//		{
//			if ((recvlen=recvfrom(client, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&server_addr, &server_addr_length)) == -1)
//				return recvwrong;
//			else
//				if (*(__int16*)recvbuf != (__int16)msg_DATA)
//					return wrongstep;
//			block_num = *(__int16 *)(recvbuf + 2);
//			cout << recvbuf + 4;
//
//			memset(sendbuf, 0, sizeof(sendbuf));
//			*(__int16*)sendbuf = msg_ACK;
//			*(__int16*)(sendbuf+2) = block_num;
//			if (sendto(client, sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
//				return sendwrong;
//
//			if (recvlen < 512)
//			{
//				cout << "file\""<<file_name<<"\" received!" << endl;
//				break;
//			}
//		}
//		
//		return ok;
//	}
//
//	t_status TFTP_writefile(const char* file_name, const char* ts_mode)
//	{
//		int sendlen = 0;
//		int recvlen = 0;
//		__int16 block_num = 0;
//		strcpy(sendbuf + 2, file_name);
//		strcpy(sendbuf + 2 + strlen(file_name) + 1, ts_mode);
//		if (sendto(client, sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
//			return sendwrong;
//		if ((recvlen = recvfrom(client, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&server_addr, &server_addr_length)) == -1)
//			return recvwrong;
//		else
//			if (*(__int16*)recvbuf != (__int16)msg_ACK)
//				return wrongstep;
//
//		strcpy(sendbuf, "666666666");
//		while (1)
//		{
//			if (sendto(client, sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
//				return sendwrong;
//
//			if ((recvlen = recvfrom(client, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&server_addr, &server_addr_length)) == -1)
//				return recvwrong;
//			else
//				if (*(__int16*)recvbuf != (__int16)msg_ACK)
//					return wrongstep;
//
//			if (1)
//				break;
//		}
//		return ok;
//	}
//};
//
//int main()
//{
//	WSADATA wsa_data;
//	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)	//初始化Winsock2.2库
//	{
//		cout << "WSAStartup初始化失败" << endl;
//		return -1;
//	}
//
//	SOCKET client;						//套接字
//	struct sockaddr_in server_addr;
//	memset(&server_addr,0, sizeof(server_addr));
//	server_addr.sin_family = AF_INET;
//	server_addr.sin_addr.s_addr = inet_addr(server_ip);
//	server_addr.sin_port = htons(server_port);
//	if ((client = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
//	{
//		cout << "套接字创建失败" << endl;
//		return -1;
//	}
//
//	TFTP_msg testread(client,&server_addr,msg_RRQ);
//	testread.TFTP_readfile("a.txt", "netascii");
//
//	TFTP_msg testwrite(client, &server_addr, msg_RRQ);
//	testwrite.TFTP_readfile("a.txt", "netascii");
//
//	closesocket(client);
//	WSACleanup();
//	return 0;
//}
//
