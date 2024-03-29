#include    <iostream>
#include    "demo_md_client_spi.h"
#include    <stdio.h>
#include    <string.h>

void 
demoMdClientSpi::OnTickRtnDepthMarketData(ZpquantMdsL2OnTickT *pDepthMarketData)
{
    fprintf(stdout, ">>> OnTick: " \
            "证券代码[%s], " \
            "TradePx[%d], HighPx[%d], " \
            "\n",
            pDepthMarketData->SecurityID,
            pDepthMarketData->TradePx,pDepthMarketData->HighPx);
}


void 
demoMdClientSpi::OnTradeRtnDepthMarketData(ZpquantMdsL2OnTradeT *pDepthMarketData)
{
    fprintf(stdout, ">>> OnTrade: " \
            "证券代码[%s], " \
            "TradePrice[%d], TradeQty[%d], " \
            "\n",
            pDepthMarketData->SecurityID,
            pDepthMarketData->TradePrice,pDepthMarketData->TradeQty);
}

void 
demoMdClientSpi::OnOrderRtnDepthMarketData(ZpquantMdsL2OnOrderT *pDepthMarketData)
{
    fprintf(stdout, ">>> OnOrder: " \
            "证券代码[%s], " \
            "Price[%d], OrderQty[%d], " \
            "\n",
            pDepthMarketData->SecurityID,
            pDepthMarketData->Price,pDepthMarketData->OrderQty);
}

demoMdClientSpi::demoMdClientSpi(int32 something) {
    this->something = something;
    fprintf(stdout, "demoMdClientSpi::demoMdClientSpi\n");
    //memset(sendJsonDataStr, 0, sizeof(sendJsonDataStr)*4096);
}


demoMdClientSpi::~demoMdClientSpi() {
    /* do nothing */
}
