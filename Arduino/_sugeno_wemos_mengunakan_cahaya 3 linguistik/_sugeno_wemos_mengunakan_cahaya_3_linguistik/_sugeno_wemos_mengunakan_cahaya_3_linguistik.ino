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
float rule [3][3][3];
float rule000, rule001, rule002;
float rule010, rule011, rule012;
float rule020, rule021, rule022;

float rule100, rule101, rule102;
float rule110, rule111, rule112;
float rule120, rule121, rule122;

float rule200, rule201, rule202;
float rule210, rule211, rule212;
float rule220, rule221, rule222;

float defuz, pwm, defuzzy;
float temp, minsuhuKelem;

void setup() {
  lcd.begin(); // initializing the lcd 16x2

//  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  
  Serial.begin(115200);
  delay(5000);
  
  motorServo.attach(D4); //D4
  motorServo.write(30); // set jadi 0 pertama kali

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
  sensorKelembaban = dht.readHumidity();
  sensorSuhu = dht.readTemperature();

  //sensor ldr
  int nilai = analogRead(A0);   // Baca nilai sensor
  float Vout = nilai*0.0048828125;
  sensorCahaya = 500/(10*((5-Vout)/Vout)); // ini yang benar conversi lux

// sensorSuhu = 32.00;
// sensorKelembaban = 62.00 ;
// sensorCahaya = 1.90;

//  fuzzifikasi
   FuzzySuhu(sensorSuhu);
   FuzzyKelembaban(sensorKelembaban);
   FuzzyCahaya(sensorCahaya);

// defuzzifikasi
   Defuzzy();
   
// servo
   motorServo.write(defuzzy);

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
  delay(3000); // delay 3detik
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

//  Serial.print("suhu 0: ");
//  Serial.println(suhu[0]);
//    Serial.print("suhu 1: ");
//  Serial.println(suhu[1]);

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
//  Serial.print("kelembaban functions: ");
//  Serial.println(sensorKelembaban);
}

void FuzzyCahaya(int sensorCahaya){
  // untuk cahaya normal
  if (sensorCahaya <= 0)
  { cahaya [0] = 1;}
  else if (0 <= sensorCahaya && sensorCahaya <= 0.36)
  {  cahaya [0] = (0.36 - sensorCahaya)/(0.36 - 0); }
  else
  { cahaya [0] = 0;}

  // untuk cahaya Sedang
  if (sensorCahaya <= 0 || sensorCahaya >= 1.00)
  { cahaya [1] = 0;}
  else if (0 <= sensorCahaya && sensorCahaya <= 0.36)
  { cahaya [1] = (sensorCahaya-0)/(0.36-0);}
  else if (0.36 <= sensorCahaya && sensorCahaya <= 1.00)
  { cahaya [1] = (1.00 - sensorCahaya)/(0.36-1.00);}

  // untuk cahaya tinggi
  if (sensorCahaya <= 0.36)
  { cahaya [1] = 0;}
  else if (0.36 <= sensorCahaya && sensorCahaya <= 1.00)
  { cahaya [1] = (sensorCahaya-0.36)/(1.00 - 0.36);}
  else
  { cahaya [1] = 1;}
//  Serial.print("cahaya functions 0: ");
//  Serial.println(cahaya[0]);
//  Serial.print("cahaya functions 1: ");
//  Serial.println(cahaya[1]);
}

// rule dan defuzzifikasi belum dirubah
void RuleEva (){
 int i, j, c;
 for ( i=0; i<=2; i=i+1)
 {
   for ( j=0; j<=2; j=j+1)
   {
      for ( c=0; c<=2; c=c+1)
      {
         minsuhuKelem = min(suhu[i], kelembaban[j]);
         temp = min(minsuhuKelem, cahaya[c]); 
         rule [i][j][c] = temp;
      }
   } 
 } 
 rule000 = rule [0][0][0]; // (normal,normal, normal = )
 rule001 = rule [0][0][1]; // (normal,normal, sedang = )
 rule002 = rule [0][0][2]; // (normal,normal, tinggi = )

 rule010 = rule [0][1][0]; // (normal,sedang, normal = )
 rule011 = rule [0][1][1]; // (normal,sedang, sedang = )
 rule012 = rule [0][1][2]; // (normal,sedang, tinggi = )

 rule020 = rule [0][2][0]; // (normal,tinggi, normal = )
 rule021 = rule [0][2][1]; // (normal,tinggi, sedang = )
 rule022 = rule [0][2][2]; // (normal,tinggi, tinggi = )

 rule100 = rule [1][0][0]; // (sedang,normal, normal = )
 rule101 = rule [1][0][1]; // (sedang,normal, sedang = )
 rule102 = rule [1][0][2]; // (sedang,normal, tinggi = )

 rule110 = rule [1][1][0]; // (sedang,sedang, normal = )
 rule111 = rule [1][1][1]; // (sedang,sedang, sedang = )
 rule112 = rule [1][1][2]; // (sedang,sedang, tinggi = )

 rule120 = rule [1][2][0]; // (sedang,tinggi, normal = )
 rule121 = rule [1][2][1]; // (sedang,tinggi, sedang = )
 rule122 = rule [1][2][2]; // (sedang,tinggi, tinggi = )

 rule200 = rule [2][0][0]; // (tinggi,normal, normal = )
 rule201 = rule [2][0][1]; // (tinggi,normal, sedang = )
 rule202 = rule [2][0][2]; // (tinggi,normal, tinggi = )

 rule210 = rule [2][1][0]; // (tinggi,sedang, normal = )
 rule211 = rule [2][1][1]; // (tinggi,sedang, sedang = )
 rule212 = rule [2][1][2]; // (tinggi,sedang, tinggi = )

 rule220 = rule [2][2][0]; // (tinggi,tinggi, normal = )
 rule221 = rule [2][2][1]; // (tinggi,tinggi, sedang = )
 rule222 = rule [2][2][2]; // (tinggi,tinggi, tinggi = )
 
}

void Defuzzy () {
  // metode sugeno (weighted average)
  RuleEva();
  pwm = (rule000 * 0) + (rule001 * 15) + (rule002 * 45) + (rule010 * 15)+ (rule011 * 45) + (rule012 * 45) + (rule020 * 30)+ (rule021 * 45) + (rule022 * 60) + (rule100 * 15) + (rule101 * 30) + (rule102 * 45) + (rule110 * 30) + (rule111 * 35) + (rule112 * 60) + (rule120 * 45) + (rule121 * 60) + (rule122 * 75) + (rule200 * 30) + (rule201 * 45) + (rule202 * 60) + (rule210 * 45) + (rule211 * 60) + (rule212 * 75) + (rule220 * 60) + (rule221 * 75) + (rule222 * 90);
 
  defuz = 0;
  int i, j, c;
  for ( i=0; i<=2; i=i+1)
  {
    for ( j=0; j<=2; j=j+1)
    {
      for ( c=0; c<=2; c=c+1)
      {
        defuz = defuz + rule [i][j][c];
      }
    } 
  } 
//  defuzzy = pwm / defuz;
  defuzzy = 0;
}
