#ifndef QFCOMPRETRADESYSTEM_ATMTRADEPLUGINS_KR_QUANT_TDPLUGIN_TEMPLATE_ANY_TDPlugin_H_
#define QFCOMPRETRADESYSTEM_ATMTRADEPLUGINS_KR_QUANT_TDPLUGIN_TEMPLATE_ANY_TDPlugin_H_
#include <boost/thread.hpp>
#include <thread>                // std::thread
#include <mutex>                // std::mutex, std::unique_lock
#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <tuple>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <future>
#include <boost/log/common.hpp>

#include "AtmTradePluginInterface.h"
#include "TradePluginContextInterface.h"

#include "oes_client_api.h"
#include "oes_client_spi.h"
#include "nn.hpp"
#include <nanomsg/pair.h>

#include "SeverityLevel.h"
#include "rediscpp.h"

#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/json_parser.hpp> 
using namespace boost::property_tree;

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace boost::asio;
using namespace std;
using namespace zpquant;


class CKR_QUANT_TDPlugin :
	public MAtmTradePluginInterface
{
	io_service  m_IOservice;
	deadline_timer m_StartAndStopCtrlTimer;
	std::future<bool> m_futTimerThreadFuture;

	boost::shared_mutex m_mtxProtectCancelAmount;
	map<string, int> m_mapCancelAmount;
	int m_intInitAmountOfCancelChancesPerDay;

	Quant360::OesClientApi  *pOesApi;
	Quant360::OesClientSpi  *pOesSpi;

	string m_strUsername;//Init at TDInit
	string m_strPassword;//Init at TDInit
	date GetTradeday(ptime _Current);
	date m_dateTradeDay;
	bool m_isTradeDay = true;

public:
	static const string s_strAccountKeyword;
	CKR_QUANT_TDPlugin();
	~CKR_QUANT_TDPlugin();
	nn::socket tdnnsocket;
	map<string, ClientListTdInfoT> tclient;
	zpquant::Redis Redis;


	virtual bool IsOnline();
	virtual void CheckSymbolValidity(const unordered_map<string, string> &);
	virtual string GetCurrentKeyword();
	virtual string GetProspectiveKeyword(const ptree &);
	virtual void GetState(ptree & out);
	virtual void TDInit(const ptree &, ptree &, MTradePluginContextInterface*, unsigned int AccountNumber, string &);
	virtual void TDAddClientUser(const ptree &, ptree &, MTradePluginContextInterface*, unsigned int AccountNumber, string &);
	void ShowMessage(severity_levels, const char * fmt, ...);

	//交易相关
	int32 OesClientMain_SendOrder(Quant360::OesClientApi *pOesApi,
        uint8 mktId, const char *pSecurityId, const char *pInvAcctId,
        uint8 ordType, uint8 bsType, int32 ordQty, int32 ordPrice);
	int32 OesClientMain_CancelOrder(Quant360::OesClientApi *pOesApi,
        uint8 mktId, const char *pSecurityId, const char *pInvAcctId,
        int32 origClSeqNo, int8 origClEnvId, int64 origClOrdId);
	int32 OesClientMain_QueryClientOverview(Quant360::OesClientApi *pOesApi);
	int32 OesClientMain_QueryMarketStatus(Quant360::OesClientApi *pOesApi,
        uint8 exchId, uint8 platformId);
	int32 OesClientMain_QueryCashAsset(Quant360::OesClientApi *pOesApi,
        const char *pCashAcctId);
	int32 OesClientMain_QueryStock(Quant360::OesClientApi *pOesApi,
        const char *pSecurityId, uint8 mktId, uint8 securityType,
        uint8 subSecurityType);
	int32 OesClientMain_QueryStkHolding(Quant360::OesClientApi *pOesApi,
        uint8 mktId, const char *pSecurityId);

private:
	bool Start();
	bool Stop();
	//void ShowMessage(severity_levels, const char * fmt, ...);
	void TimerHandler(boost::asio::deadline_timer* timer, const boost::system::error_code& err);
private:
    static void *       tdThreadMain(void *pParams);

};
#endif

