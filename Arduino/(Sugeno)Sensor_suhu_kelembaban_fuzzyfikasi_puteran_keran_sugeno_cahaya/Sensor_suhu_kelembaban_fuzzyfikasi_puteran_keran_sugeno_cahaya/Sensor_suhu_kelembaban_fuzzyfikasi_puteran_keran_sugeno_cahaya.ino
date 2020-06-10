#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // GANTI 0x3F Ke 0x27 kalau LCD ga muncul
#include <DHT.h>
DHT dht(2, DHT11); //Pin, Jenis DHT
#include <Servo.h>
Servo motorServo; 

/*  RL=500/lux
 *  V0=5*(RL/(RL+R))
 *  V0=LDR_value*ADC_value
 *  lux=(250/V0)-50
 *  Author: Ashish Kumar 
    Org: INVOOTECH                  */
float lux=0.00,ADC_value=0.0048828125,LDR_value;

//sensor ldr
const int pin_ldr = A0;
int powerPin = 3;    // untuk pengganti VCC/5vOLT
#define r1 8

//Rule Base
float suhu [2];
float kelembaban [2];
float cahaya [2];
float rule [2][2][2];
float rule000, rule001, rule010, rule011, rule100, rule101, rule110, rule111 ;
float defuz, pwm, defuzzy;
float temp, minsuhuKelem;
float sensorKelembaban;
 float sensorSuhu;
 float sensorCahaya;



void setup(){
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight(); 
  pinMode(pin_ldr, INPUT);
  // jadikan pin power sebagai output
  pinMode(powerPin, OUTPUT);
  // default bernilai LOW
  digitalWrite(powerPin, LOW);
   Serial.begin(9600);
   dht.begin();
   pinMode(r1, OUTPUT);
   motorServo.attach(10);
}
 
void loop(){
 digitalWrite(powerPin, HIGH);

float sensorSuhu = dht.readTemperature();
float sensorKelembaban = dht.readHumidity();

int nilai = analogRead (pin_ldr); //Membaca nilai analog dari pin A0
//delay (2000); //jeda selama dua detik
sensorCahaya=(250.000000/(ADC_value*nilai))-50.000000;

// sensorSuhu = 31.00;
// sensorKelembaban = 90.00 ;
// sensorCahaya = 0.1;

//  fuzzifikasi
 FuzzySuhu(sensorSuhu);
 FuzzyKelembaban(sensorKelembaban);
 FuzzyCahaya(sensorCahaya);

// defuzzifikasi
 Defuzzy();
// untuk print ke serial monitor
// Serial.print("k: ");
// Serial.print(sensorKelembaban);
// Serial.print(" ");
// Serial.print("s: ");
// Serial.println(sensorSuhu);
// Serial.print("c: ");
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
  lcd.print("C: ");
  lcd.setCursor(2,1);
  lcd.print(sensorCahaya);
  lcd.setCursor(8,1);
  lcd.print("F: ");
  lcd.setCursor(10,1);
  lcd.print(defuzzy);

  delay(1000); // tunggu semalam 1 detik

//  servo
motorServo.write(defuzzy);  // Turn Servo ke kiri 45 degrees
//   delay(1000);          
//   motorServo.write(0);   // Turn Servo ke kiri to 0 degrees
//   delay(1000);          
//   motorServo.write(90);  // Turn Servo ke posisi center position (90 degrees)
//   delay(1000);          
//   motorServo.write(135); // Turn Servo Ke kanan 135 degrees
//   delay(1000);          
//   motorServo.write(180); // Turn Servo ke kanan 180 degrees
//   delay(1000);          
//   motorServo.write(90);  // Turn Servo ke posisi center position (90 degrees)
//   delay(1000);      

}


void FuzzySuhu(float sensorSuhu){
  // untuk suhu hampir panas
  if (sensorSuhu <= 27.00)
  { suhu [0] = 1;}
  else if (sensorSuhu > 27.00 && sensorSuhu <= 30.00)
  {  suhu [0] = (30.00 - sensorSuhu)/(30.00 - 27.00); }
  else
  { suhu [0] = 0;}

  // untuk suhu panas
  if (sensorSuhu <= 27.00)
  { suhu [1] = 0;}
  else if (sensorSuhu > 27.00 && sensorSuhu <= 30.00)
  { suhu [1] = (sensorSuhu-27.00)/(30.00-27.00);}
  else
  { suhu [1] = 4;}

  Serial.print("suhu 0: ");
  Serial.println(suhu[0]);
    Serial.print("suhu 1: ");
  Serial.println(suhu[1]);

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
//  Serial.print("kelembaban functions: ");
//  Serial.println(sensorKelembaban);
}

void FuzzyCahaya(int sensorCahaya){
  // untuk hampir panas
  if (sensorCahaya <= 0.35)
  { cahaya [0] = 1;}
  else if (sensorCahaya > 0.35 && sensorCahaya <= 1.00)
  {  cahaya [0] = (1.00 - sensorCahaya)/(1.00 - 0.35); }
  else
  { cahaya [0] = 0;}

  // untuk panas
  if (sensorCahaya <= 0.35)
  { cahaya [1] = 0;}
  else if (sensorCahaya > 0.35 && sensorCahaya <= 1.00)
  { cahaya [1] = (sensorCahaya-0.35)/(1.00 - 0.35);}
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
 for ( i=0; i<=1; i=i+1)
 {
   for ( j=0; j<=1; j=j+1)
   {
      for ( c=0; c<=1; c=c+1)
      {
         minsuhuKelem = min(suhu[i], kelembaban[j]);
         temp = min(minsuhuKelem, cahaya[c]); 
         rule [i][j][c] = temp;
      }
   } 
 } 
 rule000 = rule [0][0][0]; // (hampir panas,hampir panas, hampir panas = )
 rule001 = rule [0][0][1]; // (hampir panas,hampir panas, panas = )
 
 rule010 = rule [0][1][0]; // (hampir panas,panas, hampir panas = )
 rule011 = rule [0][1][1]; // (hampir panas,panas, panas = )
 
 rule100 = rule [1][0][0]; // (panas,hampir panas, hampir panas = )
 rule101 = rule [1][0][1]; // (panas,hampir panas, panas = )

 rule110 = rule [1][1][0]; // (panas, panas, hampir panas = )
 rule111 = rule [1][1][1]; // (panas,panas, panas = )
 
}

void Defuzzy () {
  // metode sugeno (weighted average)
  RuleEva();
  pwm = (rule000 * 30) + (rule001 * 40)+ (rule010 * 45)+ (rule011 * 60) + (rule100 * 75) + (rule101 * 80)+ (rule110 * 85)+ (rule111 * 90);
 
  defuz = 0;
  int i, j, c;
  for ( i=0; i<=1; i=i+1)
  {
    for ( j=0; j<=1; j=j+1)
    {
      for ( c=0; c<=1; c=c+1)
      {
        defuz = defuz + rule [i][j][c];
      }
    } 
  } 
  defuzzy = pwm / defuz;
//  Serial.print("defuz: ");
//  Serial.println(defuz);
}
