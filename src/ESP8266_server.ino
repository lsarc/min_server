/**
   ESPNOW - Basic communication - Master
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Client ESP32
   Description: This sketch consists of the code for the Master module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Master >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Client ESP32 (we have added a prefix of `client` to the SSID of client for an easy setup)
   Step 3 : Once found, add Client as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Client

   Flow: Client
   Step 1 : ESPNow Init on Client
   Step 2 : Update the SSID of Client with a prefix of `client`
   Step 3 : Set Client in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Client have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Client.
         Any devices can act as master or salve.
*/

#include <espnow.h>
#include <ESP8266WiFi.h> // only for esp_wifi_set_channel()
#include <Keypad.h>

byte pinosLinhas[4]  = {D0, D1, D2, D3};

byte pinosColunas[4] = {D4, D5, D6, D7};

char teclas[4][4] = {{'1','2','3','A'},
                     {'4','5','6','B'},
                     {'7','8','9','C'},
                     {'*','0','#','D'}};

Keypad teclado1 = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, 4, 4);  

char tecla;
int senha_digitada, cent, dez, uni, ok;

// Initializes the variable "senha"
int senha, senha_nova;


// Global copy of client
uint8_t peerAddress[] = {0x3E, 0x71, 0xBF, 0x32, 0x71, 0x8C};
#define CHANNEL 1
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ERR_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

uint8_t data[2];
// send data
void sendData() {
  Serial.println("Sending: "); 
  Serial.print("Byte 1: "); Serial.println(data[1]);
  Serial.print("Byte 0: "); Serial.println(data[0]);
  int result = esp_now_send(peerAddress, (uint8_t *)data, sizeof(data));
  Serial.println("Send Status: ");
  if (result == 0) {
    Serial.println("Success");
  }else {
    Serial.println("Failure");
  }
}

// callback when data is sent from Master to Client
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(sendStatus);
}

void setup() {
  // Initializes the variable "senha"
  senha = -1;
  senha_nova = 0;
  
  Serial.begin(115200);
  analogWriteResolution(12);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  Serial.println("ESPNow/Basic/Master Example");
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("STA CHANNEL "); Serial.println(WiFi.channel());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(peerAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

int atoi(char tecla) {
  int num = tecla - '0';
  return num;
}

int isNumber(char tecla) {
  if (atoi(tecla) >= 0 && atoi(tecla) <= 9)
    return 1;
  return 0;
}

void rotina_teclado() {

tecla = 'X';
ok = 0;
  
while (ok != 1) {
  Serial.print("Nova senha: ");
  while (!isNumber(tecla)) {
    tecla = teclado1.getKey();
    yield();
    ESP.wdtFeed();
  }
  dez = atoi(tecla);
  Serial.print(tecla);
  tecla = 'X';

  while (!isNumber(tecla)) {
    tecla = teclado1.getKey();
    yield();
    ESP.wdtFeed();
  }
  uni = atoi(tecla);
  Serial.println(tecla);
  tecla = 'X';
  senha_nova = 10 * dez + uni;
  Serial.println("Aceita: A, Cancela: C");
  while (1) {
    tecla = teclado1.getKey();
    yield();
    ESP.wdtFeed();
    if (tecla == 'C') {
      ok = 0;
      break;
    }
    else if (tecla == 'A'){
      ok = 1;
      break;
    }
  }
}
  
}
  

int teste = 1024;

void loop() {
  // In the loop we scan for client
  // pair success or already paired
  // Send data to device
  rotina_teclado();
  data[0] = (uint8_t)dez;
  data[1] = (uint8_t)uni;
  sendData();

  // wait for 1 second to run the logic again
  delay(1000);
}
