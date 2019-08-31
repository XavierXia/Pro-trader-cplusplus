#include    <Windows.h>
#include    <iostream>
#include    "demo_client.h"
#include    <stdio.h>
#include    <string.h>

#define  __SPK_FMT_LL__                  "I64"
#define  __SPK_FMT_HH__                  ""
#define  __SPK_FMT_SHORT__               ""


//����ʹ�õı����������ο�,��Ҫ����ģ�ⴥ�������ͳ����ź�
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
	//���Ӻ��server TOD
	fprintf(stdout, "...CDemoClient start\n");

	//����
	pZpquantTradeApi = new Zpquant::CZpquantTradeApi();
	//����
	pZpquantMdApi = new Zpquant::CZpquantMdApi();

	if (!pZpquantTradeApi) {
		fprintf(stderr, "pZpquantTradeApi/pZpquantTradeSpi�ڴ治��!\n");
		return;
	}

	if (!pZpquantMdApi) {
		fprintf(stderr, "pZpquantMdSpi/pZpquantMdSpi �ڴ治��!\n");
		return;
	}

	/*
		*************************************
		������ӿ�ʵ��
	*/
	/* ��ӡAPI�汾��Ϣ */
	//cout << "ZpquantClientApi �汾: \n" << Zpquant::CZpquantMdApi::GetVersion() << endl;

	/* ע��spi�ص��ӿ� */
	pZpquantMdApi->RegisterSpi(this);

	ZpquantUserLoginField userLoginT;
	//�����û���������½
	strncpy(userLoginT.UserID, "md_client1", sizeof(userLoginT.UserID) - 1);
	//�����û���Ϊ��
	//strncpy(userLoginT.UserID, "", sizeof(userLoginT.UserID) - 1);
	//����Can not find <username>
	//strncpy(userLoginT.UserID, "client1", sizeof(userLoginT.UserID) - 1);

	//��������������½
	strncpy(userLoginT.UserPassword, "pass123", sizeof(userLoginT.UserPassword) - 1);
	//��������Ϊ��
	//strncpy(userLoginT.UserPassword, "", sizeof(userLoginT.UserPassword) - 1);
	//�����������
	//strncpy(userLoginT.UserPassword, "123456", sizeof(userLoginT.UserPassword) - 1);

	//strncpy(userLoginT.strIP, "47.105.111.100",sizeof(userLoginT.strIP) - 1);
	strncpy(userLoginT.strIP, "127.0.0.1", sizeof(userLoginT.strIP) - 1);
	userLoginT.uPort = 8800;

	cout << "...InitMdSource before!!!" << endl;
	if (!pZpquantMdApi->InitMdSource(&userLoginT)) {
		fprintf(stderr, "InitMdSourceʧ��!\n");
		return;
	}

	//sleep(100);

	if (!pZpquantMdApi->Start()) {
		fprintf(stderr, "����APIʧ��!\n");
		return;
	}
	Sleep(1000);

	/*
	*************************************
	������ӿ�ʵ��
	*/

	/* ��ӡAPI�汾��Ϣ */
	//cout << "pZpquantTradeApi �汾: \n" << Zpquant::CZpquantTradeApi::GetVersion() << endl;

	/* ע��spi�ص��ӿ� */
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
	 * ���õ�¼OESʱʹ�õ��û���������
	 * @note ��ͨ��API�ӿ����ã�����Բ��������ļ�������;
	 *          ֧��ͨ��ǰ׺ָ����������, �� md5:PASSWORD, txt:PASSWORD
	 */
	 // pOesApi->SetThreadUsername("customer1");
	 // pOesApi->SetThreadPassword("txt:123456");
	 // pOesApi->SetThreadPassword("md5:e10adc3949ba59abbe56e057f20f883e");

	 /* ���� */
	if (!pZpquantTradeApi->Start()) {
		fprintf(stderr, "����APIʧ��!\n");
		return;
	}

	/* ��ӡ��ǰ������ */
	fprintf(stdout, "����˽�����: %08d\n", pZpquantTradeApi->GetTradingDay());
}

