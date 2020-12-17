#include "TimerOne.h" //Adds timekeeping for inturrupt
#include <Adafruit_NeoPixel.h>  //Neopixel controls
#include "PinChangeInterrupt.h" //adds more inturrupt pins
#include <Wire.h> //i2c 
#include <Adafruit_GFX.h> //Graphics for oled
#include <Adafruit_SSD1306.h> //Oled driver

//Fonts
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(4, 6, NEO_GRB + NEO_KHZ800); //pixels,pin,...
Adafruit_SSD1306 display(128, 64, &Wire, -1); //Width, Height, Comm, Reset

//USER CHANGES ----------------------
bool CALABRATE = false; //When made true more information is sent to serial monitor
float conversionMultiplier = 0.09;  //Number used to convert from ticks/sec to mph
bool mph = true; //Used when choosing starting mode between mph and m/s
//-----------------------------------

//declare colors to use
uint32_t red = pixels.Color(255, 0, 0);
uint32_t lightred = pixels.Color(60, 0, 0);
uint32_t green = pixels.Color(0, 255, 0);
uint32_t cyan = pixels.Color(0, 100, 150);
uint32_t blue = pixels.Color(0, 0, 255);
uint32_t white = pixels.Color(225, 225, 255);

#define pinTog 8  //Button Mode1
#define pinUni 9  //Button Mode2
#define pinRec 3  //Record Inturrupt pin
#define interruptPin  2 //Fan Pulse inturrupt pin

#define brightness 255 //Neopixel standard brightness
#define timeInterval 1000000  //1,000,000 = 1second
#define clearTime 1000
int clearTarget;

bool y;
bool action = false;  //Used for standby loop
bool recording = false; //Tells program to record
bool recTogg = false; //Tell program if toggle recording is on
bool recHold = false; //Tells program if recording butting is being 'pressed'

int cycle;
int rev;
int ticks;
int totRev;
int totTicks;
int dispTicks;

float convertedSpeed;
float speedAvg;
float speedHigh;
float speedLow;
float speedPeriod;
float speedTotal;

