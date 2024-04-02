void measurement_loop(uint8_t datalog_en) {
  int8_t error = 0;
  char input = 0;

  Serial.println(F("Transmit 'm' to quit"));

  while (input != 'm') {
    if (Serial.available() > 0) {
      input = read_char();
    }

    if (WRITE_CONFIG == ENABLED) {
      wakeup_idle(TOTAL_IC);
      ADBMS1818_wrcfg(TOTAL_IC, BMS_IC);
      ADBMS1818_wrcfgb(TOTAL_IC, BMS_IC);
      print_wrconfig();
      print_wrconfigb();
    }

    if (READ_CONFIG == ENABLED) {
      wakeup_idle(TOTAL_IC);
      error = ADBMS1818_rdcfg(TOTAL_IC, BMS_IC);
      check_error(error);
      error = ADBMS1818_rdcfgb(TOTAL_IC, BMS_IC);
      check_error(error);
      print_rxconfig();
      print_rxconfigb();
    }

    if (MEASURE_CELL == ENABLED) {
      wakeup_idle(TOTAL_IC);
      ADBMS1818_adcv(ADC_CONVERSION_MODE, ADC_DCP, CELL_CH_TO_CONVERT);
      ADBMS1818_pollAdc();
      wakeup_idle(TOTAL_IC);
      error = ADBMS1818_rdcv(0, TOTAL_IC, BMS_IC);
      check_error(error);
      print_cells(datalog_en);
    }

    if (MEASURE_AUX == ENABLED) {
      wakeup_idle(TOTAL_IC);
      ADBMS1818_adax(ADC_CONVERSION_MODE, AUX_CH_ALL);
      ADBMS1818_pollAdc();
      wakeup_idle(TOTAL_IC);
      error = ADBMS1818_rdaux(0, TOTAL_IC, BMS_IC);  // Set to read back all aux registers
      check_error(error);
      print_aux(datalog_en);
    }

    if (MEASURE_STAT == ENABLED) {
      wakeup_idle(TOTAL_IC);
      ADBMS1818_adstat(ADC_CONVERSION_MODE, STAT_CH_ALL);
      ADBMS1818_pollAdc();
      wakeup_idle(TOTAL_IC);
      error = ADBMS1818_rdstat(0, TOTAL_IC, BMS_IC);  // Set to read back all aux registers
      check_error(error);
      print_stat();
    }

    if (PRINT_PEC == ENABLED) {
      print_pec_error_count();
    }

    delay(MEASUREMENT_LOOP_TIME);
  }
}

/*!*********************************
  \brief Prints the main menu
  @return void
***********************************/
void print_menu(void) {
  Serial.println(F("List of ADBMS1818 Command:"));
  Serial.println(F("Write and Read Configuration: 1                            |Loop measurements with data-log output : 12                            |Set Discharge: 23   "));
  Serial.println(F("Read Configuration: 2                                      |Clear Registers: 13                                                    |Clear Discharge: 24   "));
  Serial.println(F("Start Cell Voltage Conversion: 3                           |Run Mux Self Test: 14                                                  |Write and Read of PWM : 25"));
  Serial.println(F("Read Cell Voltages: 4                                      |Run ADC Self Test: 15                                                  |Write and  Read of S control : 26"));
  Serial.println(F("Start Aux Voltage Conversion: 5                            |ADC overlap Test : 16                                                  |Clear S control register : 27"));
  Serial.println(F("Read Aux Voltages: 6                                       |Run Digital Redundancy Test: 17                                        |SPI Communication  : 28"));
  Serial.println(F("Start Stat Voltage Conversion: 7                           |Open Wire Test for single cell detection: 18                           |I2C Communication Write to Slave :29"));
  Serial.println(F("Read Stat Voltages: 8                                      |Open Wire Test for multiple cell or two consecutive cells detection:19 |I2C Communication Read from Slave :30"));
  Serial.println(F("Start Combined Cell Voltage and GPIO1, GPIO2 Conversion: 9 |Open wire for Auxiliary Measurement: 20                                |Enable MUTE : 31"));
  Serial.println(F("Start  Cell Voltage and Sum of cells : 10                  |Print PEC Counter: 21                                                  |Disable MUTE : 32"));
  Serial.println(F("Loop Measurements: 11                                      |Reset PEC Counter: 22                                                  |Set or reset the gpio pins: 33 \n "));
  Serial.println(F("Print 'm' for menu"));
  Serial.println(F("Please enter command: \n "));
}

