//#include <iostream>
//#include <cstdlib>
//#include <string>
//using namespace std;
//
//class TFTP_Menu {
//public:
//    TFTP_Menu() {
//        clearScreen();
//        show();
//    }
//
//    int choose() {
//        int choice = 0;
//        cin >> choice;
//        clearScreen();
//        return choice;
//    }
//
//    SubMenu* getSubMenu() {
//        return &submenu;
//    }
//
//private:
//    SubMenu submenu;
//
//    void show() {
//        cout << "欢迎来到TFTP_CLIENT! by Me" << endl;
//        cout << "0.获取帮助\n1.修改服务器配置\n2.向服务器写文件(WRITE/PUT)\n3.从服务器读文件(READ/GET)\n4.查看日志位置\n5.清空日志文件\n6.退出程序\n请选择功能:";
//    }
//
//    void clearScreen() {
//        system("cls");
//    }
//};
//
//class SubMenu {
//public:
//    void getInput(string& local_file_name, string& remote_file_name) {
//        cout << "输入本地文件路径:";
//        cin >> local_file_name;
//        cout << "输入远端文件路径:";
//        cin >> remote_file_name;
//    }
//};
//
////下面是第一版设计，功能冗余
////class TFTP_menu
////{
////public:
////	enum TFTP_menu_choice
////	{
////		get_help,
////		server_config,
////		write_file,
////		read_file,
////		get_log_location,
////		clear_log,
////		exit_exe
////	}choice;
////
////	void run()
////	{
////		while (1)
////		{
////			system("CLS");
////			cout << "0.获取帮助\n1.修改服务器配置\n2.传输文件\n3.查看日志位置\n4.清空日志文件\n5.退出程序\n";
////		}
////        while (true)
////        {
////            cout << "选择操作：";
////            cin >> input;
////
////            switch (input)
////            {
////            case 0:
////                break;
////            case 1:
////                choice = server_config;
////                break;
////            case 2:
////                run_trans_file();
////                break;
////            case 3:
////                choice = get_log_location;
////                break;
////            case 4:
////                choice = clear_log;
////                break;
////            case 5:
////                choice = exit_exe;
////            default:
////                system("CLS");  // 添加此行用于清屏
////                cout << "不可识别的操作，请再次尝试。" << std::endl;
////                break;
////            }
////        }
////	}
////
////    void run_trans_file()
////    {
////
////    }
////private:
////        int input;
////};