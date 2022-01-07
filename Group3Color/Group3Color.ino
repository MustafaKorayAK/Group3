#include <WiFi.h>
#include <PubSubClient.h>
#include<Wire.h>


// Pinler

int DHT11_Pin = 14;

int S0= 32;
int S1 =33;
int S2= 25;
int S3 =26;
int sensorOut =27;
//----------------------
//renk sensörünün değerleri
int redColor;
int greenColor;
int blueColor ;

//---------------------------
//dhtnin değerleri
int Humidity = 0;
String Temp ;
bool DHTError = false;
//-----------
//beklemeler
long dhtDelay;
long ColorDelay;
//---------------------------
const char* ssid = "AndroidAP0917";
const char* password = "yaxg8456";
const char* mqtt_server = "54.224.57.65";
const char* subscribeAd = "esp32/output";
const char* DeviceName = "Group3";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Wire.begin();
  Serial.begin(115200);

 beginColor();
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dhtDelay = millis();
  ColorDelay = millis();
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  if (millis() - dhtDelay > 300) {
    dhtDelay = millis();
    ReadDHT11();
    PrintDHT11();

    client.publish("group3/humidity", String(Humidity).c_str());

    client.publish("group3/temp", String(Temp).c_str());
  }
  if (millis() - ColorDelay > 500) {
    ColorDelay = millis();
    ReadRawColor();
   String mes ="r:"+String(redColor) + " g:" + String(greenColor) + " b:" + String(blueColor);
    client.publish("group3/color", mes.c_str());
  }

}



void ReadDHT11() {
  // datasheet de yazanlar
  long int DataTime = 0;

  byte Result[45];
  byte DataArray = 0;
  byte DataCounter = 0;
  byte DHTData[4];

  bool BlockDHT = false;

  // sensörü açma

  pinMode(DHT11_Pin, OUTPUT);
  digitalWrite(DHT11_Pin, HIGH);
  delay(250); //250 ms bekle
  digitalWrite(DHT11_Pin, LOW);
  delay(30);            //30ms bekle
  digitalWrite(DHT11_Pin, HIGH);
  delayMicroseconds(50); //50 micro saniye bekle
  pinMode(DHT11_Pin, INPUT);


  do {
    if (digitalRead(DHT11_Pin) == 0 && BlockDHT == false) {//düşüşleri oku
      BlockDHT = true;
      Result[DataArray] = (micros() - DataTime);
      DataArray++;
      DataTime = micros();
    }
    if (digitalRead(DHT11_Pin) == 1) {
      BlockDHT = false;
    }


  } while ((micros() - DataTime) < 150); // dht pini 150 us den fazla yüksekte ise whiledan çık

  for (int  i = 2; i < DataArray; i++) {
    if (Result[i] <= 90) Result[i] = 0; else Result[i] = 1;
  }

  for (int  j = 0; j < 5; j++) {
    for (int  i = 0; i < 8; i++) {
      bitWrite(DHTData[j], 7 - i, Result[i + 2 + (j * 8)]);
    }

  }


  // hata var mı kontrol et
  if (DHTData[4] == (DHTData[0] + DHTData[1] + DHTData[2] + DHTData[3])) {
    Humidity = DHTData[0];

    int tempTemp = DHTData[2];
    int TempComma = DHTData[3];

    Temp = String(tempTemp) + "." + String(TempComma);

    DHTError = false;
  }
  else {
    DHTError = true;
  }
}

void PrintDHT11() {
  if (DHTError == false) {
    Serial.print("Humidity = ");
    Serial.print(Humidity);
    Serial.print("% ");
    Serial.print(" Temp = ");
    Serial.print(Temp);

    Serial.println("°C ");
  }
  else {
    //Serial.println("Error");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(DeviceName)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(subscribeAd);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {

  Serial.print(topic);
 
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
Serial.println();

  if (String(topic) == String(subscribeAd)) {

  }
}

void beginColor() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
}

void ReadRawColor() {
// kırmızı
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  
  long redFrequency = pulseIn(sensorOut, LOW);
  redColor = map(redFrequency, 25, 253, 255,0);
  Serial.print("R = ");
  Serial.print(redColor);
  delay(5);
  
  // yeşil
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
 
  long greenFrequency = pulseIn(sensorOut, LOW);
  greenColor = map(greenFrequency, 27, 310, 255, 0);
  
  Serial.print(" G = ");
  Serial.print(greenColor);
  delay(5);
  
 //mavi
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
 
  long blueFrequency = pulseIn(sensorOut, LOW);
  blueColor = map(blueFrequency, 26, 300, 255, 0);
  Serial.print(" B = ");
  Serial.println(blueColor);
  delay(5);


}
