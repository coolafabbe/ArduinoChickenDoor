#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <DS1307RTC.h>
//#include <Time.h>
#include <ezButton.h>

#define VRX_PIN  A0 // Arduino pin connected to VRX pin
#define VRY_PIN  A1 // Arduino pin connected to VRY pin
#define SW_PIN   2  // Arduino pin connected to SW  pin

ezButton button(SW_PIN);

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int adrLCD = 0x27;
const int adrRTC = 0x62;

int joystickLimits[] = {0, 1023};
int joystickThreshold = 200;

int dateTime[] = {2022, 01, 01, 00, 00};

String previousPrint[] = {"", ""};

void setup()
{
  Wire.begin();
  lcd.begin();
  
  Serial.begin(9600);
  Serial.println("\nInitializing...");
  
  bool commLCD_OK = CheckI2CAdress(adrLCD);
  bool commRTC_OK = CheckI2CAdress(adrRTC);

  button.setDebounceTime(50); // set debounce time to 50 milliseconds

  lcd.begin();
  lcd.backlight();

  setTime();
}  
void loop()
{
  button.loop(); 
  //ReadJoystick();
  
}

void setTime()
{
  String ln1, ln2;
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
        if (index < 4)
          index += 1;
      }
      else if (cmd == 40) //Down, decrease value
      {  
        if (dateTime[index] > 1)
          dateTime[index]--;
      }
      else if (cmd == 50) //Up, increase value
      {  
        if ((index == 0 and dateTime[index] < 2100) or
            (index == 1 and dateTime[index] < 12) or
            (index == 2 and dateTime[index] < 31) or
            (index == 3 and dateTime[index] < 23) or
            (index == 4 and dateTime[index] < 59)) {
          dateTime[index]++;
        }
      }
      
      ln1 = StringFormat(dateTime[0]) + "-" + StringFormat(dateTime[1]) + "-" + StringFormat(dateTime[2]) + " " + StringFormat(dateTime[3]) + ":" + StringFormat(dateTime[4]);
      Print("Set Date/Time", ln1);
      prev_cmd = cmd;
    }
  }
  Print("Finished settime");
  ln1 = StringFormat(dateTime[0]) + "-" + StringFormat(dateTime[1]) + "-" + StringFormat(dateTime[2]) + " " + StringFormat(dateTime[3]) + ":" + StringFormat(dateTime[4]);
  Print(ln1);
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
