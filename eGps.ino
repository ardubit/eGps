/*
   eGPS
   Author: Aleksey M.
*/

#define SERIAL_OUT

#include "headers/Images.h"

#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// ___ Adafruit ___
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define REV "28.06.20"

TinyGPSPlus gps;
SoftwareSerial ss(D3, D0);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define OFFSET 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TIME_DISPLAY_ON 30 * 1000
#define TIME_CALC_DISTANCE 3000
#define TIME_CALC_BATLEVEL 5000

byte activity = 0;

static unsigned long startTimeDst = 0;
static unsigned long startTimeBat = 0;
static unsigned long currentMillis = 0;

bool buttonPressed = false;
bool initMode = true;
bool displayActive = true;

void ICACHE_RAM_ATTR ISR_button();

String timeStr = "Time ERR";
String timeSec = "Secs ERR";

double sLat = 00.000000;
double sLng = 00.000000;
double lat = 0.0;
double lng = 0.0;

int ageSpd = 0;
double mps = 0.0;
double kmph = 0.0;
double minpkm = 0.0;

int ageAlt = 0;
double alt = 0.0;

int ageSat = 0;
int sat = 0;

double hdop = 0.0;
double distanceKm = 0.0;

unsigned long counterPoints = 0;

void setup()
{
  Serial.begin(9600);
  ss.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  initADC();
  pinMode(D4, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(D4), ISR_button, FALLING);

  WiFi.mode(WIFI_OFF);

  // Display
  // ============================

  // Show splash screen
  display.clearDisplay();
  displaySplashScreen();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(F("Firmware:"));
  display.println(REV);
  display.println(F("Developer: Aleksey M."));
  display.println(F("github.com/ardubit"));
  display.display();
  delay(2500);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(F("Init GPS"));
  display.print(F("Point up to the sky!"));
  display.display();
  delay(2500);

  while (!gps.location.isValid())
  {
    static int count = 0;
    while (ss.available() > 0)
      gps.encode(ss.read());

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Search GPS "));

    if (count >= 999)
      display.println(F("Chge lctn!"));
    else
      display.println(F(""));

    display.setTextSize(3);
    display.println(count);
    display.display();

#ifdef SERIAL_OUT
    Serial.println(gps.location.lat(), 6);
    Serial.println(gps.location.lng(), 6);
    Serial.println(gps.satellites.value());
#endif
    count++;
    delay(250);
  }

  if (gps.location.isValid()) // Set start point
  {
    sLat = gps.location.lat();
    sLng = gps.location.lng();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println(F("Starting..."));
    display.setTextSize(3);
    display.println(F("OK"));
    display.display();
    delay(1000);

#ifdef SERIAL_OUT
    Serial.println(F("Start Point:"));
    Serial.println(sLat);
    Serial.println(sLng);
#endif
  }
  else
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println(F("Starting..."));
    display.setTextSize(3);
    display.println(F("ERR"));
    display.display();
    delay(1000);
  }

  initMode = false;
  currentMillis = millis();
}

void ICACHE_RAM_ATTR ISR_button()
{
  if (!initMode) // if init is ended
  {
    buttonPressed = true;
    if (displayActive) // if display is active
    {
      activity++;
      activity = activity % 10;
    }
  }
}

