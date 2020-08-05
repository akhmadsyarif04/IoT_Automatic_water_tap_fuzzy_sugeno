#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // GANTI 0x3F Ke 0x27 kalau LCD ga muncul

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
Servo motorServo;

const char* ssid = "361Carwash"; 
const char* password = "361Carwash"; 
const char* host = "192.168.100.12"; //edit the host adress, ip address etc. 
 
#define pinSensor A0
int sensorValue = 0;

float sensorSuhu;
float sensorKelembaban;
float sensorCahaya;

//Rule Base
float suhu [3];
float kelembaban [3];
float cahaya [3];
float rule [3][3];
float rule00, rule01, rule02;
float rule10, rule11, rule12;
float rule20, rule21, rule22;
float defuz, pwm, defuzzy, keranBuka;
float temp, minsuhuKelem;

void setup() {
  lcd.begin(); // initializing the lcd 16x2
//  lcd.setBacklightPin(3, POSITIVE); // enable or turn on the backlight
//  lcd.setBacklight(HIGHT);

//  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  
  Serial.begin(115200);
  
  motorServo.attach(D4); //D4

  //cahaya
//  pinMode(A0, INPUT);

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
  int sensorWater = analogRead(A0);
  
  sensorKelembaban = dht.readHumidity();
  sensorSuhu = dht.readTemperature();

  //sensor ldr
//  int nilai = analogRead(A0);   // Baca nilai sensor
//  float Vout = nilai*0.0048828125;
//  sensorCahaya = 500/(10*((5-Vout)/Vout)); // ini yang benar conversi lux
//  float lux2=(2500/Vout-500)/10;
//  Serial.println(sensorCahaya);
//  Serial.println(lux2);

// sensorSuhu = 32.00;
// sensorKelembaban = 62.00;

//  fuzzifikasi
   FuzzySuhu(sensorSuhu);
   FuzzyKelembaban(sensorKelembaban);

// defuzzifikasi
   Defuzzy();
   
// servo
//motorServo.write(90);
//delay (3000);
//motorServo.write(0);

  Serial.println("water sensor");
  Serial.println(sensorWater);
  if (sensorWater > 900) {
    Serial.println("stop");
    motorServo.write(0);
    keranBuka = 0;
  }else{
    motorServo.write(defuzzy);
    Serial.println(defuzzy);
    keranBuka = defuzzy;
  }
   

//  lcd.clear();
//  lcd.setCursor(0,0);
//  lcd.print("K: ");
//  lcd.setCursor(2,0);
//  lcd.print(sensorKelembaban);
//  lcd.setCursor(8,0);
//  lcd.print("S: ");
//  lcd.setCursor(10,0);
//  lcd.print(sensorSuhu);
//
//  lcd.setCursor(0,1);
//  lcd.print("C: ");
//  lcd.setCursor(2,1);
//  lcd.print(sensorCahaya);
//  lcd.setCursor(8,1);
//  lcd.print("F: ");
//  lcd.setCursor(10,1);
//  lcd.print(defuzzy);
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
  String url = "/IoT_walet/add.php?";
  url += "suhu=";
  url += sensorSuhu;
  url += "&kelembaban=";
  url += sensorKelembaban;
  url += "&cahaya=";
  url += sensorCahaya;
  url += "&keran=";
  url += keranBuka;
  
 
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
    if (line.indexOf("sukses gaes") != -1) {
      Serial.println();
      Serial.println("Yes, data masuk");
    } else if (line.indexOf("gagal gaes") != -1) {
      Serial.println();
      Serial.println("Maaf, data gagal masuk");
    }
  }
 
//  Serial.println();
//  Serial.println("closing connection");
  delay(3000); // delay 3s
//  delay(300000); // delay 5 menit
}


