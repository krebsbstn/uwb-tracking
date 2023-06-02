#pragma once
#include "tdoa-device.h"

/* Timestamp of frame-reception. */
static uint64_t toa = 0;
static uint64_t last_toa  = 0;
/* Measured PdoA */
static int16_t pdoa = 0;
static int16_t last_pdoa = 0;

class TdoaAnchor : public TdoaDevice 
{
public:
    TdoaAnchor(uwb_addr src);
    ~TdoaAnchor(){};

    virtual void setup() override;
    virtual void loop() override;
private:
    char pdoa_str[16] = {0};
    char toa_str[50] = {0};
    /* Declaration of static ISR's. */
    static void rx_ok_cb(const dwt_cb_data_t *cb_data);
    static void rx_err_cb(const dwt_cb_data_t *cb_data);
};
