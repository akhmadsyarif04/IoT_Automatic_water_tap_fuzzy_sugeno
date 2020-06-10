#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // GANTI 0x3F Ke 0x27 kalau LCD ga muncul

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
Servo motorServo;

const char* ssid = "killu"; 
const char* password = "12345678"; 
const char* host = "192.168.1.6"; //edit the host adress, ip address etc. 
 
#define pinSensor A0
int sensorValue = 0;

float sensorSuhu;
float sensorKelembaban;
float sensorCahaya;

//Rule Base
float suhu [2];
float kelembaban [2];
float cahaya [2];
float rule [2][2];
float rule00, rule01;
float rule10, rule11;
float defuz, pwm, defuzzy;
float temp, minsuhuKelem;

void setup() {
  lcd.begin(); // initializing the lcd 16x2
//  lcd.setBacklightPin(3, POSITIVE); // enable or turn on the backlight
//  lcd.setBacklight(HIGHT);

//  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  
  Serial.begin(115200);
  delay(5000);
  
  motorServo.attach(D4); //D4
  motorServo.write(20); // set jadi 0 pertama kali

  //cahaya
  pinMode(A0, INPUT);

//  Serial.print("{\"humidity\": ");
//  Serial.print(kelembaban);
//  Serial.print(", \"temp\": ");
//  Serial.print(suhu);
//  Serial.print("}\n");


 // =========================================================
 // wifi
  Serial.println();
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
 
void loop() {
//  sensorKelembaban = dht.readHumidity();
//  sensorSuhu = dht.readTemperature();

  //sensor ldr
  int nilai = analogRead(A0);   // Baca nilai sensor
  float Vout = nilai*0.0048828125;
//  sensorCahaya = 500/(10*((5-Vout)/Vout)); // ini yang benar conversi lux
//  float lux2=(2500/Vout-500)/10;
//  Serial.println(sensorCahaya);
//  Serial.println(lux2);

 sensorSuhu = 32.00;
 sensorKelembaban = 62.00 ;
// sensorCahaya = 0.23;

//  fuzzifikasi
   FuzzySuhu(sensorSuhu);
   FuzzyKelembaban(sensorKelembaban);
//   FuzzyCahaya(sensorCahaya);

// defuzzifikasi
   Defuzzy();
   
// servo
   motorServo.write(defuzzy);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("K: ");
  lcd.setCursor(2,0);
  lcd.print(sensorKelembaban);
  lcd.setCursor(8,0);
  lcd.print("S: ");
  lcd.setCursor(10,0);
  lcd.print(sensorSuhu);

  lcd.setCursor(0,1);
  lcd.print("C: ");
  lcd.setCursor(2,1);
  lcd.print(sensorCahaya);
  lcd.setCursor(8,1);
  lcd.print("F: ");
  lcd.setCursor(10,1);
  lcd.print(defuzzy);
// ==============================================================
  // koneksi wifi
  sensorValue = analogRead(pinSensor);
 
  Serial.print("connecting to ");
  Serial.println(host);
 
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
 
  // We now create a URI for the request
  String url = "/walet/add.php?";
  url += "suhu=";
  url += sensorSuhu;
  url += "&kelembaban=";
  url += sensorKelembaban;
  url += "&cahaya=";
  url += sensorCahaya;
  url += "&keran=";
  url += defuzzy;
  
 
  Serial.print("Requesting URL: ");
  Serial.println(url);
 
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
 
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
 
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);
 
    if (line.indexOf("sukses gaes") != -1) {
      Serial.println();
      Serial.println("Yes, data masuk");
    } else if (line.indexOf("gagal gaes") != -1) {
      Serial.println();
      Serial.println("Maaf, data gagal masuk");
      //digitalWrite(alarmPin, HIGH);
    }
  }
 
//  Serial.println();
//  Serial.println("closing connection");
  delay(10000); // delay 10detik
}


void FuzzySuhu(float sensorSuhu){
  // untuk suhu hampir panas
  if (sensorSuhu <= 27.00)
  { suhu [0] = 1;}
  else if (sensorSuhu > 27.00 && sensorSuhu <= 30.00)
  {  suhu [0] = (30.00 - sensorSuhu)/(30.00 - 27.00); }
  else
  { suhu [0] = 0;}
  Serial.print("suhu functions: ");
  Serial.println(sensorSuhu);
  // untuk suhu panas
  if (sensorSuhu <= 27.00)
  { suhu [1] = 0;}
  else if (sensorSuhu > 27.00 && sensorSuhu <= 30.00)
  { suhu [1] = (sensorSuhu-27.00)/(30.00-27.00);}
  else
  { suhu [1] = 1;}
}

void FuzzyKelembaban(float sensorKelembaban){
  // untuk panas
  if (sensorKelembaban <= 65.00)
  { kelembaban [0] = 1;}
  else if (sensorKelembaban > 65.00 && sensorKelembaban <= 70.00)
  {  kelembaban [0] = (70.00 - sensorKelembaban)/(70.00 - 65.00); }
  else
  { kelembaban [0] = 0;}

  // untuk hampir panas
  if (sensorKelembaban <= 65.00)
  { kelembaban [1] = 0;}
  else if (sensorKelembaban > 65.00 && sensorKelembaban <= 70.00)
  { kelembaban [1] = (sensorKelembaban-65.00)/(70.00 - 65.00);}
  else
  { kelembaban [1] = 1;}
  Serial.print("kelembaban functions: ");
  Serial.println(sensorKelembaban);
}


// rule dan defuzzifikasi belum dirubah
void RuleEva (){
 int i, j;
 for ( i=0; i<=1; i=i+1)
 {
   for ( j=0; j<=1; j=j+1)
   {
     temp = min(suhu[i], kelembaban[j]);
     rule [i][j] = temp;
   } 
 } 
 rule00 = rule [0][0]; // (hampir panas,hampir panas = )
 rule01 = rule [0][1]; // (hampir panas,panas = )
 
 rule10 = rule [1][0]; // (panas,hampir panas = )
 rule11 = rule [1][1]; // (panas,panas = )
 
}

void Defuzzy () {
  // metode sugeno (weighted average)
  RuleEva();
  pwm = (rule00 * 30) + (rule01 * 40)+ (rule10 * 75)+ (rule11 * 90);
 
  defuz = 0;
  int i, j;
  for ( i=0; i<=1; i=i+1)
  {
    for ( j=0; j<=1; j=j+1)
    {
      defuz = defuz + rule [i][j];
    } 
  } 
  defuzzy = pwm / defuz;
}
