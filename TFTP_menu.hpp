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
		cout << "0.��ȡ����" << endl;
		cout << "1.�޸ķ�������������(Ĭ��ֵ127.0.0.1:69)" << endl;
		cout << "2.�������д�ļ�(WRITE / PUT)" << endl;
		cout << "3.�ӷ��������ļ�(READ / GET)" << endl;
		cout << "4.�鿴��־λ��" << endl;
		cout << "5.�˳�����" << endl;
		cout << "6.(DEBUG)�޸�Ĭ�Ͽͻ��˲�������" << endl;
	}
	void SubMenu_Server_Config(char *server_ip, unsigned short *server_port)
	{
		system("cls");
		cout << "���������������IP(��ǰֵ"<< server_ip<<"):";
		cin >> server_ip;
		cout << "����������������˿�(��ǰֵ" << *server_port << "):";
		cin >> *server_port;
	}
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
	void SubMenu_Client_Config(int* slicelenth, long long* default_timeout, int* max_retrytimes)
	{
		system("cls");
		cout << "�����뷢���ļ���Ƭ��С(Bytes)(��ǰֵ" << *slicelenth << "):";
		cin >> *slicelenth;
		cout << "�����������ʱʱ��(ms)(��ǰֵ" << *default_timeout << "):";
		cin >> *default_timeout;
		cout << "����������ش�����(ms)(��ǰֵ" << *max_retrytimes << "):";
		cin >> *max_retrytimes;
	}
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
};