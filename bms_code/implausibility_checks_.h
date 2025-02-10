
/***********************************<Voltage>********************************************/

bool voltage_implausibility_check() {
  float currentmillis, voltageInitMillis;
  float volt;
  int i, j, BMS_IC_j, voltage_j;

  print_array_small(voltage_timer_array);

  // to take cv values from BMS_IC data structure and save them in the BMS_CV 2d-array
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    for (int channel = 0; channel < IC_CHANNELS; channel++) {
      BMS_CV[current_ic][channel] = BMS_IC[current_ic].cells.c_codes[channel] * 0.0001;
    }
  }
  // print_array_large(BMS_CV);

  // to convert 16*12 BMC_CV array to 16*8 voltage array by skipping 4,5,10,11 channel
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    BMS_IC_j = 0;
    voltage_j = 0;
    while (BMS_IC_j < 18) {
      if (BMS_IC_j == 17 || BMS_IC_j == 18) {
        BMS_IC_j++;
      } else {
        voltage[current_ic][voltage_j] = BMS_CV[current_ic][BMS_IC_j];
        BMS_IC_j++;
        voltage_j++;
      }
    }
  }
  print_array_small(voltage);

  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    for (int channel = 0; channel < CV_CHANNELS; channel++) {
      if (voltage[current_ic][channel] < UV_in_V ) //|| voltage[current_ic][channel] > OV_in_V
      {
        // if voltage not in plausible range, record current time in millis
        voltageInitMillis = millis();

        if (voltage_timer_array[current_ic][channel] == -1) {
          // if value at given value of current_ic and channel is -1, only then update it with millis value, this is to prevent overwriting the time in next loop
          voltage_timer_array[current_ic][channel] = voltageInitMillis;
        }
      } else {
        // if voltage is in plausible range set value to -1
        voltage_timer_array[current_ic][channel] = -1;
      }
    }
  }

  //  print_array_small(voltage_timer_array);

  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {

    for (int channel = 0; channel < CV_CHANNELS; channel++) {

      if (voltage_timer_array[current_ic][channel] != -1) {
        // this is to prevent subtracting "-1" from currentmillis, which will give a value greater than 500ms after millis reaches 500, (500-(-1) == 501)

        currentmillis = millis();
        // Serial.println(currentmillis - voltage_timer_array[current_ic][channel]);
        if (currentmillis - voltage_timer_array[current_ic][channel] >= VOLTAGE_IMPLAUSIBILITY_DURATION) {
          AMS_SD_Volt_Flag = true;
          Serial.println("Kaboom_Volt!!");
        }
      }

      //      else if(voltage_timer_array[current_ic][channel] == -1){
      //
      //        Serial.println("OK");
      //
      //
      //      }
    }
  }

  // Serial.println("Hakuna matata");
  return AMS_SD_Volt_Flag;
}


/**********************************<Temperature>*********************************************/


bool temp_implausibility_check() {
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    for (int gpio_pin = 0; gpio_pin < 9; gpio_pin++) {
      if (gpio_pin == 2 || gpio_pin == 4) {  // gpio_pin 3 for internal ntc
        TEMP_arr[current_ic][gpio_pin] = 2.69;
      } else {
        float temp_val = BMS_IC[current_ic].aux.a_codes[gpio_pin] * 0.0001;
        TEMP_arr[current_ic][gpio_pin] = temp_val;
      }
    }
    float vref2 = BMS_IC[current_ic].aux.a_codes[5] * 0.0001;
    // Print Overvoltage/Undervoltage flags
    //    Serial.print(F(" OV/UV Flags : 0x"));
    //    Serial.print((uint8_t)BMS_IC[current_ic].aux.a_codes[11], HEX);
    //    Serial.println();

    for (int gpio_pins = 0; gpio_pins < 9; gpio_pins++) {
      if (TEMP_arr[current_ic][gpio_pins] < OT_temp || TEMP_arr[current_ic][gpio_pins] > SCS_TEMP) {
        int TempInitMillis = millis();  //<!if tempp not in plausible range, record current time in TempInitMillis

        /************************************************************
          If value at given value of current_ic and gpio is -1, 
          only then update it with millis() value.
          This is to prevent overwriting the time in next loop
          *************************************************************/
        if (temp_timer_array[current_ic][gpio_pins] == -1) {
          temp_timer_array[current_ic][gpio_pins] = TempInitMillis;
        }
      } else {
        temp_timer_array[current_ic][gpio_pins] = -1;  //if tmep is in plausible range set value back to -1
      }
    }

    for (int gpio_pin = 0; gpio_pin < 9; gpio_pin++) {
      if (temp_timer_array[current_ic][gpio_pin] != -1) {
        int currentmillis = millis();
        if (currentmillis - temp_timer_array[current_ic][gpio_pin] >= TEMP_IMPLAUSIBILITY_DURATION) {
          AMS_SD_Temp_Flag = false;
          Serial.println("Kaboom_Temp!!");
        }
      }
    }
  }
  return AMS_SD_Temp_Flag;
  Serial.println("----------------------------------------------------------------------------------------------------");
}
float voltage_to_temperature(float V_ntc) {

  // Convert voltage to resistance using voltage divider formula
  float R_ntc = R_PULLUP * (VCC / V_ntc - 1);
  // Calculate temperature using Steinhart-Hart equation
  float inv_T = A + B * log(R_ntc) + C * pow(log(R_ntc), 3);
  float T = 1 / inv_T - 273.15;  // Convert to Celsius
  return T;
}
void print_temp() {
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    Serial.print(F(" IC "));
    Serial.print(current_ic + 1, DEC);
    Serial.print(F(" : "));
    for (int i = 0; i < 9; i++) {
      float temp_const = voltage_to_temperature(TEMP_arr[current_ic][i]);
      temperatures[current_ic][i] = temp_const;
      Serial.print(F(" "));
      Serial.print(temperatures[current_ic][i], 4);
      Serial.print(F(" C "));
    }
    Serial.println();
  }
}

bool current_implausibility_check() {
  float Currentmillis;
  CurrentVal = (analogRead(CurrPin)) * 5 / 1023.00;
  if (CurrentVal >= OC) {
    if (currentInitMillis == -1) {
      currentInitMillis = millis();
    }
  } else {
    currentInitMillis = -1;
  }
  if (currentInitMillis != -1) {
    Currentmillis = millis();

    if (Currentmillis - currentInitMillis >= 500) {
      AMS_SD_Curr_Flag = false;  //should be true
      Serial.print("CurrentInitMillis :");
      Serial.println(currentInitMillis);
      Serial.print("Current Value :");
      Serial.println(CurrentVal);
      Serial.print("");
    }
  }

  return AMS_SD_Curr_Flag;
}

/*************************************************************************************************************/
void check_AMS_Fault(bool isFault, int fault_id) {

  if (isFault == 1) {
    switch (fault_id) {

      case 1:
        Serial.println("Voltage implausibility");
        digitalWrite(AMS_FAULT, HIGH);
        break;

      case 2:
        Serial.println("Temperature implausibility");
        digitalWrite(AMS_FAULT, HIGH);
        break;

      case 3:
        Serial.println("Current implausibility");
        digitalWrite(AMS_FAULT, HIGH);
        break;

      case 4:
        Serial.println("PEC error");
        break;

      default:
        Serial.println("BeepBopSkrrt");
        // what to add?
    }
  } else {
    Serial.println("BeepBopSkrrt");
  }
}
