#include    <iostream>
#include    "oes_client_spi.h"

#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/json_parser.hpp> 
using namespace boost::property_tree;
using namespace zpquant;

#define SOCKET_ADDRESS "tcp://47.105.111.100:8002"
#define ADDRESS1 "inproc://test"
#define ADDRESS7 "ipc:///tmp/reqrepresp.ipc"


OesClientMySpi::OesClientMySpi() {
	spisocket.socket_set(AF_SP, NN_PAIR);
	//spisocket.connect(ADDRESS7);
	sendRespData2client[0] = '\0';
	Redis.Connect();
}

OesClientMySpi::~OesClientMySpi() {
	/* do nothing */
}

void OesClientMySpi::initTdResclient(string username, ClientListTdResInfoT tdResT)
{
	tdResclient.insert(pair<string, ClientListTdResInfoT>(username, tdResT));
	spisocket.connect(tdResT.nngIpcTdResAddress);
	//多用户时需要重新考虑
	m_strUsername = username;
	tdResInfoT = tdResT;
}


/* 委托业务拒绝回报 */
void
OesClientMySpi::OnBusinessReject(int32 errorCode, const OesOrdRejectT *pOrderReject) {
      // sprintf(sendJsonDataStr, ">>> 收到委托业务拒绝回报: " \
      //       "客户端环境号[%" __SPK_FMT_HH__ "d], " \
      //       "客户委托流水号[%d], 证券账户[%s], 证券代码[%s], " \
      //       "市场代码[%" __SPK_FMT_HH__ "u], 委托类型[%" __SPK_FMT_HH__ "u], " \
      //       "买卖类型[%" __SPK_FMT_HH__ "u], 委托数量[%d], 委托价格[%d], " \
      //       "原始委托的客户订单编号[%" __SPK_FMT_LL__ "d], 错误码[%d]\n",
      //       pOrderReject->clEnvId, pOrderReject->clSeqNo,
      //       pOrderReject->invAcctId, pOrderReject->securityId,
      //       pOrderReject->mktId, pOrderReject->ordType,
      //       pOrderReject->bsType, pOrderReject->ordQty,
      //       pOrderReject->ordPrice, pOrderReject->origClOrdId, errorCode);

      // fprintf(stdout, sendJsonDataStr);
	/*
      sprintf(sendRespData2client, 
                  "{\"msgId\":%d,\"clEnvId\":%d,\"clSeqNo\":%d,\"invAcctId\":\"%s\", " \
                  "securityId\":\"%s\",\"mktId\":%d,\"ordType\":%d,\"bsType\":%d, " \
                  "ordQty\":%d,\"ordPrice\":%d, " \
                  "origClOrdId\":%d,\"errorCode\":%d" \
                  "}",
                  6,pOrderReject->clEnvId, pOrderReject->clSeqNo,
                  pOrderReject->invAcctId, pOrderReject->securityId,
                  pOrderReject->mktId, pOrderReject->ordType,
                  pOrderReject->bsType, pOrderReject->ordQty,
                  pOrderReject->ordPrice, pOrderReject->origClOrdId, errorCode);*/

      //publisher.publish("oes_resp",sendRespData2client);
      //spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);

	  ptree pt;
	  pt.put("msgId", 6);
	  pt.put("clEnvId", pOrderReject->clEnvId);
	  pt.put("clSeqNo", pOrderReject->clSeqNo);

	  pt.put("invAcctId", pOrderReject->invAcctId);
	  pt.put("securityId", pOrderReject->securityId);
	  pt.put("mktId", pOrderReject->mktId);
	  pt.put("ordType", pOrderReject->ordType);
	  pt.put("bsType", pOrderReject->bsType);
	  pt.put("ordQty", pOrderReject->ordQty);
	  pt.put("ordPrice", pOrderReject->ordPrice);
	  pt.put("origClOrdId", pOrderReject->origClOrdId);
	  pt.put("errorCode", errorCode);
	  pt.put("username", m_strUsername);

	  std::ostringstream buf;
	  write_json(buf, pt, false);
	  sprintf(sendRespData2client, "%s", buf.str().c_str());
	  spisocket.send(sendRespData2client, strlen(sendRespData2client) + 1, 0);

	  //存入内存中
	  char keyName[60];
	  keyName[0] = '\0';
	  sprintf(keyName, "jjkr-%d-%d-%d", 6, pOrderReject->clEnvId, pOrderReject->clSeqNo);
	  //const string key = keyName;
	  //const string value = sendRespData2client;
	  //Redis.Set(key, value);
	  Redis.Set(keyName, sendRespData2client);
	  fprintf(stdout, keyName);

	  /*
	  //TODO 优化字段存储，减少不必要的字段
	  tdResInfoT.rejectedlistv.push_back(sendRespData2client);
	  //test之用
	  int size = tdResInfoT.rejectedlistv.size();
	  if (size > 10)
	  {
		  for (int i = 0; i < size; i++)//方法一 
		  {
			  fprintf(stdout, tdResInfoT.rejectedlistv[i].c_str());
		  }

		  tdResInfoT.rejectedlistv.clear();
	  }
	  */
}


