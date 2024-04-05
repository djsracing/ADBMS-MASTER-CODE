
//Latest
/*! Analog Devices EVAL-ADBMS1818 Demonstration Board.
  ADBMS1818: Multicell Battery Monitors

  @verbatim
  NOTES
  Setup:
    Set the terminal baud rate to 115200 and select the newline terminator.
    Ensure all jumpers on the demo board are installed in their default positions from the factory.
    Refer to Demo Manual.-=

  USER INPUT DATA FORMAT:
  decimal : 1024
  hex     : 0x400
  octal   : 02000  (leading 0)
  binary  : B10000000000
  float   : 1024.0
  @endverbatim

  https://www.analog.com/en/products/ADBMS1818.html
  https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-ADBMS1818.html

********************************************************************************
  Copyright 2019(c) Analog Devices, Inc.

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   - Neither the name of Analog Devices, Inc. nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.
   - The use of this software may or may not infringe the patent rights
     of one or more patent holders.  This license does not release you
     from the requirement that you obtain separate licenses from these
     patent holders to use this software.
   - Use of the software either in source or binary form, must be run
     on or directly connected to an Analog Devices Inc. component.

  THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/*! @file
    @ingroup ADBMS1818
*/


#include "source.h"
/*!**********************************************************************
  //brief  Initializes hardware and variables
  //@return void
 ***********************************************************************/
void setup()
{
  Serial.begin(115200);
  quikeval_SPI_connect();
  spi_enable(SPI_CLOCK_DIV16); // This will set the Linduino to have a 1MHz Clock
  ADBMS1818_init_cfg(TOTAL_IC, BMS_IC);
  ADBMS1818_init_cfgb(TOTAL_IC, BMS_IC);
  for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
  {
    ADBMS1818_set_cfgr(current_ic, BMS_IC, REFON, ADCOPT, GPIOBITS_A, DCCBITS_A, DCTOBITS, UV, OV);
    ADBMS1818_set_cfgrb(current_ic, BMS_IC, FDRF, DTMEN, PSBITS, GPIOBITS_B, DCCBITS_B);
  }
  ADBMS1818_reset_crc_count(TOTAL_IC, BMS_IC);
  ADBMS1818_init_reg_limits(TOTAL_IC, BMS_IC);
  pinMode(Precharge, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(AMS_FAULT, OUTPUT);
  pinMode(CurrPin, INPUT);
  pinMode(A0, INPUT);
  SPI.begin(); // Intialize SPI PORT
}

/*!*********************************************************************
  \brief Main loop
   @return void
***********************************************************************/

void loop()
{
  int8_t error = 0;
  char input = 0;
  // if (Serial.available())           // Check for user input
  // {
  //   uint32_t user_command;
  //   user_command = read_int();      // Read the user command
  //   if (user_command == 'm')
  //   {
  //     print_menu();
  //   }
  //   else
  //   {
  //     Serial.println(user_command);
  //     run_command(user_command);
  //   }
  // }
  while (1)
  {
    wakeup_idle(TOTAL_IC);
    ADBMS1818_adcv(ADC_CONVERSION_MODE, ADC_DCP, CELL_CH_TO_CONVERT);
    ADBMS1818_pollAdc();
    ADBMS1818_adax(ADC_CONVERSION_MODE, AUX_CH_ALL);
    ADBMS1818_rdaux(SEL_ALL_REG, TOTAL_IC, BMS_IC);
    ADBMS1818_rdstat(SEL_ALL_REG, TOTAL_IC, BMS_IC);
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcv(SEL_ALL_REG, TOTAL_IC, BMS_IC);
    check_error(error);
    // voltage_implausibility_check();
    // temp_implausibility_check();
    // current_implausibility_check();

    volt_err = voltage_implausibility_check();
     check_AMS_Fault(volt_err, VOLT_IMP);
    curr_err = current_implausibility_check();
    // check_AMS_Fault(curr_err, CURR_IMP);
    temp_err = temp_implausibility_check();
    // check_AMS_Fault(temp_err, TEMP_IMP);
    print_temp();
    // print_sumofsegments();//total voltages of all Segments
    // print_sumofcells();
  }
}
