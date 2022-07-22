#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif
#include <IRremote.hpp>
#include "SinricPro.h"
#include "SinricProSwitch.h"

//Definir nome e senha da rede WiFi e chaves de autenticação da platadorma Sinric Pro.
#define NOME_WIFI           "xxxxxxxx"    
#define SENHA_WIFI          "xxxxxxxx"
#define CHAVE_APP           "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"      
#define SENHA_APP           "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"   

//Definir os IDs dos dispositovos criados na plataforma Sinric Pro.
#define DEVICE_ID_1    "xxxxxxxxxxxxxxxxxxxxxxxx"
#define DEVICE_ID_2    "xxxxxxxxxxxxxxxxxxxxxxxx"
#define DEVICE_ID_3    "xxxxxxxxxxxxxxxxxxxxxxxx"

//Definir os pinos de saídas e entrada da placa.
#define RELAY_PIN_1      18       //Relé Iluminação 1.
#define RELAY_PIN_2      19       //Relé Iluminação 2.
#define RELAY_PIN_3      23       //Relé Ventilador.
#define BUTTON_PIN_1     13       //Botão Iluminação 1.
#define BUTTON_PIN_2     12       //Botão Iluminação 2.
#define BUTTON_PIN_3     14       //Botão Ventilador.
#define WIFI_LED         17       //Led indicador de rede.
#define RECV_IR          27       //Receptor Infravermelho.

#define BAUD_RATE  9600          

//Código Hexadecimal teclas do controle remoto
#define IR_BUTTON_1 0xEA15BD00 //Relé Iluminação 1.
#define IR_BUTTON_2 0xE916BD00 //Relé Iluminação 2.
#define IR_BUTTON_3 0xE817BD00 //Relé Ventilador.
#define IR_OFF_ALL 0xFE01BD00 //Desliga todos.

//Estado de energia das saídas
bool POWER_STATE_1 = false;
bool POWER_STATE_2 = false;
bool POWER_STATE_3 = false;

//Controle através do aplicativo.
bool onPowerState1(const String &deviceId, bool &state) {
  Serial.printf("Device %s turned %s (via SinricPro) \r\n", deviceId.c_str(), state?"on":"off");
  POWER_STATE_1 = state;
  digitalWrite(RELAY_PIN_1, POWER_STATE_1?LOW:HIGH);
  return true;
}
bool onPowerState2(const String &deviceId, bool &state) {
  Serial.printf("Device %s turned %s (via SinricPro) \r\n", deviceId.c_str(), state?"on":"off");
  POWER_STATE_2 = state;
  digitalWrite(RELAY_PIN_2, POWER_STATE_2?LOW:HIGH);
  return true;
}
bool onPowerState3(const String &deviceId, bool &state) {
  Serial.printf("Device %s turned %s (via SinricPro) \r\n", deviceId.c_str(), state?"on":"off");
  POWER_STATE_3 = state;
  digitalWrite(RELAY_PIN_3, POWER_STATE_3?LOW:HIGH);
  return true;
}


//Função para controle através de botões físicos e controle remoto IR
void ligaDesligaRele(int rele){
    switch (rele)
    {
    case 1:
    if(POWER_STATE_1 == false){
      digitalWrite(RELAY_PIN_1, LOW);
      POWER_STATE_1 = !POWER_STATE_1;
      SinricProSwitch& mySwitch1 = SinricPro[DEVICE_ID_1];
      mySwitch1.sendPowerStateEvent(POWER_STATE_1);     
      Serial.println("Relé 1 Ligado.");
    }
    else{
      digitalWrite(RELAY_PIN_1, HIGH);
      POWER_STATE_1 = !POWER_STATE_1;
      SinricProSwitch& mySwitch1 = SinricPro[DEVICE_ID_1];
      mySwitch1.sendPowerStateEvent(POWER_STATE_1);
      Serial.println("Relé 1 Desligado.");
    }
    break;

    case 2:
    if(POWER_STATE_2 == false){
      digitalWrite(RELAY_PIN_2, LOW);
      POWER_STATE_2 = !POWER_STATE_2;
      SinricProSwitch& mySwitch2 = SinricPro[DEVICE_ID_2];
      mySwitch2.sendPowerStateEvent(POWER_STATE_2);     
      Serial.println("Relé 2 Ligado.");
    }
    else{
      digitalWrite(RELAY_PIN_2, HIGH);
      POWER_STATE_2 = !POWER_STATE_2;
      SinricProSwitch& mySwitch2 = SinricPro[DEVICE_ID_2];
      mySwitch2.sendPowerStateEvent(POWER_STATE_2);
      Serial.println("Relé 2 Desligado.");
    }
    break;

    case 3:
    if(POWER_STATE_3 == false){
      digitalWrite(RELAY_PIN_3, LOW);
      POWER_STATE_3 = !POWER_STATE_3;
      SinricProSwitch& mySwitch3 = SinricPro[DEVICE_ID_3];
      mySwitch3.sendPowerStateEvent(POWER_STATE_3);     
      Serial.println("Relé 3 Ligado.");
    }
    else{
      digitalWrite(RELAY_PIN_3, HIGH);
      POWER_STATE_3 = !POWER_STATE_3;
      SinricProSwitch& mySwitch3 = SinricPro[DEVICE_ID_3];
      mySwitch3.sendPowerStateEvent(POWER_STATE_3);
      Serial.println("Relé 3 Desligado.");
    }
    break;
    default: break;
    }
}

