#ifndef _DEMO_MD_CLIENT_SPI_H
#define _DEMO_MD_CLIENT_SPI_H


#include "ZpquantMdApi.h"

class   demoMdClientSpi: public Zpquant::CZpquantMdSpi {

public:
    ///深度tick行情通知
    virtual void OnTickRtnDepthMarketData(ZpquantMdsL2OnTickT *pDepthMarketData);

    ///深度trade行情通知
    virtual void OnTradeRtnDepthMarketData(ZpquantMdsL2OnTradeT *pDepthMarketData);

    ///深度order行情通知
    virtual void OnOrderRtnDepthMarketData(ZpquantMdsL2OnOrderT *pDepthMarketData);

public:
    demoMdClientSpi(int32 something = 0);
    virtual ~demoMdClientSpi();
    char sendJsonDataStr[4096];

private:
    int32               something;

};


#endif /* _DEMO_MD_CLIENT_SPI_H */