/*!******************************************************************************
  \brief Prints the configuration data that is going to be written to the ADBMS1818
  to the serial port.
  @return void
 ********************************************************************************/
void print_wrconfig(void) {
  int cfg_pec;
  Serial.println(F("Written Configuration A Register: "));
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F("CFGA IC "));
    Serial.print(current_ic + 1, DEC);
    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].config.tx_data[i]);
    }
    Serial.print(F(", Calculated PEC: 0x"));
    cfg_pec = pec15_calc(6, &BMS_IC[current_ic].config.tx_data[0]);
    serial_print_hex((uint8_t)(cfg_pec >> 8));
    Serial.print(F(", 0x"));
    serial_print_hex((uint8_t)(cfg_pec));
    Serial.println("\n");
  }
}

/*!******************************************************************************
  \brief Prints the Configuration Register B data that is going to be written to
  the ADBMS1818 to the serial port.
  @return void
 ********************************************************************************/
void print_wrconfigb(void) {
  int cfg_pec;
  Serial.println(F("Written Configuration B Register: "));
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F("CFGB IC "));
    Serial.print(current_ic + 1, DEC);
    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].configb.tx_data[i]);
    }
    Serial.print(F(", Calculated PEC: 0x"));
    cfg_pec = pec15_calc(6, &BMS_IC[current_ic].configb.tx_data[0]);
    serial_print_hex((uint8_t)(cfg_pec >> 8));
    Serial.print(F(", 0x"));
    serial_print_hex((uint8_t)(cfg_pec));
    Serial.println("\n");
  }
}

/*!*****************************************************************
  \brief Prints the configuration data that was read back from the
  ADBMS1818 to the serial port.
  @return void
 *******************************************************************/
void print_rxconfig(void) {
  Serial.println(F("Received Configuration A Register: "));
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F("CFGA IC "));
    Serial.print(current_ic + 1, DEC);
    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].config.rx_data[i]);
    }
    Serial.print(F(", Received PEC: 0x"));
    serial_print_hex(BMS_IC[current_ic].config.rx_data[6]);
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].config.rx_data[7]);
    Serial.println("\n");
  }
}

/*!*****************************************************************
  \brief Prints the Configuration Register B that was read back from
  the ADBMS1818 to the serial port.
  @return void
 *******************************************************************/
void print_rxconfigb(void) {
  Serial.println(F("Received Configuration B Register: "));
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F("CFGB IC "));
    Serial.print(current_ic + 1, DEC);
    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].configb.rx_data[i]);
    }
    Serial.print(F(", Received PEC: 0x"));
    serial_print_hex(BMS_IC[current_ic].configb.rx_data[6]);
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].configb.rx_data[7]);
    Serial.println("\n");
  }
}

/*!************************************************************
  \brief Prints cell voltage codes to the serial port
  @return void
 *************************************************************/
void print_cells(uint8_t datalog_en) {
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    if (datalog_en == 0) {
      Serial.print(" IC ");
      Serial.print(current_ic + 1, DEC);
      Serial.print(", ");
      for (int i = 0; i < BMS_IC[0].ic_reg.cell_channels; i++) {
        Serial.print(" C");
        Serial.print(i + 1, DEC);
        Serial.print(":");
        Serial.print(BMS_IC[current_ic].cells.c_codes[i] * 0.0001, 4);
        Serial.print(",");
      }
      Serial.println();
    } else {
      Serial.print(" Cells, ");
      for (int i = 0; i < BMS_IC[0].ic_reg.cell_channels; i++) {
        Serial.print(BMS_IC[current_ic].cells.c_codes[i] * 0.0001, 4);
        Serial.print(",");
      }
    }
  }
  Serial.println("\n");
}

/*!****************************************************************************
  \brief Prints GPIO voltage codes and Vref2 voltage code onto the serial port
  @return void
 *****************************************************************************/
