#include "subsytemDAC_ADC.h"
#include "Arduino.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "Applications/console.h"

//hardware definition for pereodic asking
#define LEDC_HS_TIMER           LEDC_TIMER_0
#define LEDC_HS_MODE            LEDC_HIGH_SPEED_MODE
//#define LEDC_HS_CH0_GPIO        (32)
#define LEDC_HS_CH0_CHANNEL      LEDC_CHANNEL_0
// hardware defenition for BUSY input
//const uint8_t RESET_ADC_PIN = 


//ADC configuration
const uint8_t ADC_MOSI1_GPIO = 13;
const uint8_t ADC_MISO1_GPIO = 12;
const uint8_t ADC_MISO2_GPIO =19;
const uint8_t ADC_SCLK_GPIO = 14;


const uint8_t ADC_CONVST_GPIO = 32;
const uint8_t ADC_BUSY_GPIO = 35;  
const uint8_t ADC_CS_GPIO = 15;
const uint8_t DAC_SLAVE_CS_GPIO = 5;


spi_device_handle_t g_spiADC;


///!!!! debug code clear it
int32_t adc_values_buffer[100];
int curInd = 0;
volatile bool g_BusyFlag = false;
// it's debug code above

void start_adc_asking(void *arg) {
    //make spi transfer struct
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 24;
    t.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
    my_printf("Collect adc data...\r\n"); 
    curInd = 0;
    g_BusyFlag = false;

    while(curInd < sizeof(adc_values_buffer)/sizeof(adc_values_buffer[0])) {
        //if (g_BusyFlag) {
        {
            g_BusyFlag = false;
            esp_err_t ret = spi_device_polling_transmit(g_spiADC, &t);
            if (ret == ESP_OK) {
                int32_t *pData = (int32_t*)t.rx_data;
                
                //debug
                if (curInd == 0)
                    my_printf("take one adc smpl\r\n");
                //debug


                adc_values_buffer[curInd++] = *pData;
            } else {
                my_printf("spi receive error occur \r\n");
                break;
            }
        }
    }
    // output readed values
    for (int i = 0; i < sizeof(adc_values_buffer)/sizeof(adc_values_buffer[0]); i++) {
        my_printf("0x%X\r\n", adc_values_buffer[i]);
    }
}


///!!!!


void init_analog_subsystem(void* arg) {
    CAnalogSubsystem& analog_subsystem = getAnalogSubsystemInstance();
    if (!analog_subsystem.isInit()) {
        bool res = analog_subsystem.init();
        my_printf("analog subsystem init %s\r\n", res ? "Ok" : "err");
    } else {
        my_printf("analog subsystem has already init\r\n");
    }
}




void IRAM_ATTR adc_BUSY_falling_edge() {
    g_BusyFlag = true;
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
            .gpio_num = ADC_CONVST_GPIO,
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
    esp_err_t ret;
    spi_bus_config_t spi_bus_cfg = {
        .mosi_io_num = ADC_MOSI1_GPIO,
        .miso_io_num = ADC_MISO1_GPIO,
        .sclk_io_num = ADC_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 3
        }; // 4 bytes per interrupt
    
    spi_device_interface_config_t dev_cfg = {
            .command_bits = 0,       
            .address_bits = 0,
            .dummy_bits = 0,
            .mode =  1, // set SPI_MODE 1, CLK - low, CLK rising edge - shifts data, CLK - falling edge - samples data
            .duty_cycle_pos = 0, // 0 here is 50 % it is equal 128
            .cs_ena_pretrans = 1, // amout spi bit cycles before transaction
            .cs_ena_posttrans = 0, //???  cs should state after transaction 
            .clock_speed_hz = 5 * 1000 * 1000,
            .input_delay_ns = 0, //???
            .spics_io_num = ADC_CS_GPIO,
            .flags = 0,//SPI_DEVICE_CLK_AS_CS,
            .queue_size = 1, // ????
        };
    
        ret = spi_bus_initialize(HSPI_HOST, &spi_bus_cfg, 0); // 0 DMA not used
        if (ret == ESP_OK) { 
            ret = spi_bus_add_device(HSPI_HOST, &dev_cfg, &g_spiADC);
            res = ret == ESP_OK;
        }
    return res;
}


bool CAnalogSubsystem::init() {
    bool res = false;
    if (isInit() == false) {
        
        // init DAC!!!

        // init DAC!!!


        initBUSYinput();
        res = initSpiSubsystem();
        if (res) {
            res = initPereodicADCAsking();
        }
    }
    if (res) {
        m_bInit = true;
    }

    return res;
}

bool CAnalogSubsystem::initBUSYinput() {
    pinMode(ADC_BUSY_GPIO, INPUT_PULLDOWN);
    attachInterrupt(ADC_BUSY_GPIO, adc_BUSY_falling_edge, FALLING);
    return true;
}
