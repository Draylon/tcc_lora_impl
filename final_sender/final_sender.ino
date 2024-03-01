#include "heltec.h"
#include "Heltecdisplay.h"
#include "ecdh.h"
#include "EEPROM.h"

//#define PRINTS_ENABLED

#define TIME_TO_SLEEP 300
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define BAND  915E6
#define SENSOR_PIN 32
//#define EEPROM_SIZE 64

RTC_DATA_ATTR uint8_t bootCount = 0;
RTC_DATA_ATTR bool secret_available=false;
RTC_DATA_ATTR uint8_t mem_secret[32];
RTC_DATA_ATTR uint16_t mqtt_cli_id;
RTC_DATA_ATTR uint32_t vibecheck;


const uint8_t dsize = 1;      // device ID size
const uint8_t fragcap = 64;   // maximum fragment size in bytes 

uint8_t mqtt_ctrl[] = {107,101,121,114,101,99,118};
int rssi = 0;
char packet[256];
//tamanho máximo de pacote: 240

float sensorValue;

AESSmall256 aes_instance;


//================================
/* 
void bt_boot_setup(){
    BLEDevice::init("ESP32 Server");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    pServer->createService()
    BLEService *pService = pServer->createService(BLEUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"));

    pCharacteristic = pService->createCharacteristic(
                        BLEUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8"),
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE
                      );

    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    Serial.println("Waiting for a connection...");

}

void bt_boot_pairing(){
  while(true){
      if (deviceConnected) {
        pCharacteristic->setValue(&value, 1);
        pCharacteristic->notify();
        value++;
        delay(1000); // Delay a second between loops to avoid flooding the BLE device
      }

      if (!deviceConnected && oldDeviceConnected) {
        delay(500); // Give the Bluetooth stack the chance to get things ready
        BLEDevice::startAdvertising();
        Serial.println("Waiting for a connection...");
        oldDeviceConnected = deviceConnected;
      }

      if (deviceConnected && !oldDeviceConnected) {
        // Do initialization stuff here
        oldDeviceConnected = deviceConnected;
      }
  }
}
 */
//================================

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void esp_sleep(){
  LoRa.end();
  LoRa.sleep();
  delay(100);
  /* pinMode(5,INPUT);
  pinMode(14,INPUT);
  pinMode(15,INPUT);
  pinMode(16,INPUT);
  pinMode(17,INPUT);
  pinMode(18,INPUT);
  pinMode(19,INPUT);

  pinMode(26,INPUT);
  pinMode(27,INPUT); */
}

void esp_define_wake(){
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 wake in " + String(TIME_TO_SLEEP) + " Seconds");
}


void query_data(){
    sensorValue = analogRead(SENSOR_PIN);
}

