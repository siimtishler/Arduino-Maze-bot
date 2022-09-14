#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* Definitsioonid ------------------------------------------------------------------ */
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

/* Globaalmuutujad -------------------------------------------------------------- */
Servo g_left_wheel;
Servo g_right_wheel;
Servo ultra_servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long g_last_debounce_time = 0;   // Viimane aeg kui väljund aktiivne
unsigned long g_debounce_delay = 50;      // Debounce aeg
int g_button_state;                       // Sisendi praegune väärtus
int g_last_button_state = LOW;            // Viimane nupu seisund
int nupp_kordaja, nupp_paaris;            // Surunupu abimuutujad
int l_kaugus;                             // Läbitud kauguse abimuutuja
int x = 0, y = 0, z = 0;                  // Koordinaatide muutujad
int nool_x = 5, nool_y = 0;               // Noolte kuvamise asukoha abimuutujad
int lcd_kast = 0, lcd_rida = 0, lcd_bit;  // Abimuutujad kaardi kuvamiseks LCD-le


/* Privaatsed funktsioonid ----------------------------------------------------------------------------------------------- */
byte readQti (byte qti) {                                         // QTI sensori funktsioon labürindi lõpu tuvastamiseks
  digitalWrite(qti, HIGH);                                        // Saadab infrapunasignaali
  delayMicroseconds(1000);                                        // Ootab 1 ms
  digitalWrite(qti, LOW);                                         // Lõpetab saatmise
  return ( analogRead(qti) > qti_threshold ? 1 : 0);              // Tagastab väärtuse
}

void setWheels(int delay_left = 1500, int delay_right = 1500) {   //Rataste servode funktsioon 
  g_left_wheel.writeMicroseconds(delay_left);
  g_right_wheel.writeMicroseconds(delay_right);
}

void setKesk(int ultraservo = 1450) {                             // Keskmine servo ehk ultraheli servo funtksioon
  ultra_servo.writeMicroseconds(ultraservo);
}

void setLed(byte value_left = LOW, byte value_right = LOW) {      // Ledide funktsioon
  digitalWrite(right_led, value_right);
  digitalWrite(left_led, value_left);
}

byte p[8][8] = {{0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}, // Custom char maatriks 8 sümbolit
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},
};

/* Arduino funktsioonid ---------------------------------------------------------------------------------------------------- */
void setup() {

  Serial.begin(9600);

  /* Määrab servomootoritele defineeritud klemmid */
  g_left_wheel.attach(left_servo_pin, min_pulse, max_pulse );     // Vasak ratas
  g_right_wheel.attach(right_servo_pin, min_pulse, max_pulse);    // Parem ratas
  ultra_servo.attach(ultra_servo_pin);                            // Ultra heli servo

  /* Ootab, et servod oleks töövalmis */
  delay(10);

  setWheels();                                                    // Rataste servod seisma
  lcd.init();                                                     // LCD sisse
  lcd.backlight();                                                // Taustavalgus sisse
}

/* Liikumise funktsioonid ---------------------------------------------------------------- */

void vasakule() {                     // Vasakule keeramine
  setWheels(1400, 1400);
  digitalWrite(left_led, HIGH);
  kuva_nool(1);                       // Kuvatakse "<"
}

void paremale() {                     //Paremale keeramine
  setWheels(1600, 1600);
  digitalWrite(right_led, HIGH);
  kuva_nool(2);                       // Kuvatakse ">"
}

void edasi() {                        //Edasi sõitmine
  setWheels(1600, 1435);
  l_kaugus++;                         //Suurendame läbitud kauguse muutujat
}

void seisajoonel() {                  // Joonel seismine
  setLed(LOW, LOW);
  delay(1000);
  setWheels(1500, 1500);
  exit(0);                            // Kui seisajoonel funktsioonini jõutakse on labürint läbitud
}                                     // ning väljutakse programmist