void loop()
{
  while (ss.available() > 0)
  {
    char temp = ss.read();
#ifdef SERIAL_OUT
    Serial.write(temp);
#endif
    gps.encode(temp);
    yield();
  }

  // Get location
  // ============================
  if (gps.location.isValid())
  {
    lat = gps.location.lat();
    lng = gps.location.lng();
#ifdef SERIAL_OUT
    Serial.println(F("=================== LOCATION "));
    Serial.println(lat, 6);
    Serial.println(lng, 6);
#endif
  }

  // Get satellites
  // ============================
  if (gps.satellites.isValid())
  {
    ageSat = gps.satellites.age();
    sat = gps.satellites.value();
#ifdef SERIAL_OUT
    Serial.print(F("SAT: "));
    Serial.println(sat);
#endif
  }

  // Get altitude
  // ============================
  if (gps.altitude.isValid())
  {
    ageAlt = gps.altitude.age();
    alt = gps.altitude.meters();
#ifdef SERIAL_OUT
    Serial.print(F("ALT: "));
    Serial.println(alt);
#endif
  }

  // Get time
  // ============================
  if (gps.time.isValid())
  {
    timeStr = String(gps.time.hour()) + ":" + String(gps.time.minute());
    timeSec = String(gps.time.second()) + " sec";
#ifdef SERIAL_OUT
    Serial.print(F("CLOCK: "));
    Serial.println(timeStr);
#endif
  }

  // Get HDOP
  // ============================
  if (gps.hdop.isValid())
  {
    hdop = gps.hdop.hdop();
  }

  // Get speed
  // ============================
  if (gps.speed.isUpdated())
  {
    // 1.0 is good a precision
    if (!(gps.speed.mps() <= 0.79))
      mps = gps.speed.mps();
    else
      mps = 0.0;

    if (!(gps.speed.kmph() <= 1.249))
      kmph = gps.speed.kmph();
    else
      kmph = 0.0;

    ageSpd = gps.speed.age();

    if (!(kmph <= 0.49))
      minpkm = 60.0 / kmph;
    else
      minpkm = 0.0;

#ifdef SERIAL_OUT
    Serial.print(F("SPEED: "));
    Serial.print(mps);
    Serial.print(F(", "));
    Serial.println(kmph);
#endif
  }

  switch (activity)
  {
  case 0:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Lt:"));
    display.setTextSize(2);
    display.println(lat, 6);
    display.setTextSize(1);
    display.print(F("Lg:"));
    display.setTextSize(2);
    display.println(lng, 6);
    display.display();
    break;
  case 1:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Tmp min/km: "));

    if (minpkm > 0.33 && minpkm <= 4.0)
      display.println(F("* * * *"));
    else if (minpkm >= 4.0 && minpkm <= 4.3)
      display.println(F("* * * "));
    else if (minpkm >= 4.3 && minpkm <= 4.5)
      display.println(F("* * "));
    else if (minpkm >= 4.5 && minpkm < 4.6)
      display.println(F("* "));
    else
      display.println(F(""));

    display.setTextSize(3);
    display.println(minpkm);
    display.display();
    break;
  case 2:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Spd m/s: "));
    display.print(F("age ms: "));
    display.println(ageSpd);
    display.setTextSize(3);
    display.println(mps);
    display.display();
    break;
  case 3:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Spd km/h: "));
    display.print(F("age ms: "));
    display.println(ageSpd);
    display.setTextSize(3);
    display.println(kmph);
    display.display();
    break;
  case 4:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Alt m: "));
    display.print(F("age ms: "));
    display.println(ageAlt);
    display.setTextSize(3);
    display.println(alt);
    display.display();
    break;
  case 5:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Sat: "));
    display.print(F("fix ms: "));
    display.println(ageSat);
    display.setTextSize(3);
    display.println(sat);
    display.display();
    break;
  case 6:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println(F("HDOP:"));
    display.setTextSize(3);
    display.println(hdop);
    display.display();
    break;
  case 7:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Dist km: "));
    display.print(F("pts: "));
    display.println(counterPoints);
    display.setTextSize(3);
    display.println(distanceKm);
    display.display();
    break;
  case 8:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Time UTC: "));
    display.println(timeSec);
    display.setTextSize(3);
    display.println(timeStr);
    display.display();
    break;
  case 9:
    if (millis() >= startTimeBat + TIME_CALC_BATLEVEL)
    {
      startTimeBat += TIME_CALC_BATLEVEL;
      display.clearDisplay();
      displayBatteryLevel(0, 0);
      display.display();
    }
    break;
  default:
    break;
  }

  if (gps.charsProcessed() < 10)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print(F("GPS ERR"));
    display.display();
  }

  if (buttonPressed)
  {
    currentMillis = millis();
    displayEnable();
    buttonPressed = false;
  }
  else
  {
    displayDisable(); // Check display and turn off after a couple minutes
  }

  // Calc distanceKm
  calcDistanceKm();
}

