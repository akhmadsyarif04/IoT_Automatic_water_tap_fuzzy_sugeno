#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // GANTI 0x3F Ke 0x27 kalau LCD ga muncul
#include <DHT.h>
DHT dht(2, DHT11); //Pin, Jenis DHT
#include <Servo.h>
Servo motorServo; 

//sensor ldr
const int pin_ldr = A0;
int powerPin = 3;    // untuk pengganti VCC/5vOLT
#define r1 8

//Rule Base
float suhu [2];
float kelembaban [2];
float cahaya [2];
float rule [2][2];
float rule00, rule01;
float rule10, rule11;
float defuz, pwm, defuzzy;
float temp;
float sensorKelembaban;
 float sensorSuhu;
// int sensorCahaya;



void setup(){
  
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight(); 
  // jadikan pin power sebagai output
  pinMode(powerPin, OUTPUT);
  // default bernilai LOW
  digitalWrite(powerPin, LOW);
 Serial.begin(9600);
 dht.begin();
 pinMode(r1, OUTPUT);
 pinMode(r1, OUTPUT);
 motorServo.attach(10);
}
 
void loop(){
 digitalWrite(powerPin, HIGH);

float sensorSuhu = dht.readTemperature();
float sensorKelembaban = dht.readHumidity();

// sensorSuhu = 32.00;
// sensorKelembaban = 79.00 ;
// sensorCahaya = 2;
// float sensorCahaya = dht.readTemperature();

//  fuzzifikasi
 FuzzySuhu(sensorSuhu);
 FuzzyKelembaban(sensorKelembaban);

// defuzzifikasi
 Defuzzy();

// Serial.print("k: ");
// Serial.print(sensorKelembaban);
// Serial.print(" ");
// Serial.print("s: ");
// Serial.println(sensorSuhu);
//  Serial.print("c: ");
// Serial.println(sensorCahaya);
// Serial.print("fuzzy: ");
// Serial.println(defuzzy);
 
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
//  lcd.print("C: ");
//  lcd.setCursor(2,1);
//  lcd.print(sensorCahaya);
  lcd.setCursor(8,1);
  lcd.print("F: ");
  lcd.setCursor(10,1);
  lcd.print(defuzzy);

  delay(1000);

//  servo
   motorServo.write(defuzzy);  // Turn Servo ke kiri 45 degrees

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
