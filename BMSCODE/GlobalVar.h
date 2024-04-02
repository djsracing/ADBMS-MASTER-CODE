/************************************* Read me *******************************************
  In this sketch book:
  -All Global Variables are in Upper casing
  -All Local Variables are in lower casing
  -The Function wakeup_sleep(TOTAL_IC) : is used to wake the ADBMS181x from sleep state.
   It is defined in ADBMS181x.cpp
  -The Function wakeup_idle(TOTAL_IC) : is used to wake the ICs connected in daisy chain
   via the LTC6820 by initiating a dummy SPI communication. It is defined in ADBMS181x.cpp
  Setup Variables
  The following variables can be modified to configure the software.
********************************************************************/

/************************* Includes *****************************/
#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>
#include <SPI.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "ADBMS181x.h"
#include "ADBMS1818.h"
// #include "ADS1X15.h"

/************************* Defines *****************************/
#define ENABLED 1
#define DISABLED 0
#define DATALOG_ENABLED 1
#define DATALOG_DISABLED 0
#define PWM 1
#define SCTL 2
#define VOLT_IMP 1
#define TEMP_IMP 2
#define CURR_IMP 3
#define CurrPin A3
#define CS_PIN1 49

const int AMS_FAULT = 4;
const int Precharge = 5;


/********************************************************************/
const uint8_t TOTAL_IC = 6;  //!< Number of ICs in the daisy chain
/*********************************************************************/


/********************************************************************
  ADC Command Configurations. See ADBMS181x.h for options
*********************************************************************/
const uint8_t ADC_OPT = ADC_OPT_ENABLED;           //!< ADC Mode option bit
const uint8_t ADC_CONVERSION_MODE = MD_7KHZ_3KHZ;  //!< ADC
const uint8_t ADC_DCP = DCP_DISABLED;              //!< Discharge Permitted
const uint8_t CELL_CH_TO_CONVERT = CELL_CH_ALL;    //!< Channel Selection for ADC conversion
const uint8_t AUX_CH_TO_CONVERT = AUX_CH_ALL;      //!< Channel Selection for ADC conversion
const uint8_t STAT_CH_TO_CONVERT = STAT_CH_ALL;    //!< Channel Selection for ADC conversion
const uint8_t SEL_ALL_REG = REG_ALL;               //!< Register Selection
const uint8_t SEL_REG_A = REG_1;                   //!< Register Selection
const uint8_t SEL_REG_B = REG_2;                   //!< Register Selection

const uint16_t MEASUREMENT_LOOP_TIME = 500;  //!< Loop Time in milliseconds(ms)

// Under Voltage and Over Voltage Thresholds
const uint16_t OV_THRESHOLD = 41000;  //!< Over voltage threshold ADC Code. LSB = 0.0001 ---(4.1V)
const uint16_t UV_THRESHOLD = 31000;  //!< Under voltage threshold ADC Code. LSB = 0.0001 ---(3.1V)


// Loop Measurement Setup. These Variables are ENABLED or DISABLED. Remember ALL CAPS
const uint8_t WRITE_CONFIG = ENABLED;   //!< This is to ENABLED or DISABLED writing into to configuration registers in a continuous loop
const uint8_t READ_CONFIG = ENABLED;    //!< This is to ENABLED or DISABLED reading the configuration registers in a continuous loop
const uint8_t MEASURE_CELL = ENABLED;   //!< This is to ENABLED or DISABLED measuring the cell voltages in a continuous loop
const uint8_t MEASURE_AUX = DISABLED;   //!< This is to ENABLED or DISABLED reading the auxiliary registers in a continuous loop
const uint8_t MEASURE_STAT = DISABLED;  //!< This is to ENABLED or DISABLED reading the status registers in a continuous loop
const uint8_t PRINT_PEC = DISABLED;     //!< This is to ENABLED or DISABLED printing the PEC Error Count in a continuous loop

/*******************************************************
  Global Battery Variables received from 181x commands
  These variables store the results from the ADBMS1818
  register reads and the array lengths must be based
  on the number of ICs on the stack
 ******************************************************/
cell_asic BMS_IC[TOTAL_IC];  //!< Global Battery Variable

/*************************************************************************
  Set configuration register. Refer to the data sheet
**************************************************************************/
bool REFON = false;                                     //!< Reference Powered Up Bit
bool ADCOPT = false;                                    //!< ADC Mode option bit
bool GPIOBITS_A[5] = { true, true, true, true, true };  //!< GPIO Pin Control // Gpio 1,2,3,4,5
bool GPIOBITS_B[4] = { true, true, true, true };        //!< GPIO Pin Control // Gpio 6,7,8,9
uint16_t UV = UV_THRESHOLD;                             //!< Under voltage Comparison Voltage
uint16_t OV = OV_THRESHOLD;                             //!< Over voltage Comparison Voltage