void displayDisable()
{
  if (millis() >= currentMillis + TIME_DISPLAY_ON)
  {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    currentMillis += TIME_DISPLAY_ON;
    displayActive = false;
  }
}

void displayEnable()
{
  display.ssd1306_command(SSD1306_DISPLAYON);
  displayActive = true;
}

void displaySplashScreen()
{
  for (int i = 0; i < TIMES; i++)
  {
    for (int i = 0; i < sizeof(RUNNING_MAN_R) / 8; i++)
    {
      display.clearDisplay();
      display.drawBitmap(((display.width() - IMG_W) / 2) - OFFSET, (display.height() - IMG_H) / 2, RUNNING_MAN_R[i], IMG_H, IMG_W, WHITE);
      display.setCursor(((display.width() - IMG_W) / 2) - OFFSET + IMG_W, (display.height() - IMG_H) / 2);
      display.println(" eGPS ...");
      display.display();

      yield();
      delay(150);
    }
  }
}

void calcDistanceKm()
{
  if (millis() >= startTimeDst + TIME_CALC_DISTANCE)
  {
    startTimeDst += TIME_CALC_DISTANCE;
    // lat, lng, sLat, sLng
    // Test values 51.616317, 33.604432, 51.617193, 33.606631 - 180 m
    double dist = TinyGPSPlus::distanceBetween(lat, lng, sLat, sLng);

    // Walking 0.8 - 1.53 m/sec
    // 0.8 is good a precision (0.8 * TIME_CALC_DISTANCE)
    if (!(dist <= 2.65))
    {
      dist = dist / 1000.0;
      distanceKm += dist;
      counterPoints++;
    }

    sLat = lat;
    sLng = lng;
  }
}

// ADC
// ==================================

#define MAX_BAT_VOLT 4.20 // 1023
#define MIN_BAT_VOLT 2.49 // 607
#define MIN_BAT_ADC 607
#define MAX_BAT_ADC 1023

// #define FULL_BAT_CHARGE 974
// #define QUAR_BAT_CHARGE 883
// #define HALF_BAT_CHARGE 792
// #define EMPT_BAT_CHARGE 701
// #define CRIT_BAT_CHARGE 609
// #define DIED_BAT_CHARGE 607
#define NO___BAT_CHARGE 49

void initADC()
{
  pinMode(A0, INPUT);
}

