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
			cout << "0.获取帮助\n1.修改服务器配置\n2.传输文件\n3.查看日志位置\n4.清空日志文件\n5.退出程序\n";
		}
        while (true)
        {
            cout << "选择操作：";
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
                system("CLS");  // 添加此行用于清屏
                cout << "不可识别的操作，请再次尝试。" << std::endl;
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