void FuzzySuhu(float sensorSuhu){
  // untuk suhu Normal
  if (sensorSuhu <= 25.00)
  { suhu [0] = 1;}
  else if (25.00 <= sensorSuhu && sensorSuhu <= 27.00)
  {  suhu [0] = (27.00 - sensorSuhu)/(27.00 - 25.00); }
  else
  { suhu [0] = 0;}
//  Serial.print("suhu functions: ");
//  Serial.println(sensorSuhu);

  // untuk suhu hampir panas / Sedang
  if (sensorSuhu <= 25.00 || sensorSuhu >= 29.00)
  { suhu [1] = 0;}
  else if (25.00 <= sensorSuhu && sensorSuhu <= 27.00)
  { suhu [1] = (sensorSuhu-25.00)/(27.00-25.00);}
  else if (27.00 <= sensorSuhu && sensorSuhu <= 29.00)
  { suhu [1] = (29.00 - sensorSuhu)/(29.00-27.00);}

  // untuk suhu panas / Tinggi
  if (sensorSuhu <= 27.00)
  { suhu [2] = 0;}
  else if (27.00 <= sensorSuhu && sensorSuhu <= 29.00)
  { suhu [2] = (sensorSuhu-27.00)/(29.00-27.00);}
  else
  { suhu [2] = 1;}
}

void FuzzyKelembaban(float sensorKelembaban){
  // untuk kelembaban Tinggi
  if (sensorKelembaban <= 65.00)
  { kelembaban [0] = 1;}
  else if (65.00 <= sensorKelembaban && sensorKelembaban <= 70.00)
  {  kelembaban [0] = (70.00 - sensorKelembaban)/(70.00 - 65.00); }
  else
  { kelembaban [0] = 0;}

  // untuk kelembaban Sedang
  if (sensorKelembaban <= 65.00 || sensorKelembaban >= 95.00)
  { kelembaban [1] = 0;}
  else if (65.00 <= sensorKelembaban && sensorKelembaban <= 70.00)
  { kelembaban [1] = (sensorKelembaban-25.00)/(27.00-25.00);}
  else if (70.00 <= sensorKelembaban && sensorKelembaban <= 95.00)
  { kelembaban [1] = (95.00 - sensorKelembaban)/(95.00-70.00);}

  // untuk kelembaban normal
  if (sensorKelembaban <= 70.00)
  { kelembaban [2] = 0;}
  else if (70.00 <= sensorKelembaban  && sensorKelembaban <= 95.00)
  { kelembaban [2] = (sensorKelembaban-70.00)/(95.00 - 70.00);}
  else
  { kelembaban [2] = 1;}
  Serial.print("kelembaban functions: ");
  Serial.println(sensorKelembaban);
}


// rule dan defuzzifikasi belum dirubah
void RuleEva (){
 int i, j;
 for ( i=0; i<=2; i=i+1)
 {
   for ( j=0; j<=2; j=j+1)
   {
     temp = min(suhu[i], kelembaban[j]);
     rule [i][j] = temp;
   } 
 } 
 rule00 = rule [0][0]; // (normal,normal = )
 rule01 = rule [0][1]; // (normal,sedang = )
 rule02 = rule [0][2]; // (normal,tinggi = )

 rule10 = rule [1][0]; // (sedang,normal = )
 rule11 = rule [1][1]; // (sedang,sedang = )
 rule12 = rule [1][2]; // (sedang,tinggi = )

 rule20 = rule [2][0]; // (tinggi,normal = )
 rule21 = rule [2][1]; // (tinggi,sedang = )
 rule22 = rule [2][2]; // (tinggi,tinggi = )
 
}

void Defuzzy () {
  // metode sugeno (weighted average)
  RuleEva();
  pwm = (rule00 * 0) + (rule01 * 22.5) + (rule02 * 45) + (rule10 * 22.5) + (rule11 * 45) + (rule12 * 67.5) + (rule20 * 45) + (rule21 * 67.5) + (rule22 * 90);
 
  defuz = 0;
  int i, j;
  for ( i=0; i<=2; i=i+1)
  {
    for ( j=0; j<=2; j=j+1)
    {
      defuz = defuz + rule [i][j];
    } 
  } 
  defuzzy = pwm / defuz;
}
