// zpquant_md_client_api.cpp : 实现文件
//

//#include "stdafx.h"
#include <iostream>
#include <string.h>
#include <boost/foreach.hpp>
#include "ZpquantMdApi.h"
#include <nanomsg/pair.h>
#include <windows.h>

#define ADDRESS1 "inproc://test"
#define ADDRESS2 "tcp://*:8000"
#define ADDRESS3 "ipc:///tmp/reqrepmd.ipc"
#define ADDRESS4 "tcp://47.105.111.100:8000"

void Communicate(const char * address, unsigned int port, const std::stringstream & in, std::stringstream & out);

namespace Zpquant {

	CZpquantMdApi::CZpquantMdApi() {
		pSpi = NULL;
		nnsocket.socket_set(AF_SP, NN_PAIR);
		//nnsocket.bind(ADDRESS3);
		//memset(sendJsonDataStr, 0, sizeof(sendJsonDataStr)*4096);
	}


	CZpquantMdApi::~CZpquantMdApi() {
		/* Do nothing */
	}

	/**
	 * 注册spi回调接口
	 *
	 * @param   pSpi        spi回调接口
	 *
	 * @retval  TRUE: 设置成功
	 * @retval  FALSE: 设置失败
	 */
	void
		CZpquantMdApi::RegisterSpi(CZpquantMdSpi *pSpi) {
		this->pSpi = pSpi;
	}

	//初始化交易源
	bool
		CZpquantMdApi::InitMdSource(ZpquantUserLoginField* userLogin) {
		stringstream in, out;
		boost::property_tree::ptree root, result;
		root.put("type", "reqaddmarketdatasource");
		root.put("sourcetype", "kr_md_quant");
		root.put("username", userLogin->UserID);
		root.put("password", userLogin->UserPassword);
		try {
			boost::property_tree::write_json(in, root);
			Communicate(userLogin->strIP, userLogin->uPort, in, out);
			/*
...11,InitMdSource,out: {
	"clientType": "md",
	"username": "md_client1",
	"nngIpcMdAddress": "ipc:\/\/\/md_client1\/md.ipc",
	"errCode": "0",
	"reason": "login succeed",
	"type": "rspaddmarketdatasource",
	"result": "market data source init succeed."
}
			*/
			std::cout << "...InitMdSource,out: " << out.str().c_str() << endl;
			boost::property_tree::read_json(out, result);
		}
		catch (std::exception & err)
		{
			std::cout << "...cannot parse from string 'out.str().c_str()' " << endl;
			//fprintf(stdout, "cannot parse from string 'out.str().c_str()' \n");
			return false;
		}

		if (result.find("clientType") == result.not_found()) return false;
		string clientType = result.get<string>("clientType");
		if (clientType == "md")
		{
			uint8 errCode = result.get<uint8>("errCode");
			if (errCode == 1)
			{
				return false;
			}
			string nngIpcMdAddress = result.get<string>("nngIpcMdAddress");
			nnsocket.bind(nngIpcMdAddress.c_str());
		}

		/*
		for (auto & node : result)
		{
			// auto MD = CString(CA2W(node.first.c_str()));
			// m_vecMDSource.push_back(MD);
			std::cout << "...InitMdSource,result:" << node.first << endl;
		}
		*/
		if (!Start()) {
			//fprintf(stderr, "启动API失败!\n");
			return false;
		}

		return true;
	}

