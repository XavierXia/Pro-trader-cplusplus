#include    <Windows.h>
#include    <iostream>
#include    "demo_client.h"
#include    <stdio.h>
#include    <string.h>

#define  __SPK_FMT_LL__                  "I64"
#define  __SPK_FMT_HH__                  ""
#define  __SPK_FMT_SHORT__               ""


//测试使用的变量，仅做参考,主要用于模拟触发买卖和撤单信号
int u = 0;
int j = 0;
int k = 0;
int m = 0;
int n = 0;
int x = 0;
bool bF = true;
bool bS = true;

CDemoClient::CDemoClient(int32 something) {
	this->something = something;
	fprintf(stdout, "...come in CDemoClient::CDemoClient\n");
}


CDemoClient::~CDemoClient() {
	/* do nothing */
}

void CDemoClient::Start()
{
	//连接后端server TOD
	fprintf(stdout, "...CDemoClient start\n");

	//交易
	pZpquantTradeApi = new Zpquant::CZpquantTradeApi();
	//行情
	pZpquantMdApi = new Zpquant::CZpquantMdApi();

	if (!pZpquantTradeApi) {
		fprintf(stderr, "pZpquantTradeApi/pZpquantTradeSpi内存不足!\n");
		return;
	}

	if (!pZpquantMdApi) {
		fprintf(stderr, "pZpquantMdSpi/pZpquantMdSpi 内存不足!\n");
		return;
	}

	/*
		*************************************
		行情类接口实现
	*/
	/* 打印API版本信息 */
	//cout << "ZpquantClientApi 版本: \n" << Zpquant::CZpquantMdApi::GetVersion() << endl;

	/* 注册spi回调接口 */
	pZpquantMdApi->RegisterSpi(this);

	ZpquantUserLoginField userLoginT;
	//测试用户名正常登陆
	strncpy(userLoginT.UserID, "md_client1", sizeof(userLoginT.UserID) - 1);
	//测试用户名为空
	//strncpy(userLoginT.UserID, "", sizeof(userLoginT.UserID) - 1);
	//测试Can not find <username>
	//strncpy(userLoginT.UserID, "client1", sizeof(userLoginT.UserID) - 1);

	//测试密码正常登陆
	strncpy(userLoginT.UserPassword, "pass123", sizeof(userLoginT.UserPassword) - 1);
	//测试密码为空
	//strncpy(userLoginT.UserPassword, "", sizeof(userLoginT.UserPassword) - 1);
	//测试密码错误
	//strncpy(userLoginT.UserPassword, "123456", sizeof(userLoginT.UserPassword) - 1);

	//strncpy(userLoginT.strIP, "47.105.111.100",sizeof(userLoginT.strIP) - 1);
	strncpy(userLoginT.strIP, "127.0.0.1", sizeof(userLoginT.strIP) - 1);
	userLoginT.uPort = 8800;

	cout << "...InitMdSource before!!!" << endl;
	if (!pZpquantMdApi->InitMdSource(&userLoginT)) {
		fprintf(stderr, "InitMdSource失败!\n");
		return;
	}

	//sleep(100);

	if (!pZpquantMdApi->Start()) {
		fprintf(stderr, "启动API失败!\n");
		return;
	}
	Sleep(1000);

	/*
	*************************************
	交易类接口实现
	*/

	/* 打印API版本信息 */
	//cout << "pZpquantTradeApi 版本: \n" << Zpquant::CZpquantTradeApi::GetVersion() << endl;

	/* 注册spi回调接口 */
	pZpquantTradeApi->RegisterSpi(this);

	ZpquantUserLoginField userLoginTradeT;
	strncpy(userLoginTradeT.UserID, "td_client1", sizeof(userLoginTradeT.UserID) - 1);
	strncpy(userLoginTradeT.UserPassword, "pass123", sizeof(userLoginTradeT.UserPassword) - 1);
	strncpy(userLoginTradeT.strIP, "127.0.0.1", sizeof(userLoginTradeT.strIP) - 1);
	userLoginTradeT.uPort = 8800;

	cout << "...InitTraderSource before!!!" << endl;
	if (!pZpquantTradeApi->InitTraderSource(&userLoginTradeT)) {
		fprintf(stderr, "...InitTraderSource!\n");
		return;
	}

	/*
	 * 设置登录OES时使用的用户名和密码
	 * @note 如通过API接口设置，则可以不在配置文件中配置;
	 *          支持通过前缀指定密码类型, 如 md5:PASSWORD, txt:PASSWORD
	 */
	 // pOesApi->SetThreadUsername("customer1");
	 // pOesApi->SetThreadPassword("txt:123456");
	 // pOesApi->SetThreadPassword("md5:e10adc3949ba59abbe56e057f20f883e");

	 /* 启动 */
	if (!pZpquantTradeApi->Start()) {
		fprintf(stderr, "启动API失败!\n");
		return;
	}

	/* 打印当前交易日 */
	fprintf(stdout, "服务端交易日: %08d\n", pZpquantTradeApi->GetTradingDay());
}

