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

// VARIÁVEIS PARA LIGAR A LAMPADA E A BOMBA DE ÁGUA
boolean PUMP = 0;
boolean LAMP = 0;

// CONTADOR PARA MOLHAR AS PLANTAS
int COUNTER = 0;

// RELAY
const int PORT_RELAY_PUMP = 5;
const int PORT_RELAY_LAMP = 4;

// VARIÁVEIS PARA CÁLCULOS
int VAR_LIGHT = 50;
float VAR_TEMP_SOIL = 28.00;
int VAR_SOIL_HUM_MIN = 40;
float VAR_AIR_TEMPERATURE = 27;


/***************************** INICIO CONFIGURAÇÃO DS18B20 ******************************/
OneWire pino(ONE_WIRE_PIN);
DallasTemperature barramento(&pino);
DeviceAddress sensor;

/******************************** CONFIGURAÇÃO 74HC595 **********************************/

int const clk = 11;  // Pino 11 do 74HC595
int const latch = 12;  // Pino 12 do 74HC595
int const data  = 13;  // Pino 13 do 74HC595


/***************************** INICIO DO SETUP DO PROGRAMA ******************************/
void setup()
{
  pinMode(clk,OUTPUT);
  pinMode(latch,OUTPUT);
  pinMode(data,OUTPUT);
  
  Serial.begin(9600);
  findDS18B20Sensor(sensor);
  define_pin_relay();
  dht.begin();
  delay(2500);
}

/**************************** INICIO DO LOOP DO PROGRAMA ******************************/
void loop()
{
  readDS18B20Sensor(sensor);
  readLDRSensor(LDR_PIN);
  readHumiditySensor(SOIL_HUM_PIN);
  readHumAndTempSensor();
  displaySensors();
  delay(3000);
  soil_hum_verify();
  soil_temperature_verify();
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

/************************* CONFIGURAÇÃO PINOS DO RELAY ***********************************/
void define_pin_relay(void) {
  pinMode(PORT_RELAY_PUMP, OUTPUT);
  pinMode(PORT_RELAY_LAMP, OUTPUT);
  digitalWrite(PORT_RELAY_PUMP, HIGH);
  digitalWrite(PORT_RELAY_LAMP, HIGH);
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

void soil_temperature_verify(void) {
  if (TEMP_SOIL_DS18B20 <= 17.00) {
    for (int i = 0; TEMP_SOIL_DS18B20 <= 20; i++) {
      digitalWrite(PORT_RELAY_LAMP, LOW);
      delay(3600000);
      readDS18B20Sensor(sensor);
    }
    digitalWrite(PORT_RELAY_LAMP, HIGH);
  }

}

void soil_hum_verify(void) {
  /* Verificação do solo se abaixo de 10% */
  if (SOIL_HUM <= 10) {
    Serial.println("Solo com humidade baixa .");
    if (TEMP_SOIL_DS18B20 < VAR_TEMP_SOIL) {
      for (int i = 0; SOIL_HUM < 50; i++) {
        Serial.println("A bomba de água foi ligada " + String(i) + " vez(es)");
        digitalWrite(PORT_RELAY_PUMP, LOW);
        if (i == 0) delay(12000);   // Verifica se é a primeira vez que a bomba está ligando
        else delay(7000);           // Se não for a primeira vez liga a bomba por apenas 7 seg
        digitalWrite(PORT_RELAY_PUMP, HIGH);
        delay(10000);
        readHumiditySensor(SOIL_HUM_PIN);
        Serial.println(SOIL_HUM);
      }
    }
  }
  /* Verificação do solo se acima de 11% e abaixo de 40% */
  if (SOIL_HUM > 11 && SOIL_HUM < 40) {
    Serial.println("Solo com humidade moderada.");
    if (TEMP_SOIL_DS18B20 < VAR_TEMP_SOIL) {
      for (int i = 0; SOIL_HUM < 50; i++) {
        Serial.println("A bomba de água foi ligada " + String(i) + " vez(es)");
        digitalWrite(PORT_RELAY_PUMP, LOW);
        if (i == 0) delay(15000);   // Verifica se é a primeira vez que a bomba está ligando
        else delay(7000);           // Se não for a primeira vez liga a bomba por apenas 7 seg
        digitalWrite(PORT_RELAY_PUMP, HIGH);
        delay(6000);
        readHumiditySensor(SOIL_HUM_PIN);
        Serial.println(SOIL_HUM);
      }
    }
  }

  SOIL_HUM = 42;

  if (SOIL_HUM > 41) {
      
    //coloca e mantém o pino latch em low enquanto ocorre a transmissão
    digitalWrite(latch, LOW);
    //transmite o valor de j, a começar pelo bit menos significativo
    shiftOut(data, clk, LSBFIRST, 0b11111111);
    //retorna o pino latch para high para sinalizar ao chip
    //que esse não precisa mais esperar por informação
    digitalWrite(latch, HIGH);
  }
  delay(1000);
  digitalWrite(PORT_RELAY_PUMP, HIGH);
}
