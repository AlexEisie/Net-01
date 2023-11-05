#include <iostream>
#include <string>
#include "TFTP_Log.hpp"
#pragma warning(disable : 4996)
using namespace std;

//TFTP_INFO通告类
class TFTP_INFO
{
	#define COMMON_INFO  0
	#define ADORABLE_ERROR 1
	#define CRITICAL_ERROR -1
public:
	//通知类型枚举定义
	enum TFTP_INFO_TYPE
	{
		//COMMON INFO=0

		//ADORABLE ERROR=1
		TIMEOUT = 1,
		RECVED_UNEXPECTED_PACKET = 1,
		//CTRITICAL ERROR=-1
		FILE_OPEN_FAILED = -1,
		WSA_INIT_FAILED = -1,
		SOCKET_CREATE_FAILED = -1,
		REQUEST_REACH_MAX_RETRYTIMES = -1,
		DATA_REACH_MAX_RETRYTIMES = -1,
		ACK_REACH_MAX_RETRYTIMES = -1,
		RECVED_ERROR_PACKET = -1,

		SENDTO_FAILED = -1,
		RECVFROM_FAILED = -1,
	} info_type;
	string message;
	int LINE;
	string FUNC;

	//TFTP_INFO构造函数
	TFTP_INFO(const char* _message, TFTP_INFO_TYPE _info_type, int _LINE, const char* _FUNC)
	{
		message = _message;
		info_type = _info_type;
		LINE = _LINE;
		FUNC = _FUNC;
		//不同等级通告
		switch (info_type)
		{
		case COMMON_INFO:
			TFTP_Log(message.c_str(), TFTP_Log::LOG_INFO, _LINE, _FUNC);
			break;
		case ADORABLE_ERROR:
			TFTP_Log(message.c_str(), TFTP_Log::LOG_WARN, _LINE, _FUNC);
			break;
		case CRITICAL_ERROR:
			throw *this;
			break;
		}
	}
	void Create_Log()
	{
		TFTP_Log(message.c_str(), TFTP_Log::LOG_ERROR, LINE, FUNC.c_str());
		return;
	}
};