void CDemoClient::test_trade()
{
	int32 dataType = MDS_SUB_DATA_TYPE_L2_SNAPSHOT | MDS_SUB_DATA_TYPE_L2_ORDER | MDS_SUB_DATA_TYPE_L2_TRADE;
	//行情订阅测试
	if (0)
	{
		string codelist1("600000.SH,000011.SZ");
		pZpquantMdApi->SubscribeMarketData(codelist1.c_str(), dataType, MDS_SUB_MODE_SET);
	}

	//追加行情订阅测试
	if (0)
	{
		string codelist2("600519.SH,000516.SZ,600547.SH,300413.SZ,000001.SZ");
		pZpquantMdApi->SubscribeMarketData(codelist2.c_str(), dataType, MDS_SUB_MODE_APPEND);
	}

	//删除行情订阅测试
	if (0)
	{
		string codelist3("300413.SZ,000011.SZ");
		pZpquantMdApi->SubscribeMarketData(codelist3.c_str(), dataType, MDS_SUB_MODE_DELETE);
	}

	if (1)
	{
		string codelist1("600000.SH,000001.SZ");
		pZpquantMdApi->SubscribeMarketData(codelist1.c_str(), dataType, MDS_SUB_MODE_SET);
	}

	/*
	*************************************
	交易下单接口实例
	*/
	/*
	{"msgId":"13", "seqNo":1,"isEnd":"Y", invAcctId":"A188800368","securityId":"600000","mktId":1,
	"originalHld":1000000, totalBuyHld":0,"totalSellHld":0,"sellFrzHld":0, totalTrsfInHld":0,"totalTrsfOutHld":0,
	"trsfOutFrzHld":0,"lockHld":0, lockFrzHld":0,"unlockFrzHld":0,"coveredFrzHld":0,"coveredHld":0,
	coveredAvlHld":0,"sumHld":1000000,"sellAvlHld":1000000,"trsfOutAvlHld":1000000,lockAvlHld":1000000}
	*/
	Sleep(1000);

	//测试 查询特定某只股票持仓，上海，若不测试该项，请使用if(0)
	if (0)
	{
		string codeString = "600000";
		pZpquantTradeApi->QueryStkHolding(codeString.c_str(), 1, 0);
	}

	//测试查询特定某只股票持仓，深圳
	if (0)
	{
		string codeString = "000001";
		pZpquantTradeApi->QueryStkHolding(codeString.c_str(), 2, 0);
	}

	//查询账号中两市场的持仓信息
	if (0)
	{
		string codeString1 = "allStk";
		uint8 sclb = 1; //1或2都可以使用
		pZpquantTradeApi->QueryStkHolding(codeString1.c_str(), sclb, 0);
	}

	Sleep(1000);

	//下单买入 上海
	if (0)
	{
		/*
		ZpquantOrdReqT zOrdReqT;
		string codeString2 = "600519";
		memset(zOrdReqT.pSecurityId, 0, 7 * sizeof(char));
		memcpy(zOrdReqT.pSecurityId, codeString2.c_str(), 7 * sizeof(char));

		zOrdReqT.mktId = 1;
		zOrdReqT.ordType = 0;
		zOrdReqT.bsType = 1; //买入
		zOrdReqT.ordQty = 200;
		zOrdReqT.ordPrice = 9800000;
		pZpquantTradeApi->SendOrder(&zOrdReqT);
		*/

		string codeString2 = "600519";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 1, 1, 9800000, 200);
	}

	//下单买入 深圳
	if (0)
	{
		string codeString2 = "000516";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 2, 1, 42000, 1000);
	}

	//下单卖出 深圳
	if (0)
	{
		string codeString2 = "000001";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 2, 2, 140000, 300);
	}

	//下单卖出 上海
	if (0)
	{
		string codeString2 = "600519";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 1, 2, 9600000, 100);
	}

	//委托之后撤单，必须在下单回报中进行测试，
	//见 委托回报函数实现
	if (0)
	{
		//通过待撤委托的 origClOrdId 进行撤单
		int64 origClOrdId = 111;//待撤委托的origClOrdId需要通过回报消息获取
		pZpquantTradeApi->SendCancelOrder(1, 0, 0, origClOrdId);
	}

	if (0)
	{
		//通过待撤委托的 origClSeqNo 进行撤单
		//origClEnvId     被撤委托的客户端环境号 (小于等于0, 则使用当前会话的 clEnvId)
		int32 origClSeqNo = 1;//待撤委托的origClSeqNo需要通过回报消息获取
		int8 origClEnvId = 11;//待撤委托的origClEnvId需要通过回报消息获取
		int64 origClOrdId = 0; //填0，表示不用该项撤单,第四项
		pZpquantTradeApi->SendCancelOrder(2, origClSeqNo, origClEnvId, 0);
	}
}

/*
************************************行情接口
*/

/*
{"msgType":"20","mktData":"{\"head\":{\"exchId\":1,\"securityType\":1,\"tradeDate\":20190729,\"updateTime\":133027000,
\"mdStreamType\":20,\"__origMdSource\":3,\"__dataVersion\":3096,\"__origTickSeq\":571346614566969126,\"__channelNo\":8,
\"__origNetTime\":133028498,\"__recvTime\":1566972636.299121,\"__collectedTime\":1566972636.299121,
\"__processedTime\":1566972636.299132,\"__pushingTime\":1566972636.299151},\"body\":{\"SecurityID\":\"600519\",
\"TradingPhaseCode\":\"T111\",\"NumTrades\":11320,\"TotalVolumeTraded\":1801349,\"TotalValueTraded\":17494352414900,
\"PrevClosePx\":9650300,\"TradePx\":9767500,\"OpenPx\":9620300,\"ClosePx\":0,\"HighPx\":9780800,\"LowPx\":9600100,
\"IOPV\":0,\"NAV\":0,\"TotalLongPosition\":0,\"TotalBidQty\":335463,\"TotalOfferQty\":371851,\"WeightedAvgBidPx\":9481790,
\"WeightedAvgOfferPx\":9961680,\"BidPriceLevel\":510,\"OfferPriceLevel\":502,
\"bidPrice\":[9765500,9765000,9764500,9764400,9761600,9761000,9760500,9758600,9758400,9758100],
\"bidNumberOfOrders\":[1,6,1,1,2,1,1,1,1,1],\"bidOrderQty\":[700,1800,100,100,700,100,100,100,300,100],
\"offerPrice\":[9768000,9769900,9770000,9771900,9772500,9772700,9774700,9775000,9776900,9777000],
\"offerNumberOfOrders\":[1,2,15,1,2,1,1,3,2,2],\"offerOrderQty\":[300,1100,2000,100,200,100,100,300,800,2900]}}"}
*/
void
CDemoClient::OnTickRtnDepthMarketData(ZpquantMdsL2OnTickT *pDepthMarketData)
{
	fprintf(stdout, ">>> OnTick: " \
		"证券代码[%s], " \
		"TradePx[%d], HighPx[%d], " \
		"\n",
		pDepthMarketData->SecurityID,
		pDepthMarketData->TradePx, pDepthMarketData->HighPx);
	/*
	u++;
	//下单买入 深圳
	if (u > 10)
	{
		string code = pDepthMarketData->SecurityID;
		if (pDepthMarketData->securityType == 1
			|| code == "000001")
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 2, 1, pDepthMarketData->OfferLevels[1].Price, 100);
		}

		//测试 将全部已发送订单添加至“当日委托”列表。
		//测试 将全部已成交订单添加至“当日成交”列表。
		for (int i = 0; i < 1; i++)
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 2, 1, pDepthMarketData->OfferLevels[1].Price, 100);
			//Sleep(200);
		}

		u = 0;
	}

	j++;
	//下单买入 上海
	if (j > 20)
	{
		
		string code = pDepthMarketData->SecurityID;
		if (pDepthMarketData->securityType == 1
			|| code == "600000")
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 1, 1, 5000, 100);

			//测试 将全部拒绝转发的委托添加至“被拒绝委托”列表。
			for (int i = 0; i < 1; i++)
			{
				pZpquantTradeApi->SendOrder(code.c_str(), 1, 1, 5000, 100);
				//Sleep(2000);
			}
		}
		j = 0;
	}

	k++;
	//下单卖出 上海
	if (k > 30)
	{
		string code = pDepthMarketData->SecurityID;
		if (pDepthMarketData->securityType == 1
			|| code == "600000")
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 1, 1, pDepthMarketData->BidLevels[2].Price, 100);

			//测试 将全部已发送订单添加至“当日委托”列表。
			//测试 将全部已成交订单添加至“当日成交”列表。
			for (int i = 0; i < 1; i++)
			{
				pZpquantTradeApi->SendOrder(code.c_str(), 1, 1, pDepthMarketData->BidLevels[2].Price, 100);
				//Sleep(2000);
			}
		}
		k = 0;
	}
	*/

	x++;
	//下单买入 深圳
	//if (x > 5)
	if(bS)
	{
		string code = pDepthMarketData->SecurityID;
		if (pDepthMarketData->securityType == 1
			|| code == "000001")
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 2, 1, pDepthMarketData->BidLevels[5].Price, 100);
		}

		//测试 将全部已发送订单添加至“当日委托”列表。
		for (int i = 0; i < 2; i++)
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 2, 1, pDepthMarketData->BidLevels[5].Price-10000, 100);
		}

		//x = 0;
		bS = false;
	}

}