/**********************************DISCHARGE_BITS*********************************************/
//!< Discharge cell switch //Dcc 1,2,3,4,5,6,7,8,9,10,11,12
bool DCCBITS_A[12] = { false, false, false, false, false, false, false, false, false, false, false, false };

//!< Discharge cell switch //Dcc 0,13,14,15
bool DCCBITS_B[7] = { false, false, false, false, false, false, false };

//!< Discharge time value //Dcto 0,1,2,3  //Currently Programed for 4 min
bool DCTOBITS[4] = { true, false, true, false };

/*Ensure that Dcto bits are set according to the required discharge time. Refer to the data sheet */

bool FDRF = false;                  //!< Force Digital Redundancy Failure Bit
bool DTMEN = true;                  //!< Enable Discharge Timer Monitor
bool PSBITS[2] = { false, false };  //!< Digital Redundancy Path Selection//ps-0,1
/**************************************************************************/

/**********************************VOLT*********************************************/
uint16_t UV_in_V = 1.0;                //!< Under-voltage Comparison Voltage in volts
uint16_t OV_in_V = 4.3;                //!< Over-voltage Comparison Voltage in volts
const int CV_CHANNELS = 16;            // cv to measured on 16 channels
const int IC_CHANNELS = 18;            // total 18 channels present on IC
float voltage[TOTAL_IC][CV_CHANNELS];  // 8*16 Array to hold cell voltages, skipping the channels where cells are not connected
float BMS_CV[TOTAL_IC][IC_CHANNELS];   // 8*18 Array to hold all the values getting from the 6811 without skipping above mentioned channels
float VOLTAGE_IMPLAUSIBILITY_DURATION = 500;
float voltage_timer_array[TOTAL_IC][16]{
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
  // { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  // { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
};

/**********************************TEMP*********************************************/
float TEMP_IMPLAUSIBILITY_DURATION = 1000;
uint16_t OT_temp = 1.65;  // Over-Threshold value of Temperaturein volts
uint16_t SCS_TEMP = 4.5;  // SCS checks
float TEMP_arr[TOTAL_IC][9];
float temperatures[TOTAL_IC][9];
float temp_timer_array[TOTAL_IC][9] = 
{
  { -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, -1, -1, -1, -1, -1, -1 }
  // { -1, -1, -1, -1, -1, -1, -1, -1, -1 },
  // { -1, -1, -1, -1, -1, -1, -1, -1, -1 }
};
// Constants for the Steinhart-Hart equation
#define A 0.001129148
#define B 0.000234125
#define C 0.0000000876741

// Pull-up resistor value
#define R_PULLUP 10000  // 10K ohms

// Supply voltage
#define VCC 5.0  // 5V
/**********************************CURRENT*********************************************/
float CurrentVal;
uint8_t currentInitMillis = -1;
float OC = 2.7500;
float CURRENT_IMPLAUSIBILITY_DURATION = 500;

/**********************************AMS_FLT*********************************************/
bool temp_err;
bool curr_err;
bool volt_err;
bool AMS_SD_Volt_Flag;
bool AMS_SD_Temp_Flag;
bool AMS_SD_Curr_Flag;

/**************** Local Function Declaration *******************/
void measurement_loop(uint8_t datalog_en);
void print_menu(void);
void print_wrconfig(void);
void print_wrconfigb(void);
void print_rxconfig(void);
void print_rxconfigb(void);
void print_cells(uint8_t datalog_en);
void print_aux(uint8_t datalog_en);
void print_stat(void);
void print_aux1(uint8_t datalog_en);
void print_sumofcells(void);
void check_mux_fail(void);
void print_selftest_errors(uint8_t adc_reg, int8_t error);
void print_overlap_results(int8_t error);
void print_digital_redundancy_errors(uint8_t adc_reg, int8_t error);
void print_open_wires(void);
void print_pec_error_count(void);
int8_t select_s_pin(void);
void print_wrpwm(void);
void print_rxpwm(void);
void print_wrsctrl(void);
void print_rxsctrl(void);
void print_wrpsb(uint8_t type);
void print_rxpsb(uint8_t type);
void print_wrcomm(void);
void print_rxcomm(void);
void check_mute_bit(void);
void print_conv_time(uint32_t conv_time);
void check_error(int error);
void serial_print_text(char data[]);
void serial_print_hex(uint8_t data);
char read_hex(void);
char get_char(void);
bool voltage_implausibility_check();
bool temp_implausibility_check();
bool current_implausibility_check();
void check_AMS_Fault(bool isFault, int fault_id);
void print_array_small(float arr[TOTAL_IC][CV_CHANNELS]);
void print_array_large(float arr[TOTAL_IC][IC_CHANNELS]);