/* Ultraheli funktsioonid --------------------------------------------------------------------------- */

float ultraheli() {                   // Kauguse lugemine ultrahelisensoriga
  float duration, u_kaugus;
  pinMode(UltraH, OUTPUT);
  digitalWrite(UltraH, LOW);
  delayMicroseconds(2);
  digitalWrite(UltraH, HIGH);
  delayMicroseconds(10);
  digitalWrite(UltraH, LOW);

  pinMode(UltraH, INPUT);
  duration = pulseIn(UltraH, HIGH);

  u_kaugus = microsecondsToCentimeters(duration);

  //Serial.print("Kaugus = ");
  //Serial.print(u_kaugus);
  //Serial.println();
  delay(50);
  return u_kaugus;                    // Tagastab mõõdetud kauguse
}

float microsecondsToCentimeters(float microseconds) {      // Heli kiirus on 340 m/s või 29 microsekundit sentimeetri kohta.
  return microseconds / 29 / 2;                            // Signaal liigub objektini ja tagasi, ehk peame võtma poole levinud ajast.
}

/* Ultraheli servomootori funktsioonid ------------------------------------------------------------------------------------------------ */

float UH_vasakule() {
  setKesk(2300);                        // Ultraheli servo vaatab vasakule
  float kokku = 0;
  delay(1000);
  for (int i = 0; i <= 4; i++) {
    Serial.print("Vasakul ");           // Kaugust loetakse 5 korda
    kokku = kokku + ultraheli();        // ning arvutatakse keskmine, et vältida vale lugemist
  }
  float vasak_keskmine = kokku / 5;
  // Serial.print("Vasak keskmine= ");
  //Serial.println(vasak_keskmine);
  //Serial.println();
  return vasak_keskmine;                // Tagastatakse 5 lugemise keskmine kaugus
}

float UH_paremale() {
  setKesk(500);                         // Korratakse samu tegevusi ainult paremale poole vaadates
  float kokku = 0;
  delay(1000);
  for (int i = 0; i <= 4; i++) {
    Serial.print("Paremal ");
    kokku = kokku + ultraheli();
  }
  float parem_keskmine = kokku / 5;
  //Serial.print("Parem keskmine= ");
  //Serial.println(parem_keskmine);
  //Serial.println();
  return parem_keskmine;
}

/* Surunupu funktsioon ---------------------------------------------- */