void ledBlink(){
  digitalWrite(25, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(300);                       // wait for a second
  digitalWrite(25, LOW);    // turn the LED off by making the voltage LOW
}

void printbin(uint8_t* b, uint8_t len) {
  for (int i=0; i<len; i++){
    
    //if (b[i]<0x10) Serial.print("0");
    //Serial.print(b[i],HEX);
    Serial.print((uint8_t)b[i]);
    
    Serial.print(" ");
  }
  Serial.println("\nDone printing");
}

void setup(){
  Serial.begin(115200);
  while(!Serial){
    delay(50);
  }
  pinMode(25, OUTPUT);
  
  /* if (!EEPROM.begin(EEPROM_SIZE)){
    Serial.println("~~EEPROM FAILED!!\nAES SECRET LOST");
  }else{
    Serial.println("~~EEPROM STARTED");
    byte comp1[6];
    for (int i = 0; i < 6; i++){
      comp1[i]=(byte(EEPROM.read(i)));
    }
    Serial.print("found in eeprom: ");
    for(int i=0;i < 6;i++) Serial.print((char)comp1[i]);
    Serial.println();
    if(memcmp(comp1,"aeskey",6) == 0){
      Serial.println("aes key found");
      secret_available=true;
      for(int i=6;i < 38;i++){
        secret[i-6]=(uint8_t) EEPROM.read(i);
      }
    }else{
      Serial.println("aes is invalid");
      secret_available=false;
    }
  } */

  if(secret_available) initialize_aes(&aes_instance,mem_secret);
  else                 ecdh_setup(curve);
  //m1=new COMSAT(NULL);
  //Serial.begin(9600);
  
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(false /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
 
  //Heltec.display->init();
  //Heltec.display->flipScreenVertically();  
  //Heltec.display->setFont(ArialMT_Plain_10);
  //logo();

  //Print the wakeup reason for ESP32
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();


  delay(1000);

  LoRa.setSpreadingFactor(10);
  
  /* if(bootCount==1){
    Serial.println("signaling comms");
  m1->comm_ping();
  delay(2000);
  Serial.println("signaling clock");
  m1->clock_sync();
  delay(2000);
  Serial.println("signaling key");
  m1->comm_pair();
  delay(2000);
  } */
}

void loop(){
  if(!secret_available) pairing_procedure();
  query_data();
  //drawDisplay();
  #ifdef PRINTS_ENABLED
  Serial.print("Sensor data: |");
  Serial.print(String(sensorValue));
  Serial.println("|");
  Serial.println("sync_send_0");
  #endif
  //m1->sendData(&sendPacket,NULL,(uint8_t*)String(sensorValue).c_str(),8);
  //sendPacket((uint8_t*)String(sensorValue).c_str(),4);
  uint8_t tab2[16];
  aes_instance.encryptBlock(tab2,(uint8_t*)(String(sensorValue)+String("1234567891")).c_str());

  /* uint16_t x = 260;
  uint8_t xlow = x & 0xff;
  uint8_t xhigh = (x >> 8);
  uint16_t lmao = ((uint16_t)xhigh << 8) + xlow;
  printf("%d %d %d %d",xlow,xhigh,x,lmao);
   */
  uint8_t xlow = mqtt_cli_id & 0xff;
  uint8_t xhigh = (mqtt_cli_id >> 8);

  LoRa.beginPacket();
  LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.write(xlow);
  LoRa.write(xhigh);
  LoRa.write(tab2,16);
  LoRa.endPacket();
  
  #ifdef PRINTS_ENABLED
    Serial.println("sync_send_1");
  #endif
  ledBlink();
  esp_define_wake();
  esp_sleep();
  esp_deep_sleep_start();
  //delay(15000);
}


void pairing_procedure(){
    //envia request
    
    send_keypair_request();
    //delay(200);
    
    //aguarda foreign public key
    bool breaker=false;
    while(!breaker){
      Serial.println("Await foreign key;");
      loopFunctionCaller(&receive_foreign,&breaker,&send_keypair_request,5000,false);
    }
    delay(500);

    Serial.println("creating secret");
    if(!ecdh_init(curve,mem_secret)){
      Serial.print("shared_secret() failed (1)\n");
      return;
    }
    secret_available=true;
    Serial.println("secret created");
    initialize_aes(&aes_instance,mem_secret);
    Serial.println("initialized aes");

    //manda a propria public key
    
    send_public_key();

    //receber a ID de MQTT aqui
    breaker=false;
    while(!breaker){
      Serial.println("~receive mqtt id");
      loopFunctionCaller(&receive_mqtt_key,&breaker,&send_public_key,5000,false);
    }

    //guardar segredo do AES
    
    /* const char* aeskey = {"aeskey"};
    for(int i=0;i < 6;i++) EEPROM.write(i,(uint8_t)aeskey[i]);
    for(int i=6;i < 38;i++){
      EEPROM.write(i, secret[i-6]);
    }
    EEPROM.commit();

    byte comp1[6];
    for (int i = 0; i < 6; i++){
      comp1[i]=(byte(EEPROM.read(i)));
    }
    Serial.print("found in eeprom: ");
    for(int i=0;i < 6;i++) Serial.print((char)comp1[i]);
    Serial.println();
    if(memcmp(comp1,"aeskey",6) == 0){
      Serial.println("AES KEY STORED SUCCESSFULY");
      secret_available=true;
    }else{
      Serial.println("STORED AES IS INVALID!!!");
      secret_available=false;
    } */

    //começar a enviar o dado no formato certo
}


void loopFunctionCaller(void (*loop)(bool*),bool* value,void (*lease)(),int interval,const bool singleTrigger){
    unsigned long threshold =  millis() + interval;
    bool halt=false;
    while(millis() < threshold){
        if((singleTrigger && !halt)||!singleTrigger) loop(value);
        if( singleTrigger && !halt) halt=!halt;
    }
    if(!(*value)) lease();
}

void send_keypair_request(){
  Serial.println("Sending keypair request;");
  LoRa.beginPacket();
  LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print("keypair.");
  LoRa.endPacket();
}
void receive_foreign(bool* breaker){
  //Serial.println("Await foreign key;");
  int size1=LoRa.parsePacket();
  if(size1>0){
    Serial.println("possibly a public key");
    uint8_t packet1[size1];
    if(size1!=64){
      Serial.println("wtf chegou no lora?");
    }else{
      for (int i = 0; i < size1; i++){foreign[i]=(uint8_t) LoRa.read();}
      *breaker=true;
    }
  }
  delay(10);
}

bool key_recv=false;
void send_public_key(){
  if(key_recv) return;
  Serial.println("send public key");
  LoRa.beginPacket();
  LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
  for(int i=0;i<pub_size;i++){
    LoRa.write(publkey[i]);
  }
  LoRa.endPacket();
  LoRa.endPacket();
  
}

void receive_mqtt_key(bool* breaker){
  //Serial.println("~receive mqtt id");
  int size1=LoRa.parsePacket();
  if(size1>0 && !(*breaker)){
    uint8_t packet1[size1];
    uint8_t packet2[size1];
    for (int i = 0; i < size1; i++){packet1[i]=(uint8_t) LoRa.read();}
    if(memcmp(packet1,mqtt_ctrl,size1) == 0){
      key_recv=true;
      Serial.println("mqtt signal received");
    }else{
      //Serial.print("cyphered:");
      Serial.println(size1);
      Serial.println(String((char*)packet1));
      //for (int i = 0; i < size1; i++){Serial.print((char)packet1[i]);}
      aes_instance.decryptBlock(packet2, packet1);
      //Serial.println("\ndecyphered:");
      //for (int i = 0; i < size1; i++){Serial.print((char)packet2[i]);}
      Serial.println(String((char*)packet2));
      mqtt_cli_id = atoi((String((char*)packet2)).c_str());
      *breaker=true;
    }
  }
  delay(10);
}










#ifdef  ZXCZXCCZASD

void sendPacket(uint8_t* packet,uint8_t rplen){
  /*
 * LoRa.setTxPower(txPower,RFOUT_pin);
 * txPower -- 0 ~ 20
 * RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
 *   - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
 *   - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
*/

  //  char* dev_id | uint8_t dsize | uint8_t frag | uint8_t msglen | uint8_t* msg
  
  //https://crypto.stackexchange.com/questions/11752/elliptic-curve-encryption-ciphertext-size
  //https://asecuritysite.com/ecc/ecc3

  printbin(packet,rplen);

  uint8_t rmn = 0;
  uint8_t fragc=1;
  if(rplen < fragcap) fragc = 0;
  while(rmn < rplen){
      uint8_t headerTrim;
      uint8_t headerLen = m1->calculateFragmentHeader(DEV_ID,&dsize,fragc,&headerTrim);
      
      uint8_t header[headerLen];
      m1->fragmentHeader(header,DEV_ID,&dsize,fragc);
      
      uint8_t pkt = fragcap - 1 - headerLen+headerTrim;  // tamanho disponivel de pacote
      
      uint8_t lim1;
      if(rmn + pkt < rplen){
          lim1 = rmn + pkt;
      }else{
        lim1 = rplen;
      }

      #ifdef PRINTS_ENABLED
      Serial.print("header trimmed in ");
      Serial.println(String(headerTrim));
      Serial.print("packet size available: ");
      Serial.println(String(pkt));
      
      Serial.print("sending ");
      Serial.print(String(fragc));
      Serial.print("  refs:");
      Serial.print(String(2));
      Serial.print("  head:");
      Serial.print(String(headerLen-headerTrim));
      Serial.print("  data:");
      Serial.print(String(lim1-rmn));
      Serial.print("  full:");
      Serial.println(String(2+headerLen-headerTrim+(lim1-rmn)));
      
      Serial.print(headerLen-headerTrim);
      Serial.print(' ');
      for(int ii=0;ii < headerLen-headerTrim; ii++){Serial.print((unsigned char)header[ii]);Serial.print(' ');}
      for(int ii=rmn;ii < lim1;ii++){Serial.print((unsigned char)packet[ii]);Serial.print(' ');}
      Serial.println("");
      #endif

      LoRa.beginPacket();
      LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
      
      LoRa.write((unsigned char)(headerLen-headerTrim));
      for(int ii=0;ii < headerLen-headerTrim; ii++) LoRa.write((unsigned char)header[ii]);
      
      // total - header == body :/
      //LoRa.write((unsigned char)(lim1-rmn));
      
      for(int ii=rmn;ii < lim1;ii++) LoRa.write((unsigned char)packet[ii]);
      
      LoRa.endPacket();
      rmn=lim1;
      fragc++;
      if(lim1!=RSA_BYTES)delay(300);

      #ifdef PRINTS_ENABLED
      //check base64 decryption
      unsigned char stonks[headerLen-headerTrim];
      for(int ii=0;ii < headerLen-headerTrim; ii++) stonks[ii] = header[ii];
      unsigned char outstonks[30];
      base64_decode(stonks,headerLen-headerTrim,outstonks);
      for(int ii=0;ii < 2;ii++){Serial.print(outstonks[ii]);Serial.print(' ');}
      Serial.println("");
      #endif
  }

  /* LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
  for(int ii=0;ii < RSA_BYTES;ii++)
    LoRa.print((uint8_t)packet[ii]);
  LoRa.endPacket(); */
  

  Serial.println("sent");
}

/*
 * HelTec Automation(TM) Low_Power test code, witch includ
 * Function summary:
 *
 * - Vext connected to 3.3V via a MOS-FET, the gate pin connected to GPIO21;
 *
 * - OLED display and PE4259(RF switch) use Vext as power supply;
 *
 * - WIFI Kit series V1 don't have Vext control function;
 * 
 * - Basic LoRa Function;
 *
 * - Esp_Deep_Sleep Function;
 * 
 * by lxyzn from HelTec AutoMation, ChengDu, China
 * 
 * www.heltec.cn
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
*/

//#include "heltec.h"



#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6



RTC_DATA_ATTR int bootCount = 0;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case 1  :
    {
      Serial.println("Wakeup caused by external signal using RTC_IO");
      delay(2);
    } break;
    case 2  :
    {
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      delay(2);
    } break;
    case 3  :
    {
      Serial.println("Wakeup caused by timer");
      delay(2);
    } break;
    case 4  :
    {
      Serial.println("Wakeup caused by touchpad");
      delay(2);
    } break;
    case 5  :
    {
      Serial.println("Wakeup caused by ULP program");
      delay(2);
    } break;
    default :
    {
      Serial.println("Wakeup was not caused by deep sleep");
      delay(2);
    } break;
  }
}

void setup(){
  
  Serial.begin(115200);
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  delay(2);

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
  Now that we have setup a wake cause and if needed setup the
  peripherals state in deep sleep, we can now start going to
  deep sleep.
  In the case that no wake up sources were provided but deep
  sleep was started, it will sleep forever unless hardware
  reset occurs.
  */
  LoRa.end();
  LoRa.sleep();
  delay(100);
  

  pinMode(5,INPUT);

   pinMode(14,INPUT);
   pinMode(15,INPUT);
   pinMode(16,INPUT);
   pinMode(17,INPUT);
pinMode(18,INPUT);
  pinMode(19,INPUT);

  pinMode(26,INPUT);
  pinMode(27,INPUT);


  delay(100);
  //Serial.println("Going to sleep now");
  //delay(2);
  //esp_deep_sleep_start();
  //Serial.println("This will never be printed");
}
int aylmao = 0;
void loop(){
  Serial.print("num: ");
  Serial.println(aylmao);
  aylmao++;
  if(aylmao % 4 == 0){
    esp_deep_sleep_start();
  }
  delay(1000);
  //This is not going to be called
}
#endif