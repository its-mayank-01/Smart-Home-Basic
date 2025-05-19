#include <Wire.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>
#include <PIR.h>

#define BLYNK_TEMPLATE_ID "TMPL3je9vHvns"
#define BLYNK_TEMPLATE_NAME "Smart Home"
#define BLYNK_AUTH_TOKEN "k-YGdsG2KNyG_lzEYR_tSOznIKgkgyN7"

#include <BlynkApiArduino.h>
#include <BlynkSimpleStream.h>
#include <Blynk.h>
BlynkTimer timer;

#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(2,3);
SoftwareSerial gsmSerial(19,18);

int var1, var2, var3, var4;
const char *online_pswd = "";
char auth[] = BLYNK_AUTH_TOKEN;

BLYNK_WRITE(V1) {
  var1 = param.asInt();
}

BLYNK_WRITE(V2) {
  var2 = param.asInt();
}

BLYNK_WRITE(V3) {
  var3 = param.asInt();
}

BLYNK_WRITE(V4) {
  var4 = param.asInt();
}

// BLYNK_WRITE(V0) {
//   online_pswd = param.asString();
// }

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = { { '1', '2', '3' }, { '4', '5', '6' }, { '7', '8', '9' }, { '*', '0', '#' } };
byte rowPins[ROWS] = { 22, 23, 24, 25 };
byte colPins[COLS] = { 26, 27, 28 };
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal lcd_1(2, A1, 4, 5, 6, 7);
LiquidCrystal lcd_2(9, A2, 10, 11, 12, 13);

const int pswd_length = 5;
char pswd[pswd_length];
char org_pswd[pswd_length] = "1234";
byte count = 0;

Servo servo_1;
Servo servo_2;

int doorPIR = 33;
int rpinDoor = 34;

int bedroomPIR = 50;
int rpinBedroom = 49;
int bedroomFlameSensor = 48;

int kitchenPIR = 47;
int rpinKitchen = 46;
int kitchenFlameSensor = 45;

int hallPIR = 44;
int rpinHall = 43;
int hallFlameSensor = 42;

int t=5;

int buzzer = 40;
int fire_led = 39;
int waterMotor = 38;

String msg1 = "Emergency! Fire broke out in your house. Rush!\nFIRE FIRE FIRE";
String msg2 = "Emergency! Fire broke out in my house. Plz Help! \n Address- Aryabhatta Hostel, IIT(BHU) Varanasi";

void setup() {
  Serial.begin(9600);
  delay(10);
  lcd_1.begin(16, 2);
  lcd_2.begin(20, 4);
  delay(10);
  DebugSerial.begin(9600);
  gsmSerial.begin(9600);
  delay(10);
  Blynk.begin(Serial, auth);
  delay(10);
  timer.setInterval(500L,smart_home);

  lcd_2.print("Good to see you.");
  lcd_2.setCursor(0,1);
  lcd_2.print("Have a good day.");

  servo_1.attach(30);
  servo_2.attach(31);

  pinMode(doorPIR, INPUT);
  pinMode(bedroomPIR, INPUT);
  pinMode(kitchenPIR, INPUT);
  pinMode(hallPIR, INPUT);

  pinMode(rpinDoor, OUTPUT);
  pinMode(rpinBedroom, OUTPUT);
  pinMode(rpinKitchen, OUTPUT);
  pinMode(rpinHall, OUTPUT);

  pinMode(bedroomFlameSensor, INPUT);
  pinMode(kitchenFlameSensor, INPUT);
  pinMode(hallFlameSensor, INPUT);

  digitalWrite(rpinDoor, HIGH);
  digitalWrite(rpinBedroom, HIGH);
  digitalWrite(rpinKitchen, HIGH);
  digitalWrite(rpinHall, HIGH);

  pinMode(buzzer,OUTPUT);
  pinMode(fire_led, OUTPUT);
  pinMode(waterMotor, OUTPUT);
  digitalWrite(buzzer, LOW);
  digitalWrite(fire_led, LOW);
  digitalWrite(waterMotor, LOW);

}

void loop()
{
  if(Blynk.connected()) Blynk.run();
  else Blynk.begin(Serial, auth);
  timer.run();
}

void smart_home()
{

  if( digitalRead(doorPIR) == HIGH ) {
    digitalWrite(rpinDoor, LOW);
  } else {
    digitalWrite(rpinDoor, HIGH);
  }
  lcd_1.setCursor(0, 0);
  lcd_1.print("Enter Password: ");
  delay(100);

  // if(!strcmp(org_pswd, online_pswd)) {
  //   openGate();
  // }

  char pressedKey = customKeypad.getKey();
  if (pressedKey) {
    pswd[count] = pressedKey;
    lcd_1.setCursor(count, 1);
    lcd_1.print("*");
    count++;
  }

  if (count == pswd_length - 1) {
    lcd_1.clear();
    if (!strcmp(pswd, org_pswd)) {
      openGate();
    } 
    else {
      lcd_1.setCursor(4, 0);
      lcd_1.print("INCORRECT");
      lcd_1.setCursor(4, 1);
      lcd_1.print("PASSWORD");
      delay(500);
    }
    lcd_1.clear();
    clearData();
  }

  turn_on_lights();
  if( digitalRead(bedroomFlameSensor) || digitalRead(kitchenFlameSensor) || digitalRead(hallFlameSensor) || var4 ) {
    fire_alarm();
  }

}