void CDemoClient::test_trade()
{
	int32 dataType = MDS_SUB_DATA_TYPE_L2_SNAPSHOT | MDS_SUB_DATA_TYPE_L2_ORDER | MDS_SUB_DATA_TYPE_L2_TRADE;
	//���鶩�Ĳ���
	if (0)
	{
		string codelist1("600000.SH,000011.SZ");
		pZpquantMdApi->SubscribeMarketData(codelist1.c_str(), dataType, MDS_SUB_MODE_SET);
	}

	//׷�����鶩�Ĳ���
	if (0)
	{
		string codelist2("600519.SH,000516.SZ,600547.SH,300413.SZ,000001.SZ");
		pZpquantMdApi->SubscribeMarketData(codelist2.c_str(), dataType, MDS_SUB_MODE_APPEND);
	}

	//ɾ�����鶩�Ĳ���
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
	�����µ��ӿ�ʵ��
	*/
	/*
	{"msgId":"13", "seqNo":1,"isEnd":"Y", invAcctId":"A188800368","securityId":"600000","mktId":1,
	"originalHld":1000000, totalBuyHld":0,"totalSellHld":0,"sellFrzHld":0, totalTrsfInHld":0,"totalTrsfOutHld":0,
	"trsfOutFrzHld":0,"lockHld":0, lockFrzHld":0,"unlockFrzHld":0,"coveredFrzHld":0,"coveredHld":0,
	coveredAvlHld":0,"sumHld":1000000,"sellAvlHld":1000000,"trsfOutAvlHld":1000000,lockAvlHld":1000000}
	*/
	Sleep(1000);

	//���� ��ѯ�ض�ĳֻ��Ʊ�ֲ֣��Ϻ����������Ը����ʹ��if(0)
	if (0)
	{
		string codeString = "600000";
		pZpquantTradeApi->QueryStkHolding(codeString.c_str(), 1, 0);
	}

	//���Բ�ѯ�ض�ĳֻ��Ʊ�ֲ֣�����
	if (0)
	{
		string codeString = "000001";
		pZpquantTradeApi->QueryStkHolding(codeString.c_str(), 2, 0);
	}

	//��ѯ�˺������г��ĳֲ���Ϣ
	if (0)
	{
		string codeString1 = "allStk";
		uint8 sclb = 1; //1��2������ʹ��
		pZpquantTradeApi->QueryStkHolding(codeString1.c_str(), sclb, 0);
	}

	Sleep(1000);

	//�µ����� �Ϻ�
	if (0)
	{
		/*
		ZpquantOrdReqT zOrdReqT;
		string codeString2 = "600519";
		memset(zOrdReqT.pSecurityId, 0, 7 * sizeof(char));
		memcpy(zOrdReqT.pSecurityId, codeString2.c_str(), 7 * sizeof(char));

		zOrdReqT.mktId = 1;
		zOrdReqT.ordType = 0;
		zOrdReqT.bsType = 1; //����
		zOrdReqT.ordQty = 200;
		zOrdReqT.ordPrice = 9800000;
		pZpquantTradeApi->SendOrder(&zOrdReqT);
		*/

		string codeString2 = "600519";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 1, 1, 9800000, 200);
	}

	//�µ����� ����
	if (0)
	{
		string codeString2 = "000516";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 2, 1, 42000, 1000);
	}

	//�µ����� ����
	if (0)
	{
		string codeString2 = "000001";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 2, 2, 140000, 300);
	}

	//�µ����� �Ϻ�
	if (0)
	{
		string codeString2 = "600519";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 1, 2, 9600000, 100);
	}

	//ί��֮�󳷵����������µ��ر��н��в��ԣ�
	//�� ί�лر�����ʵ��
	if (0)
	{
		//ͨ������ί�е� origClOrdId ���г���
		int64 origClOrdId = 111;//����ί�е�origClOrdId��Ҫͨ���ر���Ϣ��ȡ
		pZpquantTradeApi->SendCancelOrder(1, 0, 0, origClOrdId);
	}

	if (0)
	{
		//ͨ������ί�е� origClSeqNo ���г���
		//origClEnvId     ����ί�еĿͻ��˻����� (С�ڵ���0, ��ʹ�õ�ǰ�Ự�� clEnvId)
		int32 origClSeqNo = 1;//����ί�е�origClSeqNo��Ҫͨ���ر���Ϣ��ȡ
		int8 origClEnvId = 11;//����ί�е�origClEnvId��Ҫͨ���ر���Ϣ��ȡ
		int64 origClOrdId = 0; //��0����ʾ���ø����,������
		pZpquantTradeApi->SendCancelOrder(2, origClSeqNo, origClEnvId, 0);
	}
}

