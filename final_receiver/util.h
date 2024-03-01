#ifndef RANDOM_UTILS
#define RANDOM_UTILS

void printbin(uint8_t* b, uint8_t len) {
  for (int i=0; i<len; i++) { 
    if (b[i]<0x10) Serial.print("0");
    Serial.print(b[i],HEX); 
  }
  Serial.print(" len=");
  Serial.print(len);
}

#endif