#include <direct.h>
#include "TFTP_client.hpp"
#include "TFTP_menu.hpp"

//��������������
char server_ip[64] = "127.0.0.1";
unsigned short server_port = 69;
//�ͻ��˲�������
long slicelenth = 512;		//�ļ���Ƭ��С������512����Ȼ�������
long long default_timeout = 500;
int max_retrytimes=3;
//�ͻ������ɲ���
WSADATA wsa_data;
SOCKET client;
struct sockaddr_in server_addr;
//�ļ���������
char local_file_name[128];
char remote_file_name[128];
char ts_mode[32]= "octet";
//��־����
const char log_file_name[] = "tftpclient.log";
ofstream TFTP_Log::log_file(log_file_name, ios::out | ios::app);

void Net_Init();
void Net_Clear();

int main()
{
	//������־�ļ�������
	TFTP_Log test_log (TFTP_Log::create);
	//��������
	TFTP_INFO("��������",(TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__,__func__);

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
			//�������д�ļ�Write
			//����д����
			ifstream local_file(local_file_name, ios::in | ios::binary);	//�������binaryģʽ����Ϊ���Ĭ��ʹ���ı���ʽ�ᶪ��\n
			try
			{
				if (!local_file.is_open()) {
					cout << "�޷��򿪱����ļ�" << std::endl;
					TFTP_INFO("�޷��򿪱����ļ�", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
				}
				TFTP_msg TFTPwrite(client, &server_addr, msg_WRQ);
				if (TFTPwrite.TFTP_writefile(remote_file_name, ts_mode, local_file) == ok)
					cout << "�������д�ɹ���" << endl;
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
			//����������ļ�Read

			//��鱾���ļ�
			ifstream check_file(local_file_name, ios::in | ios::binary);
			if (check_file.good())
			{
				// ����ļ�����
				char choice;
				cout << "�ļ��Ѵ��ڡ��Ƿ����´����ļ� (y/n)? ";
				cin >> choice;
				if (choice == 'y' || choice == 'Y') {
					check_file.close();
					remove(local_file_name);  // ɾ���ļ�
				}
				else {
					cout << "δ���´����ļ���" << endl;
					check_file.close();
					Net_Clear();
					system("pause");
					break;
				}
			}
			//����������
			ofstream local_file(local_file_name, ios::out | ios::binary);
			try
			{
				if (!local_file.is_open()) {
					TFTP_INFO("�޷���������ļ�", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
				}

				TFTP_msg TFTPread(client, &server_addr, msg_RRQ);
				if (TFTPread.TFTP_readfile(remote_file_name, "octet", local_file) == ok)
					cout << "����������ɹ���" << endl;
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
			TFTP_INFO("������������", (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
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

//��ʼ������
void Net_Init()
{
	//��ʼ������˵�ַ
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);
	//��ʼ��WinSOCK���׽���
	try
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)	//��ʼ��Winsock2.2��
		{
			cout << "WSAStartup��ʼ��ʧ��" << endl;
			TFTP_INFO("WSAStartup��ʼ��ʧ��", TFTP_INFO::WSA_INIT_FAILED, __LINE__, __func__);
		}


		if ((client = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			cout << "�׽��ִ���ʧ��" << endl;
			TFTP_INFO("�׽��ִ���ʧ��", TFTP_INFO::SOCKET_CREATE_FAILED, __LINE__, __func__);
		}
	}
	catch (TFTP_INFO& TFTP_error)
	{
		TFTP_error.Create_Log();
		exit(-1);	//��ʼ��ʧ��ֱ���˳�
	}
}

void Net_Clear()
{
	closesocket(client);
	WSACleanup();
}