/*
{"msgType":"22","mktData":"{\"exchId\":2,\"securityType\":1,\"tradeDate\":20190729,\"TransactTime\":133208750,
\"ChannelNo\":2011,\"ApplSeqNum\":7339359,\"SecurityID\":\"000516\",\"ExecType\":\"F\",\"TradeBSFlag\":\"N\",
\"TradePrice\":50600,\"TradeQty\":2000,\"TradeMoney\":101200000,\"BidApplSeqNum\":7339356,\"OfferApplSeqNum\":6865614,
\"__origTickSeq\":82742506,\"__channelNo\":8,\"__origNetTime\":133209916,\"__recvTime\":1566972802.269135,
\"__collectedTime\":1566972802.269135,\"__processedTime\":1566972802.269182,\"__pushingTime\":1566972802.269212}"}
*/
void
CDemoClient::OnTradeRtnDepthMarketData(ZpquantMdsL2OnTradeT *pDepthMarketData)
{
	fprintf(stdout, ">>> OnTrade: " \
		"证券代码[%s], " \
		"TradePrice[%d], TradeQty[%d], " \
		"\n",
		pDepthMarketData->SecurityID,
		pDepthMarketData->TradePrice, pDepthMarketData->TradeQty);
}

/*
{"msgType":"23","mktData":"{\"exchId\":2,\"securityType\":1,\"tradeDate\":20190729,\"TransactTime\":133147290,
\"ChannelNo\":2011,\"ApplSeqNum\":7329796,\"SecurityID\":\"000516\",\"Side\":\"1\",\"OrderType\":\"2\",\"Price\":50500,
\"OrderQty\":17100,\"__origTickSeq\":82612009,\"__channelNo\":8,\"__origNetTime\":133148441,\"__recvTime\":1566972767.448407,
\"__collectedTime\":1566972767.448407,\"__processedTime\":1566972767.448473,\"__pushingTime\":1566972767.448481}"}
*/
void
CDemoClient::OnOrderRtnDepthMarketData(ZpquantMdsL2OnOrderT *pDepthMarketData)
{
	fprintf(stdout, ">>> OnOrder: " \
		"证券代码[%s], " \
		"Price[%d], OrderQty[%d], " \
		"\n",
		pDepthMarketData->SecurityID,
		pDepthMarketData->Price, pDepthMarketData->OrderQty);
}

/*
************************************交易接口
*/
/* 委托业务拒绝回报 */
void
CDemoClient::OnBusinessReject(int32 errorCode, const ZpquantOrdReject *pOrderReject) {
	sprintf(sendJsonDataStr, ">>> 收到委托业务拒绝回报: " \
		"客户端环境号[%" __SPK_FMT_HH__ "d], " \
		"客户委托流水号[%d], 证券账户[%s], 证券代码[%s], " \
		"市场代码[%" __SPK_FMT_HH__ "u], 委托类型[%" __SPK_FMT_HH__ "u], " \
		"买卖类型[%" __SPK_FMT_HH__ "u], 委托数量[%d], 委托价格[%d], " \
		"原始委托的客户订单编号[%" __SPK_FMT_LL__ "d], 错误码[%d]\n",
		pOrderReject->clEnvId, pOrderReject->clSeqNo,
		pOrderReject->invAcctId, pOrderReject->securityId,
		pOrderReject->mktId, pOrderReject->ordType,
		pOrderReject->bsType, pOrderReject->ordQty,
		pOrderReject->ordPrice, pOrderReject->origClOrdId, errorCode);

	fprintf(stdout, sendJsonDataStr);
}