void clearData() {
  while (count != 0) {
    pswd[count--] = 0;
  }
  return;
}

void openGate() {
  lcd_1.setCursor(3, 0);
  lcd_1.print("HELLO DEAR");
  lcd_1.setCursor(2, 1);
  lcd_1.print("WELCOME HOME");
  delay(500);
  servo_1.write(180);
  servo_2.write(180);
  delay(10000);
  servo_1.write(0);
  servo_2.write(0);
}

void turn_on_lights() {
  if ((digitalRead(bedroomPIR) == HIGH) || var1) {
    lcd_2.clear();
    lcd_2.setCursor(2, 0);
    lcd_2.print("Person is in");
    lcd_2.setCursor(4, 1);
    lcd_2.print("Bedroom");

    digitalWrite(rpinBedroom, LOW);
    delay(1000);
    while ((digitalRead(bedroomPIR) == HIGH) || var1) {
      if( digitalRead(bedroomFlameSensor) || digitalRead(kitchenFlameSensor) || digitalRead(hallFlameSensor) || var4) {
        fire_alarm();
      }
      delay(500);
    }
    digitalWrite(rpinBedroom, HIGH);
    lcd_2.clear();
  }

  if ((digitalRead(kitchenPIR) == HIGH) || var2) {
    lcd_2.clear();
    lcd_2.setCursor(2, 0);
    lcd_2.print("Person is in");
    lcd_2.setCursor(4, 1);
    lcd_2.print("Kitchen");

    digitalWrite(rpinKitchen, LOW);
    delay(1000);
    while ((digitalRead(kitchenPIR) == HIGH) || var2) {
      if( digitalRead(bedroomFlameSensor) || digitalRead(kitchenFlameSensor) || digitalRead(hallFlameSensor) || var4) {
        fire_alarm();
      }
      delay(500);
    }
    digitalWrite(rpinKitchen, HIGH);
    lcd_2.clear();
  }

  if ((digitalRead(hallPIR) == HIGH) || var3) {
    lcd_2.clear();
    lcd_2.setCursor(2, 0);
    lcd_2.print("Person is in");
    lcd_2.setCursor(6, 1);
    lcd_2.print("Hall");

    digitalWrite(rpinHall, LOW);
    delay(1000);
    while ((digitalRead(hallPIR) == HIGH) || var3) {
      if( digitalRead(bedroomFlameSensor) || digitalRead(kitchenFlameSensor) || digitalRead(hallFlameSensor) || var4) {
        fire_alarm();
      }
      delay(500);
    }
    digitalWrite(rpinHall, HIGH);
    lcd_2.clear();
  }
}

void fire_alarm()
{
  // int fire_in_bedroom = digitalRead(bedroomFlameSensor);
  // int fire_in_kitchen = digitalRead(kitchenFlameSensor);
  // int fire_in_hall = digitalRead(hallFlameSensor);
  // int fire_detected = (fire_in_bedroom || fire_in_kitchen || fire_in_hall);
  sendMessage("+919548076825", msg1);
  sendMessage("+916395140156", msg2);
  digitalWrite(waterMotor, HIGH);
  lcd_2.clear();
  while( digitalRead(bedroomFlameSensor) || digitalRead(kitchenFlameSensor) || digitalRead(hallFlameSensor) || var4 ) {
    digitalWrite(fire_led, HIGH);
    digitalWrite(buzzer, HIGH);
    lcd_2.setCursor(6,0);
    lcd_2.print("ALERT");
    lcd_2.setCursor(1,1);
    lcd_2.print("FIRE FIRE FIRE");
    delay(1000);
    digitalWrite(fire_led, LOW);
    lcd_2.clear();
    delay(200);
    digitalWrite(fire_led, HIGH);
    lcd_2.print("Putting out fire");
    delay(1000);
    digitalWrite(fire_led, LOW);
    lcd_2.clear();
    delay(200);
    digitalWrite(fire_led, HIGH);
    lcd_2.print("Calling for HELP");
    delay(1000);
    digitalWrite(fire_led, LOW);
    lcd_2.clear();
    delay(200);
  }
  digitalWrite(waterMotor, LOW);
}

void sendMessage(String number, String msg)
{
  String mnumber = "AT+CMGS=\""+number+"\"\r";
  gsmSerial.println("AT+CMGF=1");
  delay(500);
  gsmSerial.println(mnumber);
  delay(500);
  gsmSerial.println(msg);
  delay(100);
  gsmSerial.println((char)26);
  delay(500);
}
