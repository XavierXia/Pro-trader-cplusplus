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

// ./thunder-trader thunder-trader.conf.default 1
int main(int argc,char *argv[])
{
    if (argc < 3)
    {
        cout<<"Usage:ATM.exe %ConfigFileName%.json sysNumber [daemon]"<<endl;
        return 0;
    }
    try
    {
        strncpy(ProcessName, argv[0], sizeof(ProcessName));
        if(atoi(argv[2])>_MaxAccountNumber)
            throw std::runtime_error("sysNumber error");
        InitLog(argv[1]);
        
        CTradeService service(argv[1],atoi(argv[2]));
        service.Start();
        service.Join();
    }
    catch (std::exception & exp)
    {
        std::cout << exp.what() << endl;
        return 1;
    }
    return 0;
}