/*
************************************����ӿ�
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
		"֤ȯ����[%s], " \
		"TradePx[%d], HighPx[%d], " \
		"\n",
		pDepthMarketData->SecurityID,
		pDepthMarketData->TradePx, pDepthMarketData->HighPx);
	/*
	u++;
	//�µ����� ����
	if (u > 10)
	{
		string code = pDepthMarketData->SecurityID;
		if (pDepthMarketData->securityType == 1
			|| code == "000001")
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 2, 1, pDepthMarketData->OfferLevels[1].Price, 100);
		}

		//���� ��ȫ���ѷ��Ͷ��������������ί�С��б�
		//���� ��ȫ���ѳɽ���������������ճɽ����б�
		for (int i = 0; i < 1; i++)
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 2, 1, pDepthMarketData->OfferLevels[1].Price, 100);
			//Sleep(200);
		}

		u = 0;
	}

	j++;
	//�µ����� �Ϻ�
	if (j > 20)
	{
		
		string code = pDepthMarketData->SecurityID;
		if (pDepthMarketData->securityType == 1
			|| code == "600000")
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 1, 1, 5000, 100);

			//���� ��ȫ���ܾ�ת����ί������������ܾ�ί�С��б�
			for (int i = 0; i < 1; i++)
			{
				pZpquantTradeApi->SendOrder(code.c_str(), 1, 1, 5000, 100);
				//Sleep(2000);
			}
		}
		j = 0;
	}

	k++;
	//�µ����� �Ϻ�
	if (k > 30)
	{
		string code = pDepthMarketData->SecurityID;
		if (pDepthMarketData->securityType == 1
			|| code == "600000")
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 1, 1, pDepthMarketData->BidLevels[2].Price, 100);

			//���� ��ȫ���ѷ��Ͷ��������������ί�С��б�
			//���� ��ȫ���ѳɽ���������������ճɽ����б�
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
	//�µ����� ����
	//if (x > 5)
	if(bS)
	{
		string code = pDepthMarketData->SecurityID;
		if (pDepthMarketData->securityType == 1
			|| code == "000001")
		{
			pZpquantTradeApi->SendOrder(code.c_str(), 2, 1, pDepthMarketData->BidLevels[5].Price, 100);
		}

		//���� ��ȫ���ѷ��Ͷ��������������ί�С��б�
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
		"֤ȯ����[%s], " \
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
		"֤ȯ����[%s], " \
		"Price[%d], OrderQty[%d], " \
		"\n",
		pDepthMarketData->SecurityID,
		pDepthMarketData->Price, pDepthMarketData->OrderQty);
}

/*
************************************���׽ӿ�
*/
/* ί��ҵ��ܾ��ر� */
void
CDemoClient::OnBusinessReject(int32 errorCode, const ZpquantOrdReject *pOrderReject) {
	sprintf(sendJsonDataStr, ">>> �յ�ί��ҵ��ܾ��ر�: " \
		"�ͻ��˻�����[%" __SPK_FMT_HH__ "d], " \
		"�ͻ�ί����ˮ��[%d], ֤ȯ�˻�[%s], ֤ȯ����[%s], " \
		"�г�����[%" __SPK_FMT_HH__ "u], ί������[%" __SPK_FMT_HH__ "u], " \
		"��������[%" __SPK_FMT_HH__ "u], ί������[%d], ί�м۸�[%d], " \
		"ԭʼί�еĿͻ��������[%" __SPK_FMT_LL__ "d], ������[%d]\n",
		pOrderReject->clEnvId, pOrderReject->clSeqNo,
		pOrderReject->invAcctId, pOrderReject->securityId,
		pOrderReject->mktId, pOrderReject->ordType,
		pOrderReject->bsType, pOrderReject->ordQty,
		pOrderReject->ordPrice, pOrderReject->origClOrdId, errorCode);

	fprintf(stdout, sendJsonDataStr);
}


/* ί�����ջر� */
void
CDemoClient::OnOrderInsert(const ZpquantOrdCnfm *pOrderInsert) {
	//sprintf(sendjsondatastr, ">>> �յ�ί�����ջر�: " \
      //      "�ͻ��˻�����[%" __spk_fmt_hh__ "d], �ͻ�ί����ˮ��[%d], " \
      //      "��Ա�ڲ����[%" __spk_fmt_ll__ "d], ֤ȯ�˻�[%s], ֤ȯ����[%s], " \
      //      "�г�����[%" __spk_fmt_hh__ "u], ��������[%" __spk_fmt_hh__ "u], " \
      //      "��������[%" __spk_fmt_hh__ "u], ί��״̬[%" __spk_fmt_hh__ "u], " \
      //      "ί������[%d], ί�н���ʱ��[%d], ί��ȷ��ʱ��[%d], " \
      //      "ί������[%d], ί�м۸�[%d], ��������[%d], �ۼƳɽ�����[%d], " \
      //      "�ۼƳɽ����[%" __spk_fmt_ll__ "d], �ۼ�ծȯ��Ϣ[%" __spk_fmt_ll__ "d], " \
      //      "�ۼƽ���Ӷ��[%" __spk_fmt_ll__ "d], ���ύ�׽��[%" __spk_fmt_ll__ "d], " \
      //      "����ծȯ��Ϣ[%" __spk_fmt_ll__ "d], ���ύ��Ӷ��[%" __spk_fmt_ll__ "d], " \
      //      "�����ڲ�ί�б��[%" __spk_fmt_ll__ "d], �ܾ�ԭ��[%d], ������������[%d]\n",
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

	sprintf(sendJsonDataStr, ">>> �յ�ί�����ջر�: " \
		"�ͻ��˻�����[%" __SPK_FMT_HH__ "d], �ͻ�ί����ˮ��[%d], " \
		"��Ա�ڲ����[%" __SPK_FMT_LL__ "d],��������[%" __SPK_FMT_HH__ "u]",
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
		//ͨ������ί�е� origClOrdId ���г���
		int64 origClOrdId = pOrderInsert->clOrdId;//����ί�е�origClOrdId��Ҫͨ���ر���Ϣ��ȡ
		pZpquantTradeApi->SendCancelOrder(pOrderInsert->mktId, 0, 0, origClOrdId);

		//m = 0;
		bF = false;
	}
	*/
	
}