	//采集报单回调数据
	/*
	MdsL2TradeT:
	...client,mds_data_onTrade...subscribe,topic:mds_data_onTrade,
	msg: {"msgType":22, "sendDCT":53533711, "LastRecvT":1564037533,
	 "mktData":{"exchId":2,"securityType":1,"tradeDate":20190430,"TransactTime":132421330,
	 "ChannelNo":2011,"ApplSeqNum":8976101,"SecurityID":"000001","ExecType":"F","TradeBSFlag":"N",
	 "TradePrice":139400,"TradeQty":300,"TradeMoney":41820000,"BidApplSeqNum":8976100,
	 "OfferApplSeqNum":8976040,"__origTickSeq":91555155,"__channelNo":8,"__origNetTime":132420994,
	 "__recvTime":1564037533.700277,"__collectedTime":1564037533.700277,"__processedTime":1564037533.700333,
	 "__pushingTime":1564037533.700370}}

	MdsL2OrderT:
	 ...client,mds_data_onOrder...subscribe,topic:mds_data_onOrder,msg: {"msgType":23, "sendDCT":60222207,
	 "LastRecvT":1564044222, "mktData":{"exchId":2,"securityType":1,"tradeDate":20190430,"TransactTime":141521560,
	 "ChannelNo":2011,"ApplSeqNum":11395784,"SecurityID":"000001","Side":"2","OrderType":"2","Price":138400,
	 "OrderQty":1000,"__origTickSeq":116227503,"__channelNo":8,"__origNetTime":141521261,"__recvTime":1564044222.199936,
	 "__collectedTime":1564044222.199936,"__processedTime":1564044222.199991,"__pushingTime":1564044222.200038}}

	MdsMktDataSnapshotT:
	...client,mds_data_onTick...subscribe,topic:mds_data_onTick,msg:
	{"msgType":20, "sendDCT":60126251, "LastRecvT":1564044126,
	"mktData":{"head":{"exchId":2,"securityType":1,"tradeDate":20190430,"updateTime":141442000,"mdStreamType":20,
	"__origMdSource":5,"__dataVersion":3953,"__origTickSeq":607488764396705094,"__channelNo":4,
	"__origNetTime":141442152,"__recvTime":1564044126.244194,"__collectedTime":1564044126.244194,
	"__processedTime":1564044126.244213,"__pushingTime":1564044126.244319},"body":{"SecurityID":"002415",
	"TradingPhaseCode":"T0      ","NumTrades":35400,"TotalVolumeTraded":26480882,"TotalValueTraded":8615623012700,
	"PrevClosePx":320300,"TradePx":327200,"OpenPx":321900,"ClosePx":0,"HighPx":328200,"LowPx":318200,"IOPV":0,"NAV":0,
	"TotalLongPosition":0,"TotalBidQty":2365473,"TotalOfferQty":5489346,"WeightedAvgBidPx":318000,
	"WeightedAvgOfferPx":336700,"BidPriceLevel":0,"OfferPriceLevel":0,
	"bidPrice":[327200,327100,327000,326900,326800,326600,326500,326400,326300,326200],
	"bidNumberOfOrders":[3,10,7,2,2,3,7,1,2,1],"bidOrderQty":[6800,22800,6800,800,14500,2400,33900,2600,1500,10000],
	"offerPrice":[327300,327400,327500,327600,327700,327800,327900,328000,328100,328200],
	"offerNumberOfOrders":[18,13,40,16,10,25,34,154,33,104],
	"offerOrderQty":[16430,41200,101700,10500,7400,37100,40091,112384,26300,113700]}}}

	*/

