#include "TFTP_client.hpp"

#undef server_ip
#undef server_port
#undef slicelenth

#define server_ip "127.0.0.1"
#define server_port 69
#define slicelenth 512

const char log_file_name[] = "D:\\VisualStudio\\C++PRO\\NN-01\\tftpclient.log";
ofstream TFTP_Log::log_file(log_file_name, ios::out | ios::app);

int menu(char*);

int main()
{
	//测试日志文件可用性
	TFTP_Log test_log (TFTP_Log::create);
	//程序启动和初始化操作
	TFTP_INFO("程序启动",(TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__,__func__);
	WSADATA wsa_data;
	SOCKET client;						//套接字
	struct sockaddr_in server_addr;		//服务器地址结构体
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);
		//初始化WinSOCK和套接字
	try
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)	//初始化Winsock2.2库
		{
			cout << "WSAStartup初始化失败" << endl;
			TFTP_INFO("WSAStartup初始化失败", TFTP_INFO::WSA_INIT_FAILED, __LINE__, __func__);
		}


		if ((client = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			cout << "套接字创建失败" << endl;
			TFTP_INFO("套接字创建失败", TFTP_INFO::SOCKET_CREATE_FAILED, __LINE__, __func__);
		}
	}
	catch (TFTP_INFO& TFTP_error)
	{
		TFTP_error.Create_Log();
		exit(-1);	//初始化失败直接退出
	}

	//选择操作开始运行
	char file_name[128];
	start:
	if (menu(file_name) == 1)
	{
		//向服务器写文件Write
		ifstream local_file("D:\\VisualStudio\\C++PRO\\NN-01\\a.txt", ios::in);
		try 
		{
			if (!local_file.is_open()) {
				cout << "无法打开本地文件" << std::endl;
				TFTP_INFO("无法打开本地文件", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
			}
			TFTP_msg testwrite(client, &server_addr, msg_WRQ);
			if (testwrite.TFTP_writefile("a.txt", "netascii", local_file) == ok)
				cout << "向服务器写成功！" << endl;
			local_file.close();
		}
		catch (TFTP_INFO& TFTP_error)
		{
			TFTP_error.Create_Log();
			local_file.close();
			goto start;
		}
	}
	else
	{
		//向服务器读文件Read
		const char local_file_name[] = "D:\\VisualStudio\\C++PRO\\NN-01\\b.txt";
		ifstream check_file(local_file_name, ios::in);
		if (check_file.good())
		{
			// 如果文件存在
			char choice;
			cout << "文件已存在。是否重新创建文件 (y/n)? ";
			cin >> choice;

			if (choice == 'y' || choice == 'Y') {
				check_file.close();
				remove(local_file_name);  // 删除文件
			}
			else {
				cout << "未重新创建文件。" << endl;
				check_file.close();
				return 0;
			}
		}
		ofstream local_file(local_file_name, ios::out);
		try
		{
			if (!local_file.is_open()) {
				TFTP_INFO("无法创建或打开文件", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
			}

			TFTP_msg testread(client, &server_addr, msg_RRQ);
			if (testread.TFTP_readfile("a.txt", "netascii", local_file) == ok)
				cout << "向服务器读成功！" << endl;
			local_file.close();
		}
		catch (TFTP_INFO& TFTP_error)
		{
			TFTP_error.Create_Log();
			local_file.close();
			goto start;
		}
		
	}
	
	closesocket(client);
	WSACleanup();
	TFTP_INFO("程序正常结束", (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
	return 0;
}

int menu(char *file_name)
{
	int choice = 0;
	cout << "WRITE OR READ(1/2)";
	cin >> choice;
	return choice;
}