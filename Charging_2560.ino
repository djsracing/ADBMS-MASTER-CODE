#include <CAN.h>

#define TX_GPIO_NUM   17  // Connects to CTX

#define RX_GPIO_NUM   16  // Connects to CRX

 

uint8_t arr[8];

uint16_t combineBytes();

uint16_t voltage,current;

 

void setup() {

  int cs = 49;

  int irq = 2;

  Serial.begin(115200);

  while (!Serial);

  // CAN.setPins(RX_GPIO_NUM, TX_GPIO_NUM);

  CAN.setPins(cs, irq);

  Serial.println("CAN Sender");

  // Start the CAN bus at 500 kbps

  if (!CAN.begin(500E3)) {

    Serial.println("Starting CAN failed!");

    while (1);

  }

  Serial.println("CAN Receiver");

}

 

void loop() {

  canSender();

  canReceiver();

  delay(100);

}

void canSender(){

// Send extended packet: id is 29 bits, packet can contain up to 8 bytes of data

  uint16_t voltage = 3804;    

  uint16_t current = 40;       

 

  Serial.print("Sending extended packet ... ");

 

  CAN.beginExtendedPacket(0x1806E5F4);  // Start building an extended packet with ID 0xabcdef

  CAN.write((uint8_t)highByte(voltage));                  

  CAN.write((uint8_t)lowByte(voltage));                    

  CAN.write((uint8_t)highByte(current));                    

  CAN.write((uint8_t)lowByte(current));                    

  CAN.write((uint8_t)0);

  CAN.write((uint8_t)0);

  CAN.write((uint8_t)0);

  CAN.write((uint8_t)0);  

                       

  CAN.endPacket();                    // Send the packet

  Serial.println("done");

 

  delay(100);

}

void checkStatusFlag(uint8_t sflag){

  if(sflag & 1<<0){

   

  Serial.println("Hardware Failure");

  }else{

    //normal

  }

 

  if(sflag & 1<<1){

   

 //Charger Temperature is too high

    Serial.println("Charger Temperature is too high");

  }else{

   //normal

  }

 

  if(sflag & 1<<2){

   

 //Input Voltage Error

    Serial.println("Input Voltage Error");

  }else{

   //Input Voltage is Normal

  }

 

  if(sflag & 1<<3){

    //Off, to prevent reverse battery connection

    Serial.println("Off, to prevent reverse battery connection");

  }else{

 

    //Charger detects the battery voltage in the start up state

  }

 

  if(sflag & 1<<4){

   

 //communication reception timeout

    Serial.println("communication reception timeout");

  }else{

   //communication is normal

  }

}

 

void canReceiver() {

  // try to parse packet

  int packetSize = CAN.parsePacket();

 

  if (packetSize) {

    // received a packet

 

    if(CAN.packetId()) {

    Serial.print("Received ");

 

    if (CAN.packetExtended()) {

      Serial.print("extended ");

    }

 

    if (CAN.packetRtr()) {

      // Remote transmission request, packet contains no data

      Serial.print("RTR ");

    }

 

    Serial.print("packet with id 0x");

    Serial.print(CAN.packetId(), HEX);

 

    if (CAN.packetRtr()) {

      Serial.print(" and requested length ");

      Serial.println(CAN.packetDlc());

    } else {

      Serial.print(" and length ");

      Serial.println(packetSize);

 

      // only print packet data for non-RTR packets

      while (CAN.available()) {

        // Serial.println(CAN.read());

        for ( int i=0;i<8;i++){

 

           arr[i] = CAN.read();

          //  Serial.println(arr[i]);

        }  

       if(arr[4] != 0){

         Serial.println("Ayee vedya");

         checkStatusFlag(arr[4]);

         Serial.println(arr[4]);

       }

       voltage = combineBytes(arr[0],arr[1]);

       Serial.print("Output voltage: ");

       Serial.println(voltage/10.0);

       current = combineBytes(arr[2],arr[3]);

       Serial.print("Output current: ");

       Serial.println(current/10.0);    

      }

      Serial.println();

    }

    Serial.println();

  }

  }

}

 

uint16_t combineBytes(uint8_t highByte, uint8_t lowByte) {

    return ((uint16_t)highByte << 8) | lowByte;

}