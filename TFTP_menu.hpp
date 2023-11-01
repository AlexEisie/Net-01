#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

class TFTP_Menu
{
public:
	enum MenuChoice
	{
		GET_HELP = 0,
		SERVER_CONFIG = 1,
		WRITEFILE = 2,
		READFILE = 3,
		GET_LOG_LOCATION = 4,
		EXITPRO = 5,
		CLIENT_CONFIG = 6
	};
	void MainMenu()
	{
		system("cls");
		cout << "Welcome to Noob TFTP Client v114.514 by U2022***86" << endl;
		cout << "0.获取帮助" << endl;
		cout << "1.修改服务器监听配置(默认值127.0.0.1:69)" << endl;
		cout << "2.向服务器写文件(WRITE / PUT)" << endl;
		cout << "3.从服务器读文件(READ / GET)" << endl;
		cout << "4.查看日志位置" << endl;
		cout << "5.退出程序" << endl;
		cout << "6.(DEBUG)修改默认客户端参数配置" << endl;
	}
	void SubMenu_Server_Config(char *server_ip, unsigned short *server_port)
	{
		system("cls");
		cout << "请输入服务器监听IP(当前值"<< server_ip<<"):";
		cin >> server_ip;
		cout << "请输入服务器监听端口(当前值" << *server_port << "):";
		cin >> *server_port;
	}
	void SubMenu_File_Config(char* local_file_name, char* remote_file_name,char * ts_mode)
	{
		system("cls");
		cout << "输入本地文件路径:";
		cin >> local_file_name;
		cout << "输入远端文件路径:";
		cin >> remote_file_name;
		cout<<"输入TFTP传输模式（netascii/octet)(当前值" << ts_mode << "):";
		cin >> ts_mode;
	}
	void SubMenu_Client_Config(int* slicelenth, long long* default_timeout, int* max_retrytimes)
	{
		system("cls");
		cout << "请输入发送文件分片大小(Bytes)(当前值" << *slicelenth << "):";
		cin >> *slicelenth;
		cout << "请输入基础超时时长(ms)(当前值" << *default_timeout << "):";
		cin >> *default_timeout;
		cout << "请输入最大重传次数(ms)(当前值" << *max_retrytimes << "):";
		cin >> *max_retrytimes;
	}
	int choose()
	{
		int result=0;
		cout << "请输入选项:";
		cin >> result;
		return result;
	}
	void Wait_User()
	{
		system("pause");
	}
private:
};