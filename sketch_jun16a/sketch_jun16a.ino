#include <bits/stdc++.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

//Lembrando que o Wemos possui somente uma porta analogica como INPUT, a A0
int umidadeAnalogica = A0; //Atribui o pino A0 a variável umidade - leitura analógica do sensor
int umidadeDigital = D7; //Atribui o pino 7 a variável umidadeDigital - leitura digital do sensor
int LedVermelho = D5; //Atribui o pino 7 a variável LedVermelho
int LedVerde = D6; //Atribui o pino 6 a variável LedVerde
int bombaAgua = D4; //Atribui o pino 4 ao rele de controle da bomba de agua

int valorumidade = 0; //Declaração da variável que armazenará o valor da umidade lida - saída analogica
int valorumidadeDigital = 0; //Declaração da variável que armazenara a saída digital do sensor de umidade do solo

bool sistemaLigado = false;

String status = "Sem Solo";

// Funções
boolean connectWifi();
void inicializarD1();

// Rede
const char* ssid = "Anisio-Vivo";
const char* password = "fernandesmontes";

boolean wifiConnected = false;
ESP8266WebServer server(80);
std::map<String,int> portaValorD1;


//Por se tratar de um rele de controle, ele liga a bomba sem energia no pino de controle, e desliga chegando energia
void ligarReleBomba(){
  digitalWrite(LedVermelho, HIGH);
  digitalWrite(LedVerde, LOW);
  digitalWrite(bombaAgua,0);
}

void desligarReleBomba(){
  digitalWrite(LedVermelho, LOW);
  digitalWrite(LedVerde, HIGH);
  digitalWrite(bombaAgua,1);
}

void setup()
{
  // Inicializando Sistema
  Serial.begin(115200);
  pinMode(umidadeAnalogica, INPUT); //Define umidadeAnalogica como entrada
  pinMode(umidadeDigital, INPUT); //Define umidadeDigital como entrada
  pinMode(bombaAgua, OUTPUT); //Define LedVermelho como saída
  pinMode(LedVermelho, OUTPUT); //Define LedVermelho como saída
  pinMode(LedVerde, OUTPUT); //Define LedVerde como saída
  wifiConnected = connectWifi();
  inicializarD1();
  
  if(wifiConnected){
    sistemaLigado = true;
    //Configurando EndPoints
    server.on("/", HTTP_GET, [](){
      server.send(200, "text/plain", "Servidor está ligado.");
    });

    server.on("/valor-humidade-porcentagem", HTTP_GET, [](){
      String porcentagem = String(valorumidade);
      server.send(200, "application/json", "{\"valor\": " + porcentagem + "%}");
    });

    server.on("/estado-solo", HTTP_GET, [](){
      server.send(200, "application/json", "{\"estado\": " + status + "}");
    });

    server.on("/desligar-sistema", HTTP_GET, [](){
      sistemaLigado = false;
      desligarReleBomba();
      server.send(200, "application/json", "{\"sistemaLigado\": false}");
    });

    server.on("/ligar-sistema", HTTP_GET, [](){
      sistemaLigado = true;
      server.send(200, "application/json", "{\"sistemaLigado\": true}");
    });

    server.onNotFound([](){
      server.send(404, "text/plain", "Not found");
    });
  }else{
    while (1){
      Serial.println("Cannot connect to WiFi. Please check data and reset the ESP.");
      delay(2500);
    }
  }
}
 
void loop()
{
  //Sistema foi desligado para alguma manutenção (repor àgua)
  if(!sistemaLigado){
    return;
  }
  valorumidade = analogRead(umidadeAnalogica); //Realiza a leitura analógica do sensor e armazena em valorumidade
  valorumidade = map(valorumidade, 1023, 315, 0, 100); //Transforma os valores analógicos em uma escala de 0 a 100
  Serial.print("Umidade encontrada: "); //Imprime mensagem
  Serial.print(valorumidade); //Imprime no monitor serial o valor de umidade em porcentagem
  Serial.println(" % " );
  valorumidadeDigital = digitalRead(umidadeDigital); //Realiza a leitura digital do sensor e armazena em valorumidadeDigital
  if (valorumidadeDigital == 0) { //Se esse valor for igual a 0, será mostrado no monitor serial que o solo está úmido e o led verde se acende
    status = "Solo úmido";
    Serial.println(status);
    desligarReleBomba();
  }
  else { // se esse valor for igual a 1, será mostrado no monitor serial que o solo está seco e o led vermelho se acende
    status = "Solo seco";
    Serial.println(status);
    ligarReleBomba();
  }
  delay(20);
}

//So para ajudar em algum futuro do projeto, as portas no Wemos funcionam diferente, a porta digital 5 no Wemos eh D5, e assim por diante, então isso aqui é para ajudar num controle futuro, pra não ter que colocar D
void inicializarD1(){
  portaValorD1["0"] = D0;
  portaValorD1["1"] = D1;
  portaValorD1["2"] = D2;
  portaValorD1["3"] = D3;
  portaValorD1["4"] = D4;
  portaValorD1["5"] = D5;
  portaValorD1["6"] = D6;
  portaValorD1["7"] = D7;
  portaValorD1["8"] = D8;
  portaValorD1["9"] = D9;
}

boolean connectWifi(){
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false; break;
    }
    i++;
  }
  Serial.println("");
  if (state){
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("Connection failed.");
  }
  delay(100);
  return state;
}