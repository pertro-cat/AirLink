#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#ifdef ESP32

// Wi-Fi credentials
const char* ssid = "admin";
const char* password = "domestos1216";

// Telegram Bot Token
const char* token = "7183687852:AAHH2_q1T0uUVPuFAvw_nGiDhn_BbZD484c"; // Replace with your Telegram Bot token
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
#define CHAT_ID "892934709"
WiFiClientSecure client;
UniversalTelegramBot bot(token, client);

const int relayPin = 2;
const int lockOutput = 5;

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int Password_Length = 4;
String Data;
String Master = "4567";
byte data_count = 0;

const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {14, 27, 26, 25};
byte colPins[COLS] = {33, 32, 18, 19};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void toggleRelay() {
  digitalWrite(relayPin, !digitalRead(relayPin));
  lcd.setCursor(0, 1);
  lcd.print("Relay: ");
  lcd.print(digitalRead(relayPin) ? "ON " : "OFF");
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/led_on to turn GPIO ON \n";
      welcome += "/led_off to turn GPIO OFF \n";
      welcome += "/state to request current GPIO state \n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }

    if (text == "/led_on") {
      bot.sendMessage(CHAT_ID, "LED state set to ON", "");
      digitalWrite(relayPin, HIGH);
    }
    
    if (text == "/led_off") {
      bot.sendMessage(CHAT_ID, "LED state set to OFF", "");
      digitalWrite(relayPin, LOW);
    }
    
    if (text == "/state") {
      if (digitalRead(relayPin)){
        bot.sendMessage(CHAT_ID, "LED is ON", "");
      }
      else{
        bot.sendMessage(CHAT_ID, "LED is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  pinMode(lockOutput, OUTPUT);

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Ensure TELEGRAM_CERTIFICATE_ROOT is defined
  #endif

  lcd.init();
  lcd.backlight();

  lcd.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("Wi-Fi Connected");
  lcd.setCursor(0, 1);
  lcd.print("Relay: OFF");

  lastTimeBotRan = millis();
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  char customKey = customKeypad.getKey();
  if (customKey) {
    if (data_count == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Password:");
    }

    Data += customKey;
    lcd.setCursor(data_count, 1);
    lcd.print('*');
    data_count++;
    
    if (data_count == Password_Length) {
      lcd.clear();
      lcd.setCursor(0, 0);

      Serial.println("Checking password...");
      if (Data == Master) {
        lcd.print("Access Granted");
        Serial.println("Password correct, activating relay...");
        digitalWrite(relayPin, HIGH);
        Serial.println("Password correct, activating relay...");
        delay(5000);
        digitalWrite(relayPin, LOW);
        Serial.println("Relay deactivated.");
      } else {
        lcd.print("Access Denied");
        delay(2000); // Display message for 2 seconds
      }

      clearData(); // Reset data after showing the result
    }
  }
}

void clearData() {
  data_count = 0;
  Data = "";  
}

#endif 