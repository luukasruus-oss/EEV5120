
/*
 * AUTOMAATNE KAABLIRULL
 * Kaabli pikkuse mõõtmine ja automaatne tagasikerimine
 * Mikrokontroller: Arduino Uno
 */

#include <LiquidCrystal.h>

// Ühendusviigud
const int ANDUR = 2;
const int NUPP = 3;
const int LOPPLULITI = 4;
const int MOOTOR = 5;
const int SUMISTI = 6;

// LCD-ekraan
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Seadistatavad parameetrid
const float MAX_PIKKUS = 10.0;
const float RATTA_UMBERMOOT = 0.20;
const int IMPULSSE_POORDEL = 20;
const unsigned long MAX_KERIMISAEG = 30000;

// Muutujad
volatile long impulsid = 0;
volatile bool kerimine = false;

float pikkus = 0.0;
bool viga = false;
unsigned long kerimiseAlgus = 0;

// Pöörlemisanduri impulsside loendamine
void loendaImpulss() {
  if (kerimine) {
    if (impulsid > 0) {
      impulsid--;
    }
  } else {
    impulsid++;
  }
}

// 1. Süsteemi lähtestamine
void setup() {
  digitalWrite(MOOTOR, LOW);

  pinMode(ANDUR, INPUT_PULLUP);
  pinMode(NUPP, INPUT_PULLUP);
  pinMode(LOPPLULITI, INPUT_PULLUP);
  pinMode(MOOTOR, OUTPUT);
  pinMode(SUMISTI, OUTPUT);

  digitalWrite(SUMISTI, LOW);

  lcd.begin(16, 2);
  Serial.begin(9600);

  impulsid = 0;

  attachInterrupt(
    digitalPinToInterrupt(ANDUR),
    loendaImpulss,
    FALLING
  );

  lcd.print("Kaablirull");
  delay(1000);
  lcd.clear();
}

// Põhiprogramm
void loop() {

  // 2. Loe pöörlemisanduri impulsid
  noInterrupts();
  long loendur = impulsid;
  interrupts();

  // 3. Arvuta kaabli pikkus meetrites
  pikkus = loendur * RATTA_UMBERMOOT
           / IMPULSSE_POORDEL;

  // 4. Kuva pikkus ekraanil
  lcd.setCursor(0, 0);
  lcd.print("L: ");
  lcd.print(pikkus, 2);
  lcd.print(" m       ");

  Serial.print("Kaabli pikkus: ");
  Serial.println(pikkus);

  // 5. ja 6. Kontrolli lubatud pikkust
  // ning anna vajadusel hoiatussignaal
  if (pikkus > MAX_PIKKUS || viga) {
    digitalWrite(SUMISTI, HIGH);
  } else {
    digitalWrite(SUMISTI, LOW);
  }

  bool kaabelTagasi =
    digitalRead(LOPPLULITI) == LOW;

  // 7. Kontrolli tagasikerimise nuppu
  // 8. Käivita kerimismootor
  if (digitalRead(NUPP) == LOW &&
      !kerimine && !kaabelTagasi && !viga) {

    kerimine = true;
    kerimiseAlgus = millis();
    digitalWrite(MOOTOR, HIGH);
  }

  // 9. Kontrolli, kas kaabel on tagasi
  // 10. Peata kerimismootor
  if (kaabelTagasi) {
    digitalWrite(MOOTOR, LOW);

    noInterrupts();
    kerimine = false;
    impulsid = 0;
    interrupts();
  }

  // Kaitse liiga pika kerimise eest
  if (kerimine &&
      millis() - kerimiseAlgus >= MAX_KERIMISAEG) {

    digitalWrite(MOOTOR, LOW);
    kerimine = false;
    viga = true;
  }

  // Süsteemi oleku kuvamine
  lcd.setCursor(0, 1);

  if (viga) {
    lcd.print("MOOTORI VIGA    ");
  } else if (kerimine) {
    lcd.print("Kerimine...     ");
  } else if (kaabelTagasi) {
    lcd.print("Kaabel tagasi   ");
  } else if (pikkus > MAX_PIKKUS) {
    lcd.print("HOIATUS!        ");
  } else {
    lcd.print("Mootmine...     ");
  }

  delay(20);
}
