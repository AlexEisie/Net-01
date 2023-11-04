#include <direct.h>
#include "TFTP_client.hpp"
#include "TFTP_menu.hpp"

//服务器监听配置
char server_ip[64] = "127.0.0.1";
unsigned short server_port = 69;
//客户端参数配置
long slicelenth = 512;		//文件分片大小必须是512，不然不会接收
long long default_timeout = 500;
int max_retrytimes=3;
//客户端生成参数
WSADATA wsa_data;
SOCKET client;
struct sockaddr_in server_addr;
//文件传输设置
char local_file_name[128];
char remote_file_name[128];
char ts_mode[32]= "octet";
//日志设置
const char log_file_name[] = "tftpclient.log";
ofstream TFTP_Log::log_file(log_file_name, ios::out | ios::app);

void Net_Init();
void Net_Clear();

int main()
{
	//测试日志文件可用性
	TFTP_Log test_log (TFTP_Log::create);
	//程序启动
	TFTP_INFO("程序启动",(TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__,__func__);

	while (true)
	{
		Net_Init();
		TFTP_Menu menu;
		menu.MainMenu();
		switch (menu.choose())
		{
		case TFTP_Menu::MenuChoice::SERVER_CONFIG:
			menu.SubMenu_Server_Config(server_ip,&server_port);
			Net_Clear();
			system("pause");
			break;
		case TFTP_Menu::MenuChoice::WRITEFILE:
		{
			menu.SubMenu_File_Config(local_file_name, remote_file_name,ts_mode);
			//向服务器写文件Write
			//启动写操作
			ifstream local_file(local_file_name, ios::in | ios::binary);	//这里加入binary模式是因为如果默认使用文本格式会丢弃\n
			try
			{
				if (!local_file.is_open()) {
					cout << "无法打开本地文件" << std::endl;
					TFTP_INFO("无法打开本地文件", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
				}
				TFTP_msg TFTPwrite(client, &server_addr, msg_WRQ);
				if (TFTPwrite.TFTP_writefile(remote_file_name, ts_mode, local_file) == ok)
					cout << "向服务器写成功！" << endl;
				local_file.close();
			}
			catch (TFTP_INFO& TFTP_error)
			{
				TFTP_error.Create_Log();
				local_file.close();
				Net_Clear();
				system("pause");
				break;
			}
			Net_Clear();
			system("pause");
			break;
		}
		case TFTP_Menu::MenuChoice::READFILE:
		{
			menu.SubMenu_File_Config(local_file_name, remote_file_name, ts_mode);
			//向服务器读文件Read

			//检查本地文件
			ifstream check_file(local_file_name, ios::in | ios::binary);
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
					Net_Clear();
					system("pause");
					break;
				}
			}
			//启动读操作
			ofstream local_file(local_file_name, ios::out | ios::binary);
			try
			{
				if (!local_file.is_open()) {
					TFTP_INFO("无法创建或打开文件", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
				}

				TFTP_msg TFTPread(client, &server_addr, msg_RRQ);
				if (TFTPread.TFTP_readfile(remote_file_name, "octet", local_file) == ok)
					cout << "向服务器读成功！" << endl;
				local_file.close();
			}
			catch (TFTP_INFO& TFTP_error)
			{
				TFTP_error.Create_Log();
				local_file.close();
				Net_Clear();
				system("pause");
				break;
			}
			Net_Clear();
			system("pause");
			break;
		}
		case TFTP_Menu::MenuChoice::GET_LOG_LOCATION:
		{
			system("cls");
			const int MAXPATH = 250;
			char local_path[MAXPATH];
			getcwd(local_path, MAXPATH);
			cout << local_path<<"\\"<<log_file_name << endl;
			Net_Clear();
			system("pause");
			break;
		}
		case TFTP_Menu::MenuChoice::EXITPRO:
		{
			Net_Clear();
			TFTP_INFO("程序正常结束", (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
			return 0;
		}
		case TFTP_Menu::MenuChoice::CLIENT_CONFIG:
		{
			menu.SubMenu_Client_Config(&slicelenth, &default_timeout, &max_retrytimes);
			Net_Clear();
			system("pause");
			break;
		}
		case TFTP_Menu::MenuChoice::GET_HELP:
		default:
			Net_Clear();
			break;
		}
	}
	return 0;
}

//初始化操作
void Net_Init()
{
	//初始化服务端地址
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
}

void Net_Clear()
{
	closesocket(client);
	WSACleanup();
}