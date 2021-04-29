#include "DriverAD5761.h"
#include <string.h>
//AD5761 register adresses

//Commands
const uint8_t NO_OP_1 = 0x0;
const uint8_t WRITE_INPUT_REG = 0x1;
const uint8_t UPDATE_DAC_REG_FR_INPUT_REG = 0x2;
const uint8_t WRITE_AND_UPDATE_DAC_REG = 0x3;
const uint8_t WRITE_CONTROL_REG = 0x4;
const uint8_t NO_OP_2 = 0x5;
const uint8_t NO_OP_3 = 0x6;
const uint8_t SOFTWARE_DATA_RESET = 0x7;
//const uint8_t RESERVED = 0x8;
const uint8_t DISABLE_DAISY_CHAIN_FUNC = 0x9;
const uint8_t READ_INPUT_REG = 0xA;
const uint8_t READ_DAC_REG = 0xB;
const uint8_t READ_CONTROL_REG = 0xC;
const uint8_t SOFTWARE_FULL_RESET = 0xF;

typedef struct {
    uint16_t RA : 3;
    uint16_t PV : 2;
    uint16_t reserved : 1;
    uint16_t ETS : 1;
    uint16_t B2C : 1;
    uint16_t OVR : 1;
    uint16_t CV  : 2;
    uint16_t res_1: 5;
}Control_reg;

// CV constants: Clear voltage selection
const uint8_t CV_Zero_Scale = 0;
const uint8_t CV_Mid_Scale = 1;
const uint8_t CV_Full_Scale = 3;

//OVR constants: 5% overange
const uint8_t Overrange_Disabled = 0;
const uint8_t Overrange_Enabled = 1;

//B2C constants: Bipolar range
const uint8_t Range_Stright_Coded = 0;
const uint8_t Range_Twos_Complement_Coded = 1;

//ETS constants: Thermal shutdown alert. 
const uint8_t No_PwrDown_If_Temper_Exceeds_150 = 0;
const uint8_t Pwr_down_If_Temper_Exceeds_150 = 1;

//PV constants: Power-up voltage
const uint8_t PV_Zero_Scale = 0;
const uint8_t PV_Mid_Scale = 1;
const uint8_t PV_Full_Scale = 3;

//RA constants: Output range. Before an output range configuration, the device must be reset.
const uint8_t RA_m10V_p10V = 0;
const uint8_t RA_zero_p10V = 1;
const uint8_t RA_m5V_p5V = 2;
const uint8_t RA_zero_p5V = 3;
const uint8_t RA_m2_5V_p7_5V = 4;
const uint8_t RA_m3V_p3V = 5;
const uint8_t RA_zero_16V = 6;
const uint8_t RA_zero_20V = 7;

uint32_t MakeCommand(uint8_t command_code, uint16_t data) {
    uint32_t code_sequence = 0;
    if (command_code > SOFTWARE_FULL_RESET) {
        return code_sequence;
    }
    code_sequence |= ((uint32_t)command_code << 16);
    code_sequence |= data;
    return code_sequence;
}

uint32_t GetFullResetCmdCode() {
    return MakeCommand(SOFTWARE_FULL_RESET, 0);
}

uint32_t UseDefaultDACCfg() {
    Control_reg ctrl_reg;
    memset(&ctrl_reg, 0, sizeof(ctrl_reg)); 
    ctrl_reg.RA = RA_m5V_p5V;
    ctrl_reg.PV = PV_Mid_Scale;
    ctrl_reg.ETS = Pwr_down_If_Temper_Exceeds_150;
    ctrl_reg.B2C = Range_Twos_Complement_Coded;
    ctrl_reg.OVR = Overrange_Disabled;
    ctrl_reg.CV = CV_Mid_Scale;
    uint16_t *pCtrlReg = (uint16_t*)&ctrl_reg;
    return MakeCommand(WRITE_CONTROL_REG, *pCtrlReg);
}

uint32_t SetDACValue(uint16_t val) {
    uint32_t res = MakeCommand(WRITE_AND_UPDATE_DAC_REG, val);
    return res;
}
