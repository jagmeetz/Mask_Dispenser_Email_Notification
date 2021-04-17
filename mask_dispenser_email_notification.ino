//********************Libraries********************//
#include "ESP32_MailClient.h"
#include <EEPROM.h>
//********************Libraries********************//

//********************PINs********************//
#define LED 2
#define MotorIn1 26
#define MotorIn2 27
#define button_pin 32
#define echoPin 12
#define trigPin 13
//********************PINs********************//

//********************Wifi********************//
const char* ssid = "Hotspot";
const char* password = "ambanikuta";
//********************Wifi********************//

//********************EEPROM********************//
#define EEPROM_SIZE 2
//********************EEPROM********************//

//********************Email Settings********************//
#define emailSenderAccount    "maskdispensergndec@gmail.com"
#define emailSenderPassword   "m@skd1spenser"
#define emailRecipient        "jagmeettakkar.33@gmail.com"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "[Mask Dispenser] Notification"
//********************Email Settings********************//

#define maxMasks              10
int masksLeft = 10;
long duration;
float Distance;
bool lock = 0;
SMTPData smtpData;

//********************Interrupt********************//
void IRAM_ATTR interrupt_handler()
{

  masksLeft = maxMasks;
  lock = 0;
}
//********************Interrupt********************//

//********************Setup********************//
void setup()
{
  //********************Pin Settings********************//

  pinMode (LED, OUTPUT);
  pinMode (MotorIn1, OUTPUT);
  pinMode (MotorIn2, OUTPUT);
  pinMode (button_pin, INPUT_PULLUP);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  attachInterrupt(button_pin, interrupt_handler, HIGH);

  //********************Serial Communication Setup********************//
  Serial.begin(115200);

  //********************Wifi Connection********************//
  Serial.print("Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("WiFi connected.");

  //********************EEPROM Setup********************//
  EEPROM.begin(EEPROM_SIZE);
  masksLeft = EEPROM.read(0);
  lock = EEPROM.read(1);

  //********************Email Service Setup********************//
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  smtpData.setSender("MaskDispenserGNDEC", emailSenderAccount);
  smtpData.setPriority("Normal");
  smtpData.setSubject(emailSubject);
  smtpData.setMessage("<div style=\"color:#2f4468;\"><h1>Mask Dispenser is out of masks</h1><p>Kindly refill. Process to refill can be seen from this video ,link,</p></div>", true);

  //********************Email Recipients********************//
  smtpData.addRecipient(emailRecipient);
  //smtpData.addRecipient("other_recipient@EXAMPLE.com");

  smtpData.setSendCallback(sendCallback);
}
//********************Setup********************//


//********************Functions********************//
float ultrasonic_Distance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  Distance = duration * 0.034 / 2;
  return Distance;
}

void eeprom_write()
{
  if (EEPROM.read(0) != masksLeft) {
    EEPROM.write(0, masksLeft);
    EEPROM.commit();
    Serial.println("Mask number saved in flash memory");
  }
    if (EEPROM.read(1) != lock) {
    EEPROM.write(1, lock);
    EEPROM.commit();
    Serial.println("Lock");
  }
}
//********************Functions********************//


//********************Loop********************//
void loop()
{
  eeprom_write();
  while (lock)
  {
    Serial.println("Dispenser Empty");
  }
  
  float dist;
  dist = ultrasonic_Distance();

  if (dist <= 5.0)
  {
    digitalWrite(LED, HIGH); // LED High
    digitalWrite(MotorIn1, HIGH);
    digitalWrite(MotorIn2, LOW);
    delay(500);
    digitalWrite(LED, LOW);
    digitalWrite(MotorIn1, LOW);
    digitalWrite(MotorIn2, LOW);
    masksLeft--;
    Serial.println("___________________________________________");
    Serial.println(masksLeft);
    if (masksLeft == 0)
    {
      if (!MailClient.sendMail(smtpData))
        Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
      smtpData.empty();
      lock = 1;
    }

    eeprom_write();

    delay(2500);
  }

}
//********************Loop********************//


//********************Callback to get status of Email********************//
void sendCallback(SendStatus msg) {
  Serial.println(msg.info());

  if (msg.success()) {
    Serial.println("----------------");
  }
}
//********************Callback to get status of Email********************//
