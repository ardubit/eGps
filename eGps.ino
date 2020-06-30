/*
   eGPS
   Author: Aleksey M.
*/

#define SERIAL_OUT
#define REV "28.06.20"

#include "headers/Images.h"

#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <FS.h> // SPIFFS

// ___ Adafruit ___
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

TinyGPSPlus gps;
SoftwareSerial ss(D3, D0);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define OFFSET 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TIME_DISPLAY_ON 50 * 1000
#define TIME_CHANGE_ACTIVITY 10 * 1000
#define TIME_CALC_DISTANCE 3000
#define TIME_CALC_BATLEVEL 5000
#define TIME_LONGPRESS 2000
#define TIME_WRITE_FILE 5000

#define ACTIVITIES_NUM 12 // activity + 1
byte activity = 0;

unsigned long sTimeAct = 0, sTimeDst = 0, sTimeBat = 0, sTimeWrt = 0;
unsigned long sTimeBtn = 0;

bool statusRec = false;
bool fileCreated = false;

bool btnPressed = false;
bool btnLongPressed = false;
bool initMode = true;
bool dispActive = true;

String devMac = "MACAddress ERR";
String dateStr = "Date ERR";
String timeStr = "Time ERR";
String timeHor = "H ERR";
String timeMin = "M ERR";
String timeSec = "S ERR";

double sLat = 00.000000, sLng = 00.000000;
double lat = 0.0, lng = 0.0;

int ageSpd = 0, ageAlt = 0, ageSat = 0;
double mps = 0.0, kmph = 0.0, minpkm = 0.0, alt = 0.0, hdop = 0.0;
double maxKmph = 0.0, maxMps = 0.0;
int sat = 0;

double distanceKm = 0.0;
unsigned long counterPoints = 0;

String msgLog = "";

void ICACHE_RAM_ATTR ISR_button_falling();

void setup()
{
#ifdef SERIAL_OUT
  Serial.begin(9600);
#endif
  ss.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  initADC();
  pinMode(D4, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(D4), ISR_button_falling, FALLING);

  devMac = WiFi.macAddress();
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
  delay(2000);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(F("Init GPS"));
  display.println(F("* Point up to the sky"));
  display.println(F("* Hold to delete data"));
  display.display();
  delay(2000);

  SPIFFSinit();

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
      display.println(F(" | RF | "));
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
  sTimeAct = sTimeBtn = millis();
}

void ICACHE_RAM_ATTR ISR_button_falling()
{
  if (!initMode) // If only init is ended
  {
    btnPressed = true;
    if (dispActive) // If display is active
    {
      activity++;
      activity = activity % ACTIVITIES_NUM;
    }
  }
}

void loop()
{
  // Grab data from GPS
  // ============================
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
    Serial.println(F(""));
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
    timeHor = String(gps.time.hour());
    if (timeHor.toInt() <= 9)
    {
      String t = "0";
      t += timeHor;
      timeHor = t;
    }

    timeMin = String(gps.time.minute());
    if (timeMin.toInt() <= 9)
    {
      String t = "0";
      t += timeMin;
      timeMin = t;
    }

    timeSec = String(gps.time.second());
    if (timeSec.toInt() <= 9)
    {
      String t = "0";
      t += timeSec;
      timeSec = t;
    }

    timeStr = timeHor + ":" + timeMin;

#ifdef SERIAL_OUT
    Serial.print(F("CLOCK: "));
    Serial.println(timeStr);
#endif
  }

  // Get date
  // ============================
  if (gps.date.isValid())
  {
    dateStr = String(gps.date.value());
#ifdef SERIAL_OUT
    Serial.print(F("DATE: "));
    Serial.println(dateStr);
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

    maxKmph = calcMaxSpeed(maxKmph, kmph);
    maxMps = calcMaxSpeed(maxMps, mps);

#ifdef SERIAL_OUT
    Serial.print(F("SPEED: "));
    Serial.print(mps);
    Serial.print(F(", "));
    Serial.println(kmph);
#endif
  }

  // Activities
  // ============================
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
    display.setTextSize(2);
    display.print(maxMps);
    display.setTextSize(1);
    display.println(F("Max m/s:"));
    display.println(F(" "));
    display.setTextSize(2);
    display.print(maxKmph);
    display.setTextSize(1);
    display.println(F("Max km/h:"));
    display.display();
    break;
  case 5:
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
  case 6:
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
  case 7:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println(F("HDOP:"));
    display.setTextSize(3);
    display.println(hdop);
    display.display();
    break;
  case 8:
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
  case 9:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Time UTC: "));
    display.print(timeSec);
    display.println(" sec");
    display.setTextSize(3);
    display.println(timeStr);
    display.display();
    break;
  case 10:
    if (millis() >= sTimeBat + TIME_CALC_BATLEVEL)
    {
      sTimeBat += TIME_CALC_BATLEVEL;
      display.clearDisplay();
      displayBatteryLevel();
      display.display();
    }
    break;
  case 11:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Files: "));
    display.println(SPIFFScountFiles());
    display.print(SPIFFSlistFiles());
    display.display();