	void* CZpquantMdApi::MdThreadMain(void *pParams)
	{
		CZpquantMdApi *mdapi = (CZpquantMdApi *)pParams;
		CZpquantMdSpi *mdspi = mdapi->pSpi;
		char buf[4096];
		while (1)
		{
			buf[0] = '\0';
			int rc = mdapi->nnsocket.recv(buf, 4096, 0);
			cout << "...CZpquantMdApi,MdThreadMain recv: " << buf << endl;

			ptree c_Config;
			std::stringstream jmsg(buf);
			try {
				boost::property_tree::read_json(jmsg, c_Config);
			}
			catch (std::exception & e) {
				fprintf(stdout, "cannot parse from string 'msg(mds_data)' \n");
				return false;
			}

			if (c_Config.find("mktData") == c_Config.not_found()) return false;
			if (c_Config.find("msgType") == c_Config.not_found()) return false;

			int8 msgType = c_Config.get<int8>("msgType");
			
			switch (msgType)
			{
			case MDS_MSGTYPE_L2_TRADE:
			{
				string mktData = c_Config.get<string>("mktData");
				if (mktData == "nil") return false;
				ptree md_Config;
				if (mktData != "nil")//有json值
				{
					std::stringstream mdmsg(mktData.c_str());
					try {
						boost::property_tree::read_json(mdmsg, md_Config);
					}
					catch (std::exception & e) {
						fprintf(stdout, "cannot parse from string 'msg(mktData，md_Config)' \n");
						return false;
					}
				}
				/*
				uint8 exchId = c_Config.get<uint8>("mktData.exchId");
				uint8 securityType = c_Config.get<uint8>("mktData.securityType");
				int32 tradeDate = c_Config.get<int32>("mktData.tradeDate");
				int32 TransactTime = c_Config.get<int32>("mktData.TransactTime");
				int32 ChannelNo = c_Config.get<int32>("mktData.ChannelNo");
				int32 ApplSeqNum = c_Config.get<int32>("mktData.ApplSeqNum");
				string SecurityID = c_Config.get<string>("mktData.SecurityID");
				string ExecType = c_Config.get<string>("mktData.ExecType");
				string TradeBSFlag = c_Config.get<string>("mktData.TradeBSFlag");
				int32 TradePrice = c_Config.get<int32>("mktData.TradePrice");
				int32 TradeQty = c_Config.get<int32>("mktData.TradeQty");
				int64 TradeMoney = c_Config.get<int64>("mktData.TradeMoney");
				int64 BidApplSeqNum = c_Config.get<int64>("mktData.BidApplSeqNum");
				int64 OfferApplSeqNum = c_Config.get<int64>("mktData.OfferApplSeqNum");
				*/
				/*
				uint8 exchId = md_Config.get<uint8>("exchId");
				uint8 securityType = md_Config.get<uint8>("securityType");
				int32 tradeDate = md_Config.get<int32>("tradeDate");
				int32 TransactTime = md_Config.get<int32>("TransactTime");
				int32 ChannelNo = md_Config.get<int32>("ChannelNo");
				int32 ApplSeqNum = md_Config.get<int32>("ApplSeqNum");
				string SecurityID = md_Config.get<string>("SecurityID");
				char ExecType = md_Config.get<char>("ExecType");
				char TradeBSFlag = md_Config.get<char>("TradeBSFlag");
				int32 TradePrice = md_Config.get<int32>("TradePrice");
				int32 TradeQty = md_Config.get<int32>("TradeQty");
				int64 TradeMoney = md_Config.get<int64>("TradeMoney");
				int64 BidApplSeqNum = md_Config.get<int64>("BidApplSeqNum");
				int64 OfferApplSeqNum = md_Config.get<int64>("OfferApplSeqNum");
				*/


				ZpquantMdsL2OnTradeT msgBody;
				msgBody.exchId = md_Config.get<uint8>("exchId");;
				msgBody.securityType = md_Config.get<uint8>("securityType");
				msgBody.tradeDate = md_Config.get<int32>("tradeDate");
				msgBody.TransactTime = md_Config.get<int32>("TransactTime");
				msgBody.ChannelNo = md_Config.get<int32>("ChannelNo");
				msgBody.ApplSeqNum = md_Config.get<int32>("ApplSeqNum");

				string SecurityID = md_Config.get<string>("SecurityID");
				memset(msgBody.SecurityID, 0, 9 * sizeof(char));
				if (SecurityID.length() > 0) memcpy(msgBody.SecurityID, SecurityID.c_str(), 9 * sizeof(char));
				msgBody.ExecType = md_Config.get<char>("ExecType");
				msgBody.TradeBSFlag = md_Config.get<char>("TradeBSFlag");

				msgBody.TradePrice = md_Config.get<int32>("TradePrice");
				msgBody.TradeQty = md_Config.get<int32>("TradeQty");
				msgBody.TradeMoney = md_Config.get<int64>("TradeMoney");
				msgBody.BidApplSeqNum = md_Config.get<int64>("BidApplSeqNum");
				msgBody.OfferApplSeqNum = md_Config.get<int64>("OfferApplSeqNum");

				mdspi->OnTradeRtnDepthMarketData(&msgBody);
				break;
			}
			case MDS_MSGTYPE_L2_ORDER:
			{
				string mktData = c_Config.get<string>("mktData");
				if (mktData == "nil") return false;
				ptree md_Config;
				if (mktData != "nil")//有json值
				{
					std::stringstream mdmsg(mktData.c_str());
					try {
						boost::property_tree::read_json(mdmsg, md_Config);
					}
					catch (std::exception & e) {
						fprintf(stdout, "cannot parse from string 'msg(mktData，md_Config)' \n");
						return false;
					}
				}
				/*
				uint8 exchId = md_Config.get<uint8>("exchId");
				uint8 securityType = md_Config.get<uint8>("securityType");
				int32 tradeDate = md_Config.get<int32>("tradeDate");
				int32 TransactTime = md_Config.get<int32>("TransactTime");
				int32 ChannelNo = md_Config.get<int32>("ChannelNo");
				int32 ApplSeqNum = md_Config.get<int32>("ApplSeqNum");
				string SecurityID = md_Config.get<string>("SecurityID");

				char Side = md_Config.get<char>("Side");
				char OrderType = md_Config.get<char>("OrderType");
				int32 Price = md_Config.get<int32>("Price");
				int32 OrderQty = md_Config.get<int32>("OrderQty");
				*/

				ZpquantMdsL2OnOrderT msgBody;
				msgBody.exchId = md_Config.get<uint8>("exchId");
				msgBody.securityType = md_Config.get<uint8>("securityType");
				msgBody.tradeDate = md_Config.get<int32>("tradeDate");
				msgBody.TransactTime = md_Config.get<int32>("TransactTime");
				msgBody.ChannelNo = md_Config.get<int32>("ChannelNo");
				msgBody.ApplSeqNum = md_Config.get<int32>("ApplSeqNum");

				string SecurityID = md_Config.get<string>("SecurityID");
				memset(msgBody.SecurityID, 0, 9 * sizeof(char));
				if (SecurityID.length() > 0) memcpy(msgBody.SecurityID, SecurityID.c_str(), 9 * sizeof(char));
				msgBody.Side = md_Config.get<char>("Side");
				msgBody.OrderType = md_Config.get<char>("OrderType");
				msgBody.Price = md_Config.get<int32>("Price");
				msgBody.OrderQty = md_Config.get<int32>("OrderQty");

				mdspi->OnOrderRtnDepthMarketData(&msgBody);
				break;
			}
			case MDS_MSGTYPE_L2_MARKET_DATA_SNAPSHOT:
			{
				/** 十档买盘价位信息 */
				//MdsPriceLevelEntryT BidLevels[10];
				/** 十档卖盘价位信息 */
				//MdsPriceLevelEntryT OfferLevels[10]; 
				string mktData = c_Config.get<string>("mktData");
				if (mktData == "nil") return false;
				ptree md_Config;
				if (mktData != "nil")//有json值
				{
					std::stringstream mdmsg(mktData.c_str());
					try {
						boost::property_tree::read_json(mdmsg, md_Config);
					}
					catch (std::exception & e) {
						fprintf(stdout, "cannot parse from string 'msg(mktData，md_Config)' \n");
						return false;
					}
				}

				uint8 exchId = md_Config.get<uint8>("head.exchId");
				uint8 securityType = md_Config.get<uint8>("head.securityType");
				int32 tradeDate = md_Config.get<int32>("head.tradeDate");
				int32 updateTime = md_Config.get<int32>("head.updateTime");
				int32 mdStreamType = md_Config.get<int32>("head.mdStreamType");

				string SecurityID = md_Config.get<string>("body.SecurityID");
				string TradingPhaseCode = md_Config.get<string>("body.TradingPhaseCode");
				uint64 NumTrades = md_Config.get<uint64>("body.NumTrades");
				uint64 TotalVolumeTraded = md_Config.get<uint64>("body.TotalVolumeTraded");
				//int64 TotalValueTraded = md_Config.get<int64>("body.TotalValueTraded");
				int32 PrevClosePx = md_Config.get<int32>("body.PrevClosePx");
				int32 OpenPx = md_Config.get<int32>("body.OpenPx");
				int32 HighPx = md_Config.get<int32>("body.HighPx");
				int32 LowPx = md_Config.get<int32>("body.LowPx");
				int32 TradePx = md_Config.get<int32>("body.TradePx");
				int32 ClosePx = md_Config.get<int32>("body.ClosePx");
				int32 IOPV = md_Config.get<int32>("body.IOPV");
				int32 NAV = md_Config.get<int32>("body.NAV");
				uint64 TotalLongPosition = md_Config.get<uint64>("body.TotalLongPosition");
				int64 TotalBidQty = md_Config.get<int64>("body.TotalBidQty");
				int64 TotalOfferQty = md_Config.get<int64>("body.TotalOfferQty");
				int32 WeightedAvgBidPx = md_Config.get<int32>("body.WeightedAvgBidPx");
				int32 WeightedAvgOfferPx = md_Config.get<int32>("body.WeightedAvgOfferPx");
				int32 BidPriceLevel = md_Config.get<int32>("body.BidPriceLevel");
				int32 OfferPriceLevel = md_Config.get<int32>("body.OfferPriceLevel");

				ZpquantMdsL2OnTickT msgBody;
				msgBody.exchId = exchId;
				msgBody.securityType = securityType;
				msgBody.tradeDate = tradeDate;
				msgBody.updateTime = updateTime;
				msgBody.mdStreamType = mdStreamType;

				//if (SecurityID != NULL) strncpy(msgBody.mktDataSnapshot.l2Stock.SecurityID, SecurityID.c_str(), sizeof(msgBody.mktDataSnapshot.l2Stock.SecurityID) - 1);
				//if (TradingPhaseCode != NULL) strncpy(msgBody.mktDataSnapshot.l2Stock.TradingPhaseCode, TradingPhaseCode.c_str(), sizeof(msgBody.mktDataSnapshot.l2Stock.TradingPhaseCode) - 1);

				memset(msgBody.SecurityID, 0, 9 * sizeof(char));
				if (SecurityID.length() > 0) memcpy(msgBody.SecurityID, SecurityID.c_str(), 9 * sizeof(char));
				if (TradingPhaseCode.length() > 0) memcpy(msgBody.TradingPhaseCode, TradingPhaseCode.c_str(), 9 * sizeof(char));

				msgBody.NumTrades = NumTrades;
				msgBody.TotalVolumeTraded = TotalVolumeTraded;
				//msgBody.TotalValueTraded = TotalValueTraded;
				msgBody.PrevClosePx = PrevClosePx;
				msgBody.TradePx = TradePx;
				msgBody.OpenPx = OpenPx;
				msgBody.ClosePx = ClosePx;
				msgBody.HighPx = HighPx;
				msgBody.LowPx = LowPx;
				msgBody.IOPV = IOPV;
				msgBody.NAV = NAV;
				msgBody.TotalLongPosition = TotalLongPosition;
				msgBody.TotalBidQty = TotalBidQty;
				msgBody.TotalOfferQty = TotalOfferQty;
				msgBody.WeightedAvgBidPx = WeightedAvgBidPx;
				msgBody.WeightedAvgOfferPx = WeightedAvgOfferPx;
				msgBody.BidPriceLevel = BidPriceLevel;
				msgBody.OfferPriceLevel = OfferPriceLevel;

				ptree child_Bid = md_Config.get_child("body.bidPrice");
				int i = 0;
				BOOST_FOREACH(boost::property_tree::ptree::value_type &vt, child_Bid) {
					msgBody.BidLevels[i].Price = vt.second.get_value<int32>();
					i++;
				}

				child_Bid = md_Config.get_child("body.bidNumberOfOrders");
				i = 0;
				BOOST_FOREACH(boost::property_tree::ptree::value_type &vt, child_Bid) {
					msgBody.BidLevels[i].NumberOfOrders = vt.second.get_value<int32>();
					i++;
				}

				child_Bid = md_Config.get_child("body.bidOrderQty");
				i = 0;
				BOOST_FOREACH(boost::property_tree::ptree::value_type &vt, child_Bid) {
					msgBody.BidLevels[i].OrderQty = vt.second.get_value<int32>();
					i++;
				}

				child_Bid = md_Config.get_child("body.offerPrice");
				i = 0;
				BOOST_FOREACH(boost::property_tree::ptree::value_type &vt, child_Bid) {
					msgBody.OfferLevels[i].Price = vt.second.get_value<int32>();
					i++;
				}

				child_Bid = md_Config.get_child("body.offerNumberOfOrders");
				i = 0;
				BOOST_FOREACH(boost::property_tree::ptree::value_type &vt, child_Bid) {
					msgBody.OfferLevels[i].NumberOfOrders = vt.second.get_value<int32>();
					i++;
				}

				child_Bid = md_Config.get_child("body.offerOrderQty");
				i = 0;
				BOOST_FOREACH(boost::property_tree::ptree::value_type &vt, child_Bid) {
					msgBody.OfferLevels[i].OrderQty = vt.second.get_value<int32>();
					i++;
				}

				mdspi->OnTickRtnDepthMarketData(&msgBody);
				break;
			}
			case MDS_MSGTYPE_L2_BEST_ORDERS_SNAPSHOT:
			{
				cout << "...CZpquantMdApi,MDS_MSGTYPE_L2_BEST_ORDERS_SNAPSHOT recv: " << buf << endl;
				break;
			}
			default:
			{
				cout << "other category info... \n" << endl;
				cout << "...CZpquantMdApi,MdThreadMain recv: " << buf << endl;
				break;
			}

			}
		}
	}


