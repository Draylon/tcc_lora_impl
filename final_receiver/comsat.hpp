#ifndef COMSATap
#define COMSATap

#include <PubSubClient.h>
#include <heltec.h>
#include <stdbool.h>
#include <string>
#include <vector>
#include <sstream>

#include "ecdh.h"

namespace customstd{
    template < typename T > std::string to_string( const T& n ){
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }

    std::string nret(const byte* b,uint8_t len){
      std::ostringstream stm;
      for(int i=0;i < len;i++) stm << b[i];
      return stm.str();
    }
    const std::vector<std::string> explode(const std::string& s, const char& c){
      std::string buff{""};
      std::vector<std::string> v;
      
      for(auto n:s){
        if(n != c) buff+=n; else
        if(n == c && buff != "") { v.push_back(buff); buff = ""; }
      }
      if(buff != "") v.push_back(buff);
      
      return v;
    }
}

/* #if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif */

class RNGPick {
  std::mt19937 rng;
public:
  RNGPick(){
    rng.seed(0);
  }
  void reset(){
    rng.seed(0);
  }
  int pick(int i){
    rng.discard(i);
    return rng();
  }
};

class COMSAT {
 public:
  PubSubClient *mqtt_client;
  uint8_t secret[32];
  COMSAT(PubSubClient *p){
    mqtt_client=p;
    for(int i=0;i < 4;i++) ongoing_stage[i]=0;
  }
  uint8_t ongoing_stage[4];
  char *lastmqtt_topic;
  byte *lastmqtt_message;
  unsigned int lastmqtt_length;


  void clock_sync(uint8_t state){
    LoRa.beginPacket();
    LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
    LoRa.print("clksync");
    LoRa.endPacket();

    /* int packetSize = LoRa.parsePacket();
    if(packetSize){
      char rcpacket[packetSize];
      for (int i = 0; i < packetSize; i++){
        rcpacket[i]=(char) LoRa.read();
      }
      if(strcmp(rcpacket,"await") == 0){
        Serial.println("Await a few seconds");
      }else{
        Serial.println("SYNC CLOCK");
      }
      //spreadfactor          antenna current factor
      //transmission power    14-20
      //transmission mode     PA_BOOST
    } */
  }
  void comm_pair(uint8_t state){
      if(state==0 && (ongoing_stage[3]==0||ongoing_stage[3]==1)){
        ongoing_stage[3]=1;
        Serial.println("took on comms -> send the public key");
        LoRa.beginPacket();
        LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
        LoRa.write(publkey,pub_size);
        LoRa.endPacket();
        LoRa.endPacket();
        Serial.println("sent public key");
        return;
      }else if(ongoing_stage[3] == 1 && state==1){
        Serial.println("key has arrived -> shared secret");
        ecdh_init(curve,secret);
        uint8_t n = create_aes();
        AESSmall256* new_aes = new_aes_obj(n);
        initialize_aes(new_aes,secret);
        Serial.print("Sending the id to mqtt:");
        char buff1[4];
        mqtt_client->publish("hyqnap5637/sensor/new_sensor",(customstd::to_string(itoa(n,buff1,DEC))+customstd::to_string("_randomID")).c_str(),14);
        Serial.println("now await mqtt response");
        
        ongoing_stage[3]=2;
        return;
      }else if(ongoing_stage[3]==2 && state==2){
        Serial.println("mqtt data received");
        std::string msg_str = customstd::nret(lastmqtt_message,lastmqtt_length);
        //Serial.println("string compound");
        std::vector<std::string> list_1 = customstd::explode(msg_str,'_');
        //Serial.println("explode");
        uint8_t device_id = std::atoi(list_1[0].c_str());
        uint16_t mqtt_id = std::atoi(list_1[1].c_str());
        //Serial.println("mapping");
        queue_mapping.insert(std::pair<uint16_t,uint8_t>(mqtt_id,device_id));
        //Serial.println("exploded");
        uint8_t tab2_e[16];
        uint8_t tab2[16];
        for(int i=0;i < 16;i++){
          tab2[i]='\0';
        }
        memcpy(tab2,list_1[1].c_str(),sizeof(uint8_t)*16);
        //Serial.println("\nmemcpy");
        encrypt_msg(aes_store[device_id],tab2,tab2_e);
        //Serial.println("encrypt");
        Serial.print("sending: ");
        //write encrypted message here, using that public aes key
        //write: random_mqtt_id
        for(int i=0;i < 5;i++){
          Serial.print(".");
          LoRa.beginPacket();
          LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
          LoRa.write(tab2_e,16);
          LoRa.endPacket();
          delay(100);
        }
        Serial.println("");
        
        ongoing_stage[3]=0;
        //mqtt_device_id_arrived
        return;
      }else if(ongoing_stage[3] == 3){
        return;
      }else{
        //??
        return;
      }
    
    LoRa.beginPacket();
    LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
    LoRa.print("pair_await");
    LoRa.endPacket();

    /*Protocolo:
        gateway:
            request_pair => send_key -> await_key
            
            possible_key => reg_key -> send_ack (ack+queue_pos) -> await ack
            possívelmente é uma chave, portanto envia o ack disso
            pro end-device através desse AES "publico"




    */


    /* int packetSize = LoRa.parsePacket();
    if(packetSize){
      char rcpacket[packetSize];
      for (int i = 0; i < packetSize; i++){
        rcpacket[i]=(char) LoRa.read();
      }
      if(strcmp(rcpacket,"await") == 0){
        Serial.println("Await a few seconds");
      }else{
        Serial.println("KEY retrieved");
      }
      //spreadfactor          antenna current factor
      //transmission power    14-20
      //transmission mode     PA_BOOST
    } */
  }
  void comm_ping(){
  

    /* unsigned int reqs = 5;
    unsigned long startq = millis();
    unsigned long mstopq = millis();
    while(true){
      int packetSize = LoRa.parsePacket();
      if(packetSize){
        char rcpacket[packetSize];
        for (int i = 0; i < packetSize; i++){
          rcpacket[i]=(char) LoRa.read();
        }
        if(strcmp(rcpacket,"await") == 0){
          Serial.println("Await a few seconds");
        }else if(strcmp(rcpacket,"auth") == 0){
          Serial.println("AUTH COMMS");
        }
        //spreadfactor          antenna current factor
        //transmission power    14-20
        //transmission mode     PA_BOOST
      }

      delay(10);
    } */
    
  }
  //void frag_comms();
  /* uint8_t calculateFragmentHeader(uint8_t dev_id,const uint8_t *dsize,uint8_t fragc,uint8_t* headerClip);
  void fragmentHeader(uint8_t* repl,uint8_t dev_id,const uint8_t *dsize,uint8_t fragc);
  void sendData( void(*sender)(uint8_t*,uint8_t),uint8_t* err_flag,uint8_t* msg,uint8_t msglen); */
};

