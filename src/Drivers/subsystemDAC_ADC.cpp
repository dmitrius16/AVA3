#include "subsytemDAC_ADC.h"
#include "Arduino.h"
#include "driver/ledc.h"
#include "esp_err.h"

//hardware definition for pereodic asking
#define LEDC_HS_TIMER           LEDC_TIMER_0
#define LEDC_HS_MODE            LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO        (32)
#define LEDC_HS_CH0_CHANNEL      LEDC_CHANNEL_0
// hardware defenition for BUSY input
constexpr uint8_t BUSY_PIN = 7;  //pay attention here arduino lib so pin number

void IRAM_ATTR adc_BUSY_falling_edge() {

}

//hardware definition for spi 
CAnalogSubsystem& getAnalogSubsystemInstance() {
    static CAnalogSubsystem *pInstance = nullptr;
    if (pInstance == nullptr) {
        pInstance = new CAnalogSubsystem();
    }
    return *pInstance;
}

CAnalogSubsystem::CAnalogSubsystem() : m_bInit(false) {

}



bool CAnalogSubsystem::initPereodicADCAsking() {
    bool res = false;
    // init timer
    ledc_timer_config_t ledc_timer_cfg = {
        .speed_mode = LEDC_HS_MODE,
        {.duty_resolution = LEDC_TIMER_13_BIT}, //resolution of PWM duty - ???? read about this parameter
        .timer_num = LEDC_HS_TIMER, //.clk_cfg = LEDC_AUTO_CLK,  // not worked at arduino framework
        .freq_hz = 1000
    };
    esp_err_t res_api =  ledc_timer_config(&ledc_timer_cfg);
    res = res_api == ESP_OK;
    if (res) {
        ledc_channel_config_t ledc_ch_cfg = {
            .gpio_num = LEDC_HS_CH0_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .channel = LEDC_HS_CH0_CHANNEL,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_HS_TIMER,
            .duty = 5,  //5 percent
            .hpoint = 0,
        };

        res_api = ledc_channel_config(&ledc_ch_cfg);
        res = res_api == ESP_OK;
    }


    return res;
}

bool CAnalogSubsystem::initSpiSubsystem() {
    bool res = false;
    
    return res;
}


bool CAnalogSubsystem::init() {
    bool res = false;
    if (isInit() == false) {
        initBUSYinput();
        //initSpiSubsystem();
        res = initPereodicADCAsking();
    }
    return res;
}

bool CAnalogSubsystem::initBUSYinput() {
    pinMode(BUSY_PIN, INPUT_PULLDOWN);
    attachInterrupt(BUSY_PIN, adc_BUSY_falling_edge, FALLING);
    return true;
}
