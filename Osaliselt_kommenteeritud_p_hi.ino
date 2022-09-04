#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

/* Defines ------------------------------------------------------------------ */
#define button_pin      2
#define right_servo_pin 5
#define left_servo_pin  6
#define right_led       7
#define left_led        8
#define ultra_servo_pin 10
#define UltraH          11
#define min_pulse       1300
#define max_pulse       1700
#define standstill      1500
#define left_qti        A0
#define middle_qti      A1
#define right_qti       A2
#define qti_threshold   600 
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* Global variables --------------------------------------------------------- */
Servo g_left_wheel;
Servo g_right_wheel;
Servo ultra_servo; 
int f;
unsigned long g_last_debounce_time = 0;   // the last time the output pin was toggled
unsigned long g_debounce_delay = 50;      // the debounce time; increase if the output flickers
int g_button_state;                       // the current reading from the input pin
int g_last_button_state = LOW;  

int k, v;                                 // nupu abimuutujad

int o;                                    // custom char koordinaatide abimuutujad
int x;
int y;                                            
int z=0;
int j=5, b=0;
int t=0, d=0, r;


/* Private functions ---------------------------------------------------------------- */
byte readQti (byte qti) {                               // Function to read current position on map
  digitalWrite(qti, HIGH);                              // Send an infrared signal
  delayMicroseconds(1000);                              // Wait for 1ms, very important!
  digitalWrite(qti, LOW);                               // Set the pin low again
  return ( analogRead(qti) > qti_threshold ? 1 : 0);    // Return the converted result: if analog value more then 100 return 1, else 0
}

void setWheels(int delay_left = 1500, int delay_right = 1500) {
  g_left_wheel.writeMicroseconds(delay_left);
  g_right_wheel.writeMicroseconds(delay_right);
}

void setKesk(int ultraservo=1450){                       // Keskmine servo ehk Ultra heli servo funtksioon 
  ultra_servo.writeMicroseconds(ultraservo);
}

void setLed(byte value_left = LOW, byte value_right = LOW) {
  digitalWrite(right_led, value_right);
  digitalWrite(left_led, value_left);
}

byte p[8][8] = {{0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, 0b00000, 0b00000},    // Custom char maatriks 8 sümbolit
                {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, 0b00000, 0b00000},
                {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, 0b00000, 0b00000},
                {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, 0b00000, 0b00000},
                {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, 0b00000, 0b00000},
                {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, 0b00000, 0b00000},
                {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, 0b00000, 0b00000},
                {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, 0b00000, 0b00000},
               };

/* Arduino functions ---------------------------------------------------------------- */
void setup() {
  
  Serial.begin(9600);

  /* Attach servos to digital pins defined earlier */
  g_left_wheel.attach(left_servo_pin, min_pulse, max_pulse );     // Vasak ratas
  g_right_wheel.attach(right_servo_pin, min_pulse, max_pulse);    // Parem ratas
  ultra_servo.attach(ultra_servo_pin);                            // Ultra heli servo

  /* Wait to make sure servos were attached */
  delay(10);

  setWheels();
  lcd.init();                                                     // LCD sisse
  lcd.backlight();                                                // Taustavalgus sisse
}

///////////////////////////LIIKUMISE FUNKTSIOONID////////////////////////////////

void vasakule() {
  setWheels(1400, 1400);
  digitalWrite(left_led, HIGH);
  kuva_nool(1);       // Kuvatakse "<"
  }

void paremale() {
  setWheels(1600, 1600);
  digitalWrite(right_led, HIGH);
  kuva_nool(2);       // Kuvatakse ">"
}

void edasi() {
  setWheels(1600, 1435);
  o++;                 //Suurendame abimuutujat o
  }

void seisajoonel(){    // Funktsioon joonel seismiseks
  setLed(LOW, LOW); 
  delay(1000);   
  setWheels(1500,1500);
  exit(0);             // Kui seisajoonel funktsioonini jõutakse on labürint läbitud
  }                    // ning väljutakse programmist

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////ULTRAHELI FUNKTSIOONID//////////////////////////////////

float uh(){
  float duration, kaugus;
  pinMode(UltraH, OUTPUT);
  digitalWrite(UltraH, LOW);
  delayMicroseconds(2);
  digitalWrite(UltraH, HIGH);
  delayMicroseconds(10);
  digitalWrite(UltraH, LOW);

  pinMode(UltraH, INPUT);
  duration = pulseIn(UltraH, HIGH);

 kaugus = microsecondsToCentimeters(duration);
  
  //Serial.print("Kaugus = ");
  //Serial.print(kaugus);
  //Serial.println();
  delay(50);
  return kaugus;
  }
  