//Função desligar todos, desliga todos os equipamentos ao precionar o botão off-all
void desligarTodos(){
  POWER_STATE_1 = false; digitalWrite(RELAY_PIN_1, HIGH); SinricProSwitch& mySwitch1 = SinricPro[DEVICE_ID_1]; mySwitch1.sendPowerStateEvent(POWER_STATE_1); delay(100);
  POWER_STATE_2 = false; digitalWrite(RELAY_PIN_2, HIGH); SinricProSwitch& mySwitch2 = SinricPro[DEVICE_ID_2]; mySwitch2.sendPowerStateEvent(POWER_STATE_2); delay(100);
  POWER_STATE_3 = false; digitalWrite(RELAY_PIN_3, HIGH); SinricProSwitch& mySwitch3 = SinricPro[DEVICE_ID_3]; mySwitch3.sendPowerStateEvent(POWER_STATE_3); delay(100);
  Serial.println("Todos os relés Desligados.");
}

//Função para controle manual através de botões físicos
void controleManual(){
    if(digitalRead(BUTTON_PIN_1) == LOW){
        delay(200);
        ligaDesligaRele(1);
    }
    else if(digitalRead(BUTTON_PIN_2) == LOW){
        delay(200);
        ligaDesligaRele(2);
    }
    else if(digitalRead(BUTTON_PIN_3) == LOW){
        delay(200);
        ligaDesligaRele(3);
    }
}

//Função para controle através do controle remoto infravermelho.
void controleRemoto(){
  if (IrReceiver.decode()) {
      switch(IrReceiver.decodedIRData.decodedRawData){
        case IR_BUTTON_1: ligaDesligaRele(1);  break;
        case IR_BUTTON_2: ligaDesligaRele(2);  break;
        case IR_BUTTON_3: ligaDesligaRele(3);  break;
        case IR_OFF_ALL:  desligarTodos();     break;

        default: break;
        }   
        Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);    
        IrReceiver.resume();
  } 
}

//Função de configuração para conexão WiFi
void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Conectando");
  WiFi.begin(NOME_WIFI, SENHA_WIFI);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }  
  Serial.printf("Conectado!\r\n[WiFi]: Endereço IP é %s\r\n", WiFi.localIP().toString().c_str());
}

//Função para configuração do SinricPro
void setupSinricPro() {
 //Adiciona o dispositivo ao SinricPro
  SinricProSwitch& mySwitch1 = SinricPro[DEVICE_ID_1];
  mySwitch1.onPowerState(onPowerState1);

  SinricProSwitch& mySwitch2 = SinricPro[DEVICE_ID_2];
  mySwitch2.onPowerState(onPowerState2);

  SinricProSwitch& mySwitch3 = SinricPro[DEVICE_ID_3];
  mySwitch3.onPowerState(onPowerState3);

  //Configurar SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(CHAVE_APP, SENHA_APP);
}

//Função de configurações.
void setup() {
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);

  pinMode(RELAY_PIN_1, OUTPUT); 
  pinMode(RELAY_PIN_2, OUTPUT); 
  pinMode(RELAY_PIN_3, OUTPUT); 

  digitalWrite(RELAY_PIN_1, !POWER_STATE_1);
  digitalWrite(RELAY_PIN_2, !POWER_STATE_2);
  digitalWrite(RELAY_PIN_3, !POWER_STATE_3);
  
  digitalWrite(WIFI_LED, LOW);

  IrReceiver.begin(RECV_IR, ENABLE_LED_FEEDBACK);
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  setupWiFi();
  setupSinricPro();
}

void loop() {
  controleManual();
  controleRemoto();
  SinricPro.handle();
}