#ifdef SERIAL_OUT
    SPIFFSreadFile();
#endif
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

  // Display Auto PowerOFF
  // ============================
  if (btnPressed)
  {
    sTimeAct = sTimeBtn = millis();
    displayEnable();
    btnPressed = false;
  }

  if (millis() >= sTimeBtn + TIME_DISPLAY_ON)
  {
    sTimeBtn += TIME_DISPLAY_ON;
    displayDisable(); // Check display and turn off after a couple minutes
  }

  // Calc distanceKm
  // ============================ Task
  if (millis() >= sTimeDst + TIME_CALC_DISTANCE)
  {
    sTimeDst += TIME_CALC_DISTANCE;
    calcDistanceKm();
  }

  // Detect LongPress (Listener)
  // ============================ Timer
  if (millis() >= sTimeBtn + TIME_LONGPRESS) // (sTimeBtn) updates when button is pressed
  {
    BTNdetectLongPress();
  }

  // Button LongPress (Handler)
  // ============================
  static bool latch = false;
  if (btnLongPressed == true && latch == false)
  {
    latch = !latch;
    // Recording flag
    statusRec = !statusRec; // Invert status

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);

    if (statusRec)
      display.print(F("-- REC Started! --"));
    else
      display.print(F("-- REC Finished --"));

    display.display();
    delay(500);
  }
  else if (btnLongPressed == false)
  {
    latch = false;
  }

#ifdef SERIAL_OUT
  Serial.print(F("BUTTON LONG PRESS: "));
  Serial.println(btnLongPressed);
  Serial.print(F("STATUS REC: "));
  Serial.println(statusRec);
#endif

  // Recording
  // ============================ Task
  if (millis() >= sTimeWrt + TIME_WRITE_FILE)
  {
    sTimeWrt += TIME_WRITE_FILE;
    SPIFFSwriteFile();
  }

  // Display REC
  if (statusRec)
  {
    display.fillRect(display.width() - 2, 0, display.width() - 1, display.height() - 1, SSD1306_WHITE);
    display.display();
  }

  // Auto change activity
  // ============================ Task
  if (millis() >= sTimeAct + TIME_CHANGE_ACTIVITY)
  {
    sTimeAct += TIME_CHANGE_ACTIVITY;
    displayChangeActivity();
  }

  // Print Log
  if (msgLog.length() > 0)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(msgLog);
    display.display();
    delay(500);
    msgLog = "";
  }
}

// ***************************************************************
// END OF LOOP
// ***************************************************************

// SPIFFS
// ============================
void SPIFFSinit()
{
  if (!SPIFFS.begin())
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("SPIFFS ERR"));
    display.display();
    delay(1000);
  }
  else if (digitalRead(D4) == LOW)
  {
    SPIFFS.format(); // Format and delete all data files
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Delete Files OK"));
    display.display();
    delay(1000);
  }
}

String SPIFFScountFiles() {
  int counter = 0;
  Dir dir = SPIFFS.openDir("/");
  while (dir.next())
  {
    counter++;
    yield();
  }
  return String(counter);
}

String SPIFFSlistFiles()
{
  String listFilesStr = "";
  Dir dir = SPIFFS.openDir("/"); // File name should starting with "/"
  while (dir.next())
  {
    listFilesStr += dir.fileName();
    listFilesStr += "\n";
    yield();
  }

  if (listFilesStr.length() != 0)
  {
#ifdef SERIAL_OUT
    Serial.println(F("LIST OF FILES: "));
    Serial.println(listFilesStr);
#endif
    return listFilesStr;
  }
  else
    return listFilesStr = "No data";
}

void SPIFFSreadFile()
{
  Dir dir = SPIFFS.openDir("/"); // File name should starting with "/"
  while (dir.next())
  {
    yield();

    // Read a file name
    String fileName = dir.fileName();
#ifdef SERIAL_OUT
    Serial.println(F(" --> DIR: "));
    Serial.print(F("Dir Name: "));
    Serial.println(dir.fileName());
    Serial.print(F("File Size: "));
    Serial.println(dir.fileSize()); // Get the size of the current file
#endif

    // Open a file
    File file = SPIFFS.open(fileName, "r");
    if (!file)
    {
#ifdef SERIAL_OUT
      Serial.println(F("Open R FILE ERR"));
#endif
    }
    else
    {
#ifdef SERIAL_OUT
      Serial.println(F(" -> FILE: "));
      for (int i = 0; i < file.size(); i++)
      {
        Serial.print((char)file.read()); // Read each char (byte)
        yield();
      }
      Serial.println(F(" "));
      Serial.print(F("File Size: "));
      Serial.println(file.size()); // Get the size of the current file.
      Serial.println(F(" -> END FILE: "));
#endif
      file.close();
    }
  }
}

