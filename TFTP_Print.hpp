#include<stdio.h>
#include <future>
#include<mutex>
using namespace std;

//������Ļ����߳�
class TFTP_Print
{
public:
	bool print_done;
	future<void> f_print;
	mutex print_msg_mutex;
	bool have_print;
	string print_msg;
	TFTP_Print()
	{
		print_done = 0;
		have_print = 0;
		f_print = async(launch::async, [&]()	//�����յ������ݴ�ӡ����Ļ�����߳�
			{
				while (1)
				{
					if (have_print)
					{
						lock_guard<mutex> lock(print_msg_mutex);
						printf("%s",print_msg.c_str());
						have_print = 0;
					}
					if (print_done)
					{
						break;
					}
				}
			});
	}
	~TFTP_Print()
	{
		print_done = 1;
	}
	void set_msg(string& msg)
	{
		lock_guard<mutex> lock(print_msg_mutex);
		print_msg = msg;
		have_print = 1;
	};
};