static const unsigned char PROGMEM rim_bmp[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFC, 0x0F, 0xE0, 0x60, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFC, 0x0F, 0xE0, 0x70, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFC, 0x0F, 0xE0, 0x78, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFC, 0x0F, 0xE0, 0x7C, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFC, 0x0F, 0xE0, 0x7E, 0x00, 0x00, 0x00, 0x1F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFC, 0x0F, 0xE0, 0x7F, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFC, 0x0F, 0xE0, 0x7F, 0x80, 0x00, 0x00, 0x7F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x0F, 0xFF, 0xF0, 0x00, 0x0F, 0xE0, 0x7F, 0xC0, 0x00, 0x00, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x1F, 0xFE, 0x00, 0x00, 0x0F, 0xE0, 0x7F, 0xE0, 0x00, 0x01, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x3F, 0xF8, 0x00, 0x00, 0x0F, 0xE0, 0x7F, 0xF0, 0x00, 0x03, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x7F, 0xF0, 0x00, 0x00, 0x0F, 0xE0, 0x7F, 0xF8, 0x00, 0x07, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x7F, 0xC0, 0x00, 0x00, 0x0F, 0xE0, 0x3F, 0xFC, 0x00, 0x0F, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x00, 0x0F, 0xE0, 0x1F, 0xFE, 0x00, 0x1F, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x00, 0x0F, 0xE0, 0x0F, 0xFF, 0x00, 0x3F, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x07, 0xFF, 0x80, 0x7F, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x03, 0xFF, 0xC0, 0xFF, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x01, 0xFF, 0xE1, 0xFF, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0xFF, 0xF3, 0xFF, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x3F, 0xFF, 0xFF, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x1F, 0xFF, 0xFE, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x0F, 0xFF, 0xFC, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x07, 0xFF, 0xF8, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x03, 0xFF, 0xF0, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x01, 0xFF, 0xE0, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0xFF, 0xC0, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x7F, 0xC0, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x7F, 0x80, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x7F, 0xE0, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x3F, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x1E, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x1F, 0xFF, 0x80, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x0C, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFC, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFC, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFC, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xF8, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xF0, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xE0, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xC0, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x80, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xFE, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0xFC, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0F, 0xF8, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1F, 0xF0, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3F, 0xE0, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x7F, 0xC0, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xFE, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x07, 0xFC, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x0F, 0xF8, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x1F, 0xF0, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x3F, 0xE0, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x7F, 0xC0, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x03, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x07, 0xFC, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x0F, 0xF8, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x1F, 0xF0, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x3F, 0xE0, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup() {
  Serial.begin(9600);
  Serial.println("Hello Dave.");

  pixels.begin(); //Start LED Strip
  pixels.setBrightness(brightness); //Adjust Brightness
  pixels.show(); // Initialize All Pixels To 'Off'

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //Start OLED Display
  display.setRotation(2); //Flip Display 180
  display.clearDisplay(); //Clear Display
  display.drawBitmap(0, 0, rim_bmp, 128, 64, 1);  //Draw Logo
  display.display();  //Display Image
  delay(3000);  //Pause With Logo for 3 Seconds

  startup();

  pinMode(interruptPin, INPUT_PULLUP);  //Fan Data Pin
  attachInterrupt(digitalPinToInterrupt(pinRec), intRecord, CHANGE);  //Inturrupt to start recording
  Serial.println("You're looking well today.");
  attachPinChangeInterrupt(0, toggle, RISING);  //Mode Inturrupt
  attachPinChangeInterrupt(1, units, RISING);   //Mode Inturrupt
  Serial.println();
}

void loop() {
  if (action == false) {
    stand(1, 50, 1, 150, false, green); //Before, After, Interval, Delay, Reset, Color
    display.clearDisplay();
    display.display();
  } else if (recording == true) {
    record();
  }
}

void intRecord() {  //interrupt pin to activate recording

  action = true;  //Turn Off Standby Mode

  if (recording != true) {  //If Not Already Recording
    Serial.println("Recording started");
    recording = true; //Start Recording Fan

    if (recTogg == true) {  //If Toggle Mode is on
      recHold = true;   //Keep Record Button Pressed Through Program
    }

    cycle = -1; //Set Cycle < 0 to allow minigate error
    rev = 0;  //Reset Revolutions Per Period
    totRev = 0; //Reset Total Revolutions
    totTicks = 0; //Reset Total Ticks
    speedAvg = 0;
    speedHigh = 0;
    speedLow = 100;
    speedPeriod = 0;
    speedTotal = 0;

    pixels.fill(red); //Color Strip Red
    pixels.show();  //Display Color Change

    detachPinChangeInterrupt(0);  //Detach Mode Inturrupt
    detachPinChangeInterrupt(1);   //Detach Mode Inturrupt

    Timer1.initialize(timeInterval);  //Record Period Interval => 1,000,000 = 1second
    Timer1.attachInterrupt(timecycle);  //Start Time Interrupt - Direct to void Timecycle each Period
    Timer1.restart();
    attachInterrupt(digitalPinToInterrupt(interruptPin), pulse, CHANGE);  //Start Fan Inturrupt - Direct to void pulse each tick
  } else if (recording == true && recTogg == true && recHold == true && digitalRead(pinRec) == HIGH) {  //If Record Toggle is on and Button is pressed a second time...
    recHold = false;
    //recEnd(); //End Recording Sequence
  }
}

void record() { //what happens while recording {

  oledAction(speedPeriod, cycle, totRev / cycle);

  if (digitalRead(pinRec) == HIGH || recHold == true) {
  } else {
    recEnd(); //End Recording Sequence
  }
}

void recEnd() {
  detachInterrupt(digitalPinToInterrupt(interruptPin));
  Timer1.stop();
  detachInterrupt(digitalPinToInterrupt(pinRec));

  Serial.println("Recording ended");

  pixels.fill(white);
  pixels.show();
  delay (300);
  pixels.fill();
  pixels.show();
  display.clearDisplay();
  delay (200);
  display.display();
  delay (200);
  display.setCursor(0, 13);
  delay (10);
  speedAvg = speedTotal / (cycle - 1);
  display.print("MEAN: ");
  display.println(speedAvg);
  delay (10);
  display.setCursor(0, 32);
  display.print("HIGH: ");
  display.print(speedHigh);
  delay (10);
  display.setCursor(0, 51);
  display.print("LOW: ");
  display.print(speedLow);
  delay (10);
  display.setCursor(94, 60);
  if (mph == true) {
    display.print("mph");
  } else {
    display.print("m/s");
  }
  display.display();

  delay(1000);

  Serial.print("Average Speed: ");
  Serial.println(speedAvg);
  speedAvg = 0;
  Serial.print("Highest Speed: ");
  Serial.println(speedHigh);
  Serial.print("Lowest Speed: ");
  Serial.println(speedLow);

  while (digitalRead(pinTog) == LOW && digitalRead(pinUni) == LOW) {} //Wait for either mode button to be pressed
  lineWipe(5);

  recording = false;
  recHold = false;
  action = false; //Begin Standby Loop

  attachInterrupt(digitalPinToInterrupt(pinRec), intRecord, RISING);  //Inturrupt to start recording
  attachPinChangeInterrupt(0, toggle, RISING);  //Mode Inturrupt
  attachPinChangeInterrupt(1, units, RISING);   //Mode Inturrupt
}

void oledAction(int num, int cyc, int avg) {
  display.clearDisplay();
  display.setTextColor(WHITE);

  //Main Speed
  if (num < 10) {
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(46, 48);
    display.setTextSize(2);
    display.println(num);
  } else if ((num >= 10) && (num < 100)) {
    //Serial.println("10");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(26, 48);
    display.setTextSize(2);
    display.println(num);
  } else if (num >= 100) {
    //Serial.println("100");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(35, 36); //6 to center x2 font
    display.setTextSize(1);
    display.println(num);
  } else {
    //Lightspeed
  }

  //Set Units
  display.setFont(&FreeSans9pt7b);
  display.setCursor(94, 60);
  display.setTextSize(1);
  if (mph == true) {
    display.println("mph");
  } else {
    display.println("m/s");
  }

  //Cycle count
  if (cyc < 10) {
    display.setCursor(118, 12);
    display.println(cyc);
  } else if ((cyc >= 10) && (cyc < 100)) {
    display.setCursor(108, 12);
    display.println(cyc);
  } else if (cyc >= 100) {
    display.setCursor(118, 12);
    display.println("+");
  }

  //Display If Toggle Mode Is On
  if (recTogg == true) {
    display.setCursor(0, 6);
    display.println("+");
  }

  //display.invertDisplay(true);
  delay(10);
  display.display();
}

void timecycle() { //Interrupt Triggers After Each Time Interval
  Serial.println("TIME");
  detachInterrupt(digitalPinToInterrupt(interruptPin)); //Temporarily Disable Inturrupt

  if (cycle > 0) {  //Cycle Set < 0 to Decrease Error

    if (CALABRATE == true) {  //Use The Following Serial Data Output To Calibrate Speed Conversion Number
      Serial.println();

      Serial.print("Cycle: ");
      Serial.println(cycle);  //How Many Recordings Have Occured During This Recording

      Serial.print("Spins per second: ");
      Serial.println(rev);  //Complete Revolutions Occured During Period

      Serial.print("Ticks per second: ");
      Serial.println(ticks);  //Number Of Ticks Recordrd During Period
    }

    totRev += rev;  //Add This Period's Revolutions to Total Revolutions
    totTicks += ticks;  //Add This Period's Ticks to Total Ticks

    dispTicks = ticks;  //For OLED Display

    speedPeriod = ticks * conversionMultiplier;
    if (mph == false) {
      speedPeriod = speedPeriod * 0.44704; //If Mode Set To m/s, Convert To m/s
    }
    if (speedPeriod > speedHigh) {
      speedHigh = speedPeriod;
    }
    //if (cycle == 1) speedLow = speedPeriod;
    if (speedPeriod < speedLow) {
      speedLow = speedPeriod;
    }
    speedTotal = speedTotal + speedPeriod;
    //speedAvg = speedTotal / cycle;

    //Serial.println(speedPeriod);
    //Serial.println(speedAvg);
    //Serial.println(speedHigh);
    //Serial.println(speedLow);
  }
  rev = 0;  //Reset Period Revolutions
  ticks = 0;  //Reset Period Ticks
  cycle++;  //Add One For Next Cycle

  attachInterrupt(digitalPinToInterrupt(interruptPin), pulse, CHANGE);  //Enable Inturrupt
}

void pulse() {  //interrupt pin connected to fan pulse

  ticks++;  //Add +1 Every Time Inturrupt Is Triggered

  if (ticks % 4 == 0) { //If One Revolution Has Occured...
    rev++;  //Add +1 To Revolutions

    pixels.fill(lightred);  //Fill Pixels Light Red
    pixels.fill(red, totRev % 4, 1);  //Move One Bright Pixel Every Revolution
    pixels.show();  //Show Light Changes
  }
}

void units() {  //interrupt pin to change between m/s and mph
  Serial.println("Units");
  int buttonState = digitalRead(pinUni);
  for (int i = 1; i > 0; i++) {
    if (buttonState != HIGH) break;
    buttonState = digitalRead(pinUni);
    Serial.println(i);
    delay(100);
    if (i >= 200) {
      Serial.println("UNITS CHANGE");

      quickBlink(cyan, false, 200);

      mph = !mph;
      break;
    }
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE);
  display.setCursor(20, 36);
  display.print("Units: ");
  if (mph == false) {
    display.print("m/s");
  } else {
    display.print("mph");
  }
  //delay(100);
  if (y) {
    display.display();
    delay(500);
  }
}

void toggle() { //interrupt pin to change through the recording modes

  Serial.println("Toggle");
  int buttonState = digitalRead(pinTog);

  for (int i = 1; i > 0; i++) {
    if (buttonState != HIGH) break;
    buttonState = digitalRead(pinTog);
    Serial.println(i);
    delay(100);
    if (i >= 200) {
      Serial.println("MODE CHANGE");

      quickBlink(cyan, false, 200);
      recTogg = !recTogg;
      attachInterrupt(digitalPinToInterrupt(pinRec), intRecord, RISING);
      break;
    }
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE);
  display.setCursor(15, 36);
  display.print("Toggle: ");
  if (recTogg == false) {
    display.print("OFF");
  } else {
    display.print("ON");
  }
  //delay(100);
  if (y) {
    display.display();
    delay(500);
  }
}

//Sequences ----------------------------------------------------------------------------
void stand(int bef, int aft, int itv, int del, bool reset, uint32_t color) { //Standby neopixel sequence: runs through loop when action == false
  if (!y) {
    for (int j = bef; j <= aft; j++) {
      if (action == true) break;
      //Serial.println(j);
      pixels.clear();
      pixels.fill(color);
      pixels.setBrightness(j);
      pixels.show();
      display.display();
      delay(del);
      j += itv;
    }
    for (int j = aft; j >= bef; j--) { //darken
      if (action == true) break;
      //Serial.println(j);
      pixels.fill(color);
      pixels.setBrightness(j);
      pixels.show();
      display.display();
      delay(del);
      j -= itv;
    }
    Serial.println("standby");
  } else {
    n();
  }
}

void startup() {  //Starting sequence (splashscreen) anamates neopixels before going into loop
  int stBri = 125;
  pixels.setBrightness(stBri);

  pixels.setPixelColor(0, white);
  pixels.setPixelColor(3, white);
  pixels.show();
  delay(500);

  pixels.clear();
  pixels.setPixelColor(1, white);
  pixels.setPixelColor(2, white);
  pixels.show();
  delay(500);

  pixels.clear();
  pixels.setPixelColor(0, white);
  pixels.setPixelColor(1, white);
  pixels.show();
  delay(500);

  pixels.clear();
  pixels.setPixelColor(2, white);
  pixels.setPixelColor(3, white);
  pixels.show();
  delay(500);

  pixels.clear();
  pixels.fill(white, 0, 4);
  pixels.show();
  delay(500);
  if (digitalRead(pinUni) == HIGH && digitalRead(pinTog) == HIGH)y = !y;
  lineWipe(5);
  darken(stBri, 0, 3, 10, true); //(before, after, interval, delay, reset brightness)
  delay(1000);
}

//Screen animations --------------------------------------------------------------------
void lineWipe(int thick) {
  //Serial.println("linewipe");
  for (int i = 64; i < display.width(); i += thick) {
    for (int j = 0; j < thick; j++) {
      display.drawLine((i + j), 0, (i + j), 63, SSD1306_BLACK); //(x1, y1, x2, y2, color)
      display.drawLine(128 - (i + j), 0, 128 - (i + j), 63, SSD1306_BLACK);
    }
    display.display();
    delay(1);
  }
}

void n() {
  static uint16_t j = 0;
  for (int i = 0; i < pixels.numPixels(); i++) {
    if (action == true) break;
    pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
  }
  pixels.show();
  j++;
  if (j >= 256 * 5) j = 0;
  pixels.show();
  delay(50);
}

//Neopixel animations ------------------------------------------------------------------
void quickBlink(uint32_t color, bool blkOut, int wait) {
  pixels.setBrightness(brightness / 2);
  pixels.fill(color);
  pixels.show();
  
  delay (wait);
  
  if (blkOut == true) {
    pixels.fill();
    pixels.show();
  }
}

void darken(int bef, int aft, int itv, int del, bool reset) { //(before, after, interval, delay, reset brightness) //fades down neopixel strip

  for (int j = bef; j >= aft; j - 1) {
    //Serial.println(j);
    pixels.setBrightness(j);
    pixels.show();
    delay(del);
    j -= itv;
  }
  
  pixels.fill();
  pixels.show();

  if (reset) {
    //Serial.println("resetting brightness");
    pixels.setBrightness(brightness);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