/* 委托已收回报 */
void
CDemoClient::OnOrderInsert(const ZpquantOrdCnfm *pOrderInsert) {
	//sprintf(sendjsondatastr, ">>> 收到委托已收回报: " \
      //      "客户端环境号[%" __spk_fmt_hh__ "d], 客户委托流水号[%d], " \
      //      "会员内部编号[%" __spk_fmt_ll__ "d], 证券账户[%s], 证券代码[%s], " \
      //      "市场代码[%" __spk_fmt_hh__ "u], 订单类型[%" __spk_fmt_hh__ "u], " \
      //      "买卖类型[%" __spk_fmt_hh__ "u], 委托状态[%" __spk_fmt_hh__ "u], " \
      //      "委托日期[%d], 委托接收时间[%d], 委托确认时间[%d], " \
      //      "委托数量[%d], 委托价格[%d], 撤单数量[%d], 累计成交份数[%d], " \
      //      "累计成交金额[%" __spk_fmt_ll__ "d], 累计债券利息[%" __spk_fmt_ll__ "d], " \
      //      "累计交易佣金[%" __spk_fmt_ll__ "d], 冻结交易金额[%" __spk_fmt_ll__ "d], " \
      //      "冻结债券利息[%" __spk_fmt_ll__ "d], 冻结交易佣金[%" __spk_fmt_ll__ "d], " \
      //      "被撤内部委托编号[%" __spk_fmt_ll__ "d], 拒绝原因[%d], 交易所错误码[%d]\n",
	  //      porderinsert->clenvid, porderinsert->clseqno,
	  //      porderinsert->clordid, porderinsert->invacctid,
	  //      porderinsert->securityid, porderinsert->mktid,
	  //      porderinsert->ordtype, porderinsert->bstype,
	  //      porderinsert->ordstatus, porderinsert->orddate,
	  //      porderinsert->ordtime, porderinsert->ordcnfmtime,
	  //      porderinsert->ordqty, porderinsert->ordprice,
	  //      porderinsert->canceledqty, porderinsert->cumqty,
	  //      porderinsert->cumamt, porderinsert->cuminterest,
	  //      porderinsert->cumfee, porderinsert->frzamt,
	  //      porderinsert->frzinterest, porderinsert->frzfee,
	  //      porderinsert->origclordid, porderinsert->ordrejreason,
	  //      porderinsert->excherrcode);

	sprintf(sendJsonDataStr, ">>> 收到委托已收回报: " \
		"客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
		"会员内部编号[%" __SPK_FMT_LL__ "d],订单类型[%" __SPK_FMT_HH__ "u]",
		pOrderInsert->clEnvId, pOrderInsert->clSeqNo,
		pOrderInsert->clOrdId, pOrderInsert->bsType);

	fprintf(stdout, sendJsonDataStr);

	// sprintf(sendRespData2client, 
	//             "{\"clEnvId\":\"%d\",\"clSeqNo\":\"%d\",\"clOrdId\":\"%d\",\"invAcctId\":\"%s\", " \
      //             "securityId\":\"%s\",\"mktId\":\"%d\",\"ordType\":\"%d\",\"bsType\":\"%d\", " \
      //             "ordStatus\":\"%d\",\"ordDate\":\"%d\",\"ordTime\":\"%d\",\"ordCnfmTime\":\"%d\", " \
      //             "ordQty\":\"%d\",\"ordPrice\":\"%d\",\"canceledQty\":\"%d\",\"cumQty\":\"%d\", " \
      //             "cumAmt\":\"%d\",\"cumInterest\":\"%d\",\"cumFee\":\"%d\",\"cumInterest\":\"%d\", " \
      //             "cumFee\":\"%d\",\"frzAmt\":\"%d\",\"frzInterest\":\"%d\",\"frzFee\":\"%d\", " \
      //             "origClOrdId\":\"%d\",\"ordRejReason\":\"%d\",\"exchErrCode\":\"%d\", " \
      //             "}",
	  //             pOrderInsert->clEnvId, pOrderInsert->clSeqNo,
	  //             pOrderInsert->clOrdId, pOrderInsert->invAcctId,
	  //             pOrderInsert->securityId, pOrderInsert->mktId,
	  //             pOrderInsert->ordType, pOrderInsert->bsType,
	  //             pOrderInsert->ordStatus, pOrderInsert->ordDate,
	  //             pOrderInsert->ordTime, pOrderInsert->ordCnfmTime,
	  //             pOrderInsert->ordQty, pOrderInsert->ordPrice,
	  //             pOrderInsert->canceledQty, pOrderInsert->cumQty,
	  //             pOrderInsert->cumAmt, pOrderInsert->cumInterest,
	  //             pOrderInsert->cumFee, pOrderInsert->frzAmt,
	  //             pOrderInsert->frzInterest, pOrderInsert->frzFee,
	  //             pOrderInsert->origClOrdId, pOrderInsert->ordRejReason,
	  //             pOrderInsert->exchErrCode);

	/*
	m++;
	//if (m > 3)
	if(bF)
	{
		//通过待撤委托的 origClOrdId 进行撤单
		int64 origClOrdId = pOrderInsert->clOrdId;//待撤委托的origClOrdId需要通过回报消息获取
		pZpquantTradeApi->SendCancelOrder(pOrderInsert->mktId, 0, 0, origClOrdId);

		//m = 0;
		bF = false;
	}
	*/
	
}


