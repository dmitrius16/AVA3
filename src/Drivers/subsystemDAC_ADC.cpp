#include "subsytemDAC_ADC.h"
#include "Arduino.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "DriverAD5761.h"
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
const gpio_num_t DAC_MOSI1_GPIO = GPIO_NUM_13;
const gpio_num_t ADC_MISO1_GPIO = GPIO_NUM_12;
const gpio_num_t ADC_MOSI2_GPIO = GPIO_NUM_19;
const gpio_num_t ADC_SCLK_GPIO =  GPIO_NUM_14;
const gpio_num_t ADC_SLAVE_SCLK_GPIO = GPIO_NUM_18;

const gpio_num_t ADC_CONVST_GPIO = GPIO_NUM_32;
const gpio_num_t ADC_BUSY_GPIO = GPIO_NUM_35;  
const gpio_num_t ADC_CS_GPIO = GPIO_NUM_15;
const gpio_num_t ESP_SLAVE_CS_GPIO = GPIO_NUM_5;


spi_device_handle_t g_spiMaster;
///spi_device_handle_t g_spiSlave;


///!!!! debug code clear it
int16_t adc_buffer_ch1[100];
int16_t adc_buffer_ch2[100];
int curInd = 0;
volatile bool g_BusyFlag = false;
// it's debug code above

spi_slave_transaction_t g_tr_slave_spi;
volatile uint32_t g_isr_spi_counter = 0;
WORD_ALIGNED_ATTR uint8_t rx_buffer[4];
WORD_ALIGNED_ATTR uint8_t tx_buffer[4];

void IRAM_ATTR spi_slave(spi_slave_transaction_t *trans) {
    g_isr_spi_counter++;
}


