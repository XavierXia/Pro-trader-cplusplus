// zpquant_trader_client_api.cpp : 实现文件
//

//#include "stdafx.h"
#include <iostream>
#include <string.h>
#include "ZpquantTraderApi.h"
#include <nanomsg/pair.h>
#include <windows.h>

/*
#define SOCKET_ADDRESS "tcp://127.0.0.1:8001"
#define ADDRESS1 "inproc://test"
#define ADDRESS2 "tcp://*:8001"
#define ADDRESS3 "ipc:///tmp/reqreptd.ipc"
#define ADDRESS7 "ipc:///tmp/reqrepresp.ipc"
#define ADDRESS4 "tcp://*:8002"
#define ADDRESS5 "tcp://47.105.111.100:8001"
#define ADDRESS6 "tcp://47.105.111.100:8002"
*/

void Communicate(const char * address, unsigned int port, const std::stringstream & in, std::stringstream & out);

namespace Zpquant {

	CZpquantTradeApi::CZpquantTradeApi() {
		pSpi = NULL;
		tdnnsocket.socket_set(AF_SP, NN_PAIR);
		//tdnnsocket.bind(ADDRESS3);

		tdnnsocket_resp.socket_set(AF_SP, NN_PAIR);
		//tdnnsocket_resp.bind(ADDRESS7);
	}