void print_aux(uint8_t datalog_en) {
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    if (datalog_en == 0) {
      Serial.print(" IC ");
      Serial.print(current_ic + 1, DEC);
      for (int i = 0; i < 5; i++) {
        Serial.print(F(" GPIO-"));
        Serial.print(i + 1, DEC);
        Serial.print(":");
        Serial.print(BMS_IC[current_ic].aux.a_codes[i] * 0.0001, 4);
        Serial.print(",");
      }

      for (int i = 6; i < 10; i++) {
        Serial.print(F(" GPIO-"));
        Serial.print(i, DEC);
        Serial.print(":");
        Serial.print(BMS_IC[current_ic].aux.a_codes[i] * 0.0001, 4);
      }

      Serial.print(F(" Vref2"));
      Serial.print(":");
      Serial.print(BMS_IC[current_ic].aux.a_codes[5] * 0.0001, 4);
      Serial.println();

      Serial.print(" OV/UV Flags : 0x");
      Serial.print((uint8_t)BMS_IC[current_ic].aux.a_codes[11], HEX);
      Serial.println();
    } else {
      Serial.print(" AUX, ");

      for (int i = 0; i < 12; i++) {
        Serial.print((uint8_t)BMS_IC[current_ic].aux.a_codes[i] * 0.0001, 4);
        Serial.print(",");
      }
    }
  }
  Serial.println("\n");
}

/*!****************************************************************************
  \brief Prints Status voltage codes and Vref2 voltage code onto the serial port
  @return void
 *****************************************************************************/
void print_stat(void) {
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    double itmp;

    Serial.print(F(" IC "));
    Serial.print(current_ic + 1, DEC);
    Serial.print(F(" SOC:"));
    Serial.print(BMS_IC[current_ic].stat.stat_codes[0] * 0.0001 * 30, 4);
    Serial.print(F(","));
    Serial.print(F(" Itemp:"));
    itmp = (double)((BMS_IC[current_ic].stat.stat_codes[1] * (0.0001 / 0.0076)) - 276);  // Internal Die Temperature(°C) = ITMP • (100 µV / 7.6mV)°C - 276°C
    Serial.print(itmp, 4);
    Serial.print(F(","));
    Serial.print(F(" VregA:"));
    Serial.print(BMS_IC[current_ic].stat.stat_codes[2] * 0.0001, 4);
    Serial.print(F(","));
    Serial.print(F(" VregD:"));
    Serial.print(BMS_IC[current_ic].stat.stat_codes[3] * 0.0001, 4);
    Serial.println();
    Serial.print(F(" OV/UV Flags:"));
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].stat.flags[0]);
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].stat.flags[1]);
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].stat.flags[2]);
    Serial.print(F("\tMux fail flag:"));
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].stat.mux_fail[0]);
    Serial.print(F("\tTHSD:"));
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].stat.thsd[0]);
    Serial.println();
  }
  Serial.println("\n");
}

/*!****************************************************************************
  \brief Prints GPIO voltage codes (GPIO 1 & 2)
  @return void
 *****************************************************************************/
void print_aux1(uint8_t datalog_en) {

  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    if (datalog_en == 0) {
      Serial.print(" IC ");
      Serial.print(current_ic + 1, DEC);
      for (int i = 0; i < 2; i++) {
        Serial.print(F(" GPIO-"));
        Serial.print(i + 1, DEC);
        Serial.print(":");
        Serial.print(BMS_IC[current_ic].aux.a_codes[i] * 0.0001, 4);
        Serial.print(",");
      }
    } else {
      Serial.print("AUX, ");

      for (int i = 0; i < 12; i++) {
        Serial.print(BMS_IC[current_ic].aux.a_codes[i] * 0.0001, 4);
        Serial.print(",");
      }
    }
  }
  Serial.println("\n");
}

/*!****************************************************************************
  \brief Prints Status voltage codes for SOC onto the serial port
 *****************************************************************************/
void print_sumofcells(void) {
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F(" IC "));
    Serial.print(current_ic + 1, DEC);
    Serial.print(F(" SOC:"));
    Serial.print(BMS_IC[current_ic].stat.stat_codes[0] * 0.0001 * 30, 4);
    Serial.print(F(","));
  }
  Serial.println("\n");
}

/*!****************************************************************
  \brief Function to check the MUX fail bit in the Status Register
   @return void
*******************************************************************/
void check_mux_fail(void) {
  int8_t error = 0;
  for (int ic = 0; ic < TOTAL_IC; ic++) {
    Serial.print(" IC ");
    Serial.println(ic + 1, DEC);
    if (BMS_IC[ic].stat.mux_fail[0] != 0)
      error++;

    if (error == 0)
      Serial.println(F("Mux Test: PASS \n"));
    else
      Serial.println(F("Mux Test: FAIL \n"));
  }
}