/* 委托确认回报 */
void
CDemoClient::OnOrderReport(int32 errorCode, const ZpquantOrdCnfm *pOrderReport) {
	////sprintf(sendJsonDataStr, ">>> 收到委托回报: " \
      //      "客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
      //      "会员内部编号[%" __SPK_FMT_LL__ "d], 证券账户[%s], 证券代码[%s], " \
      //      "市场代码[%" __SPK_FMT_HH__ "u], 订单类型[%" __SPK_FMT_HH__ "u], " \
      //      "买卖类型[%" __SPK_FMT_HH__ "u], 委托状态[%" __SPK_FMT_HH__ "u], " \
      //      "委托日期[%d], 委托接收时间[%d], 委托确认时间[%d], "
	  //      "委托数量[%d], 委托价格[%d], 撤单数量[%d], 累计成交份数[%d], " \
      //      "累计成交金额[%" __SPK_FMT_LL__ "d], 累计债券利息[%" __SPK_FMT_LL__ "d], " \
      //      "累计交易佣金[%" __SPK_FMT_LL__ "d], 冻结交易金额[%" __SPK_FMT_LL__ "d], " \
      //      "冻结债券利息[%" __SPK_FMT_LL__ "d], 冻结交易佣金[%" __SPK_FMT_LL__ "d], " \
      //      "被撤内部委托编号[%" __SPK_FMT_LL__ "d], 拒绝原因[%d], 交易所错误码[%d]\n",
	  //      pOrderReport->clEnvId, pOrderReport->clSeqNo,
	  //      pOrderReport->clOrdId, pOrderReport->invAcctId,
	  //      pOrderReport->securityId, pOrderReport->mktId,
	  //      pOrderReport->ordType, pOrderReport->bsType,
	  //      pOrderReport->ordStatus, pOrderReport->ordDate,
	  //      pOrderReport->ordTime, pOrderReport->ordCnfmTime,
	  //      pOrderReport->ordQty, pOrderReport->ordPrice,
	  //      pOrderReport->canceledQty, pOrderReport->cumQty,
	  //      pOrderReport->cumAmt, pOrderReport->cumInterest,
	  //      pOrderReport->cumFee, pOrderReport->frzAmt,
	  //      pOrderReport->frzInterest, pOrderReport->frzFee,
	  //      pOrderReport->origClOrdId, pOrderReport->ordRejReason,
	  //      pOrderReport->exchErrCode);

	  //fprintf(stdout, sendJsonDataStr);

	sprintf(sendJsonDataStr, ">>> 收到委托回报: " \
		"客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
		"会员内部编号[%" __SPK_FMT_LL__ "d],订单类型[%" __SPK_FMT_HH__ "u]\n",
		pOrderReport->clEnvId, pOrderReport->clSeqNo,
		pOrderReport->clOrdId, pOrderReport->bsType);

	fprintf(stdout, sendJsonDataStr);
	switch (pOrderReport->bsType)
	{
	case 1:
	{
		fprintf(stdout, "...............买入......委托确认回报............\n");
		break;
	}
	case 2:
	{
		fprintf(stdout, "...............卖出......委托确认回报............\n");
		break;
	}
	case 30:
	{
		fprintf(stdout, "...............撤单......委托确认回报............\n");
		break;
	}
	}

	m++;
	//if (m > 3)
	if (bF)
	{
		//通过待撤委托的 origClOrdId 进行撤单
		int64 origClOrdId = pOrderReport->clOrdId;//待撤委托的origClOrdId需要通过回报消息获取
		pZpquantTradeApi->SendCancelOrder(pOrderReport->mktId, 0, 0, origClOrdId);

		//m = 0;
		bF = false;
	}
}


/* 成交回报 */
void
CDemoClient::OnTradeReport(const ZpquantTrdCnfm *pTradeReport) {
	sprintf(sendJsonDataStr, ">>> 收到成交回报: " \
		"成交编号[%" __SPK_FMT_LL__ "d], 会员内部编号[%" __SPK_FMT_LL__ "d], " \
		"委托客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
		"证券账户[%s], 证券代码[%s], 市场代码[%" __SPK_FMT_HH__ "u], " \
		"买卖方向[%" __SPK_FMT_HH__ "u], 委托买卖类型[%" __SPK_FMT_HH__ "u], "
		"成交日期[%d], 成交时间[%d], 成交数量[%d], 成交价格[%d], " \
		"成交金额[%" __SPK_FMT_LL__ "d], 累计成交数量[%d], " \
		"累计成交金额[%" __SPK_FMT_LL__ "d], 累计债券利息[%" __SPK_FMT_LL__ "d], " \
		"累计交易费用[%" __SPK_FMT_LL__ "d], PBU代码[%d]\n",
		pTradeReport->exchTrdNum, pTradeReport->clOrdId,
		pTradeReport->clEnvId, pTradeReport->clSeqNo,
		pTradeReport->invAcctId, pTradeReport->securityId,
		pTradeReport->mktId, pTradeReport->trdSide,
		pTradeReport->ordBuySellType, pTradeReport->trdDate,
		pTradeReport->trdTime, pTradeReport->trdQty, pTradeReport->trdPrice,
		pTradeReport->trdAmt, pTradeReport->cumQty, pTradeReport->cumAmt,
		pTradeReport->cumInterest, pTradeReport->cumFee,
		pTradeReport->pbuId);

	fprintf(stdout, sendJsonDataStr);
}