/* ί��ȷ�ϻر� */
void
CDemoClient::OnOrderReport(int32 errorCode, const ZpquantOrdCnfm *pOrderReport) {
	////sprintf(sendJsonDataStr, ">>> �յ�ί�лر�: " \
      //      "�ͻ��˻�����[%" __SPK_FMT_HH__ "d], �ͻ�ί����ˮ��[%d], " \
      //      "��Ա�ڲ����[%" __SPK_FMT_LL__ "d], ֤ȯ�˻�[%s], ֤ȯ����[%s], " \
      //      "�г�����[%" __SPK_FMT_HH__ "u], ��������[%" __SPK_FMT_HH__ "u], " \
      //      "��������[%" __SPK_FMT_HH__ "u], ί��״̬[%" __SPK_FMT_HH__ "u], " \
      //      "ί������[%d], ί�н���ʱ��[%d], ί��ȷ��ʱ��[%d], "
	  //      "ί������[%d], ί�м۸�[%d], ��������[%d], �ۼƳɽ�����[%d], " \
      //      "�ۼƳɽ����[%" __SPK_FMT_LL__ "d], �ۼ�ծȯ��Ϣ[%" __SPK_FMT_LL__ "d], " \
      //      "�ۼƽ���Ӷ��[%" __SPK_FMT_LL__ "d], ���ύ�׽��[%" __SPK_FMT_LL__ "d], " \
      //      "����ծȯ��Ϣ[%" __SPK_FMT_LL__ "d], ���ύ��Ӷ��[%" __SPK_FMT_LL__ "d], " \
      //      "�����ڲ�ί�б��[%" __SPK_FMT_LL__ "d], �ܾ�ԭ��[%d], ������������[%d]\n",
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

	sprintf(sendJsonDataStr, ">>> �յ�ί�лر�: " \
		"�ͻ��˻�����[%" __SPK_FMT_HH__ "d], �ͻ�ί����ˮ��[%d], " \
		"��Ա�ڲ����[%" __SPK_FMT_LL__ "d],��������[%" __SPK_FMT_HH__ "u]\n",
		pOrderReport->clEnvId, pOrderReport->clSeqNo,
		pOrderReport->clOrdId, pOrderReport->bsType);

	fprintf(stdout, sendJsonDataStr);
	switch (pOrderReport->bsType)
	{
	case 1:
	{
		fprintf(stdout, "...............����......ί��ȷ�ϻر�............\n");
		break;
	}
	case 2:
	{
		fprintf(stdout, "...............����......ί��ȷ�ϻر�............\n");
		break;
	}
	case 30:
	{
		fprintf(stdout, "...............����......ί��ȷ�ϻر�............\n");
		break;
	}
	}

	m++;
	//if (m > 3)
	if (bF)
	{
		//ͨ������ί�е� origClOrdId ���г���
		int64 origClOrdId = pOrderReport->clOrdId;//����ί�е�origClOrdId��Ҫͨ���ر���Ϣ��ȡ
		pZpquantTradeApi->SendCancelOrder(pOrderReport->mktId, 0, 0, origClOrdId);

		//m = 0;
		bF = false;
	}
}