/*!************************************************************
  \brief Prints Errors Detected during self test
   @return void
*************************************************************/
void print_selftest_errors(uint8_t adc_reg, int8_t error) {
  if (adc_reg == 1) {
    Serial.println("Cell ");
  } else if (adc_reg == 2) {
    Serial.println("Aux ");
  } else if (adc_reg == 3) {
    Serial.println("Stat ");
  }
  Serial.print(error, DEC);
  Serial.println(F(" : errors detected in Digital Filter and Memory \n"));
}

/*!************************************************************
  \brief Prints the output of  the ADC overlap test
   @return void
*************************************************************/
void print_overlap_results(int8_t error) {
  if (error == 0)
    Serial.println(F("Overlap Test: PASS \n"));
  else
    Serial.println(F("Overlap Test: FAIL \n"));
}

/*!************************************************************
  \brief Prints Errors Detected during Digital Redundancy test
   @return void
*************************************************************/
void print_digital_redundancy_errors(uint8_t adc_reg, int8_t error) {
  if (adc_reg == 2) {
    Serial.println("Aux ");
  } else if (adc_reg == 3) {
    Serial.println("Stat ");
  }

  Serial.print(error, DEC);
  Serial.println(F(" : errors detected in Measurement \n"));
}

/*!****************************************************************************
  \brief Prints Open wire test results to the serial port
 *****************************************************************************/
void print_open_wires(void) {
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    if (BMS_IC[current_ic].system_open_wire == 65535) {
      Serial.print("No Opens Detected on IC ");
      Serial.print(current_ic + 1, DEC);
      Serial.println();
    } else {
      Serial.print(F("There is an open wire on IC "));
      Serial.print(current_ic + 1, DEC);
      Serial.print(F(" Channel: "));
      Serial.println(BMS_IC[current_ic].system_open_wire);
    }
  }
  Serial.println("\n");
}

/*!****************************************************************************
   \brief Function to print the number of PEC Errors
   @return void
 *****************************************************************************/
void print_pec_error_count(void) {
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.println("");
    Serial.print(BMS_IC[current_ic].crc_count.pec_count, DEC);
    Serial.print(F(" : PEC Errors Detected on IC"));
    Serial.println(current_ic + 1, DEC);
  }
  Serial.println("\n");
}

/*!****************************************************
  \brief Function to select the S pin for discharge
  @return void
 ******************************************************/
int8_t select_s_pin(void) {
  int8_t read_s_pin = 0;

  Serial.print(F("Please enter the Spin number: "));
  read_s_pin = (int8_t)read_int();
  Serial.println(read_s_pin);
  return (read_s_pin);
}

/*!****************************************************************************
  \brief prints data which is written on PWM register onto the serial port
  @return void
 *****************************************************************************/
void print_wrpwm(void) {
  int pwm_pec;

  Serial.println(F("Written PWM Configuration: "));
  for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F("IC "));
    Serial.print(current_ic + 1, DEC);
    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].pwm.tx_data[i]);
    }
    Serial.print(F(", Calculated PEC: 0x"));
    pwm_pec = pec15_calc(6, &BMS_IC[current_ic].pwm.tx_data[0]);
    serial_print_hex((uint8_t)(pwm_pec >> 8));
    Serial.print(F(", 0x"));
    serial_print_hex((uint8_t)(pwm_pec));
    Serial.println("\n");
  }
}

/*!****************************************************************************
  \brief Prints received data from PWM register onto the serial port
  @return void
 *****************************************************************************/
void print_rxpwm(void) {
  Serial.println(F("Received pwm Configuration:"));
  for (uint8_t current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F("IC "));
    Serial.print(current_ic + 1, DEC);
    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].pwm.rx_data[i]);
    }
    Serial.print(F(", Received PEC: 0x"));
    serial_print_hex(BMS_IC[current_ic].pwm.rx_data[6]);
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].pwm.rx_data[7]);
    Serial.println("\n");
  }
}

/*!****************************************************************************
  \brief prints data which is written on S Control register
  @return void
 *****************************************************************************/
