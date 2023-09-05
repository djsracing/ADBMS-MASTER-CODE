#include <SPI.h>
#include <mcp_can.h>

#define MCP_8MHZ 8000000
#define MCP_16MHZ 16000000

/*
 * 
 * byte 0: O/P Voltage HIGH BYTE
 * byte 1: O/P Voltage LOW BYTE
 * byte 2: Max Charge current HIGH BYTE
 * byte 3: Max Charge Current LOW BYTE
 * byte 4: Charger start to recharge :0,battery protect, charger shut down
 * byte 5: 0: Batter Mode,1: Heating Mode
 * byte 6: retain
 * byte 7: retain
 * BMSA CAN ID: 0x1806E5F4
 */

 struct CanFrametoSend
{
    /* data */
    uint8_t voltage_h;
    uint8_t voltage_l;
    uint8_t current_h;
    uint8_t current_l;
    uint8_t charger_state;
    uint8_t mode_state;
};

CanFrametoSend frame;
const uint32_t BMSA_CAN_ID = 0x1806E5F4;

const int spiCSPin = 10;
int ledHIGH    = 1;
int ledLOW     = 0;


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

unsigned char stmp[8];
    
void loop()
{   
  Serial.println("In loop");
  uint16_t voltage = 5000;     //500 V
  uint16_t current = 5;        // 0.5A
  frame.voltage_h = highByte(voltage);
  frame.voltage_l = lowByte(voltage);
  frame.current_h = highByte(current);
  frame.current_l = lowByte(current);
  frame.charger_state = 0x00;
  frame.mode_state = 0x00;
  
  unsigned char stmp[8]= {frame.voltage_h,frame.voltage_l,frame.current_h,frame.current_l,frame.charger_state,frame.mode_state}; 
 
  CAN.sendMsgBuf(BMSA_CAN_ID, 1, 8, stmp);//ID,ifextendedFrame,lenOfMsg,Msg

  Serial.print((float)voltage/10);
  Serial.print(" ");
  Serial.println((float)current/10);
  
  delay(1000);
}
