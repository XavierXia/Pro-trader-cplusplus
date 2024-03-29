#include <string>
#include <sys/stat.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/core.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword.hpp>

#include <boost/log/attributes.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>

#include <boost/log/sinks/text_multifile_backend.hpp>
#include <boost/log/trivial.hpp> 

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/log/attributes/named_scope.hpp>


#include "public.h"
namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
using namespace boost::posix_time;
using namespace boost::gregorian;

#ifndef BOOST_SPIRIT_THREADSAFE
#define BOOST_SPIRIT_THREADSAFE
#endif
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/json_parser.hpp> 
using namespace boost::property_tree;

#include "trade_service.h"
#include "CommuModForServInterface.h"
using namespace std;
// g++ -D BOOST_SPIRIT_THREADSAFE

char ProcessName[256];
//安装一个关闭console事件钩子
BOOL CtrlHandler(DWORD fdwCtrlType);

void InitLog(std::string configFile)
{
    ptree g_Config;
    boost::property_tree::read_json(configFile, g_Config);
    
    auto LogConfig = g_Config.find("logconfig");
    if (LogConfig != g_Config.not_found())
    {
        auto LogtypeNode = LogConfig->second.find("logtype");
        if(LogtypeNode== LogConfig->second.not_found())
            throw std::runtime_error("[error]Can not find 'LogConfig.Logtype' Value.");
        string LogTypeString = LogtypeNode->second.data();

        if (LogTypeString.find("file") != std::string::npos)
        {
			string LogFileHead = "_";
			int RotationSize = 1024;
			bool AutoFlush = true;
			if (LogConfig->second.find("file") != LogConfig->second.not_found())
			{
				auto FileLogNode = LogConfig->second.find("file");
				if (FileLogNode->second.find("filenamehead") != FileLogNode->second.not_found())
					LogFileHead = FileLogNode->second.find("filenamehead")->second.data();

				if (FileLogNode->second.find("rotationsize") != FileLogNode->second.not_found())
					RotationSize = atoi(FileLogNode->second.find("rotationsize")->second.data().c_str());

				if (FileLogNode->second.find("autoflush") != FileLogNode->second.not_found())
					AutoFlush = FileLogNode->second.find("autoflush")->second.data() == "true" ? true : false;
			}
			else
				throw std::runtime_error("[error]could not find 'logconfig.file' node.");
			typedef sinks::synchronous_sink<sinks::text_file_backend> TextSink;
			boost::shared_ptr<sinks::text_file_backend> backend1 = boost::make_shared<sinks::text_file_backend>(
				keywords::file_name = LogFileHead + "%Y-%m-%d_%H-%M-%S.%N.log",
				keywords::rotation_size = RotationSize,
				keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
				keywords::min_free_space = 30 * 1024 * 1024);

			backend1->auto_flush(AutoFlush);
			boost::shared_ptr<TextSink> sink(new TextSink(backend1));
			//sink->set_formatter(formatter);
			logging::core::get()->add_sink(sink);
        }

        if (LogTypeString.find("console") != std::string::npos)
        {
			auto console_sink = logging::add_console_log();
			//console_sink->set_formatter(formatter);
			logging::core::get()->add_sink(console_sink);
        }
    }
    else
        throw std::runtime_error("[error]could not find 'logconfig' node.");

}

void test()
{
	printf("hello teset\n");
}

void printSystemLog(const string str)
{
	boost::log::sources::severity_logger< severity_levels > m_Logger;
	BOOST_LOG_SEV(m_Logger, normal) << "[" << to_iso_string(microsec_clock::universal_time() + hours(8)) << "] " << str.c_str();
}

// ./thunder-trader thunder-trader.conf.default 1
int main(int argc,char *argv[])
{
	//添加处理控制台消息函数，记录console信息
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		printSystemLog("SetConsoleCtrlHandler函数,处理控制台消息!");
	}
	else
	{
		printSystemLog("ERROR: could not set control handler!");
	}
	//atexit(test);

    if (argc < 3)
    {
        //cout<<"Usage:server.exe %ConfigFileName%.json sysNumber [daemon]"<<endl;
		printSystemLog("Usage:server.exe %ConfigFileName%.json sysNumber [daemon]");
        return 0;
    }
    try
    {
        strncpy(ProcessName, argv[0], sizeof(ProcessName));
        if(atoi(argv[2])>_MaxAccountNumber)
            throw std::runtime_error("sysNumber error");
        InitLog(argv[1]);

		//universal_time 取得格林威治时间
		//microsec_clock::local_time()则取得的是本地时间
		//boost::log::sources::severity_logger< severity_levels > m_Logger;
		//BOOST_LOG_SEV(m_Logger, normal) << " [" << to_iso_string(microsec_clock::universal_time()+hours(8)) << "] " << "Server.StartRun success!!!";
		printSystemLog("Server.StartRun success!!!");

        CTradeService service(argv[1],atoi(argv[2]));
        service.Start();
        service.Join();
    }
    catch (std::exception & exp)
    {
        std::cout << exp.what() << endl;
		//boost::log::sources::severity_logger< severity_levels > m_Logger;
		//BOOST_LOG_SEV(m_Logger, normal)  << " [" << to_iso_string(microsec_clock::universal_time() + hours(8)) << "] " << "Server.StartRun failure!!!";
		printSystemLog("Server.StartRun failure!!!");

        return 1;
    }

    return 0;
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		/* handle the CTRL-C signal */
	case CTRL_C_EVENT:
		//printf("CTRL-C event\n");
		printSystemLog("钩子函数CtrlHandler,CTRL-C event!");
		return TRUE;

		/* handle the CTRL-BREAK signal */
	case CTRL_BREAK_EVENT:
		//printf("CTRL-BREAK event\n");
		printSystemLog("钩子函数CtrlHandler,CTRL-BREAK event!");
		return TRUE;

		/* handle the CTRL-CLOSE signal */
	case CTRL_CLOSE_EVENT:
		//printf("点击了控制台右上角的“X”\n");
		printSystemLog("钩子函数CtrlHandler,点击了控制台右上角的“X”!");
		return TRUE;

		/* handle the CTRL-LOGOFF signal */
	case CTRL_LOGOFF_EVENT:
		//printf("CTRL-LOGOFF event\n");
		printSystemLog("钩子函数CtrlHandler,CTRL-LOGOFF event!");
		return TRUE;

		/* handle the CTRL-SHUTDOWN signal */
	case CTRL_SHUTDOWN_EVENT:
		//printf("CTRL-SHUTDOWN event\n");
		printSystemLog("钩子函数CtrlHandler,CTRL-SHUTDOWN event!");
		return TRUE;

	default:
		return FALSE;
	}
}