float microsecondsToCentimeters(float microseconds) {
  // Heli kiirus on 340 m/s või 29 microsekundit sentimeetri kohta.
  // Signaal liigub objektini ja tagasi, ehk peame võtma poole levinud ajast.
  return microseconds / 29 / 2;
  }

//////////////////////////////////////////////////////////////////////////////////
////////////////////////ULTRAHELI SERVO FUNKTSIOONID//////////////////////////////

float Uvasakule() {
  setKesk(2300);                // Ultraheli servo vaatab vasakule
  float k = 0;
  delay(1000);
  for(int i = 0; i<=4; i++){    
    Serial.print("Vasakul ");   // Kaugust loetakse 5 korda
    k = k + uh();               // ning arvutatakse keskmine, et vältida vale lugemist
    }
  float vasakk = k/5;          
 // Serial.print("Vasak keskmine= ");
  //Serial.println(vasakk);
  //Serial.println();
  return vasakk;                // Tagastatakse 5 lugemise keskmine kaugus
}

float Uparemale() {
  setKesk(500);                 // Korratakse samu tegevusi ainult paremale poole vaadates
  float k = 0;
  delay(1000);
  for(int i = 0; i<=4; i++){
    Serial.print("Paremal ");
    k = k + uh();
    }
  float paremk = k/5;
  //Serial.print("Parem keskmine= ");
  //Serial.println(paremk);
  //Serial.println();
  return paremk;
  }
  
/////////////////////////////////////////////////////////////////////////////////
///////////////////////////SURUNUPUVAJUTUS FUNKTSIOON////////////////////////////

