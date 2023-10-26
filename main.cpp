#include "TFTP_client.hpp"

#undef server_ip
#undef server_port
#undef slicelenth

#define server_ip "127.0.0.1"
#define server_port 69
#define slicelenth 512

int menu(char*);

int main()
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)	//��ʼ��Winsock2.2��
	{
		cout << "WSAStartup��ʼ��ʧ��" << endl;
		return -1;
	}

	SOCKET client;						//�׽���
	struct sockaddr_in server_addr;
	memset(&server_addr,0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);
	if ((client = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		cout << "�׽��ִ���ʧ��" << endl;
		return -1;
	}

	/*TFTP_msg testread(client,&server_addr,msg_RRQ);
	testread.TFTP_readfile("a.txt", "netascii");*/
	
	char file_name[128];

	if (menu(file_name) == 1)
	{
		ifstream local_file("D:\\VisualStudio\\C++PRO\\NN-01\\a.txt", ios::in);
		if (!local_file.is_open()) {
			cout << "�޷����ļ�" << std::endl;
			return -1;
		}

		TFTP_msg testwrite(client, &server_addr, msg_WRQ);
		if(testwrite.TFTP_writefile("a.txt", "netascii", local_file)==ok)
			cout<<"�������д�ɹ���"<<endl;
		local_file.close();
	}
	else
	{
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
		if (!local_file.is_open()) {
			cout << "�޷���������ļ�" << std::endl;
			return -1;
		}

		TFTP_msg testread(client, &server_addr, msg_RRQ);
		if (testread.TFTP_readfile("a.txt", "netascii", local_file) == ok)
			cout << "����������ɹ���" << endl;
		local_file.close();
	}

	
	closesocket(client);
	WSACleanup();
	return 0;
}

int menu(char *file_name)
{
	return 2;
}