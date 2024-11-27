#include <LiquidCrystal_I2C.h>

#include <Wire.h>

#include <WiFi.h>
#include <WiFiUdp.h>

#include <NTPClient.h>

#define UTC_OFFSET_IN_SECONDS 2*60*60

//initialize the liquid crystal library
//the first parameter is  the I2C address
//the second parameter is how many cols are on your screen
//the  third parameter is how many rows are on your screen
LiquidCrystal_I2C lcd(0x27,  16, 2);

const char * ssid = "BananaPhone";
const char * password = "thisistemporary";

int lastNTPUpdateStamp = 0;
int lastDisplayUpdateStamp = 0;

int clubNightBeginning = (19*60+30)*60+0; //19:30:00 Uhr in Sekunden
int clubNightLength = (2*60+00)*60+0; //2 Stunden in Sekunden
int clubNightFeeCents = 7000;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", UTC_OFFSET_IN_SECONDS);

int currentTimestampMinutes() {
  int currentHour = timeClient.getHours();
  int currentMinutes = timeClient.getMinutes();
  return currentHour*60 + currentMinutes;
}

int currentTimestampSeconds() {
  int currentSeconds = timeClient.getSeconds();
  return (currentTimestampMinutes())*60 + currentSeconds;
}

int secondsSinceClubNightBeginning() {
  int currentTimestamp = currentTimestampSeconds();
  int secondsElapsed = currentTimestamp - clubNightBeginning;
  if (secondsElapsed < 0) {
    return 0;
  }
  return secondsElapsed;
}

double calcMoney() {
  double moneyPerSecond = clubNightFeeCents / clubNightLength;
  return secondsSinceClubNightBeginning() * moneyPerSecond;
}

bool isOneSecondElapsed() {
  int currentStamp = currentTimestampSeconds();
  return (currentStamp - lastDisplayUpdateStamp) >= 1;
}

char * toCharArray(String text) {
  int str_len = text.length() + 1;
  char char_array[str_len];
  text.toCharArray(char_array, str_len);
  return char_array;
}

void displayWriting() {
  lcd.setCursor(0,0);
  String text = "" + timeClient.getHours();
  text = text + ":";
  text = text + timeClient.getMinutes();
  text = text + ":";
  text = text + timeClient.getSeconds();
  char * char_array = toCharArray(text);
  lcd.printstr(char_array);
  lcd.setCursor(0,1);
  double money = calcMoney() / 100;
  text = "";
  text = text + String(money, 4);
  lcd.printstr(toCharArray(text));
}

void clearDisplay() {
  lcd.clear();
}

void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

bool lastUpdateLongerThanThan10Minutes() {
  int currentStamp = currentTimestampMinutes();
  return (currentStamp - lastNTPUpdateStamp) > 10;
}

void setup() {
  Serial.begin(19200);
  //initialize lcd screen
  lcd.init();
  // turn on the backlight
  lcd.backlight();

  connectWifi();

  timeClient.begin();
  timeClient.update();
}
void loop() {
  delay(200);
  if (WiFi.status() != WL_CONNECTED || lastUpdateLongerThanThan10Minutes()) {
    connectWifi();
    timeClient.update();
    lastNTPUpdateStamp = currentTimestampMinutes();
  }
  //wait  for a second
  if (isOneSecondElapsed()) {
    clearDisplay();
    displayWriting();
    lastDisplayUpdateStamp = currentTimestampSeconds();
  }
  
}
