#include <SPI.h>
#include "mcp_can.h"

#define MCP_8MHZ 8000000
#define MCP_16MHZ 16000000

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

struct CanFrametoReci
{
  uint8_t voltage_h;
  uint8_t voltage_l;
  uint8_t current_h;
  uint8_t current_l;
  uint8_t SFLAG;
  uint8_t pilot_alarm;
  uint8_t chargerTemp;
};

void checkStatusFlag(uint8_t sflag){
  if(sflag & 1<<0){
    //normal
  }else{
    //Hardware Failure
  }

  if(sflag & 1<<1){
    //normal
  }else{
    //Charger Temperature is too high
  }
  
  if(sflag & 1<<2){
    //Input Voltage is Normal
  }else{
    //Input Voltage Error
  }
  
  if(sflag & 1<<3){
    //Charger detects the battery voltage in the start up state
  }else{
    //Off, to prevent reverse battery connection
  }
  
  if(sflag & 1<<4){
    //communication is normal
  }else{
    //communication reception timeout
  }
}
const int spiCSPin = 10;

MCP_CAN CAN(spiCSPin);

void setup()
{
    Serial.begin(9600);

    while (CAN_OK != CAN.begin(MCP_ANY ,CAN_500KBPS ,MCP_8MHZ))
    {
        Serial.println("CAN BUS init Failed");
        delay(100);
    }
    Serial.println("CAN BUS Init OK");
}

unsigned char temp[8];
CanFrametoReci frame;
uint16_t voltage,current;
void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())
    {
       
        CAN.readMsgBuf(&rxId, &len, rxBuf);

        unsigned long canId = rxId;

        Serial.println("-----------------------------");
        Serial.print("Data from ID: 0x");
        Serial.println(canId, HEX);

        for(int i = 0; i<len; i++)
        {
            Serial.print(buf[i]);
            Serial.print(" ");
            temp[i] = buf[i];
        }
        frame.voltage_l = temp[0];
        frame.voltage_h = temp[1];
        frame.current_l = temp[2];
        frame.current_h = temp[3];
        frame.SFLAG = temp[4];
        frame.pilot_alarm = temp[5];
        frame.chargerTemp = temp[6];

        checkStatusFlag(frame.SFLAG);

        voltage = frame.voltage_h<<8 + frame.voltage_l;
        current = frame.current_h<<8 + frame.current_l;

        Serial.println();
        
        Serial.print((float)voltage/10);
        Serial.print(" ");
        Serial.println((float)current/10);
        
        Serial.println();
    }
}
