#include "Base64.h"

const uint8_t base64_chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
uint8_t listFind(uint8_t c){
  for(uint8_t i=0;i<64;i++){
    if(base64_chars[i]==c) return i;
  }
  return (uint8_t)(-1);
}

bool isInRange(uint8_t c){
  if(c >= 48 && c<= 57) return true;
  if(c >= 65 && c<= 90) return true;
  if(c >= 97 && c<= 122) return true;
  return false;
}


bool is_base64(unsigned char c) {

  return (isInRange(c) || (c == '+') || (c == '/'));
}

void base64_encode(const unsigned char* buf,uint8_t bufLen,unsigned char* out) {
  uint8_t outp=0;
  int i = 0;
  int j = 0;
  uint8_t char_array_3[3];
  uint8_t char_array_4[4];

  while (bufLen--) {
    char_array_3[i++] = *(buf++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++){
        out[outp] = base64_chars[char_array_4[i]];
        outp++;
      }
      i = 0;
    }
  }

  if (i){
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++){
      out[outp]=base64_chars[char_array_4[j]];
      outp++;
    }

    while((i++ < 3)) {out[outp]='=';outp++;}
  }
}

void base64_decode(const unsigned char* buf,uint8_t bufLen,unsigned char* out){
  uint8_t outp=0;
  int i = 0;
  int j = 0;
  int in_ = 0;
  uint8_t char_array_4[4];
  uint8_t char_array_3[3];

  while (bufLen-- && ( buf[in_] != '=') && is_base64(buf[in_])) {
    char_array_4[i++] = buf[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = listFind(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++){
          out[outp]=char_array_3[i];
          outp++;
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = listFind(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++){
      out[outp] = char_array_3[j];
      outp++;
    }
  }
}

uint8_t base64_enc_len(uint8_t plainLen){
  uint8_t n = plainLen;
  return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

uint8_t base64_dec_len(const unsigned char * input, uint8_t inputLen){
  int i = 0;
  int numEq = 0;
  for (i = inputLen - 1; input[i] == '='; i--) {
    numEq++;
  }

  return ((6 * inputLen) / 8) - numEq;
}