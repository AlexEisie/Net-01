#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

class TFTP_menu
{
public:
	enum TFTP_menu_choice
	{
		get_help,
		server_config,
		write_file,
		read_file,
		get_log_location,
		clear_log,
		exit_exe
	}choice;

	void run()
	{
		while (1)
		{
			system("CLS");
			cout << "0.��ȡ����\n1.�޸ķ���������\n2.�����ļ�\n3.�鿴��־λ��\n4.�����־�ļ�\n5.�˳�����\n";
		}
        while (true)
        {
            cout << "ѡ�������";
            cin >> input;

            switch (input)
            {
            case 0:
                break;
            case 1:
                choice = server_config;
                break;
            case 2:
                run_trans_file();
                break;
            case 3:
                choice = get_log_location;
                break;
            case 4:
                choice = clear_log;
                break;
            case 5:
                choice = exit_exe;
            default:
                system("CLS");  // ��Ӵ�����������
                cout << "����ʶ��Ĳ��������ٴγ��ԡ�" << std::endl;
                break;
            }
        }
	}

    void run_trans_file()
    {

    }
private:
        int input;
};