void print_wrsctrl(void) {
  int sctrl_pec;

  Serial.println(F("Written Data in Sctrl register: "));
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F(" IC: "));
    Serial.print(current_ic + 1, DEC);
    Serial.print(F(" Sctrl register group:"));
    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrl.tx_data[i]);
    }

    Serial.print(F(", Calculated PEC: 0x"));
    sctrl_pec = pec15_calc(6, &BMS_IC[current_ic].sctrl.tx_data[0]);
    serial_print_hex((uint8_t)(sctrl_pec >> 8));
    Serial.print(F(", 0x"));
    serial_print_hex((uint8_t)(sctrl_pec));
    Serial.println("\n");
  }
}

/*!****************************************************************************
  \brief prints data which is read back from S Control register
  @return void
 *****************************************************************************/
void print_rxsctrl(void) {
  Serial.println(F("Received Data:"));
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F(" IC "));
    Serial.print(current_ic + 1, DEC);

    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrl.rx_data[i]);
    }

    Serial.print(F(", Received PEC: 0x"));
    serial_print_hex(BMS_IC[current_ic].sctrl.rx_data[6]);
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].sctrl.rx_data[7]);
    Serial.println("\n");
  }
}

/*!****************************************************************************
  \brief Prints data which is written on PWM/S control register group B onto
  the serial port
   @return void
 *****************************************************************************/
void print_wrpsb(uint8_t type) {
  int psb_pec = 0;

  Serial.println(F(" PWM/S control register group B: "));
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    if (type == 1) {
      Serial.print(F(" IC: "));
      Serial.println(current_ic + 1, DEC);
      Serial.print(F(" 0x"));
      serial_print_hex(BMS_IC[current_ic].pwmb.tx_data[0]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].pwmb.tx_data[1]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].pwmb.tx_data[2]);

      Serial.print(F(", Calculated PEC: 0x"));
      psb_pec = pec15_calc(6, &BMS_IC[current_ic].pwmb.tx_data[0]);
      serial_print_hex((uint8_t)(psb_pec >> 8));
      Serial.print(F(", 0x"));
      serial_print_hex((uint8_t)(psb_pec));
      Serial.println("\n");
    } else if (type == 2) {
      Serial.print(F(" IC: "));
      Serial.println(current_ic + 1, DEC);
      Serial.print(F(" 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrlb.tx_data[3]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrlb.tx_data[4]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrlb.tx_data[5]);

      Serial.print(F(", Calculated PEC: 0x"));
      psb_pec = pec15_calc(6, &BMS_IC[current_ic].sctrlb.tx_data[0]);
      serial_print_hex((uint8_t)(psb_pec >> 8));
      Serial.print(F(", 0x"));
      serial_print_hex((uint8_t)(psb_pec));
      Serial.println("\n");
    }
  }
}

/*!****************************************************************************
  \brief Prints received data from PWM/S control register group B
   onto the serial port
   @return void
 *****************************************************************************/
void print_rxpsb(uint8_t type) {
  Serial.println(F(" PWM/S control register group B:"));
  if (type == 1) {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
      Serial.print(F(" IC: "));
      Serial.println(current_ic + 1, DEC);
      Serial.print(F(" 0x"));
      serial_print_hex(BMS_IC[current_ic].pwmb.rx_data[0]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].pwmb.rx_data[1]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].pwmb.rx_data[2]);

      Serial.print(F(", Received PEC: 0x"));
      serial_print_hex(BMS_IC[current_ic].pwmb.rx_data[6]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].pwmb.rx_data[7]);
      Serial.println("\n");
    }
  } else if (type == 2) {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
      Serial.print(F(" IC: "));
      Serial.println(current_ic + 1, DEC);
      Serial.print(F(" 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrlb.rx_data[3]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrlb.rx_data[4]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrlb.rx_data[5]);

      Serial.print(F(", Received PEC: 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrlb.rx_data[6]);
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].sctrlb.rx_data[7]);
      Serial.println("\n");
    }
  }
}

/*!****************************************************************************
  \brief prints data which is written on COMM register onto the serial port
  @return void
 *****************************************************************************/