	CZpquantTradeApi::~CZpquantTradeApi() {
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
		CZpquantTradeApi::RegisterSpi(CZpquantTradeSpi *pSpi) {
		this->pSpi = pSpi;
	}

	//初始化交易源
	bool
	CZpquantTradeApi::InitTraderSource(ZpquantUserLoginField* userLogin) {
		stringstream in, out;
		boost::property_tree::ptree root, result;
		root.put("type", "reqaddtradesource");
		root.put("sourcetype", "kr_td_quant");
		root.put("username", userLogin->UserID);
		root.put("password", userLogin->UserPassword);
		try {
			boost::property_tree::write_json(in, root);
			Communicate(userLogin->strIP, userLogin->uPort, in, out);
			/*
			{
				"clientType": "td",
				"username": "td_client1",
				"nngIpcTdAddress": "ipc:\/\/\/td_client1\/td.ipc",
				"nngIpcTdResAddress": "ipc:\/\/\/td_client1\/tdres.ipc",
				"errCode": "0",
				"reason": "login succeed",
				"type": "rspaddtradesource",
				"result": "trade source init succeed."
			}
			*/
			std::cout << "...InitTdSource,out: " << out.str().c_str() << endl;
			boost::property_tree::read_json(out, result);
		}
		catch (std::exception & err)
		{
			std::cout << "...InitTraderSource,error!!!" << endl;
			return false;
		}
		/*
		for (auto & node : result)
		{
			// auto MD = CString(CA2W(node.first.c_str()));
			// m_vecMDSource.push_back(MD);
			std::cout << "...InitTraderSource,result:" << node.first << endl;
		}
		*/

		if (result.find("clientType") == result.not_found()) return false;
		string clientType = result.get<string>("clientType");
		if (clientType == "td")
		{
			uint8 errCode = result.get<uint8>("errCode");
			if (errCode == 1)
			{
				return false;
			}
			string nngIpcTdAddress = result.get<string>("nngIpcTdAddress");
			string nngIpcTdResAddress = result.get<string>("nngIpcTdResAddress");
			tdnnsocket.bind(nngIpcTdAddress.c_str());
			tdnnsocket_resp.bind(nngIpcTdResAddress.c_str());
		}

		/* 启动 */
		if (!Start()) {
			//fprintf(stderr, "启动API失败!\n");
			return false;
		}

		return true;
	}

	void* CZpquantTradeApi::tradeThreadMain(void *pParams)
	{
		CZpquantTradeApi *tdapi = (CZpquantTradeApi *)pParams;
		CZpquantTradeSpi *tdspi = tdapi->pSpi;
		char buf[4096];
		while (1)
		{
			int rc = tdapi->tdnnsocket_resp.recv(buf, sizeof(buf), 0);
			cout << "...CZpquantTradeApi,tradeThreadMain recv: " << buf << endl;

			ptree c_Config;
			bool isAbleParseJson = true;
			std::stringstream jmsg(buf);
			try {
				boost::property_tree::read_json(jmsg, c_Config);
			}
			catch (std::exception & e) {
				fprintf(stdout, "cannot parse from string 'msg(client,oes_resp)' \n");
				//return false;
				isAbleParseJson = false;
			}

			int32 msgId = 0;
			if (isAbleParseJson) { //正确解析
				msgId = c_Config.get<int32>("msgId");
			}
			else {
				msgId = 33; //解析有问题
			}

			cout << "...msgId:" << msgId << endl;
			switch (msgId)
			{
				//查询持仓
				/*
				{"msgId":100, "seqNo":1,"isEnd":"Y", "invAcctId":"A188800368","securityId":"600000","mktId":1,
				"originalHld":1000000, "totalBuyHld":0,"totalSellHld":0,"sellFrzHld":0, "totalTrsfInHld":0,
				"totalTrsfOutHld":0,"trsfOutFrzHld":0,"lockHld":0, "lockFrzHld":0,"unlockFrzHld":0,"coveredFrzHld":0,
				"coveredHld":0, "coveredAvlHld":0,"sumHld":1000000,"sellAvlHld":1000000,"trsfOutAvlHld":1000000,"lockAvlHld":1000000}
				  */
			case OESMSG_QRYMSG_STK_HLD:
			{
				//已经做了修复2019.8.14
				ZpquantStkHoldingItem msgBody;
				ZpquantQryCursor pCursor;

				pCursor.seqNo = c_Config.get<int32>("seqNo");
				cout << "...seqNo:" << pCursor.seqNo << endl;

				char isEnd = c_Config.get<char>("isEnd");
				cout << "...isEnd:" << isEnd << endl;
				pCursor.isEnd = isEnd;
				string invAcctId = c_Config.get<string>("invAcctId");
				string securityId = c_Config.get<string>("securityId");
				memset(msgBody.securityId, 0, 16 * sizeof(char));
				if (securityId.length() > 0) memcpy(msgBody.securityId, securityId.c_str(), 16 * sizeof(char));
				memset(msgBody.invAcctId, 0, 16 * sizeof(char));
				if (invAcctId.length() > 0) memcpy(msgBody.invAcctId, invAcctId.c_str(), 16 * sizeof(char));
				
				msgBody.mktId = c_Config.get<uint8>("mktId");
				msgBody.originalHld = c_Config.get<int64>("originalHld");
				msgBody.totalBuyHld = c_Config.get<int64>("totalBuyHld");
				msgBody.totalSellHld = c_Config.get<int64>("totalSellHld");
				msgBody.sellFrzHld = c_Config.get<int64>("sellFrzHld");
				msgBody.totalTrsfInHld = c_Config.get<int64>("totalTrsfInHld");
				msgBody.totalTrsfOutHld = c_Config.get<int64>("totalTrsfOutHld");
				msgBody.trsfOutFrzHld = c_Config.get<int64>("trsfOutFrzHld");
				msgBody.lockHld = c_Config.get<int64>("lockHld");
				msgBody.lockFrzHld = c_Config.get<int64>("lockFrzHld");
				msgBody.unlockFrzHld = c_Config.get<int64>("unlockFrzHld");
				msgBody.coveredFrzHld = c_Config.get<int64>("coveredFrzHld");
				msgBody.coveredHld = c_Config.get<int64>("coveredHld");
				msgBody.coveredAvlHld = c_Config.get<int64>("coveredAvlHld");
				msgBody.sumHld = c_Config.get<int64>("sumHld");
				msgBody.sellAvlHld = c_Config.get<int64>("sellAvlHld");
				msgBody.trsfOutAvlHld = c_Config.get<int64>("trsfOutAvlHld");
				msgBody.lockAvlHld = c_Config.get<int64>("lockAvlHld");
				int32 requestId = c_Config.get<int32>("requestId");

				tdspi->OnQueryStkHolding(&msgBody, &pCursor, requestId);
				break;
			}
			/*
			...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"7","clEnvId":0,"clSeqNo":2,"clOrdId":1375,"invAcctId":"A188800368",
			 securityId":"601899","mktId":1,"ordType":0,"bsType":1, ordStatus":1,"ordDate":20190430,"ordTime":145428627,"ordCnfmTime":0,
			 ordQty":200,"ordPrice":33000,"canceledQty":0,"cumQty":0, cumAmt":0,"cumInterest":0,"cumFee":0,"cumInterest":6600000, cumFee":0,
			 "frzAmt":50132,"frzInterest":0,"frzFee":0, origClOrdId":0,"ordRejReason":0,"exchErrCode":0, }
			cannot parse from string 'msg(client,oes_resp)'
			...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"12","cashAcctId":"1888000368","custId":"1888000368","currType":0,
			"cashType":0, cashAcctStatus":0,"beginningBal":1316134912,"beginningAvailableBal":1316134912,"beginningDrawableBal":1316134912,
			disableBal":0,"totalDepositAmt":0,"totalWithdrawAmt":0,"withdrawFrzAmt":0, totalSellAmt":0,"totalBuyAmt":3300000,
			"buyFrzAmt":9900000,"totalFeeAmt":50066, feeFrzAmt":50198,"marginAmt":0,"marginFrzAmt":0,"currentTotalBal":1312784846,
			currentAvailableBal":1302834648,"currentDrawableBal":1302834648 ,}
			cannot parse from string 'msg(client,oes_resp)'
			...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"8","clEnvId":0,"clSeqNo":2,"clOrdId":1375,"invAcctId":"A188800368",
			 securityId":"601899","mktId":1,"ordType":0,"bsType":1, ordStatus":3,"ordDate":20190430,"ordTime":145428627,"ordCnfmTime":145428627,
			 ordQty":200,"ordPrice":33000,"canceledQty":0,"cumQty":100, cumAmt":3300000,"cumInterest":0,"cumFee":50066, cumFee":3300000,"frzAmt":0,
			 "frzInterest":66,"frzFee":0, origClOrdId":0,"ordRejReason":0,"exchErrCode":1302834648, }
			cannot parse from string 'msg(client,oes_resp)'
			...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"9","exchTrdNum":1981,"clOrdId":1375,"clEnvId":0,"clSeqNo":2,
			"invAcctId":"A188800368", securityId":"601899","mktId":1,"trdSide":1, ordBuySellType":1,"trdDate":20190430,"trdTime":145428627,
			trdQty":100,"trdPrice":33000,"trdAmt":3300000,"cumQty":100, cumAmt":3300000,"cumInterest":0,cumFee":50066,"pbuId":88822, }
			cannot parse from string 'msg(client,oes_resp)'
			...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"12","cashAcctId":"1888000368","custId":"1888000368","currType":0,
			"cashType":0, cashAcctStatus":0,"beginningBal":1316134912,"beginningAvailableBal":1316134912,"beginningDrawableBal":1316134912,
			 disableBal":0,"totalDepositAmt":0,"totalWithdrawAmt":0,"withdrawFrzAmt":0, totalSellAmt":0,"totalBuyAmt":6600000,"buyFrzAmt":6600000,
			 "totalFeeAmt":100132, feeFrzAmt":132,"marginAmt":0,"marginFrzAmt":0,"currentTotalBal":1309434780, currentAvailableBal":1302834648,
			 "currentDrawableBal":1302834648 ,}
			cannot parse from string 'msg(client,oes_resp)'
			...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"13","invAcctId":"A188800368","securityId":"601899","mktId":1,
			"originalHld":0, totalBuyHld":200,"totalSellHld":0,"sellFrzHld":0,"manualFrzHld":0, totalTrsfInHld":0,"totalTrsfOutHld":0,
			"trsfOutFrzHld":0,"lockHld":0, lockFrzHld":0,"unlockFrzHld":0,"coveredFrzHld":0,"coveredHld":0, originalCostAmt":0,
			"totalBuyAmt":6600000,"totalSellAmt":0,"totalBuyFee":100132, totalSellFee":0,"costPrice":33501,"sumHld":200,"sellAvlHld":0,
			trsfOutAvlHld":200,"lockAvlHld":200,"coveredAvlHld":0, }
			cannot parse from string 'msg(client,oes_resp)'
			*/

			/* 委托已收回报 114*/
			/*...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"7","clEnvId":0,"clSeqNo":2,"clOrdId":1375,"invAcctId":"A188800368",
			 securityId":"601899","mktId":1,"ordType":0,"bsType":1, ordStatus":1,"ordDate":20190430,"ordTime":145428627,"ordCnfmTime":0,
			 ordQty":200,"ordPrice":33000,"canceledQty":0,"cumQty":0, cumAmt":0,"cumInterest":0,"cumFee":0,"cumInterest":6600000, cumFee":0,
			 "frzAmt":50132,"frzInterest":0,"frzFee":0, origClOrdId":0,"ordRejReason":0,"exchErrCode":0, }*/
			case OESMSG_RPT_ORDER_INSERT:
				/* 委托确认回报 115*/
				/*
				...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"8","clEnvId":0,"clSeqNo":2,"clOrdId":1375,"invAcctId":"A188800368",
				 securityId":"601899","mktId":1,"ordType":0,"bsType":1, ordStatus":3,"ordDate":20190430,"ordTime":145428627,"ordCnfmTime":145428627,
				 ordQty":200,"ordPrice":33000,"canceledQty":0,"cumQty":100, cumAmt":3300000,"cumInterest":0,"cumFee":50066, cumFee":3300000,"frzAmt":0,
				 "frzInterest":66,"frzFee":0, origClOrdId":0,"ordRejReason":0,"exchErrCode":1302834648, }
				*/
			case OESMSG_RPT_ORDER_REPORT:
			{
				//已经做了修复2019.8.14
				ZpquantOrdCnfm msgBody;

				msgBody.clSeqNo = c_Config.get<int32>("clSeqNo");
				msgBody.mktId = c_Config.get<uint8>("mktId");
				msgBody.ordType = c_Config.get<uint8>("ordType");
				msgBody.bsType = c_Config.get<uint8>("bsType");

				string invAcctId = c_Config.get<string>("invAcctId");
				memset(msgBody.invAcctId, 0, 16 * sizeof(char));
				if (invAcctId.length() > 0) memcpy(msgBody.invAcctId, invAcctId.c_str(), 16 * sizeof(char));

				string securityId = c_Config.get<string>("securityId");
				memset(msgBody.securityId, 0, 16 * sizeof(char));
				if (securityId.length() > 0) memcpy(msgBody.securityId, securityId.c_str(), 16 * sizeof(char));

				msgBody.ordQty = c_Config.get<int32>("ordQty");
				msgBody.ordPrice = c_Config.get<int32>("ordPrice");
				msgBody.origClEnvId = c_Config.get<int8>("origClEnvId");
				msgBody.origClSeqNo = c_Config.get<int32>("origClSeqNo");
				msgBody.origClOrdId = c_Config.get<int64>("origClOrdId");
				msgBody.clOrdId = c_Config.get<int64>("clOrdId");

				msgBody.clEnvId = c_Config.get<int8>("clEnvId");
				msgBody.ordDate = c_Config.get<int32>("ordDate");
				msgBody.ordTime = c_Config.get<int32>("ordTime");
				msgBody.ordCnfmTime = c_Config.get<int32>("ordCnfmTime");

				msgBody.ordStatus = c_Config.get<uint8>("ordStatus");
				//msgBody.ordCnfmSts = c_Config.get<uint8>("ordCnfmSts");
				//msgBody.securityType = c_Config.get<uint8>("securityType");
				//string exchOrdId = c_Config.get<string>("exchOrdId");

//				auto scode = c_Config.find("frzAmt");
//				string code;
//				if (scode != c_Config.not_found()) code = scode->second.data();

				msgBody.frzAmt = c_Config.get<int64>("frzAmt");
				msgBody.frzInterest = c_Config.get<int64>("frzInterest");
				msgBody.frzFee = c_Config.get<int64>("frzFee");
				msgBody.cumAmt = c_Config.get<int64>("cumAmt");
				msgBody.cumInterest = c_Config.get<int64>("cumInterest");

				msgBody.cumFee = c_Config.get<int64>("cumFee");
				msgBody.cumQty = c_Config.get<int32>("cumQty");
				msgBody.canceledQty = c_Config.get<int32>("canceledQty");
				msgBody.ordRejReason = c_Config.get<int32>("ordRejReason");
				msgBody.exchErrCode = c_Config.get<int32>("exchErrCode");

				if (msgId == OESMSG_RPT_ORDER_INSERT)
				{
					tdspi->OnOrderInsert(&msgBody);
				}
				else /* 委托确认回报 */
				{
					tdspi->OnOrderReport(0, &msgBody);
				}
				break;
			}
			/*
			...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"12","cashAcctId":"1888000368","custId":"1888000368","currType":0,
			"cashType":0, cashAcctStatus":0,"beginningBal":1316134912,"beginningAvailableBal":1316134912,"beginningDrawableBal":1316134912,
			disableBal":0,"totalDepositAmt":0,"totalWithdrawAmt":0,"withdrawFrzAmt":0, totalSellAmt":0,"totalBuyAmt":3300000,
			"buyFrzAmt":9900000,"totalFeeAmt":50066, feeFrzAmt":50198,"marginAmt":0,"marginFrzAmt":0,"currentTotalBal":1312784846,
			currentAvailableBal":1302834648,"currentDrawableBal":1302834648 ,}
			*/
			/* 资金变动回报 119*/
			case OESMSG_RPT_CASH_ASSET_VARIATION:
			{
				ZpquantCashAssetItem msgBody;

				string cashAcctId = c_Config.get<string>("cashAcctId");
				string custId = c_Config.get<string>("custId");
				memset(msgBody.cashAcctId, 0, 16 * sizeof(char));
				if (cashAcctId.length() > 0) memcpy(msgBody.cashAcctId, cashAcctId.c_str(), 16 * sizeof(char));
				memset(msgBody.custId, 0, 16 * sizeof(char));
				if (custId.length() > 0) memcpy(msgBody.custId, custId.c_str(), 16 * sizeof(char));

				msgBody.currType = c_Config.get<uint8>("currType");
				msgBody.cashType = c_Config.get<uint8>("cashType");
				msgBody.cashAcctStatus = c_Config.get<uint8>("cashAcctStatus");

				msgBody.beginningBal = c_Config.get<int64>("beginningBal");
				msgBody.beginningAvailableBal = c_Config.get<int64>("beginningAvailableBal");
				msgBody.beginningDrawableBal = c_Config.get<int64>("beginningDrawableBal");
				msgBody.disableBal = c_Config.get<int64>("disableBal");
				msgBody.totalDepositAmt = c_Config.get<int64>("totalDepositAmt");

				msgBody.totalWithdrawAmt = c_Config.get<int64>("totalWithdrawAmt");
				msgBody.withdrawFrzAmt = c_Config.get<int64>("withdrawFrzAmt");
				msgBody.totalBuyAmt = c_Config.get<int64>("totalBuyAmt");
				msgBody.totalSellAmt = c_Config.get<int64>("totalSellAmt");
				msgBody.buyFrzAmt = c_Config.get<int64>("buyFrzAmt");
				msgBody.totalFeeAmt = c_Config.get<int64>("totalFeeAmt");

				msgBody.feeFrzAmt = c_Config.get<int64>("feeFrzAmt");
				msgBody.marginAmt = c_Config.get<int64>("marginAmt");
				msgBody.marginFrzAmt = c_Config.get<int64>("marginFrzAmt");
				msgBody.currentTotalBal = c_Config.get<int64>("currentTotalBal");
				msgBody.currentAvailableBal = c_Config.get<int64>("currentAvailableBal");
				msgBody.currentDrawableBal = c_Config.get<int64>("currentDrawableBal");

				tdspi->OnCashAssetVariation(&msgBody);
				break;
			}
			/*
			...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":"9","exchTrdNum":1981,"clOrdId":1375,"clEnvId":0,"clSeqNo":2,
			"invAcctId":"A188800368", securityId":"601899","mktId":1,"trdSide":1, ordBuySellType":1,"trdDate":20190430,"trdTime":145428627,
			trdQty":100,"trdPrice":33000,"trdAmt":3300000,"cumQty":100, cumAmt":3300000,"cumInterest":0,cumFee":50066,"pbuId":88822, }
			cannot parse from string 'msg(client,oes_resp)'
			*/
			/* 成交回报 116*/
			case OESMSG_RPT_TRADE_REPORT:
			{
				ZpquantTrdCnfm msgBody;

				string invAcctId = c_Config.get<string>("invAcctId");
				string securityId = c_Config.get<string>("securityId");
				memset(msgBody.securityId, 0, 16 * sizeof(char));
				if (securityId.length() > 0) memcpy(msgBody.securityId, securityId.c_str(), 16 * sizeof(char));
				memset(msgBody.invAcctId, 0, 16 * sizeof(char));
				if (invAcctId.length() > 0) memcpy(msgBody.invAcctId, invAcctId.c_str(), 16 * sizeof(char));

				msgBody.exchTrdNum = c_Config.get<int64>("exchTrdNum");
				msgBody.clOrdId = c_Config.get<int64>("clOrdId");
				msgBody.clEnvId = c_Config.get<int8>("clEnvId");
				msgBody.clSeqNo = c_Config.get<int32>("clSeqNo");

				msgBody.mktId = c_Config.get<uint8>("mktId");
				msgBody.trdSide = c_Config.get<uint8>("trdSide");
				msgBody.ordBuySellType = c_Config.get<uint8>("ordBuySellType");
				msgBody.trdDate = c_Config.get<int32>("trdDate");

				msgBody.trdTime = c_Config.get<int32>("trdTime");
				msgBody.trdQty = c_Config.get<int32>("trdQty");
				msgBody.trdPrice = c_Config.get<int32>("trdPrice");
				msgBody.trdAmt = c_Config.get<int64>("trdAmt");
				msgBody.cumQty = c_Config.get<int32>("cumQty");
				msgBody.cumAmt = c_Config.get<int64>("cumAmt");
				msgBody.cumInterest = c_Config.get<int64>("cumInterest");

				msgBody.cumFee = c_Config.get<int64>("cumFee");
				msgBody.pbuId = c_Config.get<int32>("pbuId");

				tdspi->OnTradeReport(&msgBody);
				break;
			}
			/* 持仓变动回报 */
			/*
			...client,oes_resp...subscribe,topic:oes_resp,msg: {"msgId":120,"invAcctId":"A188800368","securityId":"601899",
			"mktId":1,"originalHld":0, "totalBuyHld":1400,"totalSellHld":0,"sellFrzHld":0,"manualFrzHld":0,
			"totalTrsfInHld":0,"totalTrsfOutHld":0,"trsfOutFrzHld":0,"lockHld":0, "lockFrzHld":0,"unlockFrzHld":0,
			"coveredFrzHld":0,"coveredHld":0, "originalCostAmt":0,"totalBuyAmt":45830000,"totalSellAmt":0,
			"totalBuyFee":350915, "totalSellFee":0,"costPrice":32986,"sumHld":1400,"sellAvlHld":0, "trsfOutAvlHld":1400,
			"lockAvlHld":1400,"coveredAvlHld":0}
			*/
			case OESMSG_RPT_STOCK_HOLDING_VARIATION:
			{
				//已经做了修复2019.8.14
				ZpquantStkHoldingItem msgBody;

				string invAcctId = c_Config.get<string>("invAcctId");
				string securityId = c_Config.get<string>("securityId");
				memset(msgBody.securityId, 0, 16 * sizeof(char));
				if (securityId.length() > 0) memcpy(msgBody.securityId, securityId.c_str(), 16 * sizeof(char));
				memset(msgBody.invAcctId, 0, 16 * sizeof(char));
				if (invAcctId.length() > 0) memcpy(msgBody.invAcctId, invAcctId.c_str(), 16 * sizeof(char));

				msgBody.mktId = c_Config.get<uint8>("mktId");
				msgBody.originalHld = c_Config.get<int64>("originalHld");
				msgBody.totalBuyHld = c_Config.get<int64>("totalBuyHld");
				msgBody.totalSellHld = c_Config.get<int64>("totalSellHld");
				msgBody.sellFrzHld = c_Config.get<int64>("sellFrzHld");
				msgBody.totalTrsfInHld = c_Config.get<int64>("totalTrsfInHld");
				msgBody.totalTrsfOutHld = c_Config.get<int64>("totalTrsfOutHld");
				msgBody.trsfOutFrzHld = c_Config.get<int64>("trsfOutFrzHld");
				msgBody.lockHld = c_Config.get<int64>("lockHld");
				msgBody.lockFrzHld = c_Config.get<int64>("lockFrzHld");
				msgBody.unlockFrzHld = c_Config.get<int64>("unlockFrzHld");
				msgBody.coveredFrzHld = c_Config.get<int64>("coveredFrzHld");
				msgBody.coveredHld = c_Config.get<int64>("coveredHld");
				msgBody.coveredAvlHld = c_Config.get<int64>("coveredAvlHld");
				msgBody.sumHld = c_Config.get<int64>("sumHld");
				msgBody.sellAvlHld = c_Config.get<int64>("sellAvlHld");
				msgBody.trsfOutAvlHld = c_Config.get<int64>("trsfOutAvlHld");
				msgBody.lockAvlHld = c_Config.get<int64>("lockAvlHld");

				tdspi->OnStockHoldingVariation(&msgBody);
				break;
			}
			//关于个股的信息，包括涨跌停等信息
			case OESMSG_QRYMSG_STOCK:
			{
				ZpquantStockBaseInfo msgBody;
				ZpquantQryCursor pCursor;

				pCursor.seqNo = c_Config.get<int32>("seqNo");
				pCursor.isEnd = c_Config.get<char>("isEnd");
				int32 requestId = c_Config.get<int32>("requestId");

				string securityId = c_Config.get<string>("securityId");
				memset(msgBody.securityId, 0, 16 * sizeof(char));
				if (securityId.length() > 0) memcpy(msgBody.securityId, securityId.c_str(), 16 * sizeof(char));

				msgBody.mktId = c_Config.get<uint8>("mktId");
				msgBody.securityType = c_Config.get<uint8>("securityType");
				msgBody.subSecurityType = c_Config.get<uint8>("subSecurityType");
				msgBody.securityLevel = c_Config.get<uint8>("securityLevel");
				msgBody.securityRiskLevel = c_Config.get<uint8>("securityRiskLevel");
				msgBody.currType = c_Config.get<uint8>("currType");
				msgBody.qualificationClass = c_Config.get<uint8>("qualificationClass");
				msgBody.isDayTrading = c_Config.get<uint8>("isDayTrading");
				msgBody.suspFlag = c_Config.get<uint8>("suspFlag");
				msgBody.temporarySuspFlag = c_Config.get<uint8>("temporarySuspFlag");

				msgBody.buyQtyUnit = c_Config.get<int32>("buyQtyUnit");
				msgBody.sellQtyUnit = c_Config.get<int32>("sellQtyUnit");
				msgBody.buyOrdMaxQty = c_Config.get<int32>("buyOrdMaxQty");
				msgBody.buyOrdMinQty = c_Config.get<int32>("buyOrdMinQty");
				msgBody.sellOrdMaxQty = c_Config.get<int32>("sellOrdMaxQty");
				msgBody.sellOrdMinQty = c_Config.get<int32>("sellOrdMinQty");
				msgBody.priceUnit = c_Config.get<int32>("priceUnit");
				msgBody.prevClose = c_Config.get<int32>("prevClose");
				msgBody.parPrice = c_Config.get<int64>("parPrice");
				msgBody.bondInterest = c_Config.get<int64>("bondInterest");
				msgBody.repoExpirationDays = c_Config.get<int32>("repoExpirationDays");
				msgBody.cashHoldDays = c_Config.get<int32>("cashHoldDays");

				//string securityName = c_Config.get<string>("securityName");
				//memset(msgBody.securityName, 0, 24 * sizeof(char));
				//if (securityName.length() > 0) memcpy(msgBody.securityName, securityName.c_str(), 24 * sizeof(char));

				string fundId = c_Config.get<string>("fundId");
				memset(msgBody.fundId, 0, 16 * sizeof(char));
				if (fundId.length() > 0) memcpy(msgBody.fundId, fundId.c_str(), 16 * sizeof(char));

				msgBody.priceLimit[0].ceilPrice = c_Config.get<int32>("ceilPrice_O");
				msgBody.priceLimit[0].floorPrice = c_Config.get<int32>("floorPrice_O");
				msgBody.priceLimit[1].ceilPrice = c_Config.get<int32>("ceilPrice_T");
				msgBody.priceLimit[1].floorPrice = c_Config.get<int32>("floorPrice_T");
				msgBody.priceLimit[2].ceilPrice = c_Config.get<int32>("ceilPrice_C");
				msgBody.priceLimit[2].floorPrice = c_Config.get<int32>("floorPrice_C");

				tdspi->OnQueryStock(&msgBody,&pCursor,requestId);
				break;
			}
			case 33:
				cout << "...CZpquantTradeApi,tradeThreadMain recv,parse json error!!! : " << buf << endl;
				break;
			default:
				break;
			}
		}
	}

	void* CZpquantTradeApi::tradeReqThreadMain(void *pParams)
	{
		CZpquantTradeApi *tdapi = (CZpquantTradeApi *)pParams;
		CZpquantTradeSpi *tdspi = tdapi->pSpi;
		char buf[1024];
		while (1)
		{
			int rc = tdapi->tdnnsocket.recv(buf, sizeof(buf), 0);
			cout << "...CZpquantTradeApi,tradeReqThreadMain recv: " << buf << endl;

			ptree c_Config;
			bool isAbleParseJson = true;
			std::stringstream jmsg(buf);
			try {
				boost::property_tree::read_json(jmsg, c_Config);
			}
			catch (std::exception & e) {
				fprintf(stdout, "cannot parse from string 'msg(client,oes)' \n");
				//return false;
				isAbleParseJson = false;
			}

			string type = c_Config.get<string>("type");
			if (type == "tradingDay")
			{
				ZpquantTradingDayInfo msgbody;
				msgbody.tradingDay = c_Config.get<int32>("value");
				msgbody.isTradingDay = c_Config.get<uint8>("isTradingDay");

				tdspi->OnGetTradingDay(&msgbody);
			}
			else if(type == "stockAmountCheck"
				|| type == "selfmake-transaction"
				|| type == "turnover"
				|| type == "cancelOrderCheck")
			{
				string newOrder = c_Config.get<string>("newOrder");
				string oldOrder = c_Config.get<string>("oldOrder");
				uint8 isRejected = c_Config.get<uint8>("isRejected");
				bool isFlag = isRejected ? true : false;

				tdspi->OnGetOrderResultFromRiskModel(type.c_str(),newOrder.c_str(),oldOrder.c_str(),isFlag);
			}
			else
			{

			}
		}
	}

	//采集报单回调数据
	bool
		CZpquantTradeApi::Start()
	{
		//pthread_t       rptThreadId;
		//int32           ret = 0;

		/* 创建回报接收线程 */
		//ret = pthread_create(&rptThreadId, NULL, tradeThreadMain, (void *) this);
		//if (ret != 0) {
		//    fprintf(stderr, "创建交易回调接收线程失败! error[%d - %s]\n",
		//            ret, strerror(ret));
		//    return false;
		//}

		/* 创建回报接收线程 */
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tradeThreadMain, (LPVOID)this, 0, NULL);
		//client端主动发起请求后的回调线程
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tradeReqThreadMain, (LPVOID)this, 0, NULL);

