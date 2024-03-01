#include <heltec.h>
#include <stdio.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config_file.h"

#include "WifiHelper.h"
#include "ntp.h"
#include "Webserver.h"
#include "heltecdisplay.h"
#include "comsat.hpp"

#include "ecdh.h"

#define BAND 915E6  //you can set band here directly,e.g. 868E6,915E6

///=============================================================

WiFiClient mqttNetworkClient;
PubSubClient mqtt_client(mqttNetworkClient);
COMSAT *comsat;

void mqtt_subscriber_callback(char *topic, byte *message, unsigned int length){
    Serial.print("Message arrived on topic: ");
    Serial.println(topic);
    for(int i=0;i < length;i++){
      Serial.write((unsigned char)message[i]);
    }
    Serial.print("\n========================\n");
    if(strcmp(topic,"hyqnap5637/sensor/registered_sensor") == 0){
      comsat->lastmqtt_topic=topic;
      comsat->lastmqtt_message=message;
      comsat->lastmqtt_length=length;
      comsat->comm_pair(2);
    }
}


void mqtt_reconnect(){
  if(mqtt_client.connected()) mqtt_client.loop();
  // Loop until we're reconnected
  while (!mqtt_client.connected()){
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (
        #ifdef MQTT_SECURE
            mqtt_client.connect("hyqnap_sender", mqtt_username, mqtt_password)
        #else
            mqtt_client.connect("hyqnap_sender")
        #endif
        ){
          Serial.println("connected");
          // Subscribe
          mqtt_client.subscribe("hyqnap5637/#");
      }else{
          Serial.print("failed, rc=");
          Serial.print(mqtt_client.state());
          Serial.println(" try again in 5 seconds");
          // Wait 5 seconds before retrying
          delay(5000);
      }
  }
}

unsigned long keyValidUntil = 0;

