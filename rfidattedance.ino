#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10   // SDA pin for RFID
#define RST_PIN 9   // Reset pin for RFID
#define SERVO_PIN 3 // Servo motor control pin
#define TRIG_PIN 5  // Trig pin of ultrasonic sensor
#define ECHO_PIN 6  // Echo pin of ultrasonic sensor

MFRC522 rfid(SS_PIN, RST_PIN);
Servo myServo;

byte uid1[] = { 0x23, 0xC, 0x10, 0xC4 };  // Rahul's UID u can change the names according to your prefrence
byte uid2[] = { 0x03, 0x1E, 0x11, 0xC4 }; // Sanjay's UID
unsigned long rahulEntryTime = 0, sanjayEntryTime = 0;
bool rahulInside = false, sanjayInside = false;

byte lastUID[4] = {0, 0, 0, 0};
bool waitingForPerson = false;

bool checkUID(byte *cardUID, byte *authorizedUID) {
    for (byte i = 0; i < 4; i++) {  
        if (cardUID[i] != authorizedUID[i]) {
            return false;
        }
    }
    return true;
}


float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = (duration * 0.0343) / 2; 
    return distance;
}

void setup() {
    Serial.begin(9600);
    SPI.begin();
    rfid.PCD_Init();
    
    myServo.attach(SERVO_PIN);  
    myServo.write(0);  

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    Serial.println("RFID & Ultrasonic Sensor System Ready...");
}

void loop() {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        Serial.print("Card Detected! UID: ");
        for (byte i = 0; i < rfid.uid.size; i++) {
            Serial.print(rfid.uid.uidByte[i], HEX);
            Serial.print(" ");
            lastUID[i] = rfid.uid.uidByte[i]; 
        }
        Serial.println();


        if (!checkUID(lastUID, uid1) && !checkUID(lastUID, uid2)) {
            Serial.println("❌ Access Denied");
            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();
            return; 
        }

       
        Serial.println("✅ RFID scanned. Opening door...");
        myServo.write(90);
        delay(1000);
        // Wait for person detection
        waitingForPerson = true;
    }

   
    if (waitingForPerson) {
        Serial.println("🔎 Checking for person...");
        for (int i = 0; i < 30; i++) {  
            if (getDistance() <= 10) {
                Serial.println("👤 Person detected! Granting access...");
                delay(3000);
                grantAccess();
                return;
            }
            delay(100);
        }

        Serial.println("❌ No person detected. Entry denied.");
        myServo.write(0);
        waitingForPerson = false;
    }
}


void grantAccess() {
    String userName;
    bool *userStatus;
    unsigned long *entryTime;

   
    if (checkUID(lastUID, uid1)) {
        userName = "Rahul";
        userStatus = &rahulInside;
        entryTime = &rahulEntryTime;
    } else if (checkUID(lastUID, uid2)) {
        userName = "Sanjay";
        userStatus = &sanjayInside;
        entryTime = &sanjayEntryTime;
    } else {
        return; 
    }

    if (*userStatus) {
      
        unsigned long exitTime = millis();
        unsigned long timeSpent = (exitTime - *entryTime) / 1000; 
        Serial.print("🚪 ");
        Serial.print(userName);
        Serial.print(" is leaving. Time spent inside: ");
        Serial.print(timeSpent);
        Serial.println(" seconds.");
        *userStatus = false;  
    } else {
  
        Serial.print("🚪 ");
        Serial.print(userName);
        Serial.println(" is entering.");
        *entryTime = millis();  
        *userStatus = true;  
    }

    Serial.println("🚪 Door Closed.");
    myServo.write(0);  
    waitingForPerson = false; 
}