void print_wrcomm(void) {
  int comm_pec;

  Serial.println(F("Written Data in COMM Register: "));
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F(" IC- "));
    Serial.print(current_ic + 1, DEC);

    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].com.tx_data[i]);
    }
    Serial.print(F(", Calculated PEC: 0x"));
    comm_pec = pec15_calc(6, &BMS_IC[current_ic].com.tx_data[0]);
    serial_print_hex((uint8_t)(comm_pec >> 8));
    Serial.print(F(", 0x"));
    serial_print_hex((uint8_t)(comm_pec));
    Serial.println("\n");
  }
}

/*!****************************************************************************
  \brief Prints received data from COMM register onto the serial port
  @return void
 *****************************************************************************/
void print_rxcomm(void) {
  Serial.println(F("Received Data in COMM register:"));
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F(" IC- "));
    Serial.print(current_ic + 1, DEC);

    for (int i = 0; i < 6; i++) {
      Serial.print(F(", 0x"));
      serial_print_hex(BMS_IC[current_ic].com.rx_data[i]);
    }
    Serial.print(F(", Received PEC: 0x"));
    serial_print_hex(BMS_IC[current_ic].com.rx_data[6]);
    Serial.print(F(", 0x"));
    serial_print_hex(BMS_IC[current_ic].com.rx_data[7]);
    Serial.println("\n");
  }
}

/*!********************************************************************
  \brief Function to check the Mute bit in the Configuration Register
   @return void
**********************************************************************/
void check_mute_bit(void) {
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F(" Mute bit in Configuration Register B: 0x"));
    serial_print_hex((BMS_IC[current_ic].configb.rx_data[1]) & (0x80));
    Serial.println("\n");
  }
}

/*!****************************************************************************
  \brief Function to print the Conversion Time
  @return void
 *****************************************************************************/
void print_conv_time(uint32_t conv_time) {
  uint16_t m_factor = 1000;  // to print in ms

  Serial.print(F("Conversion completed in:"));
  Serial.print(((float)conv_time / m_factor), 1);
  Serial.println(F("ms \n"));
}

/*!****************************************************************************
  \brief Function to check error flag and print PEC error message
  @return void
 *****************************************************************************/
void check_error(int error) {
  if (error == -1) {
    Serial.println(F("A PEC error was detected in the received data"));
  }
}

/*!************************************************************
  \brief Function to print text on serial monitor
  @return void
*************************************************************/
void serial_print_text(char data[]) {
  Serial.println(data);
}

/*!****************************************************************************
   \brief Function to print in HEX form
   @return void
 *****************************************************************************/
void serial_print_hex(uint8_t data) {
  if (data < 16) {
    Serial.print("0");
    Serial.print((byte)data, HEX);
  } else
    Serial.print((byte)data, HEX);
}

/*!*****************************************************************************
  \brief Hex conversion constants
 *******************************************************************************/
char hex_digits[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*!************************************************************
  \brief Global Variables
 *************************************************************/
char hex_to_byte_buffer[5] = {
  '0', 'x', '0', '0', '\0'
};

/*!************************************************************
  \brief Buffer for ASCII hex to byte conversion
 *************************************************************/
char byte_to_hex_buffer[3] = {
  '\0', '\0', '\0'
};

/*!*****************************************************************************
  \brief Read 2 hex characters from the serial buffer and convert them to a byte
  @return char data Read Data
 ******************************************************************************/
char read_hex(void) {
  byte data;
  hex_to_byte_buffer[2] = get_char();
  hex_to_byte_buffer[3] = get_char();
  get_char();
  get_char();
  data = strtol(hex_to_byte_buffer, NULL, 0);
  return (data);
}

/*!************************************************************
  \brief Read a command from the serial port
  @return char
 *************************************************************/
char get_char(void) {
  // read a command from the serial port
  while (Serial.available() <= 0)
    ;
  return (Serial.read());
}
void print_array_large(float arr[TOTAL_IC][IC_CHANNELS]) {
  for (int row = 0; row < TOTAL_IC; row++) {
    for (int col = 0; col < IC_CHANNELS; col++) {
      Serial.print(arr[row][col], 4);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println("---");
}
void print_array_small(float arr[TOTAL_IC][CV_CHANNELS]) {

  for (int row = 0; row < TOTAL_IC; row++) {
    Serial.print(" IC ");
    Serial.print(row + 1, DEC);
    Serial.print(" : ");
    for (int col = 0; col < CV_CHANNELS; col++) {
      Serial.print(arr[row][col], 4);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println("---");
}