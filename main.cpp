#include <direct.h>
#include "TFTP_client.hpp"
#include "TFTP_menu.hpp"

//服务器监听配置
char server_ip[64] = "127.0.0.1";
unsigned short server_port = 69;
//客户端参数配置
long slicelenth = 512;		//文件分片大小必须是512，不然不会接收
long long default_timeout = 500;
int max_retrytimes=10;
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

	//选择主循环
	while (true)
	{
		Net_Init();		//初始化网络
		TFTP_Menu menu;	//初始化菜单
		menu.MainMenu();
		switch (menu.choose())
		{
		case TFTP_Menu::MenuChoice::SERVER_CONFIG:	//服务器配置
			menu.SubMenu_Server_Config(server_ip,&server_port);
			Net_Clear();
			system("pause");
			break;
		case TFTP_Menu::MenuChoice::WRITEFILE:	//向服务器写文件
		{
			menu.SubMenu_File_Config(local_file_name, remote_file_name,ts_mode);
			//启动写操作
			ifstream local_file(local_file_name, ios::in | ios::binary);	//这里加入binary模式是因为如果默认使用文本格式会丢弃\n
			try
			{
				if (!local_file.is_open()) {
					cout << "无法打开本地文件" << std::endl;
					TFTP_INFO("无法打开本地文件", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
				}
				
				/*TFTP_msg* pTFTPwrite=new TFTP_msg(client, &server_addr, msg_WRQ);
				TFTP_msg& TFTPwrite = *pTFTPwrite;*/
				TFTP_msg TFTPwrite(client, &server_addr, msg_WRQ);
				if (!strcmp(ts_mode, "netascii"))	//如果是netascii模式，需要转换
				{
					string newfile =TFTPwrite.do_netascii(local_file);
					local_file.close();
					local_file.open(newfile, ios::in | ios::binary);
					if (!local_file.is_open()) {
						cout << "无法打开本地临时文件" << std::endl;
						TFTP_INFO("无法打开本地临时文件", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
					}
				}

				if (TFTPwrite.TFTP_writefile(remote_file_name, ts_mode, local_file) == ok)
				{
					string user_msg = local_file_name + string("->") + remote_file_name + string(" 向服务器写成功！");
					TFTP_INFO(user_msg.c_str(), (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
					cout << user_msg << endl;
				}
				local_file.close();
			}
			catch (TFTP_INFO& TFTP_error)
			{
				//如果出现异常，关闭文件，清理网络，退出
				TFTP_error.Create_Log();
				local_file.close();
				Net_Clear();
				system("pause");
				break;
			}
			//清理网络
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

				/*TFTP_msg* pTFTPread = new TFTP_msg(client, &server_addr, msg_RRQ);
				TFTP_msg& TFTPread = *pTFTPread;*/
				TFTP_msg TFTPread(client, &server_addr, msg_RRQ);
				if (TFTPread.TFTP_readfile(remote_file_name, ts_mode, local_file) == ok)
				{
					string user_msg =  remote_file_name + string("->") + local_file_name + string(" 从服务器读成功！");
					TFTP_INFO(user_msg.c_str(), (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
					cout<<user_msg << endl;
				}
				local_file.close();
			}
			catch (TFTP_INFO& TFTP_error)
			{
				//如果出现异常，关闭文件，清理网络，退出
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
		case TFTP_Menu::MenuChoice::GET_LOG_LOCATION:	//获取日志文件位置
		{
			system("cls");
			//获取当前路径
			const int MAXPATH = 250;
			char local_path[MAXPATH];
			getcwd(local_path, MAXPATH);
			cout << local_path<<"\\"<<log_file_name << endl;
			Net_Clear();
			system("pause");
			break;
		}
		case TFTP_Menu::MenuChoice::EXITPRO:	//退出程序
		{
			Net_Clear();
			TFTP_INFO("程序正常结束", (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
			return 0;
		}
		case TFTP_Menu::MenuChoice::CLIENT_CONFIG:		//客户端配置
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