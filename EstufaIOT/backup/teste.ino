// Importação das bibliotecas necessárias para o projeto
#include <stdlib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <SoftwareSerial.h>

/*************************** Inicio definição de variáveis ******************************/
// VARIÁVEIS PARA O SENSOR DE LUMINOSIDADE
const char LDR_PIN = A1;
int LIGHT = 0;

// VARIÁVEIS PARA O SENSOR DS18B20
const int ONE_WIRE_PIN = 6;
float TEMP_SOIL_DS18B20 = 0;

// VARIÁVEIS PARA HUMIDADE DO SOLO
const char SOIL_HUM_PIN = A0;
int SOIL_HUM = 0;

// VARIÁVEIS PARA O SENSOR DHT
float AIR_HUMIDITY = 0;
float AIR_TEMPERATURE = 0;
const int DHT_PIN = 7;
const int DHT_TYPE = DHT22;
DHT dht(DHT_PIN, DHT_TYPE);

// VARIÁVEIS PARA O ESP8266
const int ESP8266_RX = 4;
const int ESP8266_TX = 5;
const int ESP8266_RST = 3;
const int ESP8266_SPEED = 9600;
SoftwareSerial esp8266(ESP8266_RX, ESP8266_TX);
const String SSID_WIFI = "VTNC12";
const String PWD_WIFI = "familiagnich04734184513";
String STATUS_CH_WRITE_KEY = "F0TBYDRAWMNI58JX";

// VARIÁVEIS PARA OS TIMERS
long WRITE_TIMING_SECONDS = 17; // ==> TEMPO DE DELAY PARA ENVIO DOS DADOS
long START_WRITE_TIMING = 0;
long ELAPSED_WRITE_TIME = 0;

// VARIÁVEIS PARA LIGAR A LAMPADA E A BOMBA DE ÁGUA
boolean PUMP = 0;
boolean LAMP = 0;

int SPARE = 0;
boolean error;

/***************************** INICIO CONFIGURAÇÃO DS18B20 ******************************/
OneWire pino(ONE_WIRE_PIN);
DallasTemperature barramento(&pino);
DeviceAddress sensor;

/***************************** INICIO DO SETUP DO PROGRAMA ******************************/
void setup()
{
  Serial.begin(9600);

  pinMode(ESP8266_RST, OUTPUT);
  digitalWrite(ESP8266_RST, HIGH);

  findDS18B20Sensor(sensor);
  dht.begin();
  delay(2500);

  esp8266.begin(ESP8266_SPEED); // DEFINE O VELOCIDADE DO MÓDULO ESP8266 EM 9600
  EspHardwareReset();
  START_WRITE_TIMING = millis(); // COMEÇA O 'PROGRAM CLOCK'

  connectWiFi();
}

/**************************** INICIO DO LOOP DO PROGRAMA ******************************/
void loop()
{
start:
  error = 0;

  ELAPSED_WRITE_TIME = millis() - START_WRITE_TIMING;

  if (ELAPSED_WRITE_TIME > (WRITE_TIMING_SECONDS * 1000))
  {
    readDS18B20Sensor(sensor);
    readLDRSensor(LDR_PIN);
    readHumiditySensor(SOIL_HUM_PIN);
    readHumAndTempSensor();
    writeThingSpeak();
    START_WRITE_TIMING = millis();
  }

  if (error == 1)
  {
    Serial.println(" <<<< ERRO AO ENVIAR OS DADOS >>>>");
    delay(2000);
    goto start; // VOLTA PARA O INICIO SE OCORRA QUALQUER ERRO
  }
  // displaySensors();
}

/****************** INICIO SENSOR HUMIDADE E TEMPERATURA DO AR ***************************/
float readHumAndTempSensor()
{
  AIR_HUMIDITY = dht.readHumidity();
  AIR_TEMPERATURE = dht.readTemperature();
}

/************************* INICIO SENSORES HUMIDADE **************************************/
float readHumiditySensor(int humidity_sensor)
{
  SOIL_HUM = map(analogRead(humidity_sensor), 1023, 0, 0, 100);
}

/**************************** INICIO SENSORES LDR ****************************************/
float readLDRSensor(int ldr_sensor)
{
  LIGHT = map(analogRead(ldr_sensor), 1023, 0, 0, 100);
}

/*************************** INICIO SENSORES DS18B20 *************************************/
int findDS18B20Sensor(int find_sensors)
{
  Serial.println("Buscando sensores no barramento, aguarde...");
  barramento.begin();
  barramento.getAddress(find_sensors, 0);
  const int sensores = barramento.getDeviceCount();

  switch (sensores)
  {
  case 0:
    Serial.println("Não foram encontrados sensores DS18B20 neste barramento");
    break;
  case 1:
    Serial.print("Foi encontrado ");
    Serial.print(sensores);
    Serial.println(" sensor DS18B20 neste barramento.");
    break;
  default:
    Serial.print("Foram encontrados ");
    Serial.print(sensores);
    Serial.println(" sensores DS18B20 neste barramento.");
    break;
  }
}

float readDS18B20Sensor(int read_sensor)
{
  barramento.requestTemperatures();
  TEMP_SOIL_DS18B20 = barramento.getTempC(read_sensor);
}

