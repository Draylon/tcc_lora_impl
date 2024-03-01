#ifndef HELTEC_DISPLAYFUNCTIONS
#define HELTEC_DISPLAYFUNCTIONS
#include <heltec.h>
#include "images.h"

static bool heltecDisplayEnabled=false;

void heltecDisplayInit(){
    heltecDisplayEnabled=true;
    Heltec.display->init();
    //Heltec.display->flipScreenVertically();  
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
    Heltec.display->drawString(0, 10, "Wait for incoming data...");
    Heltec.display->display();
}

void heltecDisplayLogo(){
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
}

void heltecDisplayClear(){
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  //Heltec.display->drawString(0 , 15 , "Received "+ packSize + " bytes");
  //Heltec.display->drawStringMaxWidth(0 , 26 , 128, packet);
}

void heltecDisplayOnScreen(){
  Heltec.display->display();
}

#endif