#ifndef ECDH_LIB
#define ECDH_LIB
#include <heltec.h>
#include <uECC.h>

#include <Crypto.h>
#include <AES.h>

#include <map>
#include <vector>

#define prv_size 32
#define pub_size 64
#define sec_size 32

#define encr_size (pub_size/4)

static int RNG(uint8_t *dest, unsigned size){
  esp_fill_random(dest,size);
}


std::map<uint16_t,uint8_t> queue_mapping;
std::vector<AESSmall256*> aes_store;

AESSmall256* new_aes_obj(uint8_t n){
  return aes_store[n];
}
uint8_t create_aes(){
  aes_store.push_back(new AESSmall256());
  return aes_store.size()-1;
}
void delete_aes(uint8_t n){
  std::vector<AESSmall256*>::iterator iter = aes_store.begin();
  while(n>0){
    n--;
    iter++;
  }
  aes_store.erase(iter);
}

const struct uECC_Curve_t * curve = uECC_secp256r1();

uint8_t privkey[prv_size];
uint8_t publkey[pub_size];
uint8_t foreign[pub_size];
//uint8_t secret[sec_size];
//uint8_t shared_secret[sec_size];//= {108,191,119,197,190,140,119,052,132,001,241,112,158,109,166,230,19,85,076,215,191,189,109,101,28,247,238,230,218,228,195,188};

//const int strsize=8;
//const uint8_t string1[strsize] = {115,116,111,110,107,115,95,'\0'};
//const int strsize=16;
//const uint8_t string1[strsize] = {98,105,103,115,116,111,110,107,115,49,50,51,52,53,54,'\0'};
//const uint8_t string1[strsize] = {104,101,108,108,111,119,111,114,108,100,49,50,51,52,53,'\0'};


void initialize_aes(AESSmall256 *aes256,uint8_t *secret){
  /* if(memcmp(secret,old_secret,sec_size)==0){
    Serial.println("Secrets shared are equal");
  }else{
    Serial.println("Secrets shared are different");
  } */

  aes256->setKey(secret, aes256->keySize());
  return;
  /* if(!verbose) return;
  Serial.print("key size: ");
  Serial.println(aes256->keySize());
  Serial.print("key: ");
  for(int i=0;i < sec_size;i++){
    Serial.printf("%c",secret[i]);
  }
  Serial.println(""); */
}

void ecdh_setup(const uECC_Curve_t *curve){
  uECC_set_rng(&RNG);
  uECC_make_key(publkey, privkey, curve);
  Serial.println("made keys");
}

int ecdh_init(const uECC_Curve_t *curve,uint8_t *secret){
  int r = uECC_shared_secret(foreign, privkey, secret, curve);
  return r;
  /* if (!r) {
    Serial.print("shared_secret() failed (1)\n");
    return;
  } */
}

void key_iterator( void(*addr)(uint8_t) ){
    for(int i=0;i<pub_size;i++) (*addr)(publkey[i]);
}

int encr_msg_size(uint8_t input_size){
  return ((int)(input_size / encr_size)+1)*encr_size;
}

void encrypt_msg(AESSmall256 *aes256,uint8_t* input,uint8_t* output){
  aes256->encryptBlock(output,input);
}
void decrypt_msg(AESSmall256 *aes256,uint8_t* input,uint8_t* output){
  aes256->decryptBlock(output,input);
}

#endif