/* 资金变动回报 */
void
CDemoClient::OnCashAssetVariation(const ZpquantCashAssetItem *pCashAssetItem) {
	sprintf(sendJsonDataStr, ">>> 收到资金变动回报: " \
		"资金账户代码[%s], 客户代码[%s], " \
		"币种[%" __SPK_FMT_HH__ "u], " \
		"资金类型[%" __SPK_FMT_HH__ "u], " \
		"资金账户状态[%" __SPK_FMT_HH__ "u], " \
		"期初余额[%" __SPK_FMT_LL__ "d], " \
		"期初可用余额[%" __SPK_FMT_LL__ "d], " \
		"期初可取余额[%" __SPK_FMT_LL__ "d], " \
		"不可用余额[%" __SPK_FMT_LL__ "d], " \
		"累计存入金额[%" __SPK_FMT_LL__ "d], " \
		"累计提取金额[%" __SPK_FMT_LL__ "d], " \
		"当前提取冻结金额[%" __SPK_FMT_LL__ "d], " \
		"累计卖金额[%" __SPK_FMT_LL__ "d], " \
		"累计买金额[%" __SPK_FMT_LL__ "d], " \
		"当前买冻结金额[%" __SPK_FMT_LL__ "d], " \
		"累计费用金额[%" __SPK_FMT_LL__ "d], " \
		"当前费用冻结金额[%" __SPK_FMT_LL__ "d], " \
		"当前维持保证金金额[%" __SPK_FMT_LL__ "d], " \
		"当前保证金冻结金额[%" __SPK_FMT_LL__ "d], " \
		"当前余额[%" __SPK_FMT_LL__ "d], " \
		"当前可用余额[%" __SPK_FMT_LL__ "d], " \
		"当前可取余额[%" __SPK_FMT_LL__ "d]\n",
		pCashAssetItem->cashAcctId, pCashAssetItem->custId,
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

	fprintf(stdout, sendJsonDataStr);
}


/* 持仓变动回报 */
void
CDemoClient::OnStockHoldingVariation(const ZpquantStkHoldingItem *pStkHoldingItem) {
	//sprintf(sendJsonDataStr, ">>> 收到持仓变动回报: " \
      //      "证券账户[%s], 产品代码[%s], " \
      //      "市场代码[%" __SPK_FMT_HH__ "u], " \
      //      "日初持仓[%" __SPK_FMT_LL__ "d], " \
      //      "日中累计买入持仓[%" __SPK_FMT_LL__ "d], " \
      //      "日中累计卖出持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前卖出冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //      "手动冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //      "日中累计转换获得持仓[%" __SPK_FMT_LL__ "d], " \
      //      "日中累计转换付出持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前转换付出冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前已锁定持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前锁定冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前解锁定冻结持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前备兑冻结的现货持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前已备兑使用的现货持仓[%" __SPK_FMT_LL__ "d], " \
      //      "日初总持仓成本[%" __SPK_FMT_LL__ "d], " \
      //      "当日累计买入金额[%" __SPK_FMT_LL__ "d], " \
      //      "当日累计卖出金额[%" __SPK_FMT_LL__ "d], " \
      //      "当日累计买入费用[%" __SPK_FMT_LL__ "d], " \
      //      "当日累计卖出费用[%" __SPK_FMT_LL__ "d], " \
      //      "持仓成本价[%" __SPK_FMT_LL__ "d], " \
      //      "当前总持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前可卖持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前可转换付出持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前可锁定持仓[%" __SPK_FMT_LL__ "d], " \
      //      "当前可备兑/解锁持仓[%" __SPK_FMT_LL__ "d]\n",
	  //      pStkHoldingItem->invAcctId, pStkHoldingItem->securityId,
	  //      pStkHoldingItem->mktId, pStkHoldingItem->originalHld,
	  //      pStkHoldingItem->totalBuyHld, pStkHoldingItem->totalSellHld,
	  //      pStkHoldingItem->sellFrzHld, pStkHoldingItem->manualFrzHld,
	  //      pStkHoldingItem->totalTrsfInHld, pStkHoldingItem->totalTrsfOutHld,
	  //      pStkHoldingItem->trsfOutFrzHld, pStkHoldingItem->lockHld,
	  //      pStkHoldingItem->lockFrzHld, pStkHoldingItem->unlockFrzHld,
	  //      pStkHoldingItem->coveredFrzHld, pStkHoldingItem->coveredHld,
	  //      pStkHoldingItem->originalCostAmt, pStkHoldingItem->totalBuyAmt,
	  //      pStkHoldingItem->totalSellAmt, pStkHoldingItem->totalBuyFee,
	  //      pStkHoldingItem->totalSellFee, pStkHoldingItem->costPrice,
	  //      pStkHoldingItem->sumHld, pStkHoldingItem->sellAvlHld,
	  //      pStkHoldingItem->trsfOutAvlHld, pStkHoldingItem->lockAvlHld,
	  //      pStkHoldingItem->coveredAvlHld);

	sprintf(sendJsonDataStr, ">>> 收到持仓变动回报: " \
		"证券账户[%s], 产品代码[%s], " \
		"市场代码[%" __SPK_FMT_HH__ "u], " \
		"日初持仓[%" __SPK_FMT_LL__ "d], " \
		"当前可备兑/解锁持仓[%" __SPK_FMT_LL__ "d]\n",
		pStkHoldingItem->invAcctId, pStkHoldingItem->securityId,
		pStkHoldingItem->mktId, pStkHoldingItem->originalHld,
		pStkHoldingItem->coveredAvlHld);

	fprintf(stdout, sendJsonDataStr);
}


/* 出入金委托拒绝回报 */
void
CDemoClient::OnFundTrsfReject(int32 errorCode,
	const ZpquantFundTrsfReject *pFundTrsfReject) {
	sprintf(sendJsonDataStr, ">>> 收到出入金委托拒绝回报: " \
		"错误码[%d], 错误信息[%s], " \
		"客户端环境号[%" __SPK_FMT_HH__ "d], " \
		"出入金流水号[%d], 资金账户[%s], " \
		"是否仅调拨[%" __SPK_FMT_HH__ "u], " \
		"出入金方向[%" __SPK_FMT_HH__ "u], " \
		"出入金金额[%" __SPK_FMT_LL__ "d]\n",
		pFundTrsfReject->rejReason, pFundTrsfReject->errorInfo,
		pFundTrsfReject->clEnvId, pFundTrsfReject->clSeqNo,
		pFundTrsfReject->cashAcctId, pFundTrsfReject->isAllotOnly,
		pFundTrsfReject->direct, pFundTrsfReject->occurAmt);

	fprintf(stdout, sendJsonDataStr);
}


/* 出入金委托执行回报 */
void
CDemoClient::OnFundTrsfReport(int32 errorCode,
	const ZpquantFundTrsfReport *pFundTrsfReport) {
	sprintf(sendJsonDataStr, ">>> 收到出入金委托执行回报: " \
		"错误原因[%d], 主柜错误码[%d], 错误信息[%s], " \
		"客户端环境号[%" __SPK_FMT_HH__ "d], " \
		"出入金流水号[%d], 出入金编号[%d], 资金账户[%s], " \
		"是否仅调拨[%" __SPK_FMT_HH__ "u], " \
		"出入金方向[%" __SPK_FMT_HH__ "u], " \
		"出入金金额[%" __SPK_FMT_LL__ "d], " \
		"出入金状态[%d], 接收日期[%08d], " \
		"接收时间[%09d], 上报时间[%09d], 完成时间[%09d]\n",
		pFundTrsfReport->rejReason, pFundTrsfReport->counterErrCode,
		pFundTrsfReport->errorInfo, pFundTrsfReport->clEnvId,
		pFundTrsfReport->clSeqNo, pFundTrsfReport->fundTrsfId,
		pFundTrsfReport->cashAcctId, pFundTrsfReport->isAllotOnly,
		pFundTrsfReport->direct, pFundTrsfReport->occurAmt,
		pFundTrsfReport->trsfStatus, pFundTrsfReport->operDate,
		pFundTrsfReport->operTime, pFundTrsfReport->dclrTime,
		pFundTrsfReport->doneTime);

	fprintf(stdout, sendJsonDataStr);
}


/* 市场状态信息 */
void
CDemoClient::OnMarketState(const ZpquantMarketStateInfo *pMarketStateItem) {
	sprintf(sendJsonDataStr, ">>> 收到市场状态信息: " \
		"交易所代码[%hhu], 交易平台类型[%hhu], 市场类型[%hhu], "
		"市场状态[%hhu]\n",
		pMarketStateItem->exchId, pMarketStateItem->platformId,
		pMarketStateItem->mktId, pMarketStateItem->mktState);

	fprintf(stdout, sendJsonDataStr);
}


/* 查询到委托信息回调 */
void
CDemoClient::OnQueryOrder(const ZpquantOrdItem *pOrder,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> 查询到委托信息: index[%d], isEnd[%c], " \
		"客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
		"会员内部编号[%" __SPK_FMT_LL__ "d], 证券账户[%s], 证券代码[%s], " \
		"市场代码[%" __SPK_FMT_HH__ "u], 订单类型[%" __SPK_FMT_HH__ "u], " \
		"买卖类型[%" __SPK_FMT_HH__ "u], 委托状态[%" __SPK_FMT_HH__ "u], " \
		"委托日期[%d], 委托接收时间[%d], 委托确认时间[%d], " \
		"委托数量[%d], 委托价格[%d], 撤单数量[%d], 累计成交份数[%d], " \
		"累计成交金额[%" __SPK_FMT_LL__ "d], 累计债券利息[%" __SPK_FMT_LL__ "d], " \
		"累计交易佣金[%" __SPK_FMT_LL__ "d], 冻结交易金额[%" __SPK_FMT_LL__ "d], " \
		"冻结债券利息[%" __SPK_FMT_LL__ "d], 冻结交易佣金[%" __SPK_FMT_LL__ "d], " \
		"被撤内部委托编号[%" __SPK_FMT_LL__ "d], 拒绝原因[%d], " \
		"交易所错误码[%d]\n",
		pCursor->seqNo, pCursor->isEnd,
		pOrder->clEnvId, pOrder->clSeqNo, pOrder->clOrdId,
		pOrder->invAcctId, pOrder->securityId, pOrder->mktId,
		pOrder->ordType, pOrder->bsType, pOrder->ordStatus,
		pOrder->ordDate, pOrder->ordTime, pOrder->ordCnfmTime,
		pOrder->ordQty, pOrder->ordPrice, pOrder->canceledQty,
		pOrder->cumQty, pOrder->cumAmt, pOrder->cumInterest,
		pOrder->cumFee, pOrder->frzAmt, pOrder->frzInterest,
		pOrder->frzFee, pOrder->origClOrdId, pOrder->ordRejReason,
		pOrder->exchErrCode);

	fprintf(stdout, sendJsonDataStr);
}


