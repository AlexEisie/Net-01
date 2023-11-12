#ifndef TFTP_MENU_HPP
#define TFTP_MENU_HPP
#include <iostream>
#include <cstdlib>
#include <string>
#include <ctime>
using namespace std;

class TFTP_Menu
{
public:
	//模式选择枚举定义
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

	//主菜单
	void MainMenu() const
	{
		system("cls");
		cout << banner[clock() % 2 + 1] << endl;
		cout << "Welcome to Nano TFTP Client v114.514 by U202211886" << endl;
		cout << "0.获取帮助" << endl;
		cout << "1.修改服务器监听配置(默认值127.0.0.1:69)" << endl;
		cout << "2.向服务器写文件(WRITE / PUT)" << endl;
		cout << "3.从服务器读文件(READ / GET)" << endl;
		cout << "4.查看日志位置" << endl;
		cout << "5.退出程序" << endl;
		cout << "6.(DEBUG)修改默认客户端参数配置" << endl;
	}

	//服务器监听配置菜单
	static void SubMenu_Server_Config(char* server_ip, unsigned short* server_port)
	{
		system("cls");
		cout << "请输入服务器监听IP(留空以使用当前值" << server_ip << "):";
		if (cin.get() != '\n')
		{
			cin.unget();
			cin >> server_ip;
			cin.get();
		}
		cin.get();
		cout << "请输入服务器监听端口(留空以使用当前值" << *server_port << "):";
		if (cin.get() != '\n')
		{
			cin.unget();
			cin >> *server_port;
			cin.get();
		}
	}

	//文件传输配置菜单
	void SubMenu_File_Config(char* local_file_name, char* remote_file_name, char* ts_mode)
	{
		system("cls");
		cout << "输入本地文件路径:";
		cin >> local_file_name;
		cout << "输入远端文件路径:";
		cin >> remote_file_name;
		cin.get();
		cout << "输入TFTP传输模式（netascii/octet)(留空以使用当前值" << ts_mode << "):";
		if (cin.get() != '\n')
		{
			cin.unget();
			cin >> ts_mode;
			cin.get();
		}
	}

	//客户端参数配置菜单
	static void SubMenu_Client_Config(long* slicelenth, long long* default_timeout, int* max_retrytimes)
	{
		system("cls");
		cout << "请输入发送文件分片大小(<=1024Bytes)(建议值:512Bytes,当前值" << *slicelenth << "):";
		cin >> *slicelenth;
		cout << "请输入基础超时时长(ms)(建议值:500ms,当前值" << *default_timeout << "):";
		cin >> *default_timeout;
		cout << "请输入最大重传次数(建议值:3,当前值" << *max_retrytimes << "):";
		cin >> *max_retrytimes;
	}

	//请求用户选择
	static int choose()
	{
		int result = 0;
		cout << "请输入选项:";
		cin >> result;
		return result;
	}

	static void Wait_User()
	{
		system("pause");
	}

private:
	const char banner[3][2048] = {
		R"(
                       _ooOoo_                        
                      o8888888o                       
                      88" . "88                       
                      (| ^_^ |)                       
                      O\  =  /O                       
                   ____/`---'\____                    
                 .'  \\|     |//  `.                  
                /  \\|||  :  |||//  \                 
               /  _||||| -:- |||||-  \                
               |   | \\\  -  /// |   |                
               | \_|  ''\---/''  |   |                
               \  .-\__  `-`  ___/-. /                
             ___`. .'  /--.--\  `. . ___              
           ."" '<  `.___\_<|>_/___.'  >'"".           
         | | :  `- \`.;`\ _ /`;.`/ - ` : | |          
         \  \ `-.   \_ __\ /__ _/   .-` /  /          
   ========`-.____`-.___\_____/___.-`____.-'========  
                        `=---='                       
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
           佛祖保佑       永不宕机     永无BUG
)",
		R"(_____   __                         _______________________________ 
___  | / /_____ _____________      ___  __/__  ____/__  __/__  __ \
__   |/ /_  __ `/_  __ \  __ \     __  /  __  /_   __  /  __  /_/ /
_  /|  / / /_/ /_  / / / /_/ /     _  /   _  __/   _  /   _  ____/ 
/_/ |_/  \__,_/ /_/ /_/\____/      /_/    /_/      /_/    /_/      
)",
		R"(
    _   __                     ______________________ 
   / | / /___ _____  ____     /_  __/ ____/_  __/ __ \
  /  |/ / __ `/ __ \/ __ \     / / / /_    / / / /_/ /
 / /|  / /_/ / / / / /_/ /    / / / __/   / / / ____/ 
/_/ |_/\__,_/_/ /_/\____/    /_/ /_/     /_/ /_/      
)"
	};
};
#endif // TFTP_MENU_HPP