// SPIFFS Recording
// ============================================
void SPIFFSwriteFile()
{
  static String fileName;
  static bool nameNotCreated = true, jsonHeadNotCreated = true;
  bool resetFlags = false;
  static byte cycles = 0;

  if (statusRec)
  {
    if (nameNotCreated)
    {
      fileName = "/t-"; // SPIFFS limitation: of 31 chars per file name
      fileName += dateStr;
      fileName += "-";
      fileName += timeStr;
      fileName += ":";
      fileName += timeSec;
      fileName += ".jt";
      nameNotCreated = false;
    }
    else
    {
#ifdef SERIAL_OUT
      Serial.print(F("File exists: "));
      Serial.println(fileName);
#endif
    }

    File fileObject = SPIFFS.open(fileName, "a"); // Open for appending (writing at end of file)
    if (fileObject)
    {
      if (jsonHeadNotCreated)
      {
        String jsonHead;
        jsonHead = "{\n";
        jsonHead += " \"device\": {\n";
        jsonHead += "  \"id\": \"";
        jsonHead += devMac + ",\n";
        jsonHead += "  \"name\": \"esp8266\"\n";
        jsonHead += " },\n";
        //
        jsonHead += " \"file_name\": \"";
        jsonHead += fileName + "\",\n";
        jsonHead += " \"distance\": ";
        jsonHead += String(distanceKm) + ",\n";
        jsonHead += " \"start_date_utc\": \"";
        jsonHead += dateStr + "\",\n";
        jsonHead += " \"start_time_utc\": \"";
        jsonHead += timeStr + ":" + timeSec + "\",\n";
        jsonHead += " \"track\": [\n";
        // Print to the fileObject
        fileObject.print(jsonHead);
        jsonHeadNotCreated = false;
      }

      // Form JSON body from Points
      String jsonBody;
      jsonBody = "   {\n";
      jsonBody += "    lat : ";
      jsonBody += String(lat, 6);
      jsonBody += ",\n";
      jsonBody += "    lng : ";
      jsonBody += String(lng, 6);
      jsonBody += "\n";
      jsonBody += "   },\n";
      // Print to the fileObject
      fileObject.print(jsonBody);

      fileObject.close();
      displayMsg("File saved!");
    }
    else
    {
      // File does not opened successfully
      displayMsg("File OPEN ERR!");
    }
  }
  else
  {
    // End the file
    resetFlags = true;
  }

  if (resetFlags)
  {
    nameNotCreated = true;
    jsonHeadNotCreated = true;
  }
}

// Save Output
// ============================
void displayMsg(String s)
{
  msgLog = s;
}

// Button
// ============================
bool BTNdetectLongPress()
{
  if (digitalRead(D4) == LOW && btnLongPressed == false)
    return btnLongPressed = true;
  else if (digitalRead(D4) == HIGH)
    return btnLongPressed = false; // Reset longPress flag
}

// Display
// ============================
void displayDisable()
{
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  dispActive = false;
}

void displayEnable()
{
  display.ssd1306_command(SSD1306_DISPLAYON);
  dispActive = true;
}

void displayChangeActivity()
{
  activity++;
  activity = activity % ACTIVITIES_NUM;
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
      display.println("eGPS");
      display.display();
      yield();
      delay(150);
    }
  }
}

// Calculations
// ============================
void calcDistanceKm()
{
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

double calcMaxSpeed(double &maxSpd, double &spd)
{
  if (maxSpd > spd)
    return maxSpd;
  else
  {
    maxSpd = spd;
    return maxSpd;
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
#define X 0
#define Y 0

void initADC()
{
  pinMode(A0, INPUT);
}

void displayBatteryLevel()
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
    display.drawBitmap(X, Y, BATTERY[6], IMG_H, IMG_W, WHITE);
  }
  else
  {
    batADC = constrain(batADC, MIN_BAT_ADC, MAX_BAT_ADC); // limits range of value
    batPercent = map(batADC, MIN_BAT_ADC, MAX_BAT_ADC, 0, 100);
    if (batPercent == 0)
    {
      display.drawBitmap(X, Y, BATTERY[5], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 0 && batPercent <= 1)
    {
      display.drawBitmap(X, Y, BATTERY[4], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 1 && batPercent <= 25)
    {
      display.drawBitmap(X, Y, BATTERY[3], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 25 && batPercent <= 50)
    {
      display.drawBitmap(X, Y, BATTERY[2], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 50 && batPercent <= 75)
    {
      display.drawBitmap(X, Y, BATTERY[1], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 75 && batPercent <= 95)
    {
      display.drawBitmap(X, Y, BATTERY[0], IMG_H, IMG_W, WHITE);
    }
    if (batPercent > 95 && batPercent <= 100)
    {
      display.drawBitmap(X, Y, BATTERY[0], IMG_H, IMG_W, WHITE);
    }
  }

  display.setCursor(X + IMG_W, Y);
  display.setTextSize(1);
  display.print(F("Bat charge: "));
  display.setTextSize(3);
  display.setCursor(X, Y + IMG_H);
  display.print(batPercent);
  display.print("% ");
  // display.display();
}