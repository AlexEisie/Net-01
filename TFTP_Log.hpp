#include<iostream>
#include<fstream>
#include <time.h>
#pragma warning(disable : 4996)
using namespace std;

class TFTP_Log 
{
public:
	enum create_log{create, not_create};
	enum Log_level { LOG_INFO, LOG_WARN, LOG_ERROR };
	TFTP_Log();
	TFTP_Log(const char* message, Log_level level, int LINE, const char* FUNC, create_log c) :TFTP_Log(message, level, LINE, FUNC) { test_log_file(); };
	TFTP_Log(create_log c) { test_log_file(); }
	TFTP_Log(const char* message,Log_level level,int LINE,const char* FUNC)
	{
		time(&timep);
		log_file << ctime(&timep);
		switch (level)
		{
		case LOG_INFO:
			log_file << "[INFO]";break;
		case LOG_WARN:
			log_file << "[WARN]"; break; 
		case LOG_ERROR:
			log_file << "[ERROR]"; break;
		}
		log_file <<" FUNC:"<<FUNC << " LINE:" << LINE << " " << message << endl;
	}
	bool test_log_file()
	{
		if (!TFTP_Log::log_file.is_open()) {
			cout << "无法打开日志文件！" << endl;
			exit(1);
		}
	}
private:
	static ofstream log_file;
	time_t timep;
};