byte buttonRead(){ 

  int reading = digitalRead(button_pin);
  if (reading != g_last_button_state) {
    g_last_debounce_time = millis();
  }
  if ((millis() - g_last_debounce_time) > g_debounce_delay) {
    if (reading != g_button_state) {
      g_button_state = reading;
      if (g_button_state == HIGH) {                           
        return 1;
      }
    }
  }
  g_last_button_state = reading;
  return 0;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////LCD KAARDI KUVAMISE FUNKTSIOONID////////////////////////////

int displaylcd(){
  if(x>=1 && x<=5 && y<=7 && y>=0){               // Kuna custom character on 8 kasti ja igas kastis on 5x8 pikslit
    t=0;                                          // siis x ja y koordinaatidega saame selgeks teha, millises kastis
  }                                               // me parasjagu asume
  else if(x>=6 && x<=10&& y<=7 && y>=0){
    t=1;
  }
  else if(x>=11 && x<=15&& y<=7 && y>=0){
    t=2;
  }
  else if(x>=16 && x<=20&& y<=7 && y>=0){
    t=3;
  }
  else if(x>=1 && x<=5 && y<=-1 && y>=-8){
    t=4;
  }
  else if(x>=6 && x<=10 && y<=-1 && y>=-8){
    t=5;
  }
  else if(x>=11 && x<=15 && y<=-1 && y>=-8){
    t=6;
  }
  else if(x>=16 && x<=20 && y<=-1 && y>=-8){
    t=7;
  }
  else{
    return 0;
  }
  if(y==0 || y==-8){                                    //******KOMMENTEERI******
    d=7;
  }
  if(y==1 || y==-7){
    d=6;
  }
  if(y==2 || y==-6){
    d=5;
  }
  if(y==3 || y==-5){
    d=4;
  }
  if(y==4 || y==-4){
    d=3;
  }
  if(y==5 || y==-3){
    d=2;
  }
  if(y==6 || y==-2){
    d=1;
  }
  if(y==7 || y==-1){
    d=0;
  }
  if(x==1 || x==6 || x==11 || x==16){             // r = 16 on 10000 binary, ehk ainult üks kõige vasakpoolsem piksel põleb
    r=16;                                          
  }
  if(x==2 || x==7 || x==12 || x==17){
    r=8;                                          // Liikudes x teljel edasi, r = 8, ehk 01000 jne iga ruuduga, kui liidame näiteks
  }                                               // 16 ja 8 kokku saame, et robot on liikunud 2 x telje ruudu võrra ehk 11000 
  if(x==3 || x==8 || x==13 || x==18){
    r=4;
  }
  if(x==4 || x==9 || x==14 || x==19){
    r=2;
  }
  if(x==5 || x==10 || x==15 || x==20){
    r=1;
  }
  p[t][d]+=r;
  lcd.createChar(0, p[0]);
  lcd.createChar(1, p[1]);
  lcd.createChar(2, p[2]);
  lcd.createChar(3, p[3]);
  lcd.createChar(4, p[4]);
  lcd.createChar(5, p[5]);
  lcd.createChar(6, p[6]);
  lcd.createChar(7, p[7]);
  
  if(x>=1 && x<=5 && y<=7 && y>=0){               //******KOMMENTEERI******
    lcd.setCursor(0,0);
    lcd.write(0);
  }
  if(x>=6 && x<=10&& y<=7 && y>=0){
    lcd.setCursor(1,0);
    lcd.write(1);
  }
  if(x>=11 && x<=15&& y<=7 && y>=0){
    lcd.setCursor(2,0);
    lcd.write(2);
  }
  if(x>=16 && x<=20&& y<=7 && y>=0){
    lcd.setCursor(3,0);
    lcd.write(3);
  }
  if(x>=1 && x<=5 && y<=-1 && y>=-8){
    lcd.setCursor(0,1);
    lcd.write(4);
  }
  if(x>=6 && x<=10 && y<=-1 && y>=-8){
    lcd.setCursor(1,1);
    lcd.write(5);
  }
  if(x>=11 && x<=15 && y<=-1 && y>=-8){
    lcd.setCursor(2,1);
    lcd.write(6);
  }
  if(x>=16 && x<=20 && y<=-1 && y>=-8){
    lcd.setCursor(3,1);
    lcd.write(7);
  } 
}

int kuva_nool(int a){
 if (j>15){
  j=5;                
  b=1;
 }
 
 lcd.setCursor(j, b);
  if (a == 1){          // Sooritati vasakpööre
    lcd.print("<");
  }

  if (a == 2){
    lcd.print(">");     // Sooritati paremkpööre
  }
  ++j;                  // liidetakse 1, et järgmine sümbol läheks järgmisesse kasti
}



/* Infinite loop */
void loop(){            // Roboti liikumise põhimõtted ja kaardistamine:
  int i;                // Robot hakkab nupu vajutusel liikuma, otse sõites suureneb x väärtus koordinaatteljestikul
                        // Kui robot pöörab, muudetakse vastavakt z väärtust, et teada kummale poole robot pöörab
                        // See on vajalik, et teaksime millal suurendada y väärtust
                        
  if(z==3){             // Robot on näiteks teinud 3 parempööret, ehk sõidab y telje suhtes ülespoole
    z=-1;
  }
  if(z==-3){
    z=1;
  }

  
  if (buttonRead()){    // Iga nupuvajutus lisatakse k-le üks
    k++;              
  }
  
  v = k % 2;
  
  if (v == 0){          // Kui K paarisarv siis robot seisab
       setLed(LOW, LOW);
       setWheels(1500,1500);
  }
  
  else{
    if(readQti(left_qti) && readQti(right_qti)){        // Kui robot sõidab üle mustajoone jääb ta seisma ja väljub programmist, sest labürint on läbitud
      seisajoonel();
    }
    if(uh() >= 10){                                     // uh() on võrdne scannitud kaugusega, kui robot on seinast kaugemal kui 10cm, sõidab ta edasi
      edasi();
      //Serial.println(o);
      if(o % 14 == 0){                                  // edasi() funktsioonis suurendatakse o-d, ning mõõtsime, et kui o == 14, siis on robot liikunud umbes
        if(z==0){                                       // 10 cm, selle järgi muudame, koordinaatteljestikul x või y väärtust vastavalt z väärtusele
        x++;
        }
        if(z==-1){                                      // Robot on pööranud vasakule, suurendame y väärtust
        y++;
        }
        if(z==1){                                       // Robot on pööranud paremale, vähendame y väärtust
        y--;
        }
        if(z==2 || z==-2){                              // Robot on teinud kaks paremat või vasakut pööret vähendame x väärtust
        x--;
        }
        displaylcd();                                   // Kutsume välja displaylcd() iga x või y väärtuse muutumisel, et kaardistada liikumist
        o=0;
      }
    }
    else{                                               // Kui robot on jõudnud seinani, jääb ta seisma
    setWheels();
    delay(100);
  
    if(Uvasakule()>Uparemale()){                        // Robot pöörab vasakule, sest vasak sein on kaugemal
      setLed(HIGH, LOW);
      vasakule();
      z--;                                              // Abimuutuja z aitab tuvastada, mis pidi robot vaatab ning selle järgi oskame suurendada, kas x või y suurust
      setKesk();
      delay(870);
 
      } 
      else{                                              // Robot pöörab paremale, sest parem sein on kaugemal
        setLed(LOW, HIGH);
        paremale();
        z++;
        setKesk();
        delay(830);
        }
     }
  }
}