void displayBatteryLevel(byte x, byte y)
{
  /*
  ADC
  ======================
  The ADC precision is the number of ADC inputs 1023 – 10 bits. 
  The ADC range is the minimum and maximum ADC input from 0 to +1V. 
  The ADC resolution is the smallest distinguishable change in input 1V/1023, which is about 0.00098 mV. 
  The resolution is the change in input that causes the digital output to change by 1.
  
  Range(volts) = Precision(alternatives) • Resolution(volts)
  
  Uinput(ADC)   from 0 to 1V
  ADC           from 0 to 1023
  Umax          1V

  Uout(ADC input) = ADC * Umax/1023
  ADC = Uout(ADC input) * 1023 / Umax

  R3 Calculation
  ======================
  http://www.ohmslawcalculator.com/voltage-divider-calculator
  
  Us = 4.2V ---<R3 ?>--- A0 ---<R1 220k>--- ADC input (Max 1V) ---<R2 100k>---- GND
  
  Uout = Us * R2 / R1 + R3 + R2
  Uout / Us * R2 = 1/(R1 + R3 + R2)
  R3 = (Us * R2 / Uout) - R1 - R2
  R3 = (4.2V * 100k / 1) - 220k - 100k = 100k

  Real case
  ======================
  Battery Umax = 4.2V
  Real Battery Umax = 4.082V
  
  Calculated Values:
  Us = 4.082V ---<R3 100k>--- A0 ---<R1 220k>--- Uout = 0.972V (Max ADC input 1V) ---<R2 100k>---- GND
  ADC = 0.972V * 1023 / 1 = 994;

  Measured Values:
  Us = 4.082V ---<R3 102.6k>--- A0 ---<R1 220k>--- Uout = 0.952V (Max ADC input 1V) ---<R2 100k>---- GND
  ADC = 0.952 * 1023 / 1 = 974;

  Serial Output:
  ADC = 1023

  Range
  ======================
                        Working Voltage Value
  ESP8266               2.5 - 3.6V
  Wemos D1              2.5 - 6.5V
  Quectel L80-R         3.0 - 4.3V
  OLED SSD1306          1.6 - 3.3V

                            Calculated
  0.200V - NO BATTERY       Uout = 0.048  ADC = 49         // No   battery img 6
  2.490V - 0% of charge     Uout = 0.593  ADC = 607        // Died battery img 5 
  2.500V - 1%               Uout = 0.595  ADC = 609        // Сrit battery img 4 
  2.875V - 25%              Uout = 0.685  ADC = 701        // Empt battery img 3              
  3.250V - 50%              Uout = 0.774  ADC = 792        // Half battery img 2 
  3.625V - 75%              Uout = 0.863  ADC = 883        // Quar battery img 1
  4.000V - 100%             Uout = 0.952  ADC = 974        // Full battery img 0
  4.200V - MAX              Uout = 1.000  ADC = 1023

  Thinking
  ======================
  - Volts 
  - ADC Value 
  - Percent of battery charge
  */

  /*
 LiPo starts to die quickly from 2.5 - 2.7V
  */

  unsigned int batADC = analogRead(A0);
  unsigned int batPercent = 0;

#ifdef SERIAL_OUT
  Serial.println("RUN -> Task: Check battery charge");
#endif

#ifdef SERIAL_OUT
  Serial.print("ADC: ");
  Serial.println(batADC);
  float volts = 0.0;
  volts = (batADC * MAX_BAT_VOLT) / 1023;
  Serial.print("Volts: ");
  Serial.println(volts);
#endif

  // display.clearDisplay();
  if (batADC <= NO___BAT_CHARGE)
  {
    display.drawBitmap(x, y, BATTERY[6], IMG_H, IMG_W, WHITE);
  }
  else
  {
    batADC = constrain(batADC, MIN_BAT_ADC, MAX_BAT_ADC); // limits range of value
    batPercent = map(batADC, MIN_BAT_ADC, MAX_BAT_ADC, 0, 100);
    if (batPercent == 0)
    {
      display.drawBitmap(x, y, BATTERY[5], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 0 && batPercent <= 1)
    {
      display.drawBitmap(x, y, BATTERY[4], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 1 && batPercent <= 25)
    {
      display.drawBitmap(x, y, BATTERY[3], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 25 && batPercent <= 50)
    {
      display.drawBitmap(x, y, BATTERY[2], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 50 && batPercent <= 75)
    {
      display.drawBitmap(x, y, BATTERY[1], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 75 && batPercent <= 95)
    {
      display.drawBitmap(x, y, BATTERY[0], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 95 && batPercent <= 100)
    {
      display.drawBitmap(x, y, BATTERY[0], IMG_H, IMG_W, WHITE);
    }
  }

  display.setCursor(x + IMG_W, y);
  display.setTextSize(1);
  display.print(F("bat chrg: "));
  display.setTextSize(3);
  display.setCursor(x, y + IMG_H);
  display.print(batPercent);
  display.print("% ");
  // display.display();
}