/********************* MOSTRAR DADOS DOS SENSORES ***************************************/
void displaySensors(void)
{
  if (TEMP_SOIL_DS18B20 == -127.00)
  {
    Serial.println("Sensor DS18B20 está desconectado ou apresenta falha critica");
  }
  else
  {
    Serial.print("Temperatura do solo:          ");
    Serial.print(TEMP_SOIL_DS18B20);
    Serial.println("ºC");
  }

  Serial.print("Luz Ambiente:                 ");
  Serial.print(LIGHT);
  Serial.println("%");
  Serial.print("Humidade do solo:             ");
  Serial.print(SOIL_HUM);
  Serial.println("%");

  if (isnan(AIR_HUMIDITY) || isnan(AIR_TEMPERATURE))
  {
    Serial.println(" ");
    Serial.println("Falha ao tentar ler o sensor DHT");
    Serial.println(" ");
  }
  else
  {
    Serial.print("Temperatura do ar:            ");
    Serial.print(AIR_TEMPERATURE);
    Serial.println("ºC");
    Serial.print("Humidade do ar:               ");
    Serial.print(AIR_HUMIDITY);
    Serial.println("%");
  }

  Serial.println(" ");
}

/*********************** ENVIA OS DADOS PARA O THINGSPEAK ********************************/
void writeThingSpeak(void)
{
  startThingSpeakCmd();
  // PREPARA A STRING PARA ENVIO PARA O THINGSPEAK
  String getStr = "GET /update?api_key=";
  getStr += STATUS_CH_WRITE_KEY;
  getStr += "&field1=";
  getStr += String(PUMP);
  getStr += "&field2=";
  getStr += String(LAMP);
  getStr += "&field3=";
  getStr += String(AIR_TEMPERATURE);
  getStr += "&field4=";
  getStr += String(AIR_HUMIDITY);
  getStr += "&field5=";
  getStr += String(TEMP_SOIL_DS18B20);
  getStr += "&field6=";
  getStr += String(SOIL_HUM);
  getStr += "&field7=";
  getStr += String(LIGHT);
  getStr += "&field8=";
  getStr += String(SPARE);
  getStr += "\r\n\r\n";
  sendThingSpeakGetCmd(getStr);
}

/******************************* Reset do módulo wifi **************************************/
void EspHardwareReset(void)
{
  Serial.println("Reiniciando.......");
  digitalWrite(ESP8266_RST, LOW);
  delay(500);
  digitalWrite(ESP8266_RST, HIGH);
  delay(8000); //Tempo necessário para começar a ler
  Serial.println("RESET");
}

/********************** INICIA AS COMUNICAÇÕES COM O THINGSPEAK ****************************/
void startThingSpeakCmd(void)
{
  esp8266.flush(); // LIMPA O BUFFER ANTES DE COMEÇAR A ENVIAR OS DADOS

  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // ENDEREÇO IP DO api.thingspeak.com
  cmd += "\",80";
  esp8266.println(cmd);
  Serial.print("enviado ==> Start cmd: ");
  Serial.println(cmd);

  if (esp8266.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}

/************* ENVIA PELO MÉTODO GET AS INFORMAÇÕES PARA O THINGSPEAK *********************/
String sendThingSpeakGetCmd(String u_getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(u_getStr.length());
  esp8266.println(cmd);
  Serial.print("enviado ==> lenght cmd: ");
  Serial.println(cmd);

  if (esp8266.find((char *)">"))
  {
    esp8266.print(u_getStr);
    Serial.print("enviado ==> getStr: ");
    Serial.println(u_getStr);
    delay(700); // TEMPO NECESSÁRIO PARA PROCESSAR O GET, SEM ESTE DELAY ELE IRÁ APRESENTAR BUSY NO PRÓXIMO COMANDO

    String messageBody = "";
    while (esp8266.available())
    {
      String line = esp8266.readStringUntil('\n');
      if (line.length() == 1)
      { //actual content starts after empty line (that has length 1)
        messageBody = esp8266.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    esp8266.println("AT+CIPCLOSE");                      // alert user
    Serial.println("ESP8266 CIPSEND ERROR: reenviando"); //Resend...
    SPARE = SPARE + 1;
    error = 1;
    return "error";
  }
}

/******************************** MÓDULO ESP8266 ****************************************/
/* Conecta o wifi novamente quando o arduino for desconectado ou reiniciado */
void connectWiFi(void)
{
  sendData("AT+RST\r\n", 2000, 0);                                          // reset
  sendData("AT+CWJAP=\"VTNC12\",\"familiagnich04734184513\"\r\n", 2000, 0); //Connect network
  delay(3000);
  sendData("AT+CWMODE=1\r\n", 1000, 0);
  sendData("AT+CIFSR\r\n", 1000, 0); // Show IP Adress
  Serial.println("8266 Connected");
}

/******************************* ENVIA OS DADOS *****************************************/
String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  esp8266.print(command);
  long int time = millis();
  while ((time + timeout) > millis())
  {
    while (esp8266.available())
    {
      char c = esp8266.read();
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}
