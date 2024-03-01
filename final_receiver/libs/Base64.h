#ifndef _BASE64_H_
#define _BASE64_H_

#include <stdint.h>
#include <stdbool.h>

void base64_encode(const unsigned char* buf,uint8_t bufLen,unsigned char* out);
void base64_decode(const unsigned char* buf,uint8_t bufLen,unsigned char* out);
uint8_t base64_enc_len(uint8_t plainLen);
uint8_t base64_dec_len(const unsigned char * input,uint8_t inputLen);


#endif