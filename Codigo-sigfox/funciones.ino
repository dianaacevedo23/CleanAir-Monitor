//-----------------------------------------------------------VARIABLES-------------------------------------------------------------------------------------------------------
//DHT11
int h;
int t;
#include "DHT.h"           //LIBRERIA
#define DHTPIN 4           //PIN DIGITAL
#define DHTTYPE DHT11      //TIPO DE SENSOR
DHT dht(DHTPIN, DHTTYPE);  //INICIALIZAMOS EL SENSOR DHT11

//POLVO
#include <GP2Y1010AU0F.h>
int measurePin = A0;  // Connect dust sensor analog measure pin to Arduino A0 pin
int ledPin = 2;       // Connect dust sensor LED pin to Arduino pin 2
float dustDensity;
GP2Y1010AU0F dustSensor(ledPin, measurePin);  // Construct dust sensor global object

//MQ9
const int mq9_pin = A1;  // Conecta el pin de señal del sensor al pin analógico A0
int sensorValue;
float co_ppm;

//SIGFOX
const int boton = 5;  //boton para enviar datos
String bufer;
String bufer2 = "\n";

//-------------------------------------------------------------INICIO--------------------------------------------------------
void inicial() {
  Serial.begin(9600);  // Inicia la comunicación seria
  //MQ9
  pinMode(mq9_pin, INPUT);  // Configura el pin del sensor como entrada
    //POLVO
  dustSensor.begin();
  //Dht11
  dht.begin();
  //SIGFOX
  pinMode(boton, INPUT);
  pinMode(7, OUTPUT);  //enable modulo wisol
  pinMode(3, OUTPUT);
  digitalWrite(7, HIGH);
}

//------------------ENVIAR INFORMACION BOTON------------------------------------
void verificarboton() {
  delay(10000);
  mensaje1();
  delay(1800000); // Retardo de 30 minutos
 // if (digitalRead(boton) == LOW) {
 //   mensaje1();
 // }
}

//---------------------------------------------------------------SENSORES------------------------------------------------------------------------------------------------------
//MQ9****************************************************************************************************
void MQ9() {
  // Leer el valor analógico del sensor
  sensorValue = analogRead(mq9_pin);

  // Calcular la resistencia del sensor
  float sensorResistance = (float)(1023 - sensorValue) / sensorValue;

  // Medir concentración de monóxido de carbono (CO)
  co_ppm = sensorResistance / 9.52;

  // Imprimir el valor de la concentración de monóxido de carbono
  Serial.print("Concentracion de monoxido de carbono (ppm): ");
  Serial.println(co_ppm);
  Serial.println(sensorValue);
}

//************************************************************************************************

//Polvo-------------------------------------------------------------------------------------------
void sensorPOLVO() {
  dustDensity = dustSensor.read();
  // Verificar si el valor de densidad de polvo es negativo
  if (dustDensity < 0) {
    dustDensity = 0;  // Establecer el valor a 0 si es negativo
  }
  Serial.print("Dust Density = ");
  Serial.print(dustDensity);
  Serial.println(" ug/m3");
}
//---------------------------------------------------------------------------------------------------------------------------

//DHT11**************************************************************************************************************************
void temp_hum() {
  delay(5000);                //TIEMPO DE ESPERA ENTRE MEDIDAS
  h = dht.readHumidity();     //LECTURA DE LA HUMEDAD
  t = dht.readTemperature();  //LECTURA DE TEMPERATURA EN CENTIGRADOS

  if (isnan(h) || isnan(t)) {  //COMPROBAMOS SI EXISTE ALGUN ERROR
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }

  Serial.print("Humedad:");
  Serial.print(h);
  Serial.print("%\t");
  Serial.print("Temperatura:");
  Serial.print(t);
  Serial.println("*C");  // Se cambió Serial.print por Serial.println para agregar un salto de línea
}
//********************************************************************************************************************************
//----------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------- MENSAJES SIGFOX--------------------------------------------------------------------------

void mensaje1() {
   sensorPOLVO();
  MQ9();
  temp_hum();
  int deviceid = "42F7C4";
  //-----------------------------------------------------
  //AT$SF= comando para mandar la informacion por sigfox
  //Maximo 12 bytes
  bufer = "AT$SF=";
  //-----------------------------------------------------
   add_float(dustDensity);
  add_int(sensorValue);
  add_float(co_ppm);
  add_int(h);  //un flotante ocupa 4 bytes
  add_int(t);
  //enviamos nuestro dato por Sigfox
  send_message(bufer);
}

//----------------------------------------------------------------SIGFOX----------------------------------------------------------------------------------

void add_float(float var1)  //función para agregar flotantes al payload
{
  byte* a1 = (byte*)&var1;  //convertimos el dato a bytes
  String str1;
  //agregamos al comando AT$SF= nuestra informacion a enviar
  for (int i = 0; i < 4; i++) {
    str1 = String(a1[i], HEX);  //convertimos el valor hex a string
    if (str1.length() < 2) {
      bufer += 0 + str1;  //si no, se agrega un cero
    } else {
      bufer += str1;  //si esta completo, se copia tal cual
    }
  }
}
void add_int(int var2)  //funcion para agregar enteros al payload (hasta 255)
{
  byte* a2 = (byte*)&var2;  //convertimos el dato a bytes
  String str2;
  str2 = String(a2[0], HEX);  //convertimos el valor hex a string
  //verificamos si nuestro byte esta completo
  if (str2.length() < 2) {
    bufer += 0 + str2;  //si no, se agrega un cero
  } else {
    bufer += str2;  //si esta completo, se copia tal cual
  }
}


void send_message(String payload) {
  //agregamos el salto de linea "\n"
  bufer += bufer2;
  //*******************
  //Habilitamos el modulo Sigfox
  digitalWrite(3, HIGH);
  delay(1000);
  //Reset del canal para asegurar que manda en la frecuencia correcta
  Serial.print("AT$RC\n");
  //************************
  //Enviamos la informacion por sigfox
  Serial.print(bufer);
  delay(3000);
  //deshabilitamos el modulo Sigfox
  digitalWrite(3, LOW);
}