/* �ɽ��ر� */
void
CDemoClient::OnTradeReport(const ZpquantTrdCnfm *pTradeReport) {
	sprintf(sendJsonDataStr, ">>> �յ��ɽ��ر�: " \
		"�ɽ����[%" __SPK_FMT_LL__ "d], ��Ա�ڲ����[%" __SPK_FMT_LL__ "d], " \
		"ί�пͻ��˻�����[%" __SPK_FMT_HH__ "d], �ͻ�ί����ˮ��[%d], " \
		"֤ȯ�˻�[%s], ֤ȯ����[%s], �г�����[%" __SPK_FMT_HH__ "u], " \
		"��������[%" __SPK_FMT_HH__ "u], ί����������[%" __SPK_FMT_HH__ "u], "
		"�ɽ�����[%d], �ɽ�ʱ��[%d], �ɽ�����[%d], �ɽ��۸�[%d], " \
		"�ɽ����[%" __SPK_FMT_LL__ "d], �ۼƳɽ�����[%d], " \
		"�ۼƳɽ����[%" __SPK_FMT_LL__ "d], �ۼ�ծȯ��Ϣ[%" __SPK_FMT_LL__ "d], " \
		"�ۼƽ��׷���[%" __SPK_FMT_LL__ "d], PBU����[%d]\n",
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


/* �ʽ�䶯�ر� */
void
CDemoClient::OnCashAssetVariation(const ZpquantCashAssetItem *pCashAssetItem) {
	sprintf(sendJsonDataStr, ">>> �յ��ʽ�䶯�ر�: " \
		"�ʽ��˻�����[%s], �ͻ�����[%s], " \
		"����[%" __SPK_FMT_HH__ "u], " \
		"�ʽ�����[%" __SPK_FMT_HH__ "u], " \
		"�ʽ��˻�״̬[%" __SPK_FMT_HH__ "u], " \
		"�ڳ����[%" __SPK_FMT_LL__ "d], " \
		"�ڳ��������[%" __SPK_FMT_LL__ "d], " \
		"�ڳ���ȡ���[%" __SPK_FMT_LL__ "d], " \
		"���������[%" __SPK_FMT_LL__ "d], " \
		"�ۼƴ�����[%" __SPK_FMT_LL__ "d], " \
		"�ۼ���ȡ���[%" __SPK_FMT_LL__ "d], " \
		"��ǰ��ȡ������[%" __SPK_FMT_LL__ "d], " \
		"�ۼ������[%" __SPK_FMT_LL__ "d], " \
		"�ۼ�����[%" __SPK_FMT_LL__ "d], " \
		"��ǰ�򶳽���[%" __SPK_FMT_LL__ "d], " \
		"�ۼƷ��ý��[%" __SPK_FMT_LL__ "d], " \
		"��ǰ���ö�����[%" __SPK_FMT_LL__ "d], " \
		"��ǰά�ֱ�֤����[%" __SPK_FMT_LL__ "d], " \
		"��ǰ��֤�𶳽���[%" __SPK_FMT_LL__ "d], " \
		"��ǰ���[%" __SPK_FMT_LL__ "d], " \
		"��ǰ�������[%" __SPK_FMT_LL__ "d], " \
		"��ǰ��ȡ���[%" __SPK_FMT_LL__ "d]\n",
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


/* �ֱֲ䶯�ر� */
void
CDemoClient::OnStockHoldingVariation(const ZpquantStkHoldingItem *pStkHoldingItem) {
	//sprintf(sendJsonDataStr, ">>> �յ��ֱֲ䶯�ر�: " \
      //      "֤ȯ�˻�[%s], ��Ʒ����[%s], " \
      //      "�г�����[%" __SPK_FMT_HH__ "u], " \
      //      "�ճ��ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "�����ۼ�����ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "�����ۼ������ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ��������ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "�ֶ�����ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "�����ۼ�ת����óֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "�����ۼ�ת�������ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰת����������ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ�������ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ��������ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ����������ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ���Ҷ�����ֻ��ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ�ѱ���ʹ�õ��ֻ��ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "�ճ��ֲֳܳɱ�[%" __SPK_FMT_LL__ "d], " \
      //      "�����ۼ�������[%" __SPK_FMT_LL__ "d], " \
      //      "�����ۼ��������[%" __SPK_FMT_LL__ "d], " \
      //      "�����ۼ��������[%" __SPK_FMT_LL__ "d], " \
      //      "�����ۼ���������[%" __SPK_FMT_LL__ "d], " \
      //      "�ֲֳɱ���[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ�ֲܳ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ�����ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ��ת�������ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ�������ֲ�[%" __SPK_FMT_LL__ "d], " \
      //      "��ǰ�ɱ���/�����ֲ�[%" __SPK_FMT_LL__ "d]\n",
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

	sprintf(sendJsonDataStr, ">>> �յ��ֱֲ䶯�ر�: " \
		"֤ȯ�˻�[%s], ��Ʒ����[%s], " \
		"�г�����[%" __SPK_FMT_HH__ "u], " \
		"�ճ��ֲ�[%" __SPK_FMT_LL__ "d], " \
		"��ǰ�ɱ���/�����ֲ�[%" __SPK_FMT_LL__ "d]\n",
		pStkHoldingItem->invAcctId, pStkHoldingItem->securityId,
		pStkHoldingItem->mktId, pStkHoldingItem->originalHld,
		pStkHoldingItem->coveredAvlHld);

	fprintf(stdout, sendJsonDataStr);
}


/* �����ί�оܾ��ر� */
void
CDemoClient::OnFundTrsfReject(int32 errorCode,
	const ZpquantFundTrsfReject *pFundTrsfReject) {
	sprintf(sendJsonDataStr, ">>> �յ������ί�оܾ��ر�: " \
		"������[%d], ������Ϣ[%s], " \
		"�ͻ��˻�����[%" __SPK_FMT_HH__ "d], " \
		"�������ˮ��[%d], �ʽ��˻�[%s], " \
		"�Ƿ������[%" __SPK_FMT_HH__ "u], " \
		"�������[%" __SPK_FMT_HH__ "u], " \
		"�������[%" __SPK_FMT_LL__ "d]\n",
		pFundTrsfReject->rejReason, pFundTrsfReject->errorInfo,
		pFundTrsfReject->clEnvId, pFundTrsfReject->clSeqNo,
		pFundTrsfReject->cashAcctId, pFundTrsfReject->isAllotOnly,
		pFundTrsfReject->direct, pFundTrsfReject->occurAmt);

	fprintf(stdout, sendJsonDataStr);
}


/* �����ί��ִ�лر� */
void
CDemoClient::OnFundTrsfReport(int32 errorCode,
	const ZpquantFundTrsfReport *pFundTrsfReport) {
	sprintf(sendJsonDataStr, ">>> �յ������ί��ִ�лر�: " \
		"����ԭ��[%d], ���������[%d], ������Ϣ[%s], " \
		"�ͻ��˻�����[%" __SPK_FMT_HH__ "d], " \
		"�������ˮ��[%d], �������[%d], �ʽ��˻�[%s], " \
		"�Ƿ������[%" __SPK_FMT_HH__ "u], " \
		"�������[%" __SPK_FMT_HH__ "u], " \
		"�������[%" __SPK_FMT_LL__ "d], " \
		"�����״̬[%d], ��������[%08d], " \
		"����ʱ��[%09d], �ϱ�ʱ��[%09d], ���ʱ��[%09d]\n",
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


/* �г�״̬��Ϣ */
void
CDemoClient::OnMarketState(const ZpquantMarketStateInfo *pMarketStateItem) {
	sprintf(sendJsonDataStr, ">>> �յ��г�״̬��Ϣ: " \
		"����������[%hhu], ����ƽ̨����[%hhu], �г�����[%hhu], "
		"�г�״̬[%hhu]\n",
		pMarketStateItem->exchId, pMarketStateItem->platformId,
		pMarketStateItem->mktId, pMarketStateItem->mktState);

	fprintf(stdout, sendJsonDataStr);
}


/* ��ѯ��ί����Ϣ�ص� */
void
CDemoClient::OnQueryOrder(const ZpquantOrdItem *pOrder,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> ��ѯ��ί����Ϣ: index[%d], isEnd[%c], " \
		"�ͻ��˻�����[%" __SPK_FMT_HH__ "d], �ͻ�ί����ˮ��[%d], " \
		"��Ա�ڲ����[%" __SPK_FMT_LL__ "d], ֤ȯ�˻�[%s], ֤ȯ����[%s], " \
		"�г�����[%" __SPK_FMT_HH__ "u], ��������[%" __SPK_FMT_HH__ "u], " \
		"��������[%" __SPK_FMT_HH__ "u], ί��״̬[%" __SPK_FMT_HH__ "u], " \
		"ί������[%d], ί�н���ʱ��[%d], ί��ȷ��ʱ��[%d], " \
		"ί������[%d], ί�м۸�[%d], ��������[%d], �ۼƳɽ�����[%d], " \
		"�ۼƳɽ����[%" __SPK_FMT_LL__ "d], �ۼ�ծȯ��Ϣ[%" __SPK_FMT_LL__ "d], " \
		"�ۼƽ���Ӷ��[%" __SPK_FMT_LL__ "d], ���ύ�׽��[%" __SPK_FMT_LL__ "d], " \
		"����ծȯ��Ϣ[%" __SPK_FMT_LL__ "d], ���ύ��Ӷ��[%" __SPK_FMT_LL__ "d], " \
		"�����ڲ�ί�б��[%" __SPK_FMT_LL__ "d], �ܾ�ԭ��[%d], " \
		"������������[%d]\n",
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


/* ��ѯ���ɽ���Ϣ�ص� */
void
CDemoClient::OnQueryTrade(const ZpquantTrdItem *pTrade,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> ��ѯ���ɽ���Ϣ: index[%d], isEnd[%c], " \
		"�ɽ����[%" __SPK_FMT_LL__ "d], ��Ա�ڲ����[%" __SPK_FMT_LL__ "d], " \
		"ί�пͻ��˻�����[%" __SPK_FMT_HH__ "d], �ͻ�ί����ˮ��[%d], " \
		"֤ȯ�˻�[%s], ֤ȯ����[%s], �г�����[%" __SPK_FMT_HH__ "u], " \
		"��������[%" __SPK_FMT_HH__ "u], ί����������[%" __SPK_FMT_HH__ "u], " \
		"�ɽ�����[%d], �ɽ�ʱ��[%d], �ɽ�����[%d], �ɽ��۸�[%d], " \
		"�ɽ����[%" __SPK_FMT_LL__ "d], �ۼƳɽ�����[%d], " \
		"�ۼƳɽ����[%" __SPK_FMT_LL__ "d], �ۼ�ծȯ��Ϣ[%" __SPK_FMT_LL__ "d], " \
		"�ۼƽ��׷���[%" __SPK_FMT_LL__ "d], PBU����[%d]\n",
		pCursor->seqNo, pCursor->isEnd,
		pTrade->exchTrdNum, pTrade->clOrdId, pTrade->clEnvId,
		pTrade->clSeqNo, pTrade->invAcctId, pTrade->securityId,
		pTrade->mktId, pTrade->trdSide, pTrade->ordBuySellType,
		pTrade->trdDate, pTrade->trdTime, pTrade->trdQty, pTrade->trdPrice,
		pTrade->trdAmt, pTrade->cumQty, pTrade->cumAmt, pTrade->cumInterest,
		pTrade->cumFee, pTrade->pbuId);

	fprintf(stdout, sendJsonDataStr);
}


/* ��ѯ���ʽ���Ϣ�ص� */
void
CDemoClient::OnQueryCashAsset(const ZpquantCashAssetItem *pCashAsset,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> ��ѯ���ʽ���Ϣ: index[%d], isEnd[%c], " \
		"�ʽ��˻�����[%s], �ͻ�����[%s], ����[%" __SPK_FMT_HH__ "u], " \
		"�ʽ�����[%" __SPK_FMT_HH__ "u], �ڳ����[%" __SPK_FMT_LL__ "d], " \
		"�ڳ�����[%" __SPK_FMT_LL__ "d], �ڳ���ȡ[%" __SPK_FMT_LL__ "d], " \
		"������[%" __SPK_FMT_LL__ "d], �ۼƴ���[%" __SPK_FMT_LL__ "d], " \
		"�ۼ���ȡ[%" __SPK_FMT_LL__ "d], ��ǰ��ȡ����[%" __SPK_FMT_LL__ "d], " \
		"�ۼ�����[%" __SPK_FMT_LL__ "d], �ۼ�����[%" __SPK_FMT_LL__ "d], " \
		"��ǰ�򶳽�[%" __SPK_FMT_LL__ "d], �ۼƷ���[%" __SPK_FMT_LL__ "d], " \
		"��ǰ���ö���[%" __SPK_FMT_LL__ "d], ��ǰά�ֱ�֤��[%" __SPK_FMT_LL__ "d], " \
		"��ǰ��֤�𶳽�[%" __SPK_FMT_LL__ "d], ��ǰ���[%" __SPK_FMT_LL__ "d], " \
		"��ǰ����[%" __SPK_FMT_LL__ "d], ��ǰ��ȡ[%" __SPK_FMT_LL__ "d]\n",
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


/* ��ѯ���ֲ���Ϣ�ص� */
void
CDemoClient::OnQueryStkHolding(const ZpquantStkHoldingItem *pStkHolding,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	fprintf(stdout, "...demoTraderClientSpi::OnQueryStkHolding\n");

	sprintf(sendJsonDataStr, ">>> ��ѯ����Ʊ�ֲ���Ϣ: index[%d], isEnd[%c], " \
		"֤ȯ�˻�[%s], �г�����[%" __SPK_FMT_HH__ "u], ��Ʒ����[%s], " \
		"�ճ��ֲ�[%" __SPK_FMT_LL__ "d], " \
		"�����ۼ�����[%" __SPK_FMT_LL__ "d], " \
		"�����ۼ�����[%" __SPK_FMT_LL__ "d], " \
		"��ǰ��������[%" __SPK_FMT_LL__ "d], " \
		"�����ۼ�ת�����[%" __SPK_FMT_LL__ "d], " \
		"�����ۼ�ת������[%" __SPK_FMT_LL__ "d], " \
		"��ǰת����������[%" __SPK_FMT_LL__ "d], " \
		"��ǰ������[%" __SPK_FMT_LL__ "d], " \
		"��ǰ��������[%" __SPK_FMT_LL__ "d], " \
		"��ǰ����������[%" __SPK_FMT_LL__ "d], " \
		"��ǰ���Ҷ���[%" __SPK_FMT_LL__ "d], " \
		"��ǰ�ѱ���ʹ��[%" __SPK_FMT_LL__ "d], " \
		"��ǰ�ɱ���/����[%" __SPK_FMT_LL__ "d], " \
		"��ǰ�ֲܳ�[%" __SPK_FMT_LL__ "d], " \
		"��ǰ����[%" __SPK_FMT_LL__ "d], " \
		"��ǰ��ת������[%" __SPK_FMT_LL__ "d], " \
		"��ǰ������[%" __SPK_FMT_LL__ "d]\n",
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

/* ��ѯ���ͻ���Ϣ�ص� */
void
CDemoClient::OnQueryCustInfo(const ZpquantCustItemT *pCust,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> ��ѯ���ͻ���Ϣ: index[%d], isEnd[%c], " \
		"�ͻ�ID[%s], �ͻ�����[%" __SPK_FMT_HH__ "u], " \
		"�ͻ�״̬[%" __SPK_FMT_HH__ "u], ��������[%" __SPK_FMT_HH__ "u], " \
		"������־[%" __SPK_FMT_HH__ "u], Ͷ���߷���[%c]\n",
		pCursor->seqNo, pCursor->isEnd,
		pCust->custId, pCust->custType, pCust->status,
		pCust->riskLevel, pCust->institutionFlag,
		pCust->investorClass == INVESTOR_CLASS_NORMAL ?
		'0' : pCust->investorClass + 'A' - 1);

	fprintf(stdout, sendJsonDataStr);
}


/* ��ѯ���ɶ��˻���Ϣ�ص� */
void
CDemoClient::OnQueryInvAcct(const ZpquantInvAcctItem *pInvAcct,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	fprintf(stdout, ">>> ��ѯ��֤ȯ�˻���Ϣ: index[%d], isEnd[%c], " \
		"֤ȯ�˻�[%s], �г�����[%" __SPK_FMT_HH__ "u], " \
		"�ͻ�����[%s], �˻�״̬[%" __SPK_FMT_HH__ "u], " \
		"�¹��Ϲ��޶�[%d]\n",
		pCursor->seqNo, pCursor->isEnd,
		pInvAcct->invAcctId, pInvAcct->mktId,
		pInvAcct->custId, pInvAcct->status,
		pInvAcct->subscriptionQuota);

	fprintf(stdout, sendJsonDataStr);
}

/* ��ѯ���������ˮ��Ϣ�ص� */
void
CDemoClient::OnQueryFundTransferSerial(
	const ZpquantFundTransferSerialItemT *pFundTrsf,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> ��ѯ���������ˮ: index[%d], isEnd[%c], " \
		"�ͻ��˻�����[%" __SPK_FMT_HH__ "d], �ͻ�ί����ˮ��[%d], " \
		"�ʽ��˻�[%s], ����[%s], ���[%" __SPK_FMT_LL__ "d], " \
		"�����״̬[%" __SPK_FMT_HH__ "u], ����ԭ��[%d], ���������[%d], " \
		"������Ϣ[%s], ��̨ί�б���[%d], ��������[%08d], " \
		"����ʱ��[%09d], �ϱ�ʱ��[%09d], ���ʱ��[%09d]\n",
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


/* ��ѯ��֤ȯ��Ϣ�ص� */
void
CDemoClient::OnQueryStock(const ZpquantStockBaseInfo *pStock,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> ��ѯ���ֻ���Ʒ��Ϣ: index[%d], isEnd[%c], " \
		"֤ȯ����[%s], ֤ȯ����[%s], �������[%s], " \
		"�г�����[%" __SPK_FMT_HH__ "u], ֤ȯ����[%" __SPK_FMT_HH__ "u], " \
		"֤ȯ������[%" __SPK_FMT_HH__ "u], ֤ȯ����[%" __SPK_FMT_HH__ "u], " \
		"���յȼ�[%" __SPK_FMT_HH__ "u], ͣ�Ʊ�־[%" __SPK_FMT_HH__ "u], " \
		"�ʵ��Թ���[%" __SPK_FMT_HH__ "u], ���ջ�ת[%" __SPK_FMT_HH__ "u], " \
		"�۸�λ[%d], �������λ[%d], ��������λ[%d], " \
		"�������̼�[%d], ծȯ��Ϣ[%" __SPK_FMT_LL__ "d], " \
		"��ͣ��[%d], ��ͣ��[%d]\n",
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


/* ��ѯ���г�״̬��Ϣ�ص� */
void
CDemoClient::OnQueryMarketState(const ZpquantMarketStateInfo *pMarketState,
	const ZpquantQryCursor *pCursor, int32 requestId) {
	sprintf(sendJsonDataStr, ">>> ��ѯ���г�״̬��Ϣ: index[%d], isEnd[%c], " \
		"����������[%hhu], ����ƽ̨����[%hhu], �г�����[%hhu], " \
		"�г�״̬[%hhu]\n", \
		pCursor->seqNo, pCursor->isEnd,
		pMarketState->exchId, pMarketState->platformId,
		pMarketState->mktId, pMarketState->mktState);

	fprintf(stdout, sendJsonDataStr);
}

//��ȡ������
void CDemoClient::onGetTradingDay(const ZpquantTradingDayInfo * pTradingDayInfo)
{
	sprintf(sendJsonDataStr, ">>> ��ȡ������: ������[%d], isTradingDay[%d]\n",
		pTradingDayInfo->tradingDay, pTradingDayInfo->isTradingDay);

	fprintf(stdout, sendJsonDataStr);
}