	bool
		CZpquantMdApi::Start() {

		//pthread_t       rptThreadId;
		//int32           ret = 0;

		/* 创建回报接收线程 */
		//ret = pthread_create(&rptThreadId, NULL, MdThreadMain, (void *) this);
		//if (ret != 0) {
		//    fprintf(stderr, "创建行情盘口接收线程失败! error[%d - %s]\n",
		//            ret, strerror(ret));
		//    return false;
		//}
		/* 创建回报接收线程 */
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MdThreadMain, (LPVOID)this, 0, NULL);

		return true;
	}

	void
		CZpquantMdApi::Stop() {

	}

	int
		CZpquantMdApi::SubscribeMarketData(const char *ppInstrumentIDStr,int32 MdsSubscribeDataType, ZpquantMdsSubscribeMode mdsSubMode)
	{
		char sendJsonDataStr[1024];
		sprintf(sendJsonDataStr, "{\"type\":\"SubscribeMd\",\"codelistStr\":\"%s\",\"dataType\":%d,\"mdsSubMode\":\"%d\"}", ppInstrumentIDStr, MdsSubscribeDataType,mdsSubMode);
		std::cout << "...SubscribeMd...SubscribeMarketData: " << sendJsonDataStr << endl;
		nnsocket.send(sendJsonDataStr, strlen(sendJsonDataStr) + 1, 0);
		return 0;
	}

	/**
	 * 获取API的发行版本号
	 *
	 * @return  API的发行版本号 (如: "0.15.3")
	 */
	const char *
		CZpquantMdApi::GetVersion(void) {
		string vStr = "0.1";
		return vStr.c_str();
	}
}