		return true;
	}

	void
		CZpquantTradeApi::Stop() {

	}

	/**
	 * 发送委托申报请求
	 */
	/*
	int32
		CZpquantTradeApi::SendOrder(const ZpquantOrdReqT *pOrderReq)
	{
		string buyorsell;
		if (pOrderReq->bsType == 1) //买入
		{
			buyorsell = "buy";
		}
		else if (pOrderReq->bsType == 2) { //卖出
			buyorsell = "sell";
		}
		else { //其他
			buyorsell = "other";
		}

		char sendJsonDataStr[512];
		sprintf(sendJsonDataStr,
			"{\"type\":\"%s\",\"code\":\"%s\",\"sclb\":\"%d\",\"wtfs\":\"%d\",\"amount\":\"%d\",\"price\":\"%d\"}",
			buyorsell.c_str(), pOrderReq->pSecurityId, pOrderReq->mktId, pOrderReq->ordType, pOrderReq->ordQty, pOrderReq->ordPrice);
		cout << "...SendOrder...sendJsonDataStr: " << sendJsonDataStr << endl;
		tdnnsocket.send(sendJsonDataStr, strlen(sendJsonDataStr) + 1, 0);

		return 0;
	}
	*/

	int32
		CZpquantTradeApi::SendOrder(const char *securityId, uint8 mktId, uint8 bsType, int32 ordPrice, int32 ordQty, uint8 ordType)
	{
		string buyorsell;
		if (bsType == 1) //买入
		{
			buyorsell = "buy";
		}
		else if (bsType == 2) { //卖出
			buyorsell = "sell";
		}
		else { //其他
			buyorsell = "other";
		}

		char sendJsonDataStr[512];
		sprintf(sendJsonDataStr,
			"{\"type\":\"%s\",\"code\":\"%s\",\"sclb\":\"%d\",\"wtfs\":\"%d\",\"amount\":\"%d\",\"price\":\"%d\"}",
			buyorsell.c_str(), securityId, mktId, ordType, ordQty, ordPrice);
		cout << "...SendOrder...sendJsonDataStr: " << sendJsonDataStr << endl;
		tdnnsocket.send(sendJsonDataStr, strlen(sendJsonDataStr) + 1, 0);

		return 0;
	}

	/**
	 * 发送撤单请求
	 */
	/*
	int32
		CZpquantTradeApi::SendCancelOrder(const ZpquantOrdCancelReqT *pCancelReq)
	{
		char sendJsonDataStr[256];
		string cancelOrd("cancelOrder");
		sprintf(sendJsonDataStr,
			"{\"type\":\"%s\",\"mktId\":%d,\"origClSeqNo\":%d,\"origClEnvId\":%d,\"origClOrdId\":%d}",
			cancelOrd.c_str(), pCancelReq->mktId, pCancelReq->origClSeqNo, pCancelReq->origClEnvId, pCancelReq->origClOrdId);
		cout << "...SendCancelOrder...sendJsonDataStr: " << sendJsonDataStr << endl;
		tdnnsocket.send(sendJsonDataStr, strlen(sendJsonDataStr) + 1, 0);
		return 0;
	}
	*/

	int32
		CZpquantTradeApi::SendCancelOrder(uint8 mktId, int32 origClSeqNo, int8 origClEnvId, int64 origClOrdId)
	{
		char sendJsonDataStr[256];
		string cancelOrd("cancelOrder");
		sprintf(sendJsonDataStr,
			"{\"type\":\"%s\",\"mktId\":%d,\"origClSeqNo\":%d,\"origClEnvId\":%d,\"origClOrdId\":%d}",
			cancelOrd.c_str(), mktId, origClSeqNo, origClEnvId, origClOrdId);
		cout << "...SendCancelOrder...sendJsonDataStr: " << sendJsonDataStr << endl;
		tdnnsocket.send(sendJsonDataStr, strlen(sendJsonDataStr) + 1, 0);
		return 0;
	}

	/**
	 * 获取API的发行版本号
	 *
	 * @return  API的发行版本号 (如: "0.15.3")
	 */
	const char *
		CZpquantTradeApi::GetVersion(void) {
		string vStr = "1";
		return vStr.c_str();
	}


	/**
	 * 获取当前交易日
	 *
	 * @retval  >0          当前交易日 (格式：YYYYMMDD)
	 * @retval  0           尚未连接查询服务
	 * @retval  <0          失败 (负的错误号)
	 */
	int32
	CZpquantTradeApi::GetTradingDay() {
		string sendStr = "{\"type\":\"GetTradingDay\"}";
		std::cout << "..td..GetTradingDay: " << sendStr.c_str() << endl;
		tdnnsocket.send(sendStr.c_str(), strlen(sendStr.c_str()) + 1, 0);
		return 0;
	}

	/*
	 * 查询客户资金信息
	 */
	int32
		CZpquantTradeApi::QueryCashAsset(int32 requestId)
	{
		char sendJsonDataStr[256];
		sprintf(sendJsonDataStr, "{\"type\":\"query\",\"category\":\"cashAsset\",\"code\":\"\",\"sclb\":\"\",\"requestId\":%d}", requestId);
		cout << "...QueryCashAsset...sendJsonDataStr: " << sendJsonDataStr << endl;
		tdnnsocket.send(sendJsonDataStr, strlen(sendJsonDataStr) + 1, 0);
		return 0;
	}

	//查询 上证或深圳 股票的持仓
	int32
		CZpquantTradeApi::QueryStkHolding(const char *securityId, uint8 mktId, int32 requestId)
	{
		char sendJsonDataStr[256];
		sprintf(sendJsonDataStr,
			"{\"type\":\"query\",\"category\":\"stkHolding\",\"code\":\"%s\",\"sclb\":\"%d\",\"requestId\":%d}", securityId, mktId, requestId);
		cout << "...stkHolding...sendJsonDataStr: " << sendJsonDataStr << endl;
		tdnnsocket.send(sendJsonDataStr, strlen(sendJsonDataStr) + 1, 0);
		return 0;
	}

	/**
	 * 查询现货产品信息
	 */
	int32
		CZpquantTradeApi::QueryStock(const char *securityId, uint8 mktId, int32 requestId)
	{
		char sendJsonDataStr[256];
		sprintf(sendJsonDataStr,
			"{\"type\":\"query\",\"category\":\"stkInfo\",\"code\":\"%s\",\"sclb\":\"%d\",\"requestId\":%d}", securityId, mktId, requestId);
		cout << "...stkInfo...sendJsonDataStr: " << sendJsonDataStr << endl;
		tdnnsocket.send(sendJsonDataStr, strlen(sendJsonDataStr) + 1, 0);
		return 0;
	}

}

