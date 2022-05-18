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
#include "SinricPro.h"
#include "SinricProSwitch.h"

#include <map>

#define WIFI_SSID    "NOME_WIFI"    //Nome do Wifi do ambiente
#define WIFI_PASS    "SENHA_WIFI"   //Senha do Wifi do ambiente
#define APP_KEY       "77944b40-6844-4d98-a8b0-068ba143840a"      //Chave do app fornecida pela plataforma
#define APP_SECRET    "6938219c-a846-4a3c-86da-56d82239caf2-779d26e1-eae4-4b1f-a4d5-309ef6d48a8b"  //Senha do app fornecida pela plataforma

//Definir os IDs de cada Dispositivo, neste caso tem apenas três dispositivos, pois está no plano gratuito.
#define device_ID_1   "6105ebeebab19d405818e8f8" //Rele1
#define device_ID_2   "610858d3fc89cf405088d0ce" //Rele2
#define device_ID_3   "6105ecb4bab19d405818e900" //Rele3 
#define device_ID_4   "xxxxxxxxxxxxxxxxxxxxxxxx" //Null

// Definir as entradas no esp32.
//Saídas dos dispositivos
#define RelayPin1 18  //Rele1 
#define RelayPin2 19  //Rele2
#define RelayPin3 23 //Rele3
#define RelayPin4 05 //Null

//Botões físicos
#define SwitchPin1 13  //Botão físico do Rele1
#define SwitchPin2 12  // Botão físico do Rele2
#define SwitchPin3 14  //Botão fisico do Rele3
#define SwitchPin4 27  //Null

#define wifiLed   16   //Led indicador de rede, acender ao conectar a rede.

// comente a seguinte linha se você usar interruptores em vez de botões táteis.
#define TACTILE_BUTTON 1
#define BAUD_RATE   9600
#define DEBOUNCE_TIME 250

typedef struct {      // estrutura para o std :: map abaixo
  int relayPIN;
  int flipSwitchPIN;
} deviceConfig_t;


// esta é a configuração principal
// insira em seu deviceId, o PIN para Relay e o PIN para flipSwitch
// isso pode ser até N dispositivos ... dependendo de quantos pinos está disponível em seu dispositivo;)
// agora temos 4 devicesIds indo para 4 relés e 4 interruptores flip para alternar o relé manualmente

std::map<String, deviceConfig_t> devices = {
    //{deviceId, {relayPIN,  flipSwitchPIN}}
    {device_ID_1, {  RelayPin1, SwitchPin1 }},
    {device_ID_2, {  RelayPin2, SwitchPin2 }},
    {device_ID_3, {  RelayPin3, SwitchPin3 }},
    {device_ID_4, {  RelayPin4, SwitchPin4 }}     
};

typedef struct {      // struct for the std::map below
  String deviceId;
  bool lastFlipSwitchState;
  unsigned long lastFlipSwitchChange;
} flipSwitchConfig_t;

std::map<int, flipSwitchConfig_t> flipSwitches;   // este mapa é usado para mapear PINs flipSwitch para deviceId e lidar com verificações de estado de debounce e última flipSwitch
                                                  // será configurado na função "setupFlipSwitches", usando informações do mapa de dispositivos
void setupRelays() { 
  for (auto &device : devices) {           // para cada dispositivo (relé, combinação flipSwitch)
    int relayPIN = device.second.relayPIN; // obter o pino de retransmissão
    pinMode(relayPIN, OUTPUT);             // define o pino do relé para OUTPUT
    digitalWrite(relayPIN, HIGH);          //define o estado do relé para HIGH, assim ao ligar os relés permanecem desligados
  }
}

void setupFlipSwitches() {
  for (auto &device : devices)  {                     // para cada dispositivo (combinação relé / flipSwitch)
    flipSwitchConfig_t flipSwitchConfig;              // cria uma nova configuração flipSwitch

    flipSwitchConfig.deviceId = device.first;         // definir o deviceId
    flipSwitchConfig.lastFlipSwitchChange = 0;        // definir o tempo de debounce
    flipSwitchConfig.lastFlipSwitchState = true;     // defina lastFlipSwitchState como false (LOW) -

    int flipSwitchPIN = device.second.flipSwitchPIN;  // obtenha o flipSwitchPIN

    flipSwitches[flipSwitchPIN] = flipSwitchConfig;   // salve a configuração flipSwitch no mapa flipSwitches
    pinMode(flipSwitchPIN, INPUT_PULLUP);                   // defina o pino flipSwitch para INPUT
  }
}

bool onPowerState(String deviceId, bool &state)
{
  Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "on" : "off");
  int relayPIN = devices[deviceId].relayPIN; // obtenha o pino de retransmissão para o dispositivo correspondente
  digitalWrite(relayPIN, !state);             // define o novo estado do relé
  return true;
}

void handleFlipSwitches() {
  unsigned long actualMillis = millis();                                          // get actual millis
  for (auto &flipSwitch : flipSwitches) {                                         // para cada flipSwitch no mapa flipSwitches
    unsigned long lastFlipSwitchChange = flipSwitch.second.lastFlipSwitchChange;  // obtém o carimbo de data / hora quando flipSwitch foi pressionado da última vez (usado para eliminar / limitar eventos)

    if (actualMillis - lastFlipSwitchChange > DEBOUNCE_TIME) {                    // se o tempo for > tempo de debounce ...

      int flipSwitchPIN = flipSwitch.first;                                       // obtém o pino flipSwitch da configuração
      bool lastFlipSwitchState = flipSwitch.second.lastFlipSwitchState;           // obtenha o lastFlipSwitchState
      bool flipSwitchState = digitalRead(flipSwitchPIN);                          // lê o estado flipSwitch atual
      if (flipSwitchState != lastFlipSwitchState) {                               // se o flipSwitchState mudou ...
#ifdef TACTILE_BUTTON
        if (flipSwitchState) {                                                    // se o botão tátil for pressionado 
#endif      
          flipSwitch.second.lastFlipSwitchChange = actualMillis;                  // atualiza a hora do lastFlipSwitchChange
          String deviceId = flipSwitch.second.deviceId;                           // obtenha o deviceId da configuração
          int relayPIN = devices[deviceId].relayPIN;                              // obtém o relayPIN da configuração
          bool newRelayState = !digitalRead(relayPIN);                            // define o novo estado do relé
          digitalWrite(relayPIN, newRelayState);                                  // define o relé para o novo estado

          SinricProSwitch &mySwitch = SinricPro[deviceId];                        // obter o dispositivo Switch do SinricPro
          mySwitch.sendPowerStateEvent(!newRelayState);                            // envie o evento
#ifdef TACTILE_BUTTON
        }
#endif      
        flipSwitch.second.lastFlipSwitchState = flipSwitchState;                  // atualizar lastFlipSwitchState
      }
    }
  }
}

void setupWiFi()
{
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    delay(250);
  }
  //Quando conectar a internet o led acende.     
  digitalWrite(wifiLed, HIGH);
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro()
{
  for (auto &device : devices)
  {
    const char *deviceId = device.first.c_str();
    SinricProSwitch &mySwitch = SinricPro[deviceId];
    mySwitch.onPowerState(onPowerState);
  }

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void setup()
{
  Serial.begin(BAUD_RATE);

  pinMode(wifiLed, OUTPUT); //definir led indicador como saída.
 
  setupRelays();
  setupFlipSwitches();
  setupWiFi();
  setupSinricPro();
}

void loop()
{
  SinricPro.handle();
  handleFlipSwitches();
}