/* 查询到成交信息回调 */
void
CDemoClient::OnQueryTrade(const ZpquantTrdItem *pTrade,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> 查询到成交信息: index[%d], isEnd[%c], " \
		"成交编号[%" __SPK_FMT_LL__ "d], 会员内部编号[%" __SPK_FMT_LL__ "d], " \
		"委托客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
		"证券账户[%s], 证券代码[%s], 市场代码[%" __SPK_FMT_HH__ "u], " \
		"买卖方向[%" __SPK_FMT_HH__ "u], 委托买卖类型[%" __SPK_FMT_HH__ "u], " \
		"成交日期[%d], 成交时间[%d], 成交数量[%d], 成交价格[%d], " \
		"成交金额[%" __SPK_FMT_LL__ "d], 累计成交数量[%d], " \
		"累计成交金额[%" __SPK_FMT_LL__ "d], 累计债券利息[%" __SPK_FMT_LL__ "d], " \
		"累计交易费用[%" __SPK_FMT_LL__ "d], PBU代码[%d]\n",
		pCursor->seqNo, pCursor->isEnd,
		pTrade->exchTrdNum, pTrade->clOrdId, pTrade->clEnvId,
		pTrade->clSeqNo, pTrade->invAcctId, pTrade->securityId,
		pTrade->mktId, pTrade->trdSide, pTrade->ordBuySellType,
		pTrade->trdDate, pTrade->trdTime, pTrade->trdQty, pTrade->trdPrice,
		pTrade->trdAmt, pTrade->cumQty, pTrade->cumAmt, pTrade->cumInterest,
		pTrade->cumFee, pTrade->pbuId);

	fprintf(stdout, sendJsonDataStr);
}


/* 查询到资金信息回调 */
void
CDemoClient::OnQueryCashAsset(const ZpquantCashAssetItem *pCashAsset,
	const ZpquantQryCursor *pCursor, int32 requestId) {
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
		pCursor->seqNo, pCursor->isEnd,
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
}


/* 查询到持仓信息回调 */
void
CDemoClient::OnQueryStkHolding(const ZpquantStkHoldingItem *pStkHolding,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	fprintf(stdout, "...demoTraderClientSpi::OnQueryStkHolding\n");

	sprintf(sendJsonDataStr, ">>> 查询到股票持仓信息: index[%d], isEnd[%c], " \
		"证券账户[%s], 市场代码[%" __SPK_FMT_HH__ "u], 产品代码[%s], " \
		"日初持仓[%" __SPK_FMT_LL__ "d], " \
		"日中累计买入[%" __SPK_FMT_LL__ "d], " \
		"日中累计卖出[%" __SPK_FMT_LL__ "d], " \
		"当前卖出冻结[%" __SPK_FMT_LL__ "d], " \
		"日中累计转换获得[%" __SPK_FMT_LL__ "d], " \
		"日中累计转换付出[%" __SPK_FMT_LL__ "d], " \
		"当前转换付出冻结[%" __SPK_FMT_LL__ "d], " \
		"当前已锁定[%" __SPK_FMT_LL__ "d], " \
		"当前锁定冻结[%" __SPK_FMT_LL__ "d], " \
		"当前解锁定冻结[%" __SPK_FMT_LL__ "d], " \
		"当前备兑冻结[%" __SPK_FMT_LL__ "d], " \
		"当前已备兑使用[%" __SPK_FMT_LL__ "d], " \
		"当前可备兑/解锁[%" __SPK_FMT_LL__ "d], " \
		"当前总持仓[%" __SPK_FMT_LL__ "d], " \
		"当前可卖[%" __SPK_FMT_LL__ "d], " \
		"当前可转换付出[%" __SPK_FMT_LL__ "d], " \
		"当前可锁定[%" __SPK_FMT_LL__ "d]\n",
		pCursor->seqNo, pCursor->isEnd,
		pStkHolding->invAcctId, pStkHolding->mktId, pStkHolding->securityId,
		pStkHolding->originalHld, pStkHolding->totalBuyHld,
		pStkHolding->totalSellHld, pStkHolding->sellFrzHld,
		pStkHolding->totalTrsfInHld, pStkHolding->totalTrsfOutHld,
		pStkHolding->trsfOutFrzHld, pStkHolding->lockHld,
		pStkHolding->lockFrzHld, pStkHolding->unlockFrzHld,
		pStkHolding->coveredFrzHld, pStkHolding->coveredHld,
		pStkHolding->coveredAvlHld, pStkHolding->sumHld,
		pStkHolding->sellAvlHld, pStkHolding->trsfOutAvlHld,
		pStkHolding->lockAvlHld);

	fprintf(stdout, sendJsonDataStr);
}