void cbk(int packetSize) {
  if(packetSize==0)return;
  unsigned char packet[packetSize];

  Serial.print("size: ");
  Serial.println(packetSize);
  Serial.print("data:  |");
  //String strPacketSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++){
    packet[i]=(unsigned char) LoRa.read();
    Serial.print((char)packet[i]);
  }
  Serial.println("|");
  
  //uint16_t map_pos = (unsigned int)packet[0] << 8 | packet[1];
  //std::map<uint16_t,uint8_t>::iterator mapiter = queue_mapping.find()
  if(aes_store.size() > 0&&packetSize<60){
    uint8_t cypher_size =  (((int)((packetSize-2)/16.01f))+1)*16;
    Serial.println("looking for valid keys");
    uint8_t decph[cypher_size];
    uint8_t cyphd[cypher_size];
    for(int i=2;i < packetSize;i++){
      cyphd[i-2]=packet[i];
    }
    
    /* uint16_t x = 260;
    uint8_t xlow = x & 0xff;
    uint8_t xhigh = (x >> 8);
    uint16_t lmao = ((uint16_t)xhigh << 8) + xlow;
    printf("%d %d %d %d",xlow,xhigh,x,lmao);
    */
    uint16_t mqtt_cli_id = ((uint16_t)packet[1] << 8) + packet[0];
    std::map<uint16_t,uint8_t>::iterator queueiter = queue_mapping.find(mqtt_cli_id);
    Serial.print("MQTT key:");
    Serial.println(mqtt_cli_id);
    
    if (queueiter == queue_mapping.end()){
      Serial.println("incorrect cypher key / random data inserted");
    }else{
      uint8_t tm1p = queueiter->second;
      Serial.print("range found:");
      Serial.print(tm1p);
      Serial.print(" of ");
      Serial.println(aes_store.size());
      if(tm1p >=0 && aes_store.size() > tm1p){
          //attempt to decypher
          //aes_store[0]
          decrypt_msg(aes_store[tm1p],cyphd,decph);
          Serial.println("decyphered:");
          for(int i=0;i <cypher_size;i++){
            Serial.print((char) decph[i]);
          }
          Serial.println();
          mqtt_client.publish(
            "hyqnap5637/data",
            (customstd::to_string(mqtt_cli_id)+customstd::to_string("_")+customstd::to_string((unsigned char*)decph)).c_str(),
            cypher_size+customstd::to_string(mqtt_cli_id).length()+customstd::to_string("_").length()
          );
      }else{
        Serial.println("outside range / random data inserted");
      }
    }

  }
  if(packetSize==8){
    if(       memcmp(packet,"keypair.",8) == 0 ){
      delay(500);
      Serial.print("received ECDH public key request: ");
      if(comsat->ongoing_stage[3] != 0){
          if(keyValidUntil < millis()){
            Serial.println("giving up previous key");
            comsat->ongoing_stage[3] =0;
            keyValidUntil = millis() + 25000;
          }else{
            Serial.println("ongoing");
          }
      }else{
        Serial.println("available");
        keyValidUntil = millis() + 25000;
      }
      comsat->comm_pair(0);
      return;
    }else if( memcmp(packet,"clksync.",8) == 0){
      Serial.println("received clock sync request");
      comsat->clock_sync(0);
      return;
    }else if( memcmp(packet,"comsatr.",8) == 0){
      Serial.println("received coms relaying request");
      return;
    }
  }else if(packetSize == 64){
    Serial.println("possibly a public key");
    if(comsat->ongoing_stage[3] == 1){
      Serial.println("comsat is awaiting a key");
      memcpy(foreign,packet,packetSize);
      comsat->comm_pair(1);
    }

    if(memcmp(foreign,packet,packetSize) == 0){
      Serial.println("same key currently in use");
      for(int i=0;i < 2;i++){
        LoRa.beginPacket();
        LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
        LoRa.print("keyrecv");
        LoRa.endPacket();
        delay(100);
      }
    }else{
      Serial.println("new user rejected");
      for(int i=0;i < 2;i++){
        LoRa.beginPacket();
        LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
        LoRa.print("keyrjct");
        LoRa.endPacket();
        delay(100);
      }
    }
    
  }

  /*
  gateway:
received_join_network  => send_key      -> await_ed_key
possible_key_received  => shared_secret -> send_ack+ed_id(cyphered)
ed_encription_ready    => store_aes_pointer+pos+id_ed
----------------------------
cifrar:
  2 byte  = pos_na_fila
  16 byte = msg
	2 byte  = id
	5 byte  = time
	9 byte  = info

checagem rapida:
	uint16_t pos_na_fila, id_ed;
	map < pos_na_fila , id_ed >
  */

      //String rssi = "RSSI " + String(LoRa.packetRssi(), DEC);
      /* Serial.println("");
      for(int i=0;i<128;i++){
        if(packet[i]=='\0') break;
        Serial.print(packet[i]);
        Serial.print(" ");
      }
      Serial.println(""); */

      /*
      Pipeline geral:
      comm_test -> sync -> pair -> normal-procedures
      comm_test:
          end-device requests comm_test
          end-device awaits clearance on gateway ( could receive 'await x seconds' from gateway )
          end-device sends 5 packets
          end-device receives analysis from gateway
          end-device adopts configuration received from gateway
      sync:
          end-device requests sync configuration
          end-device awaits 1st sync msg from gateway
          end-device replies with ack immediately
          end-device awaits final clock sync message from gateway
      pair:
          end-device sends public key request
          gateway sends key
          end-device stores key
          end-device sends verification message   ( aborted - gateway does not decrypt message )


      fragc = 0 == s/ fragmentação
      alternativa: 
          mensagem dedicada pra acionar / remover fragmentação
          critério pra mensagem: poucas mensagens chegando no gateway
          critério de operação: 
            gateway envia um teste de comunicação no começo do pareamento



      Gateway não decifra nada, e só joga no MQTT (de forma segura preferencialmente)
      

                    packet spec
      .______________________________________________.
      |________header_________|_________body_________|
      |     16bit mqtt_id     | 128bit cyphered body |
      |-----------------------|----------------------|
      */
}