#endif



#ifdef asdasdasd

/* void COMSAT::sendData(void (*sender)(uint8_t*,uint8_t),uint8_t* err_flag,uint8_t* msg,uint8_t msglen){
  uint8_t rnd_enc[RSA_BYTES];
  //note: rnd_enc should be random, but this is a test to check encrypted value.
  //thats why it's set to 0xAA. It already helps a bit to not initialize rnd_enc at all.
  //memset(rnd_enc, 0xAA, RSA_BYTES);
  if(err_flag!=NULL) *err_flag = rsa_encrypt_pkcs(binKey, msg, msglen, rnd_enc);
  else rsa_encrypt_pkcs(binKey, msg, msglen, rnd_enc);
  
  (*sender)(rnd_enc,RSA_BYTES);
}

uint8_t COMSAT::calculateFragmentHeader(uint8_t dev_id,const uint8_t *dsize,uint8_t fragc,uint8_t* headerClip){
  uint8_t add_data=1;
  uint8_t hsize = ((*dsize)+add_data) * sizeof(uint8_t);
  *headerClip = 3-(hsize%3);
  if(*headerClip >=3) *headerClip=0;
  return base64_enc_len(hsize);
}


void COMSAT::fragmentHeader(uint8_t* repl,uint8_t dev_id,const uint8_t *dsize,uint8_t fragc){
  // dsize | frag_c | msglen | dev_id
  uint8_t add_data=1;
  uint8_t hsize = ((*dsize)+add_data)*sizeof(uint8_t);
  unsigned char descm[hsize];
  
  descm[1]=(unsigned char)dev_id;
  descm[0]=fragc;
  /* for(uint8_t b = add_data;b < (*dsize)+add_data;b++){
      descm[b] = dev_id[b-add_data];
  } */
  /*base64_encode(descm,hsize,repl);
  
}
 */

#endif