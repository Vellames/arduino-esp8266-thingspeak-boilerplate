/**
	@author Cassiano Vellames
*/

// Biblioteca para comunicação serial
#include "SoftwareSerial.h"

// NOME DA SUA REDE WIFI
#define WIRELESS_NAME "SUA-REDE-WIFI"

// SENHA DA SUA REDE WIFI
#define WIRELESS_PASSWORD "SUA-SENHA-WIFI"

// Tempo para envio dos dados (em MS)
#define DELAY_SEND_DATA 20000

//  Estamos definindo o objeto que representará o ESP8266 ESP-01. É necessário informar quais portas serão respostaveis pelo RX e pelo TX
SoftwareSerial ESP_Serial(10, 11); // RX, TX

// Aqui nós configuramos as informações do ThingSpeak:
// Primeiro informamos a chave do canal
const String API_KEY = "CHAVE-DO-SEU-CANAL";

// Em seguida informamos qual será o host da nossa requisição (Não precisa mudar essa informação)
const String HOST = "api.thingspeak.com";

// Informamos tambem por qual porta iremos realizar a requisição. No caso de uma requisição HTTP, a porta padrão é a 80
const short PORT = 80;

// Tambem informamos o nome do campo que iremos enviar
const String FIELD_NAME = "field1";

String response = "";

// Variavel que verifica a ultima vez que a leitura do serial monitor foi realizada 
unsigned long lastSerialPrintCheck = 0;

// Variavel que verifica a ultima vez que algum dado foi enviado para o servidor
unsigned long lastSendData = 0;

// Aqui ficam declaradas algumas funções auxiliares que usaremos no código
void sendCommand(String cmd);
void readResponse(unsigned int timeout);
void sendData();
int readSensor();

void setup() {
  // Inicializa o Serial monitor e o ESP
  Serial.begin(9600);
  ESP_Serial.begin(9600);
  
  Serial.println("Chamando atencao do modulo com AT...");
  sendCommand("AT");
  readResponse(1000);

  // https://github.com/espressif/ESP8266_AT/wiki/CWMODE
  Serial.println("Mudando o modo do ESP para estação...");
  sendCommand("AT+CWMODE=1");
  readResponse(1000);

  Serial.println("Conectando a rede...");
  String CWJAP = "\"AT+CWJAP=\"";
  CWJAP += WIRELESS_NAME;
  CWJAP += "\",\"";
  CWJAP += WIRELESS_PASSWORD;
  CWJAP += "\"";
  sendCommand(CWJAP);
  readResponse(10000);

  delay(2000); //espera de seguranca

  // Se não existir OK na resposta. Envia uma mensagem de erro no serial
  if (response.indexOf("OK") == -1) {
    Serial.println("Atencao: Nao foi possivel conectar a rede WiFi.");
    Serial.println("Verifique se o nome da rede e senha foram preenchidos corretamente no codigo e tente novamente.");
  }
}

void loop() {
	// Verifica se o ultimo envio já demorou o tempo estipulado no começo do código
  if (millis() - lastSendData >= DELAY_SEND_DATA) {
  	// Atualiza o ultimo momento do envio
    lastSendData = millis();

    // String com as informações da requisição
    String getData = "GET /update?api_key="+ API_KEY +"&"+ FIELD_NAME +"="+String(readSensor());

    // Habilita multiplas conexões no ESP
    // https://github.com/espressif/ESP8266_AT/wiki/CIPMUX
    sendCommand("AT+CIPMUX=1");
    readResponse(1000);
    
    // Estabelece uma conexão TCP com um host
    // https://github.com/espressif/ESP8266_AT/wiki/CIPSTART
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT);    
    readResponse(1000); 
    
    // Seta o tamanho da informação que será enviada
    // https://github.com/espressif/ESP8266_AT/wiki/CIPSEND
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4));
    readResponse(1500);
    
    // Envia os dados para o ESP
    ESP_Serial.println(getData);

    // Aguarda dois segundos para fechar a conexão
    if (millis() - lastSerialPrintCheck >= 2000) {
    	// Atualiza o ultimo momento de verificação do envio
      lastSerialPrintCheck = millis();

      // Fecha a conexão com o host
      // https://github.com/espressif/ESP8266_AT/wiki/CIPCLOSE
      sendCommand("AT+CIPCLOSE=0");
      readResponse(1500); 
    }
  }
}

/*
	Simula a leitura de um sensor
*/
int readSensor() {
	return random(1, 100);
}

/*
	Realiza a leitura de um comando enviado para o ESP
*/
void readResponse(unsigned int timeout) {
  unsigned long timeIn = millis(); //momento que entramos nessa funcao é salvo
  response = "";
  //cada comando AT tem um tempo de resposta diferente...
  while (timeIn + timeout > millis()) {
    if (ESP_Serial.available()) {
      char c = ESP_Serial.read();
      response += c;
    }
  }
  Serial.println(response);
}

/*
	Essa função envia algum comando para o ESP8266
*/
void sendCommand(String cmd) {
  ESP_Serial.println(cmd);
}