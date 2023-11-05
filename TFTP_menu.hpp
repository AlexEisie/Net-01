#include <iostream>
#include <cstdlib>
#include <string>
#include <ctime>
using namespace std;

class TFTP_Menu
{
public:
	//ģʽѡ��ö�ٶ���
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

	//���˵�
	void MainMenu()
	{
		system("cls");
		cout << banner[clock()%3] << endl;
		cout << "Welcome to Nano TFTP Client v114.514 by U202211886" << endl;
		cout << "0.��ȡ����" << endl;
		cout << "1.�޸ķ�������������(Ĭ��ֵ127.0.0.1:69)" << endl;
		cout << "2.�������д�ļ�(WRITE / PUT)" << endl;
		cout << "3.�ӷ��������ļ�(READ / GET)" << endl;
		cout << "4.�鿴��־λ��" << endl;
		cout << "5.�˳�����" << endl;
		cout << "6.(DEBUG)�޸�Ĭ�Ͽͻ��˲�������" << endl;
	}
	//�������������ò˵�
	void SubMenu_Server_Config(char *server_ip, unsigned short *server_port)
	{
		system("cls");
		cout << "���������������IP(��ǰֵ"<< server_ip<<"):";
		cin >> server_ip;
		cout << "����������������˿�(��ǰֵ" << *server_port << "):";
		cin >> *server_port;
	}
	//�ļ��������ò˵�
	void SubMenu_File_Config(char* local_file_name, char* remote_file_name,char * ts_mode)
	{
		system("cls");
		cout << "���뱾���ļ�·��:";
		cin >> local_file_name;
		cout << "����Զ���ļ�·��:";
		cin >> remote_file_name;
		cout<<"����TFTP����ģʽ��netascii/octet)(��ǰֵ" << ts_mode << "):";
		cin >> ts_mode;
	}
	//�ͻ��˲������ò˵�
	void SubMenu_Client_Config(long* slicelenth, long long* default_timeout, int* max_retrytimes)
	{
		system("cls");
		cout << "�����뷢���ļ���Ƭ��С(<=1024Bytes)(����ֵ:512Bytes,��ǰֵ" << *slicelenth << "):";
		cin >> *slicelenth;
		cout << "�����������ʱʱ��(ms)(����ֵ:500ms,��ǰֵ" << *default_timeout << "):";
		cin >> *default_timeout;
		cout << "����������ش�����(����ֵ:3,ֵ" << *max_retrytimes << "):";
		cin >> *max_retrytimes;
	}
	//�����û�ѡ��
	int choose()
	{
		int result=0;
		cout << "������ѡ��:";
		cin >> result;
		return result;
	}
	void Wait_User()
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
           ���汣��       ����崻�     ����BUG
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