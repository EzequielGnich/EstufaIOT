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

/***************************** INICIO CONFIGURAÇÃO DS18B20 ******************************/
OneWire pino(ONE_WIRE_PIN);
DallasTemperature barramento(&pino);
DeviceAddress sensor;

/***************************** INICIO DO SETUP DO PROGRAMA ******************************/
void setup()
{
  Serial.begin(9600);
  findDS18B20Sensor(sensor);
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
  delay(5000);
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
