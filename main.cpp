#include "TFTP_client.hpp"

#undef server_ip
#undef server_port
#undef slicelenth

#define server_ip "10.10.10.10"
#define server_port 69
#define slicelenth 512

int menu(char*);

int main()
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)	//初始化Winsock2.2库
	{
		cout << "WSAStartup初始化失败" << endl;
		return -1;
	}

	SOCKET client;						//套接字
	struct sockaddr_in server_addr;
	memset(&server_addr,0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);
	if ((client = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		cout << "套接字创建失败" << endl;
		return -1;
	}

	/*TFTP_msg testread(client,&server_addr,msg_RRQ);
	testread.TFTP_readfile("a.txt", "netascii");*/
	
	char file_name[128];

	if (menu(file_name) == 1)
	{
		ifstream local_file("D:\\VisualStudio\\C++PRO\\NN-01\\a.txt", ios::in);
		if (!local_file.is_open()) {
			cout << "无法打开文件" << std::endl;
			return -1;
		}

		TFTP_msg testwrite(client, &server_addr, msg_WRQ);
		if(testwrite.TFTP_writefile("a.txt", "netascii", local_file)==ok)
			cout<<"向服务器写成功！"<<endl;
		local_file.close();
	}
	else
	{
		ofstream local_file("D:\\VisualStudio\\C++PRO\\NN-01\\b.txt", ios::in);
		if (!local_file.is_open()) {
			cout << "无法打开文件" << std::endl;
			return -1;
		}

		TFTP_msg testread(client, &server_addr, msg_RRQ);
		if (testread.TFTP_readfile("a.txt", "netascii", local_file) == ok)
			cout << "向服务器读成功！" << endl;
		local_file.close();
	}

	
	closesocket(client);
	WSACleanup();
	return 0;
}

int menu(char *file_name)
{
	return 1;
}