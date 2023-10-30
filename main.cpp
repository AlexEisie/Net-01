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
	//������־�ļ�������
	TFTP_Log test_log (TFTP_Log::create);
	//���������ͳ�ʼ������
	TFTP_INFO("��������",(TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__,__func__);
	WSADATA wsa_data;
	SOCKET client;						//�׽���
	struct sockaddr_in server_addr;		//��������ַ�ṹ��
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

	//ѡ�������ʼ����
	char file_name[128];
	start:
	if (menu(file_name) == 1)
	{
		//�������д�ļ�Write
		ifstream local_file("D:\\VisualStudio\\C++PRO\\NN-01\\a.txt", ios::in);
		try 
		{
			if (!local_file.is_open()) {
				cout << "�޷��򿪱����ļ�" << std::endl;
				TFTP_INFO("�޷��򿪱����ļ�", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
			}
			TFTP_msg testwrite(client, &server_addr, msg_WRQ);
			if (testwrite.TFTP_writefile("a.txt", "netascii", local_file) == ok)
				cout << "�������д�ɹ���" << endl;
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
		//����������ļ�Read
		const char local_file_name[] = "D:\\VisualStudio\\C++PRO\\NN-01\\b.txt";
		ifstream check_file(local_file_name, ios::in);
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
				return 0;
			}
		}
		ofstream local_file(local_file_name, ios::out);
		try
		{
			if (!local_file.is_open()) {
				TFTP_INFO("�޷���������ļ�", TFTP_INFO::FILE_OPEN_FAILED, __LINE__, __func__);
			}

			TFTP_msg testread(client, &server_addr, msg_RRQ);
			if (testread.TFTP_readfile("a.txt", "netascii", local_file) == ok)
				cout << "����������ɹ���" << endl;
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
	TFTP_INFO("������������", (TFTP_INFO::TFTP_INFO_TYPE)0, __LINE__, __func__);
	return 0;
}

int menu(char *file_name)
{
	int choice = 0;
	cout << "WRITE OR READ(1/2)";
	cin >> choice;
	return choice;
}