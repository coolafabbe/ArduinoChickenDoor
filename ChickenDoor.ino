#include <DS1307RTC.h>
#include <TimeLib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ezButton.h>

#define VRX_PIN  A0 // Arduino pin connected to VRX pin
#define VRY_PIN  A1 // Arduino pin connected to VRY pin
#define SW_PIN   2  // Arduino pin connected to SW  pin

ezButton button(SW_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int adrLCD = 0x27;
const int adrRTC = 0x68;

const int joystickLimits[] = {0, 1023};
const int joystickThreshold = 200;

String menuLevels[] = {"Set Time", "Set Open", "Set Close", "Man Open", "Man Close", "Exit"};

int timeOpen[] = {8, 0};
int timeClose[] = {21, 30};

String previousPrint[] = {"", ""};

void setup()
{
  tmElements_t tm;
  
  Wire.begin();
  lcd.begin();
  
  Serial.begin(9600);
  Serial.println("Initializing...");
  
  bool commLCD_OK = CheckI2CAdress(adrLCD);
  bool commRTC_OK = CheckI2CAdress(adrRTC);

  button.setDebounceTime(50); // set debounce time to 50 milliseconds

  lcd.begin();
  lcd.backlight();

  if (commLCD_OK and commRTC_OK) {
    if (RTC.read(tm))
      Print("Time is set.");
    else if (RTC.chipPresent())
      setTime();
  }
  else {
    Print("Communication error.");
    while (true) {
      delay(1000);
    }
  }  
}  
void loop()
{
  button.loop(); 
  tmElements_t tm;
  RTC.read(tm);
  Print(tmToString(tm));

  if (button.isPressed()) {
    settings(); // todo add menu system, to change time, change open time, change closing time, manually controlled
  }

  // check if door open and should be closed

  // check if door not open and should not be closed
}

void settings()
{
  int currentMenuLevel = 0;
  int cmd = 0, prev_cmd = 0;

  bool finished = false;
  while (not finished) {
    Print("Settings", menuLevels[currentMenuLevel]);

    button.loop();
    int cmd = ReadJoystick();

    if (cmd != prev_cmd) {
      if (cmd == 10) {
        finished = true;
        if (currentMenuLevel == 0)
          setTime();
        else if (currentMenuLevel == 1) 
          setOpenClose(timeOpen, "Set opening");
        else if (currentMenuLevel == 2) 
          setOpenClose(timeClose, "Set closening");
      }
      else if (cmd == 20) //Left, previous value
      {
        if (currentMenuLevel > 0)
          currentMenuLevel += -1;
      }
      else if (cmd == 30) //Right, next value
      {
        if (currentMenuLevel < sizeof(menuLevels))
          currentMenuLevel += 1;
      }
      prev_cmd = cmd;
    }
  }
}

void setTime()
{
  tmElements_t tm;
  String ln1, ln2;
  int cmd = 0, prev_cmd = 0;
  int index = 0;

  tm.Year = 52;
  tm.Month = 1; 
  tm.Day = 1;
  tm.Hour = 1;
  tm.Minute = 1;
  tm.Second = 1;
  
  bool finished = false;
  while (not finished) {
    button.loop();
    int cmd = ReadJoystick();
    if (cmd != prev_cmd) {
      if (cmd == 10) 
        finished = true;
      else if (cmd == 20) //Left, previous value
      {
        if (index > 0)
          index += -1;
      }
      else if (cmd == 30) //Right, next value
      {
        if (index < 4)
          index += 1;
      }
      else if (cmd == 40) //Down, decrease value
      {  
        if (index == 0 and tm.Year > 0)
          tm.Year--;
        else if (index == 1 and tm.Month > 1)
          tm.Month--;
        else if (index == 2 and tm.Day > 1)
          tm.Day--;
        else if (index == 3 and tm.Hour > 1)
          tm.Hour--;
        else if (index == 4 and tm.Minute > 1) 
          tm.Minute--;
      }
      else if (cmd == 50) //Up, increase value
      {  
        if (index == 0 and tm.Year < 99)
          tm.Year++;
        else if (index == 1 and tm.Month < 12)
          tm.Month++;
        else if (index == 2 and tm.Day < 31)
          tm.Day++;
        else if (index == 3 and tm.Hour < 23)
          tm.Hour++;
        else if (index == 4 and tm.Minute < 59) 
          tm.Minute++;
      }
      
      Print("Set Date/Time", tmToString(tm));
      prev_cmd = cmd;
    }
  }

  Print("Finished settime", tmToString(tm));

  RTC.write(tm);
}

void setOpenClose(int time[], String funccmd) 
{
  Print(funccmd, StringFormat(time[0])+":"+StringFormat(time[1]));
  int cmd = 0, prev_cmd = 0;
  int index = 0;
  bool finished = false;
  while (not finished) {
    button.loop();
    int cmd = ReadJoystick();
    if (cmd != prev_cmd) {
      if (cmd == 10) 
        finished = true;
      else if (cmd == 20) //Left, previous value
      {
        if (index > 0)
          index += -1;
      }
      else if (cmd == 30) //Right, next value
      {
        if (index < 1)
          index += 1;
      }
      else if (cmd == 40) //Down, decrease value
      {  
        if (time[index] > 0)
          time[index]--;
        Print(funccmd, StringFormat(time[0])+":"+StringFormat(time[1]));
      }
      else if (cmd == 50) //Up, increase value
      {  
        if (index == 0 and time[index] < 24)
          time[index]++;
        else if (index == 1 and time[index] < 60)
          time[index]++;
        Print(funccmd, StringFormat(time[0])+":"+StringFormat(time[1]));
      }      
      prev_cmd = cmd;
    }
  }
  Print("Done " + funccmd + ":", StringFormat(time[0])+":"+StringFormat(time[1]));
  //return time;
}

String tmToString(tmElements_t tm) 
{
  return StringFormat(tm.Year + 1970) + "-" + StringFormat(tm.Month) + "-" + StringFormat(tm.Day) + " " + StringFormat(tm.Hour) + ":" + StringFormat(tm.Minute);
}

String StringFormat(int number) 
{
  String text = "";
  if (number < 10)
   text += 0;
  text +=  String(number);
  return text;
}

int ReadJoystick() 
{
  // read analog X and Y analog values
  int joystickValueX = analogRead(VRX_PIN);
  int joystickValueY = analogRead(VRY_PIN);

  // Read the button value
  int bValue = button.getState();

  if (button.isPressed())
    return 10; // Pressed
  else if (joystickValueX < joystickLimits[0] + joystickThreshold)
    return 20; // Left
  else if (joystickValueX > joystickLimits[1] - joystickThreshold)
    return 30; // Right
  else if (joystickValueY < joystickLimits[0] + joystickThreshold)
    return 40; // Down
  else if (joystickValueY > joystickLimits[1] - joystickThreshold)
    return 50; // Up
  else {
    return -1;
  }  
}

bool CheckI2CAdress(int address)
{
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  if (error == 0) {
    Serial.print("Device found at I2C adress 0x");
    Serial.println(address,HEX);
    return true;
  }
  else {
    Serial.print("No device at I2C adress 0x");
    Serial.print(address,HEX);
    Serial.println(" !");
    return false;
  }
}

void Print(String text)
{
  if (text.length() <= 16) 
    Print(text, "");
  else 
    Print(text.substring(0,16), text.substring(16, 32));
}

void Print(String textLn1, String textLn2) {
  if (textLn1 != previousPrint[0] or textLn2 != previousPrint[1])  {
    Serial.println(textLn1);
    if (textLn2 != "")
      Serial.println(textLn2);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(textLn1);
    if (textLn2 != "") {
      lcd.setCursor(0,1);
      lcd.print(textLn2);
    }
    // Save current text as prevoious
    previousPrint[0] = textLn1;
    previousPrint[1] = textLn2;
  }
}