void ledBlink(){
  digitalWrite(25, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(300);                       // wait for a second
  digitalWrite(25, LOW); 
}

void setup(){
  pinMode(25, OUTPUT); // Sets the echoPin as an INPUT
  
    Serial.begin(115200);
    Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
    //heltecDisplayInit();
    delay(1000);
    
    mqtt_client.setServer(mqtt_server, 1883);
    mqtt_client.setCallback(mqtt_subscriber_callback);

    setup_wifi(false); // ntp fetch

    ntp_initialize();

    //disconnect_wifi(); // shuts down mqtt
    //LoRa.receive();  // locks LoRa to receiver-only

    ecdh_setup(curve);

    comsat = new COMSAT(&mqtt_client);
    delay(100);
    LoRa.setSpreadingFactor(10);
}

unsigned long mqtt_pooling=0;

void loop(){
    if(millis() + 1000 > mqtt_pooling){
      mqtt_pooling=millis();
      mqtt_client.loop();
    }
    // reconnect to broker
    if (!mqtt_client.connected()){mqtt_reconnect();}
    //receive LoRa packet
    cbk(LoRa.parsePacket());
    delay(10);
}




#ifdef ASDASD
uint8_t encHeaderSize = (uint8_t) packet[0];
      unsigned char header[encHeaderSize];
      for(uint8_t i=0;i < encHeaderSize;i++){
        header[i] = packet[i+1];
      }
      uint8_t decHeader=base64_dec_len(header,encHeaderSize);
      uint8_t decodedHeader[decHeader];
      base64_decode(header,encHeaderSize,decodedHeader);
      
      uint8_t fragc = decodedHeader[0];
      uint8_t dev_id = decodedHeader[1];
      /* char dev_id[decHeader-1];
      for(int i=1;i < decHeader;i++){
        dev_id[i-1] = (char)decodedHeader[i];
      } */
      
      uint8_t msgStart = encHeaderSize+1;
      unsigned char msg[packetSize-msgStart+1];
      int i=0;
      while(msgStart < packetSize){
        msg[i]=packet[msgStart];
        msgStart+=1;
        i+=1;
      }
      msg[i]='\0';
      msgStart = encHeaderSize+1;
      //uint8_t msgLen = packet[encHeaderSize+1];

      Serial.print("Encoded header size:");
      Serial.println(encHeaderSize);
      Serial.print("Decoded header size:");
      Serial.println(decHeader);
      Serial.print("Message length:");
      Serial.println(packetSize-msgStart);
      Serial.print("encoded header: '");
      for(uint8_t i=0;i < encHeaderSize;i++) Serial.print((uint8_t)header[i]);
      Serial.println("'");
      
      Serial.print("decoded header: '");
      for(uint8_t i=1;i < decHeader;i++) Serial.print((uint8_t)decodedHeader[i]);
      Serial.println("'");


      DynamicJsonDocument doc(256);
      
      Serial.print("Sending: '");
        Serial.print(dev_id);
        //for(int i=0;i < decHeader-1;i++) Serial.print((char)dev_id[i]);
      Serial.println("'");
      
      //https://www.google.com/search?q=c+unsigned+char+to+hex

      Serial.print("Frag: ");
      Serial.println(String(fragc));
      Serial.print("MSG: '");
      for(int i=0;i < packetSize-msgStart;i++) Serial.print((char)msg[i]);
      Serial.println("'");
      Serial.println("");

      doc[0] = dev_id;
      doc[1] = fragc;
      doc[2] = msg;

      
      char buffer[256];
      size_t n = serializeJson(doc, buffer);
      mqtt_client.loop();
      mqtt_client.publish("hyqnap5637/data", buffer, n);
#endif