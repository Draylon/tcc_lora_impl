<div align="center" id="top"> 
  <img src="./.github/app.gif" alt="Mon" />

  &#xa0;

  <!-- <a href="https://mon.netlify.app">Demo</a> -->
</div>

<h1 align="center">TCC - LoRa</h1>

<p align="center">
  <img alt="Github top language" src="https://img.shields.io/github/languages/top/draylon/tcc_lora_impl?color=56BEB8">

  <img alt="Github language count" src="https://img.shields.io/github/languages/count/draylon/tcc_lora_impl?color=56BEB8">

  <img alt="Repository size" src="https://img.shields.io/github/repo-size/draylon/tcc_lora_impl?color=56BEB8">

  <img alt="License" src="https://img.shields.io/github/license/draylon/tcc_lora_impl?color=56BEB8">

  <!-- <img alt="Github issues" src="https://img.shields.io/github/issues/draylon/tcc_lora_impl?color=56BEB8" /> -->

  <!-- <img alt="Github forks" src="https://img.shields.io/github/forks/draylon/tcc_lora_impl?color=56BEB8" /> -->

  <!-- <img alt="Github stars" src="https://img.shields.io/github/stars/draylon/tcc_lora_impl?color=56BEB8" /> -->
</p>

<!-- Status -->

<!-- <h4 align="center"> 
	🚧  Mon 🚀 Under construction...  🚧
</h4> 

<hr> -->

<p align="center">
  <a href="https://github.com/Draylon/tcc_impl/?tab=readme-ov-file#trabalho-de-conclus%C3%A3o-de-curso">Voltar</a> &#xa0; | &#xa0;
  <a href="#sparkles-features">Features</a> &#xa0; | &#xa0;
  <a href="#rocket-technologies">Technologies</a> &#xa0; | &#xa0;
  <a href="#white_check_mark-requirements">Requirements</a> &#xa0; | &#xa0;
  <a href="#checkered_flag-starting">Starting</a> &#xa0; | &#xa0;
  <a href="#memo-license">License</a> &#xa0; | &#xa0;
  <a href="https://github.com/draylon" target="_blank">Author</a>
</p>

<br>

## :dart: About ##

Implementação de LoRa em [Arduino](https://arduino.cc) como parte do [trabalho de conclusão de curso](https://github.com/Draylon/tcc_impl).

A implementação é dividida em dispositivos receptores (Gateway / receiver) e dispositivos transmissores (End-Device / sender).



## :sparkles: Features ##

:heavy_check_mark: LoRa Half-Duplex;\
:heavy_check_mark: Optimized extensive procedures;\
:heavy_check_mark: Communication safety concerns;

## :rocket: Technologies ##

The following tools were used in this project:

- [Arduino](https://arduino.cc/)
- [ESP32 Library](https://github.com/espressif/arduino-esp32)
- [Heltec Library](https://github.com/HelTecAutomation/Heltec_ESP32)

## :white_check_mark: Requirements ##

Before starting :checkered_flag:, you need to have [Git](https://git-scm.com) installed.

## :checkered_flag: Starting ##

```bash
# Clone this project
$ git clone https://github.com/draylon/tcc_lora_impl

# Access
$ cd tcc_lora_impl

# Open Arduino
$ cd final_sender
$ arduino-cli final_sender.ino
```

### Install dependencies:

Adicione a seguinte [Board Manager](https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/0.0.5/package_heltec_esp32_index.json) ao ambiente Arduino. (Veja em [Third-Party platforms](https://support.arduino.cc/hc/en-us/articles/360016466340-Add-third-party-platforms-to-the-Boards-Manager-in-Arduino-IDE))

Adicione as seguintes bibliotecas usando [Library Manager](https://support.arduino.cc/hc/en-us/articles/5145457742236-Add-libraries-to-Arduino-IDE):
(Baixe os releases dos repositórios e adicione ao [Library Manager](https://support.arduino.cc/hc/en-us/articles/5145457742236-Add-libraries-to-Arduino-IDE))
- [micro-ecc](https://github.com/kmackay/micro-ecc)
- [PubSubClient](https://github.com/knolleary/pubsubclient/)
- [Heltec Esp32 Dev-Boards](https://github.com/HelTecAutomation/Heltec_ESP32)



## :memo: License ##

This project is under license from MIT. For more details, see the [LICENSE](LICENSE.md) file.


Made with :heart: by <a href="https://github.com/draylon" target="_blank">Draylon</a>

&#xa0;

<a href="#top">Back to top</a>
