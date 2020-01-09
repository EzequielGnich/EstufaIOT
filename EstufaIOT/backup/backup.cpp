// Importação de bibliotecas
#include <DHT.h>
#include <SoftwareSerial.h>

//#define speed8266 9600

//SoftwareSerial EspSerial(6, 7); //Rx ==> Pin 6; TX ==> Pin7

// DHT
int pinoDHT = 7;
char tipoDHT = DHT22;
DHT dht(pinoDHT, tipoDHT); // Função que define o pino de entrada do sensor e o tipo do sensor
int airTemp = 0;           // variavel que armazenara a temperatura do ar
int airHum = 0;            // variavel que armazenara a humidade do ar

void setup()
{
  // DHT
  dht.begin();

  // ESP8266
  //EspSerial.begin (EspSerial);
  //Serial.begin(EspSerial);
  //Serial.println("ESP8266 Setup test - use AT coomands");
  //connectWiFi();
}

void loop()
{
  //while (EspSerial.available())
  //{
  //  Serial.write(EspSerial.read());
  //}
  //while (Serial.available())
  //{
  //  EspSerial.write(Serial.read());
  //}

  //output_value= analogRead(sensor_pin);
  //output_value = map(output_value,550,0,0,100);

  readSensors();
  displaySensors();
  pump();

  delay(5000);
}

/***************************************************
  Connect WiFi
****************************************************/
void connectWiFi(void)
{
  sendData("AT+RST\r\n", 2000, 0);                                             // reset
  sendData("AT+CWJAP=\"SSID_VTNC\",\"familiagnich04734184513\"\r\n", 2000, 0); //Connect network
  delay(3000);
  sendData("AT+CWMODE=1\r\n", 1000, 0);
  sendData("AT+CIFSR\r\n", 1000, 0); // Show IP Adress
  Serial.println("ESP8266 Connected");
}

/***************************************************
  Send AT commands to module
****************************************************/

// String sendData(String command, const int timeout, boolean debug)
// {
//   String response = "";
//   EspSerial.print(command);
//   long int time = millis();
//   while ( (time + timeout) > millis())
//   {
//     while (EspSerial.available())
//     {
//       // The esp has data so display its output to the serial window
//       char c = EspSerial.read(); // read the next character.
//       response += c;
//     }
//   }
//   if (debug)
//   {
//     Serial.print(response);
//   }
//   return response;
// }

// Mostra endereço do(s) sensor(es) DS18B20
// void mostra_endereco_sensor(DeviceAddress deviceAddress)
// {
//   for (uint8_t i = 0; i < 8; i++)
//   {
//     // Adiciona zeros se necessário
//     if (deviceAddress[i] < 16) Serial.print("0");
//     Serial.print(deviceAddress[i], HEX);
//   }
// }

/*********************** Ler valores dos sensores *************************************/
void readSensors(void)
{
  //airTemp = dht.readTemperature();
  //airHum = dht.readHumidity();
}

/*********************Mostrar dados dos sensores ****************************************/
void displaySensors(void)
{
  Serial.print("airTemp  (ºC): ");
  Serial.println(airTemp);
  Serial.print("airHum    (%): ");
  Serial.println(airHum);
  // Serial.print("Mositure : ");
  // Serial.print(output_value);
  Serial.println("%");
  verifica_luminosidade();
  read_DS18B20(sensor);
}

/*****************************************************************************************************************/
// Função para verificação da luminosidade, esta função não deve ser utilizada só pois
// apenas com estes valores não é possivel determinar a melhor opção para irrigamento
void verifica_luminosidade()
{
  if (light <= 34)
  {
    Serial.println("luminosidade boa para irrigação");
  }
  else if (light >= 35)
  {
    Serial.println("luminosidade ruim para irrigação");
  }
}