/* 委托已收回报 */
void
OesClientMySpi::OnOrderInsert(const OesOrdCnfmT *pOrderInsert) {
       sprintf(sendJsonDataStr, ">>> 收到委托已收回报: " \
             "客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
             "会员内部编号[%" __SPK_FMT_LL__ "d], 证券账户[%s], 证券代码[%s], " \
             "市场代码[%" __SPK_FMT_HH__ "u], 订单类型[%" __SPK_FMT_HH__ "u], " \
             "买卖类型[%" __SPK_FMT_HH__ "u], 委托状态[%" __SPK_FMT_HH__ "u], " \
             "委托日期[%d], 委托接收时间[%d], 委托确认时间[%d], " \
             "委托数量[%d], 委托价格[%d], 撤单数量[%d], 累计成交份数[%d], " \
             "累计成交金额[%" __SPK_FMT_LL__ "d], 累计债券利息[%" __SPK_FMT_LL__ "d], " \
             "累计交易佣金[%" __SPK_FMT_LL__ "d], 冻结交易金额[%" __SPK_FMT_LL__ "d], " \
             "冻结债券利息[%" __SPK_FMT_LL__ "d], 冻结交易佣金[%" __SPK_FMT_LL__ "d], " \
             "被撤内部委托编号[%" __SPK_FMT_LL__ "d], 拒绝原因[%d], 交易所错误码[%d]\n",
             pOrderInsert->clEnvId, pOrderInsert->clSeqNo,
             pOrderInsert->clOrdId, pOrderInsert->invAcctId,
             pOrderInsert->securityId, pOrderInsert->mktId,
             pOrderInsert->ordType, pOrderInsert->bsType,
             pOrderInsert->ordStatus, pOrderInsert->ordDate,
             pOrderInsert->ordTime, pOrderInsert->ordCnfmTime,
             pOrderInsert->ordQty, pOrderInsert->ordPrice,
             pOrderInsert->canceledQty, pOrderInsert->cumQty,
             pOrderInsert->cumAmt, pOrderInsert->cumInterest,
             pOrderInsert->cumFee, pOrderInsert->frzAmt,
             pOrderInsert->frzInterest, pOrderInsert->frzFee,
             pOrderInsert->origClOrdId, pOrderInsert->ordRejReason,
             pOrderInsert->exchErrCode);

       fprintf(stdout, sendJsonDataStr);

      //sprintf(sendRespData2client, 
      //            "{\"msgId\":%d,\"clEnvId\":%d,\"clSeqNo\":%d,\"clOrdId\":%d,\"invAcctId\":\"%s\", " \
      //            "\"securityId\":\"%s\",\"mktId\":%u,\"ordType\":%d,\"bsType\":%d, " \
      //            "\"ordStatus\":%d,\"ordDate\":%d,\"ordTime\":%d,\"ordCnfmTime\":%d, " \
      //            "\"ordQty\":%d,\"ordPrice\":%d,\"canceledQty\":%d,\"cumQty\":%d, " \
      //            "\"cumAmt\":%d,\"cumInterest\":%d,\"cumFee\":%d, " \
      //            "\"frzAmt\":%d,\"frzInterest\":%d,\"frzFee\":%d, " \
      //            "\"origClOrdId\":%d,\"ordRejReason\":%d,\"exchErrCode\":%d" \
      //            "}",
      //            114,pOrderInsert->clEnvId, pOrderInsert->clSeqNo,
      //            pOrderInsert->clOrdId, pOrderInsert->invAcctId,
      //            pOrderInsert->securityId, pOrderInsert->mktId,
      //            pOrderInsert->ordType, pOrderInsert->bsType,
      //            pOrderInsert->ordStatus, pOrderInsert->ordDate,
      //            pOrderInsert->ordTime, pOrderInsert->ordCnfmTime,
      //            pOrderInsert->ordQty, pOrderInsert->ordPrice,
      //            pOrderInsert->canceledQty, pOrderInsert->cumQty,
      //            pOrderInsert->cumAmt, pOrderInsert->cumInterest,
      //            pOrderInsert->cumFee, pOrderInsert->frzAmt,
      //            pOrderInsert->frzInterest, pOrderInsert->frzFee,
      //            pOrderInsert->origClOrdId, pOrderInsert->ordRejReason,
      //            pOrderInsert->exchErrCode);

      //publisher.publish("oes_resp",sendRespData2client);
	  ptree pt;
	  pt.put("msgId", 114);
	  pt.put("clEnvId", pOrderInsert->clEnvId);
	  pt.put("clSeqNo", pOrderInsert->clSeqNo);
	  pt.put("clOrdId", pOrderInsert->clOrdId);

	  pt.put("invAcctId", pOrderInsert->invAcctId);
	  pt.put("securityId", pOrderInsert->securityId);
	  pt.put("mktId", pOrderInsert->mktId);
	  pt.put("ordType", pOrderInsert->ordType);
	  pt.put("bsType", pOrderInsert->bsType);
	  pt.put("ordStatus", pOrderInsert->ordStatus);
	  pt.put("ordDate", pOrderInsert->ordDate);
	  pt.put("ordTime", pOrderInsert->ordTime);
	  pt.put("ordCnfmTime", pOrderInsert->ordCnfmTime);
	  pt.put("ordQty", pOrderInsert->ordQty);
	  pt.put("ordPrice", pOrderInsert->ordPrice);
	  pt.put("canceledQty", pOrderInsert->canceledQty);
	  pt.put("cumQty", pOrderInsert->cumQty);
	  pt.put("cumAmt", pOrderInsert->cumAmt);
	  pt.put("cumInterest", pOrderInsert->cumInterest);
	  pt.put("cumFee", pOrderInsert->cumFee);
	  pt.put("frzAmt", pOrderInsert->frzAmt);
	  pt.put("frzInterest", pOrderInsert->frzInterest);
	  pt.put("frzFee", pOrderInsert->frzFee);
	  pt.put("origClEnvId", pOrderInsert->origClEnvId);
	  pt.put("origClSeqNo", pOrderInsert->origClSeqNo);
	  pt.put("origClOrdId", pOrderInsert->origClOrdId);
	  pt.put("ordRejReason", pOrderInsert->ordRejReason);
	  pt.put("exchErrCode", pOrderInsert->exchErrCode);
	  pt.put("username", m_strUsername);

	  std::ostringstream buf;
	  write_json(buf, pt, false);
	  sprintf(sendRespData2client, "%s", buf.str().c_str());
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 委托确认回报 */
void
OesClientMySpi::OnOrderReport(int32 errorCode, const OesOrdCnfmT *pOrderReport) {
      // sprintf(sendJsonDataStr, ">>> 收到委托回报: " \
      //       "客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
      //       "会员内部编号[%" __SPK_FMT_LL__ "d], 证券账户[%s], 证券代码[%s], " \
      //       "市场代码[%" __SPK_FMT_HH__ "u], 订单类型[%" __SPK_FMT_HH__ "u], " \
      //       "买卖类型[%" __SPK_FMT_HH__ "u], 委托状态[%" __SPK_FMT_HH__ "u], " \
      //       "委托日期[%d], 委托接收时间[%d], 委托确认时间[%d], "
      //       "委托数量[%d], 委托价格[%d], 撤单数量[%d], 累计成交份数[%d], " \
      //       "累计成交金额[%" __SPK_FMT_LL__ "d], 累计债券利息[%" __SPK_FMT_LL__ "d], " \
      //       "累计交易佣金[%" __SPK_FMT_LL__ "d], 冻结交易金额[%" __SPK_FMT_LL__ "d], " \
      //       "冻结债券利息[%" __SPK_FMT_LL__ "d], 冻结交易佣金[%" __SPK_FMT_LL__ "d], " \
      //       "被撤内部委托编号[%" __SPK_FMT_LL__ "d], 拒绝原因[%d], 交易所错误码[%d]\n",
      //       pOrderReport->clEnvId, pOrderReport->clSeqNo,
      //       pOrderReport->clOrdId, pOrderReport->invAcctId,
      //       pOrderReport->securityId, pOrderReport->mktId,
      //       pOrderReport->ordType, pOrderReport->bsType,
      //       pOrderReport->ordStatus, pOrderReport->ordDate,
      //       pOrderReport->ordTime, pOrderReport->ordCnfmTime,
      //       pOrderReport->ordQty, pOrderReport->ordPrice,
      //       pOrderReport->canceledQty, pOrderReport->cumQty,
      //       pOrderReport->cumAmt, pOrderReport->cumInterest,
      //       pOrderReport->cumFee, pOrderReport->frzAmt,
      //       pOrderReport->frzInterest, pOrderReport->frzFee,
      //       pOrderReport->origClOrdId, pOrderReport->ordRejReason,
      //       pOrderReport->exchErrCode);

      // fprintf(stdout, sendJsonDataStr);

      //sprintf(sendRespData2client, 
      //            "{\"msgId\":%d,\"clEnvId\":%d,\"clSeqNo\":%d,\"clOrdId\":%d,\"invAcctId\":\"%s\", " \
      //            "\"securityId\":\"%s\",\"mktId\":%u,\"ordType\":%d,\"bsType\":%d, " \
      //            "\"ordStatus\":%d,\"ordDate\":%d,\"ordTime\":%d,\"ordCnfmTime\":%d, " \
      //            "\"ordQty\":%d,\"ordPrice\":%d,\"canceledQty\":%d,\"cumQty\":%d, " \
      //            "\"cumAmt\":%d,\"cumInterest\":%d,\"cumFee\":%d, " \
      //            "\"cumFee\":%d,\"frzAmt\":%d,\"frzInterest\":%d,\"frzFee\":%d, " \
      //            "\"origClOrdId\":%d,\"ordRejReason\":%d,\"exchErrCode\":%d" \
      //            "}",
      //            115,pOrderReport->clEnvId, pOrderReport->clSeqNo,
      //            pOrderReport->clOrdId, pOrderReport->invAcctId,
      //            pOrderReport->securityId, pOrderReport->mktId,
      //            pOrderReport->ordType, pOrderReport->bsType,
      //            pOrderReport->ordStatus, pOrderReport->ordDate,
      //            pOrderReport->ordTime, pOrderReport->ordCnfmTime,
      //            pOrderReport->ordQty, pOrderReport->ordPrice,
      //            pOrderReport->canceledQty, pOrderReport->cumQty,
      //            pOrderReport->cumAmt, pOrderReport->cumInterest,
      //            pOrderReport->cumFee, pOrderReport->frzAmt,
      //            pOrderReport->frzInterest, pOrderReport->frzFee,
      //            pOrderReport->origClOrdId, pOrderReport->ordRejReason,
      //            pOrderReport->exchErrCode);

      //publisher.publish("oes_resp",sendRespData2client);
      //spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);

	  ptree pt;
	  pt.put("msgId", 115);
	  pt.put("clEnvId", pOrderReport->clEnvId);
	  pt.put("clSeqNo", pOrderReport->clSeqNo);
	  pt.put("clOrdId", pOrderReport->clOrdId);

	  pt.put("invAcctId", pOrderReport->invAcctId);
	  pt.put("securityId", pOrderReport->securityId);
	  pt.put("mktId", pOrderReport->mktId);
	  pt.put("ordType", pOrderReport->ordType);
	  pt.put("bsType", pOrderReport->bsType);
	  pt.put("ordStatus", pOrderReport->ordStatus);
	  pt.put("ordDate", pOrderReport->ordDate);
	  pt.put("ordTime", pOrderReport->ordTime);
	  pt.put("ordCnfmTime", pOrderReport->ordCnfmTime);
	  pt.put("ordQty", pOrderReport->ordQty);
	  pt.put("ordPrice", pOrderReport->ordPrice);
	  pt.put("canceledQty", pOrderReport->canceledQty);
	  pt.put("cumQty", pOrderReport->cumQty);
	  pt.put("cumAmt", pOrderReport->cumAmt);
	  pt.put("cumInterest", pOrderReport->cumInterest);
	  pt.put("cumFee", pOrderReport->cumFee);
	  pt.put("frzAmt", pOrderReport->frzAmt);
	  pt.put("frzInterest", pOrderReport->frzInterest);
	  pt.put("frzFee", pOrderReport->frzFee);
	  pt.put("origClEnvId", pOrderReport->origClEnvId);
	  pt.put("origClSeqNo", pOrderReport->origClSeqNo);
	  pt.put("origClOrdId", pOrderReport->origClOrdId);
	  pt.put("ordRejReason", pOrderReport->ordRejReason);
	  pt.put("exchErrCode", pOrderReport->exchErrCode);
	  pt.put("username", m_strUsername);

	  std::ostringstream buf;
	  write_json(buf, pt, false);
	  sprintf(sendRespData2client, "%s", buf.str().c_str());
	  spisocket.send(sendRespData2client, strlen(sendRespData2client) + 1, 0);

	  char keyName[60];
	  if (pOrderReport->bsType != 30)//下单委托等
	  {
		  //插入已委托中,115
		  keyName[0] = '\0';
		  sprintf(keyName, "ywt-%d-%s-%d-%d", 115, pOrderReport->securityId, pOrderReport->mktId, pOrderReport->clOrdId);
		  Redis.Set(keyName, sendRespData2client);
		  fprintf(stdout, keyName);
		  
		  //插入未成交,123
		  keyName[0] = '\0';
		  sprintf(keyName, "wcj-%d-%s-%d-%d", 123, pOrderReport->securityId, pOrderReport->mktId, pOrderReport->clOrdId);
		  Redis.Set(keyName, sendRespData2client);
		  fprintf(stdout, keyName);
	  }
	  else
	  {
		  //从未成交中删除已撤单的订单，123
		  keyName[0] = '\0';
		  sprintf(keyName, "wcj-%d-%s-%d-%d", 123, pOrderReport->securityId, pOrderReport->mktId, pOrderReport->clOrdId);
		  Redis.Del(keyName);
		  fprintf(stdout, keyName);

		  //插入已撤单中，122
		  keyName[0] = '\0';
		  sprintf(keyName, "ycx-%d-%s-%d-%d", 122, pOrderReport->securityId, pOrderReport->mktId, pOrderReport->clOrdId);
		  Redis.Set(keyName, sendRespData2client);
		  fprintf(stdout, keyName);
	  }
	  //const string key = keyName;
	  //const string value = sendRespData2client;
	  //Redis.Set(key, value);
	  

	  /*
	  //TODO 优化字段存储，减少不必要的字段
	  tdResInfoT.orderlistv.push_back(sendRespData2client);
	  //test之用
	  int size = tdResInfoT.orderlistv.size();
	  if (size > 10)
	  {
		  for (int i = 0; i < size; i++)//方法一 
		  {
			  fprintf(stdout, tdResInfoT.orderlistv[i].c_str());
		  }

		  tdResInfoT.orderlistv.clear();
	  }
	  */
}


/* 成交回报 */
void
OesClientMySpi::OnTradeReport(const OesTrdCnfmT *pTradeReport) {
	//有bug
      // sprintf(sendJsonDataStr, ">>> 收到成交回报: " \
      //       "成交编号[%" __SPK_FMT_LL__ "d], 会员内部编号[%" __SPK_FMT_LL__ "d], " \
      //       "委托客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
      //       "证券账户[%s], 证券代码[%s], 市场代码[%" __SPK_FMT_HH__ "u], " \
      //       "买卖方向[%" __SPK_FMT_HH__ "u], 委托买卖类型[%" __SPK_FMT_HH__ "u], "
      //       "成交日期[%d], 成交时间[%d], 成交数量[%d], 成交价格[%d], " \
      //       "成交金额[%" __SPK_FMT_LL__ "d], 累计成交数量[%d], " \
      //       "累计成交金额[%" __SPK_FMT_LL__ "d], 累计债券利息[%" __SPK_FMT_LL__ "d], " \
      //       "累计交易费用[%" __SPK_FMT_LL__ "d], PBU代码[%d]\n",
      //       pTradeReport->exchTrdNum, pTradeReport->clOrdId,
      //       pTradeReport->clEnvId, pTradeReport->clSeqNo,
      //       pTradeReport->invAcctId, pTradeReport->securityId,
      //       pTradeReport->mktId, pTradeReport->trdSide,
      //       pTradeReport->ordBuySellType, pTradeReport->trdDate,
      //       pTradeReport->trdTime, pTradeReport->trdQty, pTradeReport->trdPrice,
      //       pTradeReport->trdAmt, pTradeReport->cumQty, pTradeReport->cumAmt,
      //       pTradeReport->cumInterest, pTradeReport->cumFee,
      //       pTradeReport->pbuId);

      // fprintf(stdout, sendJsonDataStr);

      //sprintf(sendRespData2client, 
      //            "{\"msgId\":%d,\"exchTrdNum\":%d,\"clOrdId\":%d,\"clEnvId\":%d,\"clSeqNo\":%d,\"invAcctId\":\"%s\", " \
      //            "securityId\":\"%s\",\"mktId\":%d,\"trdSide\":%d, " \
      //            "ordBuySellType\":%d,\"trdDate\":%d,\"trdTime\":%d, " \
      //            "trdQty\":%d,\"trdPrice\":%d,\"trdAmt\":%d,\"cumQty\":%d, " \
      //            "cumAmt\":%d,\"cumInterest\":%d," \
      //            "cumFee\":%d,\"pbuId\":%d" \
      //            "}",
      //            116,pTradeReport->exchTrdNum, pTradeReport->clOrdId,
      //            pTradeReport->clEnvId, pTradeReport->clSeqNo,
      //            pTradeReport->invAcctId, pTradeReport->securityId,
      //            pTradeReport->mktId, pTradeReport->trdSide,
      //            pTradeReport->ordBuySellType, pTradeReport->trdDate,
      //            pTradeReport->trdTime, pTradeReport->trdQty, pTradeReport->trdPrice,
      //            pTradeReport->trdAmt, pTradeReport->cumQty, pTradeReport->cumAmt,
      //            pTradeReport->cumInterest, pTradeReport->cumFee,
      //            pTradeReport->pbuId);

      ////publisher.publish("oes_resp",sendRespData2client);
      //spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);

	  ptree pt;
	  pt.put("msgId", 116);
	  pt.put("exchTrdNum", pTradeReport->exchTrdNum);
	  pt.put("clOrdId", pTradeReport->clOrdId);
	  pt.put("clEnvId", pTradeReport->clEnvId);
	  pt.put("clSeqNo", pTradeReport->clSeqNo);

	  pt.put("invAcctId", pTradeReport->invAcctId);
	  pt.put("securityId", pTradeReport->securityId);
	  pt.put("mktId", pTradeReport->mktId);
	  pt.put("trdSide", pTradeReport->trdSide);
	  pt.put("ordBuySellType", pTradeReport->ordBuySellType);
	  pt.put("trdDate", pTradeReport->trdDate);
	  pt.put("trdTime", pTradeReport->trdTime);
	  pt.put("trdQty", pTradeReport->trdQty);
	  pt.put("trdPrice", pTradeReport->trdPrice);
	  pt.put("trdAmt", pTradeReport->trdAmt);
	  pt.put("cumQty", pTradeReport->cumQty);
	  pt.put("cumAmt", pTradeReport->cumAmt);
	  pt.put("cumInterest", pTradeReport->cumInterest);
	  pt.put("cumFee", pTradeReport->cumFee);
	  pt.put("pbuId", pTradeReport->pbuId);
	  pt.put("username", m_strUsername);

	  std::ostringstream buf;
	  write_json(buf, pt, false);
	  sprintf(sendRespData2client, "%s", buf.str().c_str());
	  spisocket.send(sendRespData2client, strlen(sendRespData2client) + 1, 0);

	  //存入到 已成交中，116
	  char keyName[60];
	  
	  //从未成交中删除已成交的订单，123
	  keyName[0] = '\0';
	  sprintf(keyName, "wcj-%d-%s-%d-%d", 123, pTradeReport->securityId, pTradeReport->mktId, pTradeReport->clOrdId);
	  Redis.Del(keyName);
	  fprintf(stdout, keyName);

	  keyName[0] = '\0';
	  sprintf(keyName, "ycj-%d-%s-%d-%d", 116, pTradeReport->securityId, pTradeReport->mktId, pTradeReport->clOrdId);
	  //const string key = keyName;
	  //const string value = sendRespData2client;
	  //Redis.Set(key, value);
	  Redis.Set(keyName, sendRespData2client);
	  fprintf(stdout, keyName);



	  /*
	  //TODO 优化字段存储，减少不必要的字段
	  tdResInfoT.transactedlistv.push_back(sendRespData2client);
	  //test之用
	  int size = tdResInfoT.transactedlistv.size();
	  if (size > 10)
	  {
		  for (int i = 0; i < size; i++)//方法一 
		  {
			  fprintf(stdout, tdResInfoT.transactedlistv[i].c_str());
		  }

		  tdResInfoT.transactedlistv.clear();
	  }
	  */
}


/* 资金变动回报 */
void
OesClientMySpi::OnCashAssetVariation(const OesCashAssetItemT *pCashAssetItem) {
      // sprintf(sendJsonDataStr, ">>> 收到资金变动回报: " \
      //       "资金账户代码[%s], 客户代码[%s], " \
      //       "币种[%" __SPK_FMT_HH__ "u], " \
      //       "资金类型[%" __SPK_FMT_HH__ "u], " \
      //       "资金账户状态[%" __SPK_FMT_HH__ "u], " \
      //       "期初余额[%" __SPK_FMT_LL__ "d], " \
      //       "期初可用余额[%" __SPK_FMT_LL__ "d], " \
      //       "期初可取余额[%" __SPK_FMT_LL__ "d], " \
      //       "不可用余额[%" __SPK_FMT_LL__ "d], " \
      //       "累计存入金额[%" __SPK_FMT_LL__ "d], " \
      //       "累计提取金额[%" __SPK_FMT_LL__ "d], " \
      //       "当前提取冻结金额[%" __SPK_FMT_LL__ "d], " \
      //       "累计卖金额[%" __SPK_FMT_LL__ "d], " \
      //       "累计买金额[%" __SPK_FMT_LL__ "d], " \
      //       "当前买冻结金额[%" __SPK_FMT_LL__ "d], " \
      //       "累计费用金额[%" __SPK_FMT_LL__ "d], " \
      //       "当前费用冻结金额[%" __SPK_FMT_LL__ "d], " \
      //       "当前维持保证金金额[%" __SPK_FMT_LL__ "d], " \
      //       "当前保证金冻结金额[%" __SPK_FMT_LL__ "d], " \
      //       "当前余额[%" __SPK_FMT_LL__ "d], " \
      //       "当前可用余额[%" __SPK_FMT_LL__ "d], " \
      //       "当前可取余额[%" __SPK_FMT_LL__ "d]\n",
      //       pCashAssetItem->cashAcctId, pCashAssetItem->custId,
      //       pCashAssetItem->currType, pCashAssetItem->cashType,
      //       pCashAssetItem->cashAcctStatus, pCashAssetItem->beginningBal,
      //       pCashAssetItem->beginningAvailableBal,
      //       pCashAssetItem->beginningDrawableBal,
      //       pCashAssetItem->disableBal, pCashAssetItem->totalDepositAmt,
      //       pCashAssetItem->totalWithdrawAmt, pCashAssetItem->withdrawFrzAmt,
      //       pCashAssetItem->totalSellAmt, pCashAssetItem->totalBuyAmt,
      //       pCashAssetItem->buyFrzAmt, pCashAssetItem->totalFeeAmt,
      //       pCashAssetItem->feeFrzAmt, pCashAssetItem->marginAmt,
      //       pCashAssetItem->marginFrzAmt, pCashAssetItem->currentTotalBal,
      //       pCashAssetItem->currentAvailableBal,
      //       pCashAssetItem->currentDrawableBal);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
                  "{\"msgId\":%d,\"cashAcctId\":\"%s\",\"custId\":\"%s\",\"currType\":%d,\"cashType\":%d, " \
                  "\"cashAcctStatus\":%d,\"beginningBal\":%d,\"beginningAvailableBal\":%d,\"beginningDrawableBal\":%d, " \
                  "\"disableBal\":%d,\"totalDepositAmt\":%d,\"totalWithdrawAmt\":%d,\"withdrawFrzAmt\":%d, " \
                  "\"totalSellAmt\":%d,\"totalBuyAmt\":%d,\"buyFrzAmt\":%d,\"totalFeeAmt\":%d, " \
                  "\"feeFrzAmt\":%d,\"marginAmt\":%d,\"marginFrzAmt\":%d,\"currentTotalBal\":%d, " \
                  "\"currentAvailableBal\":%d,\"currentDrawableBal\":%d" \
                  "}",
                  119,pCashAssetItem->cashAcctId, pCashAssetItem->custId,
                  pCashAssetItem->currType, pCashAssetItem->cashType,
                  pCashAssetItem->cashAcctStatus, pCashAssetItem->beginningBal,
                  pCashAssetItem->beginningAvailableBal,
                  pCashAssetItem->beginningDrawableBal,
                  pCashAssetItem->disableBal, pCashAssetItem->totalDepositAmt,
                  pCashAssetItem->totalWithdrawAmt, pCashAssetItem->withdrawFrzAmt,
                  pCashAssetItem->totalSellAmt, pCashAssetItem->totalBuyAmt,
                  pCashAssetItem->buyFrzAmt, pCashAssetItem->totalFeeAmt,
                  pCashAssetItem->feeFrzAmt, pCashAssetItem->marginAmt,
                  pCashAssetItem->marginFrzAmt, pCashAssetItem->currentTotalBal,
                  pCashAssetItem->currentAvailableBal,
                  pCashAssetItem->currentDrawableBal);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 持仓变动回报 */
void
OesClientMySpi::OnStockHoldingVariation(const OesStkHoldingItemT *pStkHoldingItem) {
      // sprintf(sendJsonDataStr, ">>> 收到持仓变动回报: " \
      //       "证券账户[%s], 产品代码[%s], " \
      //       "市场代码[%" __SPK_FMT_HH__ "u], " \
      //       "日初持仓[%" __SPK_FMT_LL__ "d], " \
      //       "日中累计买入持仓[%" __SPK_FMT_LL__ "d], " \
      //       "日中累计卖出持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前卖出冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //       "手动冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //       "日中累计转换获得持仓[%" __SPK_FMT_LL__ "d], " \
      //       "日中累计转换付出持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前转换付出冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前已锁定持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前锁定冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前解锁定冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前备兑冻结的现货持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前已备兑使用的现货持仓[%" __SPK_FMT_LL__ "d], " \
      //       "日初总持仓成本[%" __SPK_FMT_LL__ "d], " \
      //       "当日累计买入金额[%" __SPK_FMT_LL__ "d], " \
      //       "当日累计卖出金额[%" __SPK_FMT_LL__ "d], " \
      //       "当日累计买入费用[%" __SPK_FMT_LL__ "d], " \
      //       "当日累计卖出费用[%" __SPK_FMT_LL__ "d], " \
      //       "持仓成本价[%" __SPK_FMT_LL__ "d], " \
      //       "当前总持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前可卖持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前可转换付出持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前可锁定持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前可备兑/解锁持仓[%" __SPK_FMT_LL__ "d]\n",
      //       pStkHoldingItem->invAcctId, pStkHoldingItem->securityId,
      //       pStkHoldingItem->mktId, pStkHoldingItem->originalHld,
      //       pStkHoldingItem->totalBuyHld, pStkHoldingItem->totalSellHld,
      //       pStkHoldingItem->sellFrzHld, pStkHoldingItem->manualFrzHld,
      //       pStkHoldingItem->totalTrsfInHld, pStkHoldingItem->totalTrsfOutHld,
      //       pStkHoldingItem->trsfOutFrzHld, pStkHoldingItem->lockHld,
      //       pStkHoldingItem->lockFrzHld, pStkHoldingItem->unlockFrzHld,
      //       pStkHoldingItem->coveredFrzHld, pStkHoldingItem->coveredHld,
      //       pStkHoldingItem->originalCostAmt, pStkHoldingItem->totalBuyAmt,
      //       pStkHoldingItem->totalSellAmt, pStkHoldingItem->totalBuyFee,
      //       pStkHoldingItem->totalSellFee, pStkHoldingItem->costPrice,
      //       pStkHoldingItem->sumHld, pStkHoldingItem->sellAvlHld,
      //       pStkHoldingItem->trsfOutAvlHld, pStkHoldingItem->lockAvlHld,
      //       pStkHoldingItem->coveredAvlHld);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
                  "{\"msgId\":%d,\"invAcctId\":\"%s\",\"securityId\":\"%s\",\"mktId\":%d,\"originalHld\":%d, " \
                  "\"totalBuyHld\":%d,\"totalSellHld\":%d,\"sellFrzHld\":%d,\"manualFrzHld\":%d, " \
                  "\"totalTrsfInHld\":%d,\"totalTrsfOutHld\":%d,\"trsfOutFrzHld\":%d,\"lockHld\":%d, " \
                  "\"lockFrzHld\":%d,\"unlockFrzHld\":%d,\"coveredFrzHld\":%d,\"coveredHld\":%d, " \
                  "\"originalCostAmt\":%d,\"totalBuyAmt\":%d,\"totalSellAmt\":%d,\"totalBuyFee\":%d, " \
                  "\"totalSellFee\":%d,\"costPrice\":%d,\"sumHld\":%d,\"sellAvlHld\":%d, " \
                  "\"trsfOutAvlHld\":%d,\"lockAvlHld\":%d,\"coveredAvlHld\":%d" \
                  "}",
                  120,pStkHoldingItem->invAcctId, pStkHoldingItem->securityId,
                  pStkHoldingItem->mktId, pStkHoldingItem->originalHld,
                  pStkHoldingItem->totalBuyHld, pStkHoldingItem->totalSellHld,
                  pStkHoldingItem->sellFrzHld, pStkHoldingItem->manualFrzHld,
                  pStkHoldingItem->totalTrsfInHld, pStkHoldingItem->totalTrsfOutHld,
                  pStkHoldingItem->trsfOutFrzHld, pStkHoldingItem->lockHld,
                  pStkHoldingItem->lockFrzHld, pStkHoldingItem->unlockFrzHld,
                  pStkHoldingItem->coveredFrzHld, pStkHoldingItem->coveredHld,
                  pStkHoldingItem->originalCostAmt, pStkHoldingItem->totalBuyAmt,
                  pStkHoldingItem->totalSellAmt, pStkHoldingItem->totalBuyFee,
                  pStkHoldingItem->totalSellFee, pStkHoldingItem->costPrice,
                  pStkHoldingItem->sumHld, pStkHoldingItem->sellAvlHld,
                  pStkHoldingItem->trsfOutAvlHld, pStkHoldingItem->lockAvlHld,
                  pStkHoldingItem->coveredAvlHld);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 出入金委托拒绝回报 */
void
OesClientMySpi::OnFundTrsfReject(int32 errorCode,
        const OesFundTrsfRejectT *pFundTrsfReject) {
      // sprintf(sendJsonDataStr, ">>> 收到出入金委托拒绝回报: " \
      //       "错误码[%d], 错误信息[%s], " \
      //       "客户端环境号[%" __SPK_FMT_HH__ "d], " \
      //       "出入金流水号[%d], 资金账户[%s], " \
      //       "是否仅调拨[%" __SPK_FMT_HH__ "u], " \
      //       "出入金方向[%" __SPK_FMT_HH__ "u], " \
      //       "出入金金额[%" __SPK_FMT_LL__ "d]\n",
      //       pFundTrsfReject->rejReason,pFundTrsfReject->errorInfo,
      //       pFundTrsfReject->clEnvId, pFundTrsfReject->clSeqNo,
      //       pFundTrsfReject->cashAcctId, pFundTrsfReject->isAllotOnly,
      //       pFundTrsfReject->direct, pFundTrsfReject->occurAmt);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
                  "{\"msgId\":%d,\"rejReason\":%d,\"errorInfo\":\"%s\",\"clEnvId\":%d, " \
                  "clSeqNo\":%d,\"cashAcctId\":\"%s\",\"isAllotOnly\":%d,\"direct\":%d, " \
                  "occurAmt\":%d" \
                  "}",
                  10,pFundTrsfReject->rejReason,pFundTrsfReject->errorInfo,
                  pFundTrsfReject->clEnvId, pFundTrsfReject->clSeqNo,
                  pFundTrsfReject->cashAcctId, pFundTrsfReject->isAllotOnly,
                  pFundTrsfReject->direct, pFundTrsfReject->occurAmt);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 出入金委托执行回报 */
void
OesClientMySpi::OnFundTrsfReport(int32 errorCode,
        const OesFundTrsfReportT *pFundTrsfReport) {
      // sprintf(sendJsonDataStr, ">>> 收到出入金委托执行回报: " \
      //       "错误原因[%d], 主柜错误码[%d], 错误信息[%s], " \
      //       "客户端环境号[%" __SPK_FMT_HH__ "d], " \
      //       "出入金流水号[%d], 出入金编号[%d], 资金账户[%s], " \
      //       "是否仅调拨[%" __SPK_FMT_HH__ "u], " \
      //       "出入金方向[%" __SPK_FMT_HH__ "u], " \
      //       "出入金金额[%" __SPK_FMT_LL__ "d], " \
      //       "出入金状态[%d], 接收日期[%08d], " \
      //       "接收时间[%09d], 上报时间[%09d], 完成时间[%09d]\n",
      //       pFundTrsfReport->rejReason, pFundTrsfReport->counterErrCode,
      //       pFundTrsfReport->errorInfo, pFundTrsfReport->clEnvId,
      //       pFundTrsfReport->clSeqNo, pFundTrsfReport->fundTrsfId,
      //       pFundTrsfReport->cashAcctId, pFundTrsfReport->isAllotOnly,
      //       pFundTrsfReport->direct, pFundTrsfReport->occurAmt,
      //       pFundTrsfReport->trsfStatus, pFundTrsfReport->operDate,
      //       pFundTrsfReport->operTime, pFundTrsfReport->dclrTime,
      //       pFundTrsfReport->doneTime);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
                  "{\"msgId\":%d,\"rejReason\":%d,\"counterErrCode\":%d,\"errorInfo\":\"%s\",\"clEnvId\":%d,\"clSeqNo\":%d, " \
                  "fundTrsfId\":%d,\"mktId\":%d,\"trdSide\":%d, " \
                  "ordBuySellType\":%d,\"cashAcctId\":\"%s\",\"isAllotOnly\":%d, " \
                  "direct\":%d,\"occurAmt\":%d,\"trsfStatus\":%d,\"operDate\":\"%08d\", " \
                  "operTime\":\"%09d\",\"dclrTime\":\"%09d\",\"doneTime\":\"%09d\", " \
                  "}",
                  11,pFundTrsfReport->rejReason, pFundTrsfReport->counterErrCode,
                  pFundTrsfReport->errorInfo, pFundTrsfReport->clEnvId,
                  pFundTrsfReport->clSeqNo, pFundTrsfReport->fundTrsfId,
                  pFundTrsfReport->cashAcctId, pFundTrsfReport->isAllotOnly,
                  pFundTrsfReport->direct, pFundTrsfReport->occurAmt,
                  pFundTrsfReport->trsfStatus, pFundTrsfReport->operDate,
                  pFundTrsfReport->operTime, pFundTrsfReport->dclrTime,
                  pFundTrsfReport->doneTime);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 市场状态信息 */
void
OesClientMySpi::OnMarketState(const OesMarketStateItemT *pMarketStateItem) {
      // sprintf(sendJsonDataStr, ">>> 收到市场状态信息: " \
      //       "交易所代码[%hhu], 交易平台类型[%hhu], 市场类型[%hhu], "
      //       "市场状态[%hhu]\n",
      //       pMarketStateItem->exchId, pMarketStateItem->platformId,
      //       pMarketStateItem->mktId, pMarketStateItem->mktState);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
                  "{\"msgId\":%d,\"exchId\":%d,\"platformId\":%d, " \
                  "mktId\":%d,\"mktState\":%d" \
                  "}",
                  4,pMarketStateItem->exchId, pMarketStateItem->platformId,
                  pMarketStateItem->mktId, pMarketStateItem->mktState);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到委托信息回调 */
void
OesClientMySpi::OnQueryOrder(const OesOrdItemT *pOrder,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到委托信息: index[%d], isEnd[%c], " \
      //       "客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
      //       "会员内部编号[%" __SPK_FMT_LL__ "d], 证券账户[%s], 证券代码[%s], " \
      //       "市场代码[%" __SPK_FMT_HH__ "u], 订单类型[%" __SPK_FMT_HH__ "u], " \
      //       "买卖类型[%" __SPK_FMT_HH__ "u], 委托状态[%" __SPK_FMT_HH__ "u], " \
      //       "委托日期[%d], 委托接收时间[%d], 委托确认时间[%d], " \
      //       "委托数量[%d], 委托价格[%d], 撤单数量[%d], 累计成交份数[%d], " \
      //       "累计成交金额[%" __SPK_FMT_LL__ "d], 累计债券利息[%" __SPK_FMT_LL__ "d], " \
      //       "累计交易佣金[%" __SPK_FMT_LL__ "d], 冻结交易金额[%" __SPK_FMT_LL__ "d], " \
      //       "冻结债券利息[%" __SPK_FMT_LL__ "d], 冻结交易佣金[%" __SPK_FMT_LL__ "d], " \
      //       "被撤内部委托编号[%" __SPK_FMT_LL__ "d], 拒绝原因[%d], " \
      //       "交易所错误码[%d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pOrder->clEnvId, pOrder->clSeqNo, pOrder->clOrdId,
      //       pOrder->invAcctId, pOrder->securityId, pOrder->mktId,
      //       pOrder->ordType, pOrder->bsType, pOrder->ordStatus,
      //       pOrder->ordDate, pOrder->ordTime, pOrder->ordCnfmTime,
      //       pOrder->ordQty, pOrder->ordPrice, pOrder->canceledQty,
      //       pOrder->cumQty, pOrder->cumAmt, pOrder->cumInterest,
      //       pOrder->cumFee, pOrder->frzAmt, pOrder->frzInterest,
      //       pOrder->frzFee, pOrder->origClOrdId, pOrder->ordRejReason,
      //       pOrder->exchErrCode);

      // fprintf(stdout, sendJsonDataStr);

      //mgsID需要确定，暂定为8
      sprintf(sendRespData2client, 
            "{\"msgId\":%d,\"seqNo\":%d,\"isEnd\":\"%s\",\"clEnvId\":%d,\"clSeqNo\":%d,\"clOrdId\":%d,\"invAcctId\":\"%s\", " \
            "securityId\":\"%s\",\"mktId\":%d,\"ordType\":%d,\"bsType\":%d, " \
            "ordStatus\":%d,\"ordDate\":%d,\"ordTime\":%d,\"ordCnfmTime\":%d, " \
            "ordQty\":%d,\"ordPrice\":%d,\"canceledQty\":%d,\"cumQty\":%d, " \
            "cumAmt\":%d,\"cumInterest\":%d, " \
            "cumFee\":%d,\"frzAmt\":%d,\"frzInterest\":%d,\"frzFee\":%d, " \
            "origClOrdId\":%d,\"ordRejReason\":%d,\"exchErrCode\":%d" \
            "}",
            8,pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
            pOrder->clEnvId, pOrder->clSeqNo, pOrder->clOrdId,
            pOrder->invAcctId, pOrder->securityId, pOrder->mktId,
            pOrder->ordType, pOrder->bsType, pOrder->ordStatus,
            pOrder->ordDate, pOrder->ordTime, pOrder->ordCnfmTime,
            pOrder->ordQty, pOrder->ordPrice, pOrder->canceledQty,
            pOrder->cumQty, pOrder->cumAmt, pOrder->cumInterest,
            pOrder->cumFee, pOrder->frzAmt, pOrder->frzInterest,
            pOrder->frzFee, pOrder->origClOrdId, pOrder->ordRejReason,
            pOrder->exchErrCode);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到成交信息回调 */
void
OesClientMySpi::OnQueryTrade(const OesTrdItemT *pTrade,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到成交信息: index[%d], isEnd[%c], " \
      //       "成交编号[%" __SPK_FMT_LL__ "d], 会员内部编号[%" __SPK_FMT_LL__ "d], " \
      //       "委托客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
      //       "证券账户[%s], 证券代码[%s], 市场代码[%" __SPK_FMT_HH__ "u], " \
      //       "买卖方向[%" __SPK_FMT_HH__ "u], 委托买卖类型[%" __SPK_FMT_HH__ "u], " \
      //       "成交日期[%d], 成交时间[%d], 成交数量[%d], 成交价格[%d], " \
      //       "成交金额[%" __SPK_FMT_LL__ "d], 累计成交数量[%d], " \
      //       "累计成交金额[%" __SPK_FMT_LL__ "d], 累计债券利息[%" __SPK_FMT_LL__ "d], " \
      //       "累计交易费用[%" __SPK_FMT_LL__ "d], PBU代码[%d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pTrade->exchTrdNum, pTrade->clOrdId, pTrade->clEnvId,
      //       pTrade->clSeqNo, pTrade->invAcctId, pTrade->securityId,
      //       pTrade->mktId, pTrade->trdSide, pTrade->ordBuySellType,
      //       pTrade->trdDate, pTrade->trdTime, pTrade->trdQty, pTrade->trdPrice,
      //       pTrade->trdAmt, pTrade->cumQty, pTrade->cumAmt, pTrade->cumInterest,
      //       pTrade->cumFee, pTrade->pbuId);

      // fprintf(stdout, sendJsonDataStr);

      //mgsID需要确定，暂定为9
      sprintf(sendRespData2client, 
                  "{\"msgId\":%d,\"seqNo\":%d,\"isEnd\":\"%s\", " \
                  "exchTrdNum\":%d,\"clOrdId\":%d,\"clEnvId\":%d,\"clSeqNo\":%d,\"invAcctId\":\"%s\", " \
                  "securityId\":\"%s\",\"mktId\":%d,\"trdSide\":%d, " \
                  "ordBuySellType\":%d,\"trdDate\":%d,\"trdTime\":%d, " \
                  "trdQty\":%d,\"trdPrice\":%d,\"trdAmt\":%d,\"cumQty\":%d, " \
                  "cumAmt\":%d,\"cumInterest\":%d," \
                  "cumFee\":%d,\"pbuId\":%d" \
                  "}",
                  9,pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
                  pTrade->exchTrdNum, pTrade->clOrdId, pTrade->clEnvId,
                  pTrade->clSeqNo, pTrade->invAcctId, pTrade->securityId,
                  pTrade->mktId, pTrade->trdSide, pTrade->ordBuySellType,
                  pTrade->trdDate, pTrade->trdTime, pTrade->trdQty, pTrade->trdPrice,
                  pTrade->trdAmt, pTrade->cumQty, pTrade->cumAmt, pTrade->cumInterest,
                  pTrade->cumFee, pTrade->pbuId);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到资金信息回调 */
void
OesClientMySpi::OnQueryCashAsset(const OesCashAssetItemT *pCashAsset,
        const OesQryCursorT *pCursor, int32 requestId) {
       sprintf(sendJsonDataStr, ">>> 查询到资金信息: index[%d], isEnd[%c], " \
             "资金账户代码[%s], 客户代码[%s], 币种[%" __SPK_FMT_HH__ "u], " \
             "资金类型[%" __SPK_FMT_HH__ "u], 期初余额[%" __SPK_FMT_LL__ "d], " \
             "期初可用[%" __SPK_FMT_LL__ "d], 期初可取[%" __SPK_FMT_LL__ "d], " \
             "不可用[%" __SPK_FMT_LL__ "d], 累计存入[%" __SPK_FMT_LL__ "d], " \
             "累计提取[%" __SPK_FMT_LL__ "d], 当前提取冻结[%" __SPK_FMT_LL__ "d], " \
             "累计卖出[%" __SPK_FMT_LL__ "d], 累计买入[%" __SPK_FMT_LL__ "d], " \
             "当前买冻结[%" __SPK_FMT_LL__ "d], 累计费用[%" __SPK_FMT_LL__ "d], " \
             "当前费用冻结[%" __SPK_FMT_LL__ "d], 当前维持保证金[%" __SPK_FMT_LL__ "d], " \
             "当前保证金冻结[%" __SPK_FMT_LL__ "d], 当前余额[%" __SPK_FMT_LL__ "d], " \
             "当前可用[%" __SPK_FMT_LL__ "d], 当前可取[%" __SPK_FMT_LL__ "d]\n",
             pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
             pCashAsset->cashAcctId, pCashAsset->custId, pCashAsset->currType,
             pCashAsset->cashType, pCashAsset->beginningBal,
             pCashAsset->beginningAvailableBal, pCashAsset->beginningDrawableBal,
             pCashAsset->disableBal, pCashAsset->totalDepositAmt,
             pCashAsset->totalWithdrawAmt, pCashAsset->withdrawFrzAmt,
             pCashAsset->totalSellAmt, pCashAsset->totalBuyAmt,
             pCashAsset->buyFrzAmt, pCashAsset->totalFeeAmt,
             pCashAsset->feeFrzAmt, pCashAsset->marginAmt,
             pCashAsset->marginFrzAmt, pCashAsset->currentTotalBal,
             pCashAsset->currentAvailableBal, pCashAsset->currentDrawableBal);

       fprintf(stdout, sendJsonDataStr);

    //  sprintf(sendrespdata2client, 
    //              "{\"msgid\":%d,\"seqno\":%d,\"isend\":\"%s\","  \ 
				//"\"cashacctid\":\"%s\",\"custid\":\"%s\",\"currtype\":%d,\"cashtype\":%d, " \
				//"\"beginningbal\":%d,\"beginningavailablebal\":%d,\"beginningdrawablebal\":%d, " \
				//"\"disablebal\":%d,\"totaldepositamt\":%d,\"totalwithdrawamt\":%d,\"withdrawfrzamt\":%d, " \
				//"\"totalsellamt\":%d,\"totalbuyamt\":%d,\"buyfrzamt\":%d,\"totalfeeamt\":%d, " \
				//"\"feefrzamt\":%d,\"marginamt\":%d,\"marginfrzamt\":%d,\"currenttotalbal\":%d, " \
				//"\"currentavailablebal\":%d,\"currentdrawablebal\":%d" \
    //              "}",
    //              12,pcursor->seqno, pcursor->isend ? 'y' : 'n',
    //              pcashasset->cashacctid, pcashasset->custid, pcashasset->currtype,
    //              pcashasset->cashtype, pcashasset->beginningbal,
    //              pcashasset->beginningavailablebal, pcashasset->beginningdrawablebal,
    //              pcashasset->disablebal, pcashasset->totaldepositamt,
    //              pcashasset->totalwithdrawamt, pcashasset->withdrawfrzamt,
    //              pcashasset->totalsellamt, pcashasset->totalbuyamt,
    //              pcashasset->buyfrzamt, pcashasset->totalfeeamt,
    //              pcashasset->feefrzamt, pcashasset->marginamt,
    //              pcashasset->marginfrzamt, pcashasset->currenttotalbal,
    //              pcashasset->currentavailablebal, pcashasset->currentdrawablebal);

    //  publisher.publish("oes_resp",sendrespdata2client);
    //  spisocket.send(sendrespdata2client,strlen(sendrespdata2client)+1,0);
}

/*
{"msgId":"13", "seqNo":"17","isEnd":"N", invAcctId":"0188800368","securityId":"000007",
"mktId":"2","originalHld":"1000000", totalBuyHld":"0","totalSellHld":"0","sellFrzHld":"0", 
totalTrsfInHld":"0","totalTrsfOutHld":"0","trsfOutFrzHld":"0","lockHld":"0", lockFrzHld":"0",
"unlockFrzHld":"0","coveredFrzHld":"0","coveredHld":"0", coveredAvlHld":"0","sumHld":"1000000",
"sellAvlHld":"1000000","trsfOutAvlHld":"1000000", lockAvlHld":"1000000"}
*/
/* 查询到持仓信息回调 */
void
OesClientMySpi::OnQueryStkHolding(const OesStkHoldingItemT *pStkHolding,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到股票持仓信息: index[%d], isEnd[%c], " \
      //       "证券账户[%s], 市场代码[%" __SPK_FMT_HH__ "u], 产品代码[%s], " \
      //       "日初持仓[%" __SPK_FMT_LL__ "d], " \
      //       "日中累计买入[%" __SPK_FMT_LL__ "d], " \
      //       "日中累计卖出[%" __SPK_FMT_LL__ "d], " \
      //       "当前卖出冻结[%" __SPK_FMT_LL__ "d], " \
      //       "日中累计转换获得[%" __SPK_FMT_LL__ "d], " \
      //       "日中累计转换付出[%" __SPK_FMT_LL__ "d], " \
      //       "当前转换付出冻结[%" __SPK_FMT_LL__ "d], " \
      //       "当前已锁定[%" __SPK_FMT_LL__ "d], " \
      //       "当前锁定冻结[%" __SPK_FMT_LL__ "d], " \
      //       "当前解锁定冻结[%" __SPK_FMT_LL__ "d], " \
      //       "当前备兑冻结[%" __SPK_FMT_LL__ "d], " \
      //       "当前已备兑使用[%" __SPK_FMT_LL__ "d], " \
      //       "当前可备兑/解锁[%" __SPK_FMT_LL__ "d], " \
      //       "当前总持仓[%" __SPK_FMT_LL__ "d], " \
      //       "当前可卖[%" __SPK_FMT_LL__ "d], " \
      //       "当前可转换付出[%" __SPK_FMT_LL__ "d], " \
      //       "当前可锁定[%" __SPK_FMT_LL__ "d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pStkHolding->invAcctId, pStkHolding->mktId, pStkHolding->securityId,
      //       pStkHolding->originalHld, pStkHolding->totalBuyHld,
      //       pStkHolding->totalSellHld, pStkHolding->sellFrzHld,
      //       pStkHolding->totalTrsfInHld, pStkHolding->totalTrsfOutHld,
      //       pStkHolding->trsfOutFrzHld, pStkHolding->lockHld,
      //       pStkHolding->lockFrzHld, pStkHolding->unlockFrzHld,
      //       pStkHolding->coveredFrzHld, pStkHolding->coveredHld,
      //       pStkHolding->coveredAvlHld, pStkHolding->sumHld,
      //       pStkHolding->sellAvlHld, pStkHolding->trsfOutAvlHld,
      //       pStkHolding->lockAvlHld);

      // fprintf(stdout, sendJsonDataStr);
      //fprintf(stdout, "...OesClientMySpi::OnQueryStkHolding");
	/*
      sprintf(sendRespData2client, 
                  "{\"msgId\":%d, \"seqNo\":%d,\"isEnd\":\"%c\", " \
                  "\"invAcctId\":\"%s\",\"securityId\":\"%s\",\"mktId\":%u,\"originalHld\":%d, " \
                  "\"totalBuyHld\":%d,\"totalSellHld\":%d,\"sellFrzHld\":%d, " \
                  "\"totalTrsfInHld\":%d,\"totalTrsfOutHld\":%d,\"trsfOutFrzHld\":%d,\"lockHld\":%d, " \
                  "\"lockFrzHld\":%d,\"unlockFrzHld\":%d,\"coveredFrzHld\":%d,\"coveredHld\":%d, " \
                  "\"coveredAvlHld\":%d,\"sumHld\":%d,\"sellAvlHld\":%d,\"trsfOutAvlHld\":%d,"  \
                  "\"lockAvlHld\":%d" \
                  "}",
                  140,pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
                  pStkHolding->invAcctId, pStkHolding->securityId, pStkHolding->mktId,
                  pStkHolding->originalHld, pStkHolding->totalBuyHld,
                  pStkHolding->totalSellHld, pStkHolding->sellFrzHld,
                  pStkHolding->totalTrsfInHld, pStkHolding->totalTrsfOutHld,
                  pStkHolding->trsfOutFrzHld, pStkHolding->lockHld,
                  pStkHolding->lockFrzHld, pStkHolding->unlockFrzHld,
                  pStkHolding->coveredFrzHld, pStkHolding->coveredHld,
                  pStkHolding->coveredAvlHld, pStkHolding->sumHld,
                  pStkHolding->sellAvlHld, pStkHolding->trsfOutAvlHld,
                  pStkHolding->lockAvlHld);
	*/
       //fprintf(stdout, sendRespData2client);
       //publisher.publish("oes_resp",sendRespData2client);
       //spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);

	   ptree pt;
	   pt.put("msgId", 140);
	   pt.put("seqNo", pCursor->seqNo);
	   pt.put("isEnd", pCursor->isEnd);
	   pt.put("requestId", requestId);

	   pt.put("invAcctId", pStkHolding->invAcctId);
	   pt.put("securityId", pStkHolding->securityId);
	   pt.put("mktId", pStkHolding->mktId);
	   pt.put("originalHld", pStkHolding->originalHld);

	   pt.put("totalBuyHld", pStkHolding->totalBuyHld);
	   pt.put("totalSellHld", pStkHolding->totalSellHld);
	   pt.put("sellFrzHld", pStkHolding->sellFrzHld);
	   pt.put("totalTrsfInHld", pStkHolding->totalTrsfInHld);
	   pt.put("totalTrsfOutHld", pStkHolding->totalTrsfOutHld);
	   pt.put("trsfOutFrzHld", pStkHolding->trsfOutFrzHld);
	   pt.put("lockHld", pStkHolding->lockHld);
	   pt.put("lockFrzHld", pStkHolding->lockFrzHld);
	   pt.put("unlockFrzHld", pStkHolding->unlockFrzHld);
	   pt.put("coveredFrzHld", pStkHolding->coveredFrzHld);
	   pt.put("coveredHld", pStkHolding->coveredHld);
	   pt.put("coveredAvlHld", pStkHolding->coveredAvlHld);
	   pt.put("sumHld", pStkHolding->sumHld);
	   pt.put("sellAvlHld", pStkHolding->sellAvlHld);
	   pt.put("trsfOutAvlHld", pStkHolding->trsfOutAvlHld);
	   pt.put("lockAvlHld", pStkHolding->lockAvlHld);
	   pt.put("username", m_strUsername);

	   std::ostringstream buf;
	   write_json(buf, pt, false);
	   sprintf(sendRespData2client, "%s", buf.str().c_str());
	   spisocket.send(sendRespData2client, strlen(sendRespData2client) + 1, 0);
}


/* 查询到配号、中签信息回调 */
void
OesClientMySpi::OnQueryLotWinning(const OesLotWinningItemT *pLotWinning,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到新股配号、中签信息: index[%d], isEnd[%c], " \
      //       "股东账户代码[%s], 证券代码[%s], 证券名称[%s], " \
      //       "市场代码[%" __SPK_FMT_HH__ "u], " \
      //       "记录类型[%" __SPK_FMT_HH__ "u], " \
      //       "失败原因[%" __SPK_FMT_HH__ "u], 配号、中签日期[%08d], " \
      //       "配号首个号码[%" __SPK_FMT_LL__ "d], 配号成功数量/中签股数[%d], " \
      //       "最终发行价[%d], 中签金额[%" __SPK_FMT_LL__ "d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pLotWinning->invAcctId, pLotWinning->securityId,
      //       pLotWinning->securityName, pLotWinning->mktId, pLotWinning->lotType,
      //       pLotWinning->rejReason, pLotWinning->lotDate, pLotWinning->assignNum,
      //       pLotWinning->lotQty, pLotWinning->lotPrice, pLotWinning->lotAmt);

      // fprintf(stdout, sendJsonDataStr);
	/*
      sprintf(sendRespData2client, 
                  "{\"msgId\":\"13\", \"seqNo\":%d,\"isEnd\":\"%s\", " \
                  "\"invAcctId\":\"%s\",\"securityId\":\"%s\",\"securityName\":\"%s\", " \
                  "\"mktId\":%d,\"lotType\":\"%s\",\"rejReason\":%d,\"lotDate\":%d, " \
                  "\"assignNum\":%d,\"lotQty\":\"%s\",\"lotPrice\":%d,\"lotAmt\":%d, " \
                  "}",
                  pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
                  pLotWinning->invAcctId, pLotWinning->securityId,
                  pLotWinning->securityName, pLotWinning->mktId, pLotWinning->lotType,
                  pLotWinning->rejReason, pLotWinning->lotDate, pLotWinning->assignNum,
                  pLotWinning->lotQty, pLotWinning->lotPrice, pLotWinning->lotAmt);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
	  */
}


/* 查询到客户信息回调 */
void
OesClientMySpi::OnQueryCustInfo(const OesCustItemT *pCust,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到客户信息: index[%d], isEnd[%c], " \
      //       "客户ID[%s], 客户类型[%" __SPK_FMT_HH__ "u], " \
      //       "客户状态[%" __SPK_FMT_HH__ "u], 风险评级[%" __SPK_FMT_HH__ "u], " \
      //       "机构标志[%" __SPK_FMT_HH__ "u], 投资者分类[%c]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pCust->custId, pCust->custType, pCust->status,
      //       pCust->riskLevel, pCust->institutionFlag,
      //       pCust->investorClass == OES_INVESTOR_CLASS_NORMAL ?
      //               '0' : pCust->investorClass + 'A' - 1);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
                  "{\"msgId\":%d, \"seqNo\":%d,\"isEnd\":\"%s\", " \
                  "custId\":\"%s\",\"custType\":%d,\"status\":%d, " \
                  "riskLevel\":%d,\"institutionFlag\":%d,\"investorClass\":\"%s\" " \
                  "}",
                  13,pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
                  pCust->custId, pCust->custType, pCust->status,
                  pCust->riskLevel, pCust->institutionFlag,
                  pCust->investorClass == OES_INVESTOR_CLASS_NORMAL ?
                          '0' : pCust->investorClass + 'A' - 1);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到股东账户信息回调 */
void
OesClientMySpi::OnQueryInvAcct(const OesInvAcctItemT *pInvAcct,
        const OesQryCursorT *pCursor, int32 requestId) {
    // fprintf(stdout, ">>> 查询到证券账户信息: index[%d], isEnd[%c], " \
    //         "证券账户[%s], 市场代码[%" __SPK_FMT_HH__ "u], " \
    //         "客户代码[%s], 账户状态[%" __SPK_FMT_HH__ "u], " \
    //         "新股认购限额[%d]\n",
    //         pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
    //         pInvAcct->invAcctId, pInvAcct->mktId,
    //         pInvAcct->custId, pInvAcct->status,
    //         pInvAcct->subscriptionQuota);

    // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
                  "{\"msgId\":\"13\", \"seqNo\":%d,\"isEnd\":\"%s\", " \
                  "invAcctId\":\"%s\",\"custType\":%d,\"status\":%d, " \
                  "riskLevel\":%d,\"institutionFlag\":%d,\"investorClass\":\"%s\"," \
                  "}",
            pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
            pInvAcct->invAcctId, pInvAcct->mktId,
            pInvAcct->custId, pInvAcct->status,
            pInvAcct->subscriptionQuota);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到佣金信息回调 */
void
OesClientMySpi::OnQueryCommissionRate(
        const OesCommissionRateItemT *pCommissionRate,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到佣金信息: index[%d], isEnd[%c], " \
      //       "客户代码[%s], 证券代码[%s], 市场代码[%" __SPK_FMT_HH__ "u], " \
      //       "证券类型[%" __SPK_FMT_HH__ "u], 证券子类型[%" __SPK_FMT_HH__ "u], " \
      //       "买卖类型[%" __SPK_FMT_HH__ "u], 币种[%" __SPK_FMT_HH__ "u], " \
      //       "费用标识[%" __SPK_FMT_HH__ "u], 计算模式 [%" __SPK_FMT_HH__ "u], " \
      //       "费率[%" __SPK_FMT_LL__ "d], 最低费用[%d], 最高费用[%d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pCommissionRate->custId, pCommissionRate->securityId,
      //       pCommissionRate->mktId, pCommissionRate->securityType,
      //       pCommissionRate->subSecurityType, pCommissionRate->bsType,
      //       pCommissionRate->currType, pCommissionRate->feeType,
      //       pCommissionRate->calcFeeMode, pCommissionRate->feeRate,
      //       pCommissionRate->minFee, pCommissionRate->maxFee);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
                  "{\"msgId\":\"13\", \"seqNo\":%d,\"isEnd\":\"%s\", " \
                  "custId\":\"%s\",\"securityId\":\"%s\",\"mktId\":%d, " \
                  "securityType\":%d,\"subSecurityType\":%d,\"bsType\":%d,\"currType\":%d, " \
                  "feeType\":%d,\"calcFeeMode\":\"%s\",\"feeRate\":%d,\"minFee\":%d, " \
                  "maxFee\":%d, "
                  "}",
            pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
            pCommissionRate->custId, pCommissionRate->securityId,
            pCommissionRate->mktId, pCommissionRate->securityType,
            pCommissionRate->subSecurityType, pCommissionRate->bsType,
            pCommissionRate->currType, pCommissionRate->feeType,
            pCommissionRate->calcFeeMode, pCommissionRate->feeRate,
            pCommissionRate->minFee, pCommissionRate->maxFee);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到出入金流水信息回调 */
void
OesClientMySpi::OnQueryFundTransferSerial(
        const OesFundTransferSerialItemT *pFundTrsf,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到出入金流水: index[%d], isEnd[%c], " \
      //       "客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
      //       "资金账户[%s], 方向[%s], 金额[%" __SPK_FMT_LL__ "d], " \
      //       "出入金状态[%" __SPK_FMT_HH__ "u], 错误原因[%d], 主柜错误码[%d], " \
      //       "错误信息[%s], 柜台委托编码[%d], 接收日期[%08d], " \
      //       "接收时间[%09d], 上报时间[%09d], 完成时间[%09d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pFundTrsf->clEnvId, pFundTrsf->clSeqNo,
      //       pFundTrsf->cashAcctId,
      //       pFundTrsf->direct == OES_FUND_TRSF_DIRECT_IN ? "Bank->Broker" : "Broker->Bank",
      //       pFundTrsf->occurAmt, pFundTrsf->trsfStatus, pFundTrsf->rejReason,
      //       pFundTrsf->counterErrCode, pFundTrsf->errorInfo,
      //       pFundTrsf->counterEntrustNo, pFundTrsf->operDate,
      //       pFundTrsf->operTime, pFundTrsf->dclrTime,
      //       pFundTrsf->doneTime);

      // fprintf(stdout, sendJsonDataStr);
	/*
      sprintf(sendRespData2client, 
                  "{\"msgId\":\"13\", \"seqNo\":%d,\"isEnd\":\"%s\", " \
                  "clEnvId\":%d,\"clSeqNo\":%d, " \
                  "cashAcctId\":\"%s\", " \
                  "direct\":\"%s\",\"occurAmt\":%d,\"trsfStatus\":%d, " \
                  "rejReason\":%d,\"counterErrCode\":%d,\"errorInfo\":\"%s\", \"counterEntrustNo\":%d," \
                  "operDate\":\"%08d\", \"operTime\":\"%09d\",\"dclrTime\":\"%09d\",\"doneTime\":\"%09d\"," \
                  "}",
                  pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
                  pFundTrsf->clEnvId, pFundTrsf->clSeqNo,
                  pFundTrsf->cashAcctId,
                  pFundTrsf->direct == OES_FUND_TRSF_DIRECT_IN ? "Bank->Broker" : "Broker->Bank",
                  pFundTrsf->occurAmt, pFundTrsf->trsfStatus, pFundTrsf->rejReason,
                  pFundTrsf->counterErrCode, pFundTrsf->errorInfo,
                  pFundTrsf->counterEntrustNo, pFundTrsf->operDate,
                  pFundTrsf->operTime, pFundTrsf->dclrTime,
                  pFundTrsf->doneTime);
*/
      //publisher.publish("oes_resp",sendRespData2client);
      //spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到证券发行信息回调 */
void
OesClientMySpi::OnQueryIssue(const OesIssueItemT *pIssue,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到证券发行产品信息: index[%d], isEnd[%c], " \
      //       "证券代码[%s], 证券名称[%s], 正股代码[%s], " \
      //       "市场代码[%" __SPK_FMT_HH__ "u], " \
      //       "证券类型[%" __SPK_FMT_HH__ "u], " \
      //       "证券子类型[%" __SPK_FMT_HH__ "u], " \
      //       "是否允许撤单[%" __SPK_FMT_HH__ "u], " \
      //       "是否允许重复认购[%" __SPK_FMT_HH__ "u], " \
      //       "发行起始时间[%d], 发行结束时间[%d], " \
      //       "发行总量[%" __SPK_FMT_LL__ "d], " \
      //       "份数单位[%d], 最大份数[%d], 最小份数[%d], " \
      //       "发行价格[%d], 价格上限[%d], 价格下限[%d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pIssue->securityId, pIssue->securityName,
      //       pIssue->underlyingSecurityId, pIssue->mktId, pIssue->securityType,
      //       pIssue->subSecurityType, pIssue->isCancelAble,
      //       pIssue->isReApplyAble, pIssue->startDate, pIssue->endDate,
      //       pIssue->issueQty, pIssue->qtyUnit, pIssue->ordMaxQty,
      //       pIssue->ordMinQty, pIssue->issuePrice, pIssue->ceilPrice,
      //       pIssue->floorPrice);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
            "{\"msgId\":\"13\", \"seqNo\":%d,\"isEnd\":\"%s\", " \
            "securityId\":\"%s\",\"securityName\":\"%s\", " \
            "underlyingSecurityId\":\"%s\", " \
            "mktId\":%d,\"securityType\":%d,\"subSecurityType\":%d, " \
            "isCancelAble\":%d,\"isReApplyAble\":%d,\"startDate\":%d, \"endDate\":%d," \
            "issueQty\":%d, \"qtyUnit\":%d,\"ordMaxQty\":%d,\"ordMinQty\":%d," \
            "issuePrice\":%d, \"ceilPrice\":%d,\"floorPrice\":%d," \
            "}",
            pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
            pIssue->securityId, pIssue->securityName,
            pIssue->underlyingSecurityId, pIssue->mktId, pIssue->securityType,
            pIssue->subSecurityType, pIssue->isCancelAble,
            pIssue->isReApplyAble, pIssue->startDate, pIssue->endDate,
            pIssue->issueQty, pIssue->qtyUnit, pIssue->ordMaxQty,
            pIssue->ordMinQty, pIssue->issuePrice, pIssue->ceilPrice,
            pIssue->floorPrice);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到证券信息回调 */
void
OesClientMySpi::OnQueryStock(const OesStockItemT *pStock,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到现货产品信息: index[%d], isEnd[%c], " \
      //       "证券代码[%s], 证券名称[%s], 基金代码[%s], " \
      //       "市场代码[%" __SPK_FMT_HH__ "u], 证券类型[%" __SPK_FMT_HH__ "u], " \
      //       "证券子类型[%" __SPK_FMT_HH__ "u], 证券级别[%" __SPK_FMT_HH__ "u], " \
      //       "风险等级[%" __SPK_FMT_HH__ "u], 停牌标志[%" __SPK_FMT_HH__ "u], " \
      //       "适当性管理[%" __SPK_FMT_HH__ "u], 当日回转[%" __SPK_FMT_HH__ "u], " \
      //       "价格单位[%d], 买份数单位[%d], 卖份数单位[%d], " \
      //       "昨日收盘价[%d], 债券利息[%" __SPK_FMT_LL__ "d], " \
      //       "涨停价[%d], 跌停价[%d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pStock->securityId, pStock->securityName, pStock->fundId,
      //       pStock->mktId, pStock->securityType, pStock->subSecurityType,
      //       pStock->securityLevel, pStock->securityRiskLevel, pStock->suspFlag,
      //       pStock->qualificationClass, pStock->isDayTrading,
      //       pStock->priceUnit, pStock->buyQtyUnit, pStock->sellQtyUnit,
      //       pStock->prevClose, pStock->bondInterest,
      //       pStock->priceLimit[OES_TRD_SESS_TYPE_T].ceilPrice,
      //       pStock->priceLimit[OES_TRD_SESS_TYPE_T].floorPrice);

      // fprintf(stdout, sendJsonDataStr);
	/*
      sprintf(sendRespData2client, 
            "{\"msgId\":\"13\", \"seqNo\":%d,\"isEnd\":\"%s\", " \
            "securityId\":\"%s\",\"securityName\":\"%s\", " \
            "fundId\":\"%s\", " \
            "mktId\":%d,\"securityType\":%d,\"subSecurityType\":%d, " \
            "securityLevel\":%d,\"securityRiskLevel\":%d,\"suspFlag\":%d, \"qualificationClass\":%d," \
            "isDayTrading\":%d, \"priceUnit\":%d,\"buyQtyUnit\":%d,\"sellQtyUnit\":%d," \
            "prevClose\":%d, \"bondInterest\":%d,\"ceilPrice\":%d,\"floorPrice\":%d," \
            "}",
            pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
            pStock->securityId, pStock->securityName, pStock->fundId,
            pStock->mktId, pStock->securityType, pStock->subSecurityType,
            pStock->securityLevel, pStock->securityRiskLevel, pStock->suspFlag,
            pStock->qualificationClass, pStock->isDayTrading,
            pStock->priceUnit, pStock->buyQtyUnit, pStock->sellQtyUnit,
            pStock->prevClose, pStock->bondInterest,
            pStock->priceLimit[OES_TRD_SESS_TYPE_T].ceilPrice,
            pStock->priceLimit[OES_TRD_SESS_TYPE_T].floorPrice);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
	  */

	  ptree pt;
	  pt.put("msgId", 146);
	  pt.put("seqNo", pCursor->seqNo);
	  pt.put("isEnd", pCursor->isEnd);
	  pt.put("requestId", requestId);

	  pt.put("securityId", pStock->securityId);
	  pt.put("securityName", pStock->securityName);
	  pt.put("fundId", pStock->fundId);
	  pt.put("mktId", pStock->mktId);
	  pt.put("securityType", pStock->securityType);

	  pt.put("subSecurityType", pStock->subSecurityType);
	  pt.put("securityLevel", pStock->securityLevel);
	  pt.put("securityRiskLevel", pStock->securityRiskLevel);
	  pt.put("currType", pStock->currType);
	  pt.put("qualificationClass", pStock->qualificationClass);
	  pt.put("isDayTrading", pStock->isDayTrading);
	  pt.put("suspFlag", pStock->suspFlag);
	  pt.put("temporarySuspFlag", pStock->temporarySuspFlag);

	  pt.put("buyQtyUnit", pStock->buyQtyUnit);
	  pt.put("sellQtyUnit", pStock->sellQtyUnit);
	  pt.put("buyOrdMaxQty", pStock->buyOrdMaxQty);
	  pt.put("buyOrdMinQty", pStock->buyOrdMinQty);
	  pt.put("sellOrdMaxQty", pStock->sellOrdMaxQty);
	  pt.put("sellOrdMinQty", pStock->sellOrdMinQty);
	  pt.put("priceUnit", pStock->priceUnit);
	  pt.put("prevClose", pStock->prevClose);
	  pt.put("parPrice", pStock->parPrice);

	  pt.put("bondInterest", pStock->bondInterest);
	  pt.put("repoExpirationDays", pStock->repoExpirationDays);
	  pt.put("cashHoldDays", pStock->cashHoldDays);

	  pt.put("ceilPrice_T", pStock->priceLimit[OES_TRD_SESS_TYPE_T].ceilPrice);
	  pt.put("floorPrice_T", pStock->priceLimit[OES_TRD_SESS_TYPE_T].floorPrice);
	  pt.put("ceilPrice_O", pStock->priceLimit[OES_TRD_SESS_TYPE_O].ceilPrice);
	  pt.put("floorPrice_O", pStock->priceLimit[OES_TRD_SESS_TYPE_O].floorPrice);
	  pt.put("ceilPrice_C", pStock->priceLimit[OES_TRD_SESS_TYPE_C].ceilPrice);
	  pt.put("floorPrice_C", pStock->priceLimit[OES_TRD_SESS_TYPE_C].floorPrice);

	  pt.put("username", m_strUsername);

	  std::ostringstream buf;
	  write_json(buf, pt, false);
	  sprintf(sendRespData2client, "%s", buf.str().c_str());
	  spisocket.send(sendRespData2client, strlen(sendRespData2client) + 1, 0);
}


/* 查询到ETF产品信息回调 */
void
OesClientMySpi::OnQueryEtf(const OesEtfItemT *pEtf,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到ETF申赎产品信息: index[%d], isEnd[%c], " \
      //       "基金代码[%s], 证券代码[%s], " \
      //       "市场代码[%" __SPK_FMT_HH__ "u], " \
      //       "申购赎回单位[%d], 最大现金替代比例[%d], " \
      //       "前一日最小申赎单位净值[%" __SPK_FMT_LL__ "d], " \
      //       "前一日现金差额[%" __SPK_FMT_LL__ "d], " \
      //       "成分证券数目[%d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pEtf->fundId, pEtf->securityId, pEtf->mktId, pEtf->creRdmUnit,
      //       pEtf->maxCashRatio, pEtf->navPerCU, pEtf->cashCmpoent,
      //       pEtf->componentCnt);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
            "{\"msgId\":\"13\", \"seqNo\":%d,\"isEnd\":\"%s\", " \
            "custId\":\"%s\",\"securityId\":\"%s\",\"mktId\":%d, " \
            "creRdmUnit\":%d,\"maxCashRatio\":%d,\"navPerCU\":%d,\"cashCmpoent\":%d, " \
            "componentCnt\":%d, " \
            "}",
            pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
            pEtf->fundId, pEtf->securityId, pEtf->mktId, pEtf->creRdmUnit,
            pEtf->maxCashRatio, pEtf->navPerCU, pEtf->cashCmpoent,
            pEtf->componentCnt);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到ETF成分股信息回调 */
void
OesClientMySpi::OnQueryEtfComponent(const OesEtfComponentItemT *pEtfComponent,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到ETF成分股信息: index[%d], isEnd[%c], " \
      //       "成分证券代码[%s], 市场代码[%" __SPK_FMT_HH__ "u], " \
      //       "现金替代标识[%d], 昨日收盘价[%d], 成分证券数量[%d], " \
      //       "溢价比例[%d], 申购替代金额[%" __SPK_FMT_LL__ "d], " \
      //       "赎回替代金额[%" __SPK_FMT_LL__ "d]\n",
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pEtfComponent->securityId, pEtfComponent->mktId,
      //       pEtfComponent->subFlag, pEtfComponent->prevClose,
      //       pEtfComponent->qty, pEtfComponent->premiumRate,
      //       pEtfComponent->creationSubCash, pEtfComponent->redemptionCashSub);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
            "{\"msgId\":\"13\", \"seqNo\":%d,\"isEnd\":\"%s\", " \
            "securityId\":\"%s\",\"mktId\":%d, " \
            "subFlag\":%d,\"prevClose\":%d,\"qty\":%d,\"premiumRate\":%d, " \
            "creationSubCash\":%d, \"redemptionCashSub\":%d, " \
            "}",
            pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
            pEtfComponent->securityId, pEtfComponent->mktId,
            pEtfComponent->subFlag, pEtfComponent->prevClose,
            pEtfComponent->qty, pEtfComponent->premiumRate,
            pEtfComponent->creationSubCash, pEtfComponent->redemptionCashSub);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}


/* 查询到市场状态信息回调 */
void
OesClientMySpi::OnQueryMarketState(const OesMarketStateItemT *pMarketState,
        const OesQryCursorT *pCursor, int32 requestId) {
      // sprintf(sendJsonDataStr, ">>> 查询到市场状态信息: index[%d], isEnd[%c], " \
      //       "交易所代码[%hhu], 交易平台类型[%hhu], 市场类型[%hhu], " \
      //       "市场状态[%hhu]\n", \
      //       pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
      //       pMarketState->exchId, pMarketState->platformId,
      //       pMarketState->mktId, pMarketState->mktState);

      // fprintf(stdout, sendJsonDataStr);

      sprintf(sendRespData2client, 
                  "{\"msgId\":\"13\", \"seqNo\":%d,\"isEnd\":\"%s\", " \
                  "exchId\":%d,\"platformId\":%d, " \
                  "mktId\":%d,\"mktState\":%d, " \
                  "}",
            pCursor->seqNo, pCursor->isEnd ? 'Y' : 'N',
            pMarketState->exchId, pMarketState->platformId,
            pMarketState->mktId, pMarketState->mktState);

      //publisher.publish("oes_resp",sendRespData2client);
      spisocket.send(sendRespData2client,strlen(sendRespData2client)+1,0);
}