/* 查询到客户信息回调 */
void
CDemoClient::OnQueryCustInfo(const ZpquantCustItemT *pCust,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> 查询到客户信息: index[%d], isEnd[%c], " \
		"客户ID[%s], 客户类型[%" __SPK_FMT_HH__ "u], " \
		"客户状态[%" __SPK_FMT_HH__ "u], 风险评级[%" __SPK_FMT_HH__ "u], " \
		"机构标志[%" __SPK_FMT_HH__ "u], 投资者分类[%c]\n",
		pCursor->seqNo, pCursor->isEnd,
		pCust->custId, pCust->custType, pCust->status,
		pCust->riskLevel, pCust->institutionFlag,
		pCust->investorClass == INVESTOR_CLASS_NORMAL ?
		'0' : pCust->investorClass + 'A' - 1);

	fprintf(stdout, sendJsonDataStr);
}


/* 查询到股东账户信息回调 */
void
CDemoClient::OnQueryInvAcct(const ZpquantInvAcctItem *pInvAcct,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	fprintf(stdout, ">>> 查询到证券账户信息: index[%d], isEnd[%c], " \
		"证券账户[%s], 市场代码[%" __SPK_FMT_HH__ "u], " \
		"客户代码[%s], 账户状态[%" __SPK_FMT_HH__ "u], " \
		"新股认购限额[%d]\n",
		pCursor->seqNo, pCursor->isEnd,
		pInvAcct->invAcctId, pInvAcct->mktId,
		pInvAcct->custId, pInvAcct->status,
		pInvAcct->subscriptionQuota);

	fprintf(stdout, sendJsonDataStr);
}

/* 查询到出入金流水信息回调 */
void
CDemoClient::OnQueryFundTransferSerial(
	const ZpquantFundTransferSerialItemT *pFundTrsf,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> 查询到出入金流水: index[%d], isEnd[%c], " \
		"客户端环境号[%" __SPK_FMT_HH__ "d], 客户委托流水号[%d], " \
		"资金账户[%s], 方向[%s], 金额[%" __SPK_FMT_LL__ "d], " \
		"出入金状态[%" __SPK_FMT_HH__ "u], 错误原因[%d], 主柜错误码[%d], " \
		"错误信息[%s], 柜台委托编码[%d], 接收日期[%08d], " \
		"接收时间[%09d], 上报时间[%09d], 完成时间[%09d]\n",
		pCursor->seqNo, pCursor->isEnd,
		pFundTrsf->clEnvId, pFundTrsf->clSeqNo,
		pFundTrsf->cashAcctId,
		pFundTrsf->direct == FUND_TRSF_DIRECT_IN ? "Bank->Broker" : "Broker->Bank",
		pFundTrsf->occurAmt, pFundTrsf->trsfStatus, pFundTrsf->rejReason,
		pFundTrsf->counterErrCode, pFundTrsf->errorInfo,
		pFundTrsf->counterEntrustNo, pFundTrsf->operDate,
		pFundTrsf->operTime, pFundTrsf->dclrTime,
		pFundTrsf->doneTime);

	fprintf(stdout, sendJsonDataStr);
}


/* 查询到证券信息回调 */
void
CDemoClient::OnQueryStock(const ZpquantStockBaseInfo *pStock,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> 查询到现货产品信息: index[%d], isEnd[%c], " \
		"证券代码[%s], 证券名称[%s], 基金代码[%s], " \
		"市场代码[%" __SPK_FMT_HH__ "u], 证券类型[%" __SPK_FMT_HH__ "u], " \
		"证券子类型[%" __SPK_FMT_HH__ "u], 证券级别[%" __SPK_FMT_HH__ "u], " \
		"风险等级[%" __SPK_FMT_HH__ "u], 停牌标志[%" __SPK_FMT_HH__ "u], " \
		"适当性管理[%" __SPK_FMT_HH__ "u], 当日回转[%" __SPK_FMT_HH__ "u], " \
		"价格单位[%d], 买份数单位[%d], 卖份数单位[%d], " \
		"昨日收盘价[%d], 债券利息[%" __SPK_FMT_LL__ "d], " \
		"涨停价[%d], 跌停价[%d]\n",
		pCursor->seqNo, pCursor->isEnd,
		pStock->securityId, pStock->securityName, pStock->fundId,
		pStock->mktId, pStock->securityType, pStock->subSecurityType,
		pStock->securityLevel, pStock->securityRiskLevel, pStock->suspFlag,
		pStock->qualificationClass, pStock->isDayTrading,
		pStock->priceUnit, pStock->buyQtyUnit, pStock->sellQtyUnit,
		pStock->prevClose, pStock->bondInterest,
		pStock->priceLimit[TRD_SESS_TYPE_T].ceilPrice,
		pStock->priceLimit[TRD_SESS_TYPE_T].floorPrice);

	fprintf(stdout, sendJsonDataStr);
}


/* 查询到市场状态信息回调 */
void
CDemoClient::OnQueryMarketState(const ZpquantMarketStateInfo *pMarketState,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> 查询到市场状态信息: index[%d], isEnd[%c], " \
		"交易所代码[%hhu], 交易平台类型[%hhu], 市场类型[%hhu], " \
		"市场状态[%hhu]\n", \
		pCursor->seqNo, pCursor->isEnd,
		pMarketState->exchId, pMarketState->platformId,
		pMarketState->mktId, pMarketState->mktState);

	fprintf(stdout, sendJsonDataStr);
}

//获取交易日
void CDemoClient::onGetTradingDay(const ZpquantTradingDayInfo * pTradingDayInfo)
{
	sprintf(sendJsonDataStr, ">>> 获取交易日: 交易日[%d], isTradingDay[%d]\n",
		pTradingDayInfo->tradingDay, pTradingDayInfo->isTradingDay);

	fprintf(stdout, sendJsonDataStr);
}