void start_adc_asking(void *arg) {
    //make spi transfer struct
    spi_transaction_t tr_master_spi;
    //spi_slave_transaction_t tr_slave_spi;
    memset(&tr_master_spi, 0, sizeof(tr_master_spi));
    memset(&g_tr_slave_spi, 0, sizeof(g_tr_slave_spi));

    tr_master_spi.length = 24;    //check it !!! here we can set length more than 24 
    tr_master_spi.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;


    g_tr_slave_spi.length = 32;// total length in bits  here there is posibility of DMA usage
    //g_tr_slave_spi.trans_len = 24;//transaction length in bits
    g_tr_slave_spi.rx_buffer = rx_buffer;
    g_tr_slave_spi.tx_buffer = tx_buffer;


    my_printf("Collect adc data...\r\n"); 
    curInd = 0;
    g_BusyFlag = false;
    g_isr_spi_counter = 0;
    while(curInd < sizeof(adc_buffer_ch1)/sizeof(adc_buffer_ch1[0])) {
        if (g_BusyFlag) {
            g_BusyFlag = false;

            g_tr_slave_spi.trans_len = 0;
            esp_err_t ret = spi_slave_queue_trans(VSPI_HOST, &g_tr_slave_spi, 0);
            if (ret != ESP_OK)
                return;
            ret = spi_device_polling_transmit(g_spiMaster, &tr_master_spi);
            if (ret != ESP_OK)
                return;
            int32_t *pData = (int32_t*)tr_master_spi.rx_data;
            spi_slave_transaction_t *pSlaveTrans = &g_tr_slave_spi;
                
            spi_slave_get_trans_result(VSPI_HOST, &pSlaveTrans, 1);
            if (ret != ESP_OK) {
                return ;
            }
            //debug
            if (curInd == 0) {
                my_printf("take one adc smpl\r\n");
                my_printf("spi slave trans get result:\r\n");
                my_printf("trans.length %d\r\ntrans.translength %d\r\ntrans.rxAddr 0x%x\r\n",pSlaveTrans->length, pSlaveTrans->trans_len,pSlaveTrans->rx_buffer);
                uint8_t *ptrData = (uint8_t*)pSlaveTrans->rx_buffer;
                for(int i = 0; i < 4; i++) {
                    my_printf("0x%04X ",ptrData[i]);
                }
                my_printf("\r\n");
            }

            //ret = spi_slave_get_trans_result(VSPI_HOST, &pSlaveTrans, 0);
            //if (ret != ESP_OK) {
            //    return;
            //}
            //debug

            
            adc_buffer_ch1[curInd] = SPI_SWAP_DATA_RX(*pData, 16);
            pData = (int32_t*)g_tr_slave_spi.rx_buffer;
            adc_buffer_ch2[curInd] = SPI_SWAP_DATA_RX(*pData, 16);
            curInd++;
        }
    }
    my_printf("SPI interrupt %d\r\n", g_isr_spi_counter);
    // output readed values

    my_printf("%6s %6s\r\n", "ch-1:", "ch-2");
    for (int i = 0; i < sizeof(adc_buffer_ch1)/sizeof(adc_buffer_ch1[0]); i++) {
        my_printf("%05d %05d\r\n", adc_buffer_ch1[i], adc_buffer_ch2[i]);
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
        {.duty_resolution = LEDC_TIMER_13_BIT}, //{} resolution of PWM duty - ???? read about this parameter
        .timer_num = LEDC_HS_TIMER, //.clk_cfg = LEDC_AUTO_CLK,  // not worked at arduino framework
        .freq_hz = 1000,
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

bool CAnalogSubsystem::initSpiMasterSubsystem() {
    bool res = false;
    esp_err_t ret;
    spi_bus_config_t spi_bus_cfg = {
        .mosi_io_num = DAC_MOSI1_GPIO,
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
            .mode =  2, // set SPI_MODE 1, CLK - low, CLK rising edge - shifts data, CLK - falling edge - samples data
            .duty_cycle_pos = 0, // 0 here is 50 % it is equal 128
            .cs_ena_pretrans = 0,//### was 1 but wa not correct reading //amout spi bit cycles before transaction
            .cs_ena_posttrans = 0, //???  cs should state after transaction 
            .clock_speed_hz = 5 * 1000 * 1000,
            .input_delay_ns = 0, //???
            .spics_io_num = ADC_CS_GPIO,
            .flags = 0,//SPI_DEVICE_CLK_AS_CS,
            .queue_size = 1, // ????
        };
    
        ret = spi_bus_initialize(HSPI_HOST, &spi_bus_cfg, 0); // 0 DMA not used
        if (ret == ESP_OK) { 
            ret = spi_bus_add_device(HSPI_HOST, &dev_cfg, &g_spiMaster);
            res = ret == ESP_OK;
        }
    return res;
}

bool CAnalogSubsystem::initSpiSlaveSubsystem() {
    spi_bus_config_t spi_bus_cfg = {
        .mosi_io_num = ADC_MOSI2_GPIO, 
        .miso_io_num = -1,
        .sclk_io_num = ADC_SLAVE_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 3
    };

    
    spi_slave_interface_config_t dev_cfg = {
        .spics_io_num = ESP_SLAVE_CS_GPIO,
        .flags = 0,
        .queue_size = 1,
        .mode = 2,
        .post_setup_cb = spi_slave
    };
    bool res = true;
    esp_err_t ret = spi_slave_initialize(VSPI_HOST, &spi_bus_cfg, &dev_cfg, 0);

    gpio_set_pull_mode(ADC_MOSI2_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(ADC_SLAVE_SCLK_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(ESP_SLAVE_CS_GPIO, GPIO_PULLUP_ONLY);

    res = ret == ESP_OK;
    return res;
}

bool CAnalogSubsystem::initDAC_AD5761() {
    uint32_t dac_cmd = GetFullResetCmdCode();
    bool res = sendCommandToDAC(dac_cmd);    
    if (res) {
        dac_cmd = UseDefaultDACCfg();
        res = sendCommandToDAC(dac_cmd);
        if (res) {
            dac_cmd = SetDACValue(0x1000);

            res = sendCommandToDAC(dac_cmd);
        }
    }
    return res;
}

bool CAnalogSubsystem::sendCommandToDAC(uint32_t cmd) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 24;
    t.flags = 0;//SPI_TRANS_USE_TXDATA;
    
    cmd = SPI_SWAP_DATA_RX(cmd, t.length);
    t.tx_buffer = (void*)&cmd;
    esp_err_t ret = spi_device_polling_transmit(g_spiMaster, &t);

    return ret == ESP_OK;
}

bool CAnalogSubsystem::init() {
    bool res = false;
    if (isInit() == false) {
        res = initSpiMasterSubsystem() && initSpiSlaveSubsystem();
        if (!res) {
            my_printf("Can't init spi subsystem: initSpiSubsystem() returns false\r\n");
            return res;
        }
        res = initDAC_AD5761();
        if (!res) {
            my_printf("Can't init DAC - AD5761: initDAC_AD5761() returns false\r\n");
            return res;
        }
        res = initPereodicADCAsking();
        if (!res) {
            my_printf("Can't init pereodic ADC asking: initPereodicADCAscking() returns false\r\n");
            return res;
        }
        res = initBUSYinput();
        if (!res) {
            my_printf("Can't init BUSY gpio\r\n");
            return res;
        }
    }

    m_bInit = true;
    return res;
}

bool CAnalogSubsystem::initBUSYinput() {
    pinMode(ADC_BUSY_GPIO, INPUT_PULLDOWN);
    attachInterrupt(ADC_BUSY_GPIO, adc_BUSY_falling_edge, FALLING);
    return true;
}
