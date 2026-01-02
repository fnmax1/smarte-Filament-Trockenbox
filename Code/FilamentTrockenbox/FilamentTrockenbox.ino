#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_AHTX0.h>
#include <Fonts/FreeSans9pt7b.h>

//initialise display
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

//initialise AHT20
Adafruit_AHTX0 aht;

//assign display buttons
#define BUTTON_A 9
#define BUTTON_B 8
#define BUTTON_C 7

//Main Screen Cursor
enum MainScreenCursor
{
  CURSOR_HUMIDITY_CONTROL, CURSOR_SET_HUMIDITY
};
MainScreenCursor msCursor = CURSOR_HUMIDITY_CONTROL;

//menu states
enum MenuState
{
  MAIN_SCREEN, HUMIDITY_CONTROL, SET_HUMIDITY
};
MenuState menuState = MAIN_SCREEN;

//globale variables
sensors_event_t humidity; 
sensors_event_t temp;
bool humidityWarning = false;
int humidityThreshold = 40;
unsigned long lastToggle = 0;
bool showText = false;

void setup() 
{
  //Define Baudrate
  Serial.begin(115200);
  
  //Display Setup
  if( display.begin(0x3C, true) )
  {
    Serial.println("found OLED");
  }
  else
  {
    Serial.println("Didn't find OLED");
  }

  display.clearDisplay();
  display.display();
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);  
  display.setFont(&FreeSans9pt7b);

  //Button Setup
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  //Start AHT20
  if ( aht.begin() ) 
  {
    Serial.println("Found AHT20");
  }
  else 
  {
    Serial.println("Didn't find AHT20");
  }   
}

void loop() 
{
  display.clearDisplay();

  if(digitalRead(BUTTON_A) == LOW)
  {
    setButton_A();
    delay(200);
  }

  if(digitalRead(BUTTON_B) == LOW)
  {
    setButton_B();
    delay(200);
  }

  if(digitalRead(BUTTON_C) == LOW)
  {
    setButton_C();
    delay(200);
  }

  switch(menuState)
  {
    case MAIN_SCREEN:
      drawMainScreen();      
      break;

    case HUMIDITY_CONTROL:
      updateSensorData();
      drawHumidityControlScreen();
      break;

    case SET_HUMIDITY:
      drawSetHumidityScreen();
      break;  

  }

  yield();
  display.display();
  delay(100);

}

void setButton_A()
{
  switch(menuState)
  {
    case MAIN_SCREEN:
    if(msCursor > 0)
    {
      msCursor = static_cast<MainScreenCursor>(static_cast<int>(msCursor) - 1);
    }
    break;

    case SET_HUMIDITY:
      if(humidityThreshold < 100)
      {
        humidityThreshold++;
      }
    break;
  }
}

void setButton_B()
{
    switch(menuState)
  {
    case MAIN_SCREEN:

      switch(msCursor)
      {
        case CURSOR_HUMIDITY_CONTROL:
        menuState = HUMIDITY_CONTROL;
        break;

        case CURSOR_SET_HUMIDITY:
        menuState = SET_HUMIDITY;
        break;
      }

      break;

    case HUMIDITY_CONTROL:
      menuState = MAIN_SCREEN;
    break;

    case SET_HUMIDITY:
      menuState = MAIN_SCREEN;
    break;
  }
}


void setButton_C()
{
  switch(menuState)
  {
    case MAIN_SCREEN:
    if(msCursor < 1)
    {
      msCursor = static_cast<MainScreenCursor>(static_cast<int>(msCursor) + 1);
    }
    break;

    case SET_HUMIDITY:
      if(humidityThreshold > 0)
      {
        humidityThreshold--;
      }
    break;
  }
}

void drawMainScreen()
{
  display.setCursor(0,20);
  display.print("Menu:");
  switch(msCursor)
  {
    case CURSOR_HUMIDITY_CONTROL:
      display.setCursor(0,40);
      display.print(">start");
      display.setCursor(0,60);
      display.print("  set Humidity");
      break;

    case CURSOR_SET_HUMIDITY:
      display.setCursor(0,40);
      display.print("  start");
      display.setCursor(0,60);
      display.print(">set Humidity");
      break;
  }
}

void updateSensorData()
{
  //get current temp & humidity
  aht.getEvent(&humidity, &temp);
  if( (int)round(humidity.relative_humidity) >= humidityThreshold)
  {
    humidityWarning = true;
  }
  else
  {
    humidityWarning = false;
  }
}

void drawHumidityControlScreen()
{
  unsigned long currentTime = 0;
  unsigned int blinkInterval = 1500;
  
  //display current temp & humidity
  display.setCursor(0,40);
  display.print("Temp: "); display.print( (int)round(temp.temperature) ); display.println(" C");
  display.setCursor(0,60);
  display.print("Hum:  "); display.print( (int)round(humidity.relative_humidity) ); display.println(" %");

  //display WARNING! if current humidity >= humidity threshold
  if(humidityWarning)
  {
    currentTime = millis();
    if(currentTime - lastToggle > blinkInterval)
    {
      lastToggle = currentTime;
      showText = !showText;
    }
    if(showText)
    {
      display.setCursor(0, 20);
      display.print("Warning!");
    }
  }
}

void drawSetHumidityScreen()
{
  display.setCursor(0,40);
  display.print("Threshold: ");display.print(humidityThreshold);display.println("%");
}


