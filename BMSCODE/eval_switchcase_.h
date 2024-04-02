

void run_command(uint32_t cmd)
{
  uint8_t streg = 0;
  int8_t error = 0;
  uint32_t conv_time = 0;
  int8_t s_pin_read = 0;

  switch (cmd)
  {
  case 1: // Write and read Configuration Register
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_wrcfg(TOTAL_IC, BMS_IC);  // Write into Configuration Register
    ADBMS1818_wrcfgb(TOTAL_IC, BMS_IC); // Write into Configuration Register B
    print_wrconfig();
    print_wrconfigb();

    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcfg(TOTAL_IC, BMS_IC); // Read Configuration Register
    check_error(error);
    error = ADBMS1818_rdcfgb(TOTAL_IC, BMS_IC); // Read Configuration Register B
    check_error(error);
    print_rxconfig();
    print_rxconfigb();
    break;

  case 2: // Read Configuration Register
    wakeup_sleep(TOTAL_IC);
    error = ADBMS1818_rdcfg(TOTAL_IC, BMS_IC);
    check_error(error);
    error = ADBMS1818_rdcfgb(TOTAL_IC, BMS_IC);
    check_error(error);
    print_rxconfig();
    print_rxconfigb();
    break;

  case 3: // Start Cell ADC Measurement
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_adcv(ADC_CONVERSION_MODE, ADC_DCP, CELL_CH_TO_CONVERT);
    conv_time = ADBMS1818_pollAdc();
    print_conv_time(conv_time);
    break;

  case 4: // Read Cell Voltage Registers
    wakeup_sleep(TOTAL_IC);
    error = ADBMS1818_rdcv(SEL_ALL_REG, TOTAL_IC, BMS_IC); // Set to read back all cell voltage registers
    check_error(error);
    print_cells(DATALOG_DISABLED);
    break;

  case 5: // Start GPIO ADC Measurement
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_adax(ADC_CONVERSION_MODE, AUX_CH_TO_CONVERT);
    conv_time = ADBMS1818_pollAdc();
    print_conv_time(conv_time);
    break;

  case 6: // Read AUX Voltage Registers
    wakeup_sleep(TOTAL_IC);
    error = ADBMS1818_rdaux(SEL_ALL_REG, TOTAL_IC, BMS_IC); // Set to read back all aux registers
    check_error(error);
    print_aux(DATALOG_DISABLED);
    break;

  case 7: // Start Status ADC Measurement
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_adstat(ADC_CONVERSION_MODE, STAT_CH_TO_CONVERT);
    conv_time = ADBMS1818_pollAdc();
    print_conv_time(conv_time);
    break;

  case 8: // Read Status registers
    wakeup_sleep(TOTAL_IC);
    error = ADBMS1818_rdstat(SEL_ALL_REG, TOTAL_IC, BMS_IC); // Set to read back all stat registers
    check_error(error);
    print_stat();
    break;

  case 9: // Start Combined Cell Voltage and GPIO1, GPIO2 Conversion and Poll Status
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_adcvax(ADC_CONVERSION_MODE, ADC_DCP);
    conv_time = ADBMS1818_pollAdc();
    print_conv_time(conv_time);
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcv(SEL_ALL_REG, TOTAL_IC, BMS_IC); // Set to read back all cell voltage registers
    check_error(error);
    print_cells(DATALOG_DISABLED);
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdaux(SEL_REG_A, TOTAL_IC, BMS_IC); // Set to read back aux register A
    check_error(error);
    print_aux1(DATALOG_DISABLED);
    break;

  case 10: // Start Combined Cell Voltage and Sum of cells
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_adcvsc(ADC_CONVERSION_MODE, ADC_DCP);
    conv_time = ADBMS1818_pollAdc();
    print_conv_time(conv_time);
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcv(SEL_ALL_REG, TOTAL_IC, BMS_IC); // Set to read back all cell voltage registers
    check_error(error);
    print_cells(DATALOG_DISABLED);
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdstat(SEL_REG_A, TOTAL_IC, BMS_IC); // Set to read back stat register A
    check_error(error);
    print_sumofcells();
    break;

  case 11: // Loop Measurements of configuration register or cell voltages or auxiliary register or status register without data-log output
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_wrcfg(TOTAL_IC, BMS_IC);
    ADBMS1818_wrcfgb(TOTAL_IC, BMS_IC);
    measurement_loop(DATALOG_DISABLED);
    print_menu();
    break;

  case 12: // Data-log print option Loop Measurements of configuration register or cell voltages or auxiliary register or status register
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_wrcfg(TOTAL_IC, BMS_IC);
    ADBMS1818_wrcfgb(TOTAL_IC, BMS_IC);
    measurement_loop(DATALOG_ENABLED);
    print_menu();
    break;

  case 13: // Clear all ADC measurement registers
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_clrcell();
    ADBMS1818_clraux();
    ADBMS1818_clrstat();
    wakeup_idle(TOTAL_IC);
    ADBMS1818_rdcv(SEL_ALL_REG, TOTAL_IC, BMS_IC); // read back all cell voltage registers
    print_cells(DATALOG_DISABLED);
    ADBMS1818_rdaux(SEL_ALL_REG, TOTAL_IC, BMS_IC); // read back all auxiliary registers
    print_aux(DATALOG_DISABLED);
    ADBMS1818_rdstat(SEL_ALL_REG, TOTAL_IC, BMS_IC); // read back all status registers
    print_stat();
    break;

  case 14: // Run the Mux Decoder Self Test
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_diagn();
    ADBMS1818_pollAdc();
    error = ADBMS1818_rdstat(SEL_REG_B, TOTAL_IC, BMS_IC); // Set to read back register B
    check_error(error);
    check_mux_fail();
    break;

  case 15: // Run the ADC/Memory Self Test
    error = 0;
    wakeup_sleep(TOTAL_IC);
    error = ADBMS1818_run_cell_adc_st(CELL, TOTAL_IC, BMS_IC, ADC_CONVERSION_MODE, ADCOPT);
    print_selftest_errors(CELL, error);

    error = 0;
    wakeup_sleep(TOTAL_IC);
    error = ADBMS1818_run_cell_adc_st(AUX, TOTAL_IC, BMS_IC, ADC_CONVERSION_MODE, ADCOPT);
    print_selftest_errors(AUX, error);

    error = 0;
    wakeup_sleep(TOTAL_IC);
    error = ADBMS1818_run_cell_adc_st(STAT, TOTAL_IC, BMS_IC, ADC_CONVERSION_MODE, ADCOPT);
    print_selftest_errors(STAT, error);
    break;

  case 16: // Run ADC Overlap self test
    wakeup_sleep(TOTAL_IC);
    error = (int8_t)ADBMS1818_run_adc_overlap(TOTAL_IC, BMS_IC);
    print_overlap_results(error);
    break;

  case 17: // Run ADC Redundancy self test
    wakeup_sleep(TOTAL_IC);
    error = ADBMS1818_run_adc_redundancy_st(ADC_CONVERSION_MODE, AUX, TOTAL_IC, BMS_IC);
    print_digital_redundancy_errors(AUX, error);

    wakeup_sleep(TOTAL_IC);
    error = ADBMS1818_run_adc_redundancy_st(ADC_CONVERSION_MODE, STAT, TOTAL_IC, BMS_IC);
    print_digital_redundancy_errors(STAT, error);
    break;

  case 18: // Run open wire self test for single cell detection
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_run_openwire_single(TOTAL_IC, BMS_IC);
    print_open_wires();
    break;

  case 19: // Run open wire self test for multiple cell and two consecutive cells detection
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_run_openwire_multi(TOTAL_IC, BMS_IC);
    break;

  case 20: // Open wire Diagnostic for Auxiliary Measurements
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_run_gpio_openwire(TOTAL_IC, BMS_IC);
    print_open_wires();
    break;

  case 21: // Print pec counter
    print_pec_error_count();
    break;

  case 22: // Reset pec counter
    ADBMS1818_reset_crc_count(TOTAL_IC, BMS_IC);
    print_pec_error_count();
    break;

  case 23: // Enable a discharge transistor
    s_pin_read = select_s_pin();
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_set_discharge(s_pin_read, TOTAL_IC, BMS_IC);
    ADBMS1818_wrcfg(TOTAL_IC, BMS_IC);
    ADBMS1818_wrcfgb(TOTAL_IC, BMS_IC);
    print_wrconfig();
    print_wrconfigb();
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcfg(TOTAL_IC, BMS_IC);
    check_error(error);
    error = ADBMS1818_rdcfgb(TOTAL_IC, BMS_IC);
    check_error(error);
    print_rxconfig();
    print_rxconfigb();
    break;

  case 24: // Clear all discharge transistors
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_clear_discharge(TOTAL_IC, BMS_IC);
    ADBMS1818_wrcfg(TOTAL_IC, BMS_IC);
    ADBMS1818_wrcfgb(TOTAL_IC, BMS_IC);
    print_wrconfig();
    print_wrconfigb();
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcfg(TOTAL_IC, BMS_IC);
    check_error(error);
    error = ADBMS1818_rdcfgb(TOTAL_IC, BMS_IC);
    check_error(error);
    print_rxconfig();
    print_rxconfigb();
    break;

  case 25: // Write and read pwm configuration
    /*****************************************************
      PWM configuration data.
      1)Set the corresponding DCC bit to one for pwm operation.
      2)Set the DCTO bits to the required discharge time.
      3)Choose the value to be configured depending on the
       required duty cycle.
      Refer to the data sheet.
    *******************************************************/
    wakeup_sleep(TOTAL_IC);

    for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      BMS_IC[current_ic].pwm.tx_data[0] = 0x88; // Duty cycle for S pin 2 and 1
      BMS_IC[current_ic].pwm.tx_data[1] = 0x88; // Duty cycle for S pin 4 and 3
      BMS_IC[current_ic].pwm.tx_data[2] = 0x88; // Duty cycle for S pin 6 and 5
      BMS_IC[current_ic].pwm.tx_data[3] = 0x88; // Duty cycle for S pin 8 and 7
      BMS_IC[current_ic].pwm.tx_data[4] = 0x88; // Duty cycle for S pin 10 and 9
      BMS_IC[current_ic].pwm.tx_data[5] = 0x88; // Duty cycle for S pin 12 and 11
    }

    ADBMS1818_wrpwm(TOTAL_IC, 0, BMS_IC); // Write pwm configuration

    for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      BMS_IC[current_ic].pwmb.tx_data[0] = 0x88; // Duty cycle for S pin 14 and 13
      BMS_IC[current_ic].pwmb.tx_data[1] = 0x88; // Duty cycle for S pin 16 and 15
      BMS_IC[current_ic].pwmb.tx_data[2] = 0x88; // Duty cycle for S pin 18 and 17
    }

    ADBMS1818_wrpsb(TOTAL_IC, BMS_IC); //  Write PWM/S control register  group B
    print_wrpwm();
    print_wrpsb(PWM);

    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdpwm(TOTAL_IC, 0, BMS_IC); // Read pwm configuration
    check_error(error);

    error = ADBMS1818_rdpsb(TOTAL_IC, BMS_IC); // Read PWM/S Control Register Group
    check_error(error);
    print_rxpwm();
    print_rxpsb(PWM);
    break;

  case 26: // Write and read S Control Register Group
    /**************************************************************************************
       S pin control.
       1)Ensure that the pwm is set according to the requirement using the previous case.
       2)Choose the value depending on the required number of pulses on S pin.
       Refer to the data sheet.
    ***************************************************************************************/
    wakeup_sleep(TOTAL_IC);

    for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      BMS_IC[current_ic].sctrl.tx_data[0] = 0xFF; // No. of high pulses on S pin 2 and 1
      BMS_IC[current_ic].sctrl.tx_data[1] = 0xFF; // No. of high pulses on S pin 4 and 3
      BMS_IC[current_ic].sctrl.tx_data[2] = 0xFF; // No. of high pulses on S pin 6 and 5
      BMS_IC[current_ic].sctrl.tx_data[3] = 0xFF; // No. of high pulses on S pin 8 and 7
      BMS_IC[current_ic].sctrl.tx_data[4] = 0xFF; // No. of high pulses on S pin 10 and 9
      BMS_IC[current_ic].sctrl.tx_data[5] = 0xFF; // No. of high pulses on S pin 12 and 11
    }

    ADBMS1818_wrsctrl(TOTAL_IC, streg, BMS_IC); // Write S Control Register Group

    for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      BMS_IC[current_ic].sctrlb.tx_data[3] = 0xFF; // No. of high pulses on S pin 14 and 13
      BMS_IC[current_ic].sctrlb.tx_data[4] = 0xFF; // No. of high pulses on S pin 16 and 15
      BMS_IC[current_ic].sctrlb.tx_data[5] = 0xFF; // No. of high pulses on S pin 18 and 17
    }

    ADBMS1818_wrpsb(TOTAL_IC, BMS_IC); //  Write PWM/S control register group B
    print_wrsctrl();
    print_wrpsb(SCTL);

    wakeup_idle(TOTAL_IC);
    ADBMS1818_stsctrl(); // Start S Control pulsing

    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdsctrl(TOTAL_IC, streg, BMS_IC); // Read S Control Register Group
    check_error(error);

    error = ADBMS1818_rdpsb(TOTAL_IC, BMS_IC); // Read PWM/S Control Register Group
    check_error(error);
    print_rxsctrl();
    print_rxpsb(SCTL);
    break;

  case 27: // Clear S Control Register Group
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_clrsctrl();
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdsctrl(TOTAL_IC, streg, BMS_IC);
    check_error(error);
    ADBMS1818_rdpsb(TOTAL_IC, BMS_IC);
    print_rxsctrl();
    print_rxpsb(SCTL);
    break;

  case 28: // SPI Communication
    /************************************************************
       Ensure to set the GPIO bits to 1 in the CFG register group.
    *************************************************************/
    for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      // Communication control bits and communication data bytes. Refer to the data sheet.
      BMS_IC[current_ic].com.tx_data[0] = 0x81; // Icom CSBM Low(8) + data D0 (0x11)
      BMS_IC[current_ic].com.tx_data[1] = 0x10; // Fcom CSBM Low(0)
      BMS_IC[current_ic].com.tx_data[2] = 0xA2; // Icom CSBM Falling Edge (A) + data D1 (0x25)
      BMS_IC[current_ic].com.tx_data[3] = 0x50; // Fcom CSBM Low(0)
      BMS_IC[current_ic].com.tx_data[4] = 0xA1; // Icom CSBM Falling Edge (A) + data D2 (0x17)
      BMS_IC[current_ic].com.tx_data[5] = 0x79; // Fcom CSBM High(9)
    }
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_wrcomm(TOTAL_IC, BMS_IC);
    print_wrcomm();

    wakeup_idle(TOTAL_IC);
    ADBMS1818_stcomm(3);

    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcomm(TOTAL_IC, BMS_IC);
    check_error(error);
    print_rxcomm();
    break;

  case 29: // Write byte I2C Communication on the GPIO Ports(using I2C eeprom 24LC025)
    /************************************************************
      Ensure to set the GPIO bits to 1 in the CFG register group.
    *************************************************************/
    for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      // Communication control bits and communication data bytes. Refer to the data sheet.
      BMS_IC[current_ic].com.tx_data[0] = 0x6A; // Icom Start(6) + I2C_address D0 (0xA0)
      BMS_IC[current_ic].com.tx_data[1] = 0x08; // Fcom master NACK(8)
      BMS_IC[current_ic].com.tx_data[2] = 0x00; // Icom Blank (0) + eeprom address D1 (0x00)
      BMS_IC[current_ic].com.tx_data[3] = 0x08; // Fcom master NACK(8)
      BMS_IC[current_ic].com.tx_data[4] = 0x01; // Icom Blank (0) + data D2 (0x13)
      BMS_IC[current_ic].com.tx_data[5] = 0x39; // Fcom master NACK + Stop(9)
    }
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_wrcomm(TOTAL_IC, BMS_IC); // write to comm register
    print_wrcomm();                     // print transmitted data from the comm register

    wakeup_idle(TOTAL_IC);
    ADBMS1818_stcomm(3); // data length=3 // initiates communication between master and the I2C slave

    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcomm(TOTAL_IC, BMS_IC); // read from comm register
    check_error(error);
    print_rxcomm(); // print received data into the comm register
    break;

  case 30: // Read byte data I2C Communication on the GPIO Ports(using I2C eeprom 24LC025)
    /************************************************************
      Ensure to set the GPIO bits to 1 in the CFG register group.
    *************************************************************/
    for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      // Communication control bits and communication data bytes. Refer to the data sheet.
      BMS_IC[current_ic].com.tx_data[0] = 0x6A; // Icom Start (6) + I2C_address D0 (A0) (Write operation to set the word address)
      BMS_IC[current_ic].com.tx_data[1] = 0x08; // Fcom master NACK(8)
      BMS_IC[current_ic].com.tx_data[2] = 0x00; // Icom Blank (0) + eeprom address(word address) D1 (0x00)
      BMS_IC[current_ic].com.tx_data[3] = 0x08; // Fcom master NACK(8)
      BMS_IC[current_ic].com.tx_data[4] = 0x6A; // Icom Start (6) + I2C_address D2 (0xA1)(Read operation)
      BMS_IC[current_ic].com.tx_data[5] = 0x18; // Fcom master NACK(8)
    }
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_wrcomm(TOTAL_IC, BMS_IC); // write to comm register

    wakeup_idle(TOTAL_IC);
    ADBMS1818_stcomm(3); // data length=3 // initiates communication between master and the I2C slave

    for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      // Communication control bits and communication data bytes. Refer to the data sheet.
      BMS_IC[current_ic].com.tx_data[0] = 0x0F; // Icom Blank (0) + data D0 (FF)
      BMS_IC[current_ic].com.tx_data[1] = 0xF9; // Fcom master NACK + Stop(9)
      BMS_IC[current_ic].com.tx_data[2] = 0x7F; // Icom No Transmit (7) + data D1 (FF)
      BMS_IC[current_ic].com.tx_data[3] = 0xF9; // Fcom master NACK + Stop(9)
      BMS_IC[current_ic].com.tx_data[4] = 0x7F; // Icom No Transmit (7) + data D2 (FF)
      BMS_IC[current_ic].com.tx_data[5] = 0xF9; // Fcom master NACK + Stop(9)
    }
    wakeup_idle(TOTAL_IC);
    ADBMS1818_wrcomm(TOTAL_IC, BMS_IC); // write to comm register

    wakeup_idle(TOTAL_IC);
    ADBMS1818_stcomm(1); // data length=1 // initiates communication between master and the I2C slave

    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcomm(TOTAL_IC, BMS_IC); // read from comm register
    check_error(error);
    print_rxcomm(); // print received data from the comm register
    break;

  case 31: //  Enable MUTE
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_mute();
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcfgb(TOTAL_IC, BMS_IC);
    check_error(error);
    check_mute_bit();
    break;

  case 32: // To enable UNMUTE
    wakeup_sleep(TOTAL_IC);
    ADBMS1818_unmute();
    wakeup_idle(TOTAL_IC);
    error = ADBMS1818_rdcfgb(TOTAL_IC, BMS_IC);
    check_error(error);
    check_mute_bit();
    break;

  case 33: // Set and reset the gpio pins(to drive output on gpio pins)
    /***********************************************************************
      Please ensure you have set the GPIO bits according to your requirement
      in the configuration register.( check the global variable GPIOBITS_A )
    ************************************************************************/
    wakeup_sleep(TOTAL_IC);
    for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++)
    {
      ADBMS1818_set_cfgr(current_ic, BMS_IC, REFON, ADCOPT, GPIOBITS_A, DCCBITS_A, DCTOBITS, UV, OV);
      ADBMS1818_set_cfgrb(current_ic, BMS_IC, FDRF, DTMEN, PSBITS, GPIOBITS_B, DCCBITS_B);
    }

    ADBMS1818_wrcfg(TOTAL_IC, BMS_IC);
    ADBMS1818_wrcfgb(TOTAL_IC, BMS_IC);
    print_wrconfig();
    print_wrconfigb();
    break;

  case 'm': // prints menu
    print_menu();
    break;

  default:
    char str_error[] = "Incorrect Option \n";
    serial_print_text(str_error);
    break;
  }
}