byte buttonRead() {

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
/* Funktsioon kaardi kuvamiseks LCD-l ------------------------------------------------------------------------------------- */

int displaylcd() {
  
  /* Valib koordinaatide põhjal sümboli kasti */
  
  if (x >= 1 && x <= 5 && y <= 7 && y >= 0) {             // Kuna custom character on 8 kasti ja igas kastis on 5x8 pikslit
    lcd_kast = 0;                                         // siis x ja y koordinaatidega saame selgeks teha, millises kastis
  }                                                       // robot parasjagu asub
  else if (x >= 6 && x <= 10 && y <= 7 && y >= 0) {
    lcd_kast = 1;
  }
  else if (x >= 11 && x <= 15 && y <= 7 && y >= 0) {
    lcd_kast = 2;
  }
  else if (x >= 16 && x <= 20 && y <= 7 && y >= 0) {
    lcd_kast = 3;
  }
  else if (x >= 1 && x <= 5 && y <= -1 && y >= -8) {
    lcd_kast = 4;
  }
  else if (x >= 6 && x <= 10 && y <= -1 && y >= -8) {
    lcd_kast = 5;
  }
  else if (x >= 11 && x <= 15 && y <= -1 && y >= -8) {
    lcd_kast = 6;
  }
  else if (x >= 16 && x <= 20 && y <= -1 && y >= -8) {
    lcd_kast = 7;
  }
  else {
    return 0;
  }
  
  /* Valib koordinaatide põhjal pikslirea kastis */
  
  if (y == 0 || y == -8) {                              // Igas kastis on 8 rida millele vastab kindel bit
    lcd_rida = 7;                                       // Et antud biti väärtust muuta peame teadma, millisel real robot paikneb
  }                                                     // Selleks kasutame ainult y koordinaati.
  if (y == 1 || y == -7) {
    lcd_rida = 6;
  }
  if (y == 2 || y == -6) {
    lcd_rida = 5;
  }
  if (y == 3 || y == -5) {
    lcd_rida = 4;
  }
  if (y == 4 || y == -4) {
    lcd_rida = 3;
  }
  if (y == 5 || y == -3) {
    lcd_rida = 2;
  }
  if (y == 6 || y == -2) {
    lcd_rida = 1;
  }
  if (y == 7 || y == -1) {
    lcd_rida = 0;
  }

  /* Valib koordinaatide põhjal palju bitile liita */
  
  if (x == 1 || x == 6 || x == 11 || x == 16) {       // lcd_bit = 16 on 10000 binary, ehk ainult üks kõige vasakpoolsem piksel põleb
    lcd_bit = 16;
  }
  if (x == 2 || x == 7 || x == 12 || x == 17) {
    lcd_bit = 8;                                      // Liikudes x teljel edasi, lcd_bit = 8, ehk 01000 jne iga ruuduga, kui liidame näiteks
  }                                                   // 16 ja 8 kokku saame, et robot on liikunud 2 x telje ruudu võrra ehk 11000
  if (x == 3 || x == 8 || x == 13 || x == 18) {
    lcd_bit = 4;
  }
  if (x == 4 || x == 9 || x == 14 || x == 19) {
    lcd_bit = 2;
  }
  if (x == 5 || x == 10 || x == 15 || x == 20) {
    lcd_bit = 1;
  }
   
  /* Muudab biti väärtust massiivis ja loob uued custom sümbolid */
  
  p[lcd_kast][lcd_rida] += lcd_bit;                // Kasutame leitud muutujaid, et kindlat pikslit muuta
                                                   
  lcd.createChar(0, p[0]);                         // Pärast muutmist peame sümbolid uuesti looma, sest üks neist on muutunud
  lcd.createChar(1, p[1]);
  lcd.createChar(2, p[2]);
  lcd.createChar(3, p[3]);
  lcd.createChar(4, p[4]);
  lcd.createChar(5, p[5]);
  lcd.createChar(6, p[6]);
  lcd.createChar(7, p[7]);

  /* Valib koordinaatide põhjal millisesse kasti kirjutada */

  if (x >= 1 && x <= 5 && y <= 7 && y >= 0) {     // Peale sümbolite loomist on vaja ka kindel sümbol LCD-le kindla koha peale kuvada 
    lcd.setCursor(0, 0);                          // Selleks kasutame jälle koordinaate
    lcd.write(0);                                 // Kaardi jaoks on LCD-l määratud 4x2 sümboli suurune ala
  }
  if (x >= 6 && x <= 10 && y <= 7 && y >= 0) {
    lcd.setCursor(1, 0);
    lcd.write(1);
  }
  if (x >= 11 && x <= 15 && y <= 7 && y >= 0) {
    lcd.setCursor(2, 0);
    lcd.write(2);
  }
  if (x >= 16 && x <= 20 && y <= 7 && y >= 0) {
    lcd.setCursor(3, 0);
    lcd.write(3);
  }
  if (x >= 1 && x <= 5 && y <= -1 && y >= -8) {
    lcd.setCursor(0, 1);
    lcd.write(4);
  }
  if (x >= 6 && x <= 10 && y <= -1 && y >= -8) {
    lcd.setCursor(1, 1);
    lcd.write(5);
  }
  if (x >= 11 && x <= 15 && y <= -1 && y >= -8) {
    lcd.setCursor(2, 1);
    lcd.write(6);
  }
  if (x >= 16 && x <= 20 && y <= -1 && y >= -8) {
    lcd.setCursor(3, 1);
    lcd.write(7);
  }
}

/* Funktsioon noolte kuvamiseks LCD-l ------------------------------------------------------------------------------------- */

int kuva_nool(int a) {
  if (nool_x > 15) {              // Kui LCD esimene rida on nooli täis saanud vahetutakse teise rea algusesse
    nool_x = 5;                   // Noolte jaoks on LCD-l määratud 12x2 sümboli suurune ala
    nool_y = 1;
  }

  lcd.setCursor(nool_x, nool_y);  
  if (a == 1) {                   // Sooritati vasakpööre
    lcd.print("<");
  }

  if (a == 2) {
    lcd.print(">");               // Sooritati paremkpööre
  }
  nool_x++;                       // liidetakse 1, et järgmine nool läheks järgmisesse kasti
}

/* Main loop ------------------------------------------------------------------------------------------------------------------------- */

void loop() {           // Roboti liikumise põhimõtted ja kaardistamine:
                        // Robot hakkab nupu vajutusel liikuma, otse sõites suureneb x väärtus koordinaatteljestikul
                        // Kui robot pöörab, muudetakse vastavalt z väärtust, et teada kummale poole robot pöörab
                        // See on vajalik, et teaksime millal suurendada y väärtust

  if (z == 3) {         // Robot on näiteks teinud 3 parempööret, ehk sõidab y telje suhtes ülespoole
    z = -1;
  }
  if (z == -3) {
    z = 1;
  }


  if (buttonRead()) {       // Iga nupuvajutus lisatakse muutujale üks
    nupp_kordaja++;
  }

  nupp_paaris = nupp_kordaja % 2;

  if (nupp_paaris == 0) {          // Kui muutuja paarisarv siis robot seisab
    setLed(LOW, LOW);
    setWheels(1500, 1500);
  }

  else {
    if (readQti(left_qti) && readQti(right_qti)) {      // Kui robot sõidab üle mustajoone jääb ta seisma ja väljub programmist, sest labürint on läbitud
      seisajoonel();
    }

    if (ultraheli() >= 10) {                            // uh() on võrdne scannitud kaugusega, kui robot on seinast kaugemal kui 10cm, sõidab ta edasi
      edasi();
      //Serial.println(o);
      if (l_kaugus % 14 == 0) {                         // edasi() funktsioonis suurendatakse l_kaugust, ning mõõtsime, et kui l_kaugus = 14, siis on robot liikunud umbes
        if (z == 0) {                                   // 10 cm, selle järgi muudame, koordinaatteljestikul x või y väärtust vastavalt z väärtusele
          x++;
        }
        if (z == -1) {                                  // Robot on pööranud vasakule, suurendame y väärtust
          y++;
        }
        if (z == 1) {                                   // Robot on pööranud paremale, vähendame y väärtust
          y--;
        }
        if (z == 2 || z == -2) {                        // Robot on teinud kaks paremat või vasakut pööret vähendame x väärtust
          x--;
        }
        displaylcd();                                   // Kutsume välja displaylcd() iga x või y väärtuse muutumisel, et kaardistada liikumist
        l_kaugus = 0;
      }
    }
    else {                                              // Kui robot on jõudnud seinani, jääb ta seisma
      setWheels();
      delay(100);

      if (UH_vasakule() > UH_paremale()) {              // Robot pöörab vasakule, sest vasak sein on kaugemal
        setLed(HIGH, LOW);
        vasakule();
        z--;                                            // Kordinaadi z muutuja aitab tuvastada, mis pidi robot vaatab ning selle järgi oskame suurendada, kas x või y suurust
        setKesk();
        delay(870);

      }
      else {                                            // Robot pöörab paremale, sest parem sein on kaugemal
        setLed(LOW, HIGH);
        paremale();
        z++;                                              
        setKesk();
        delay(830);
      }
    }
  }
}
