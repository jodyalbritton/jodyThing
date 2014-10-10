//*****************************************************************************
/// @file
/// @brief
///   Arduino SmartThings Shield LED Example with Network Status - Motion Sensor - Illumination - Temperature and Humidity
/// @note
///              ______________
///             |              |
///             |         SW[] |
///             |[]RST         |
///             |         AREF |--
///             |          GND |--
///             |           13 |--X LED
///             |           12 |--
///             |           11 |--
///           --| 3.3V      10 |--
///           --| 5V         9 |--
///           --| GND        8 |--
///           --| GND          |
///           --| Vin        7 |--X PIR_MOTION_SENSOR
///             |            6 |--
//     Illum X--| A0         5 |--X DHT11 Temp and Humidity
///           --| A1    ( )  4 |--
///           --| A2         3 |--X THING_RX
///           --| A3  ____   2 |--X THING_TX
///           --| A4 |    |  1 |--
///           --| A5 |    |  0 |--
///             |____|    |____|
///                  |____|
///
//*****************************************************************************
#include <SoftwareSerial.h>   //TODO need to set due to some weird wire language linker, should we absorb this whole library into smartthings
#include <SmartThings.h>
#include <DHT.h>



//*****************************************************************************
// Pin Definitions    | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                    V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
#define PIN_LED         13
#define PIN_THING_RX    3
#define PIN_THING_TX    2
#define PIR_MOTION_SENSOR 7
#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

//*****************************************************************************
// Global Variables   | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                    V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
SmartThingsCallout_t messageCallout;    // call out function forward decalaration
SmartThings smartthing(PIN_THING_RX, PIN_THING_TX, messageCallout);  // constructor

bool isDebugEnabled;    // enable or disable debug in this example
int stateLED;           // state to track last set value of LED
int stateNetwork;       // state of the network 

int calibrationTime = 60;        

//the time when the sensor outputs a low impulse
long unsigned int lowIn;         

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 5000;  

boolean lockLow = true;
boolean takeLowTime;  

//*****************************************************************************
// Local Functions  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                  V V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
void on()
{
  stateLED = 1;                 // save state as 1 (on)
  digitalWrite(PIN_LED, HIGH);  // turn LED on
  smartthing.shieldSetLED(0, 0, 2);
  smartthing.send("on");        // send message to cloud
}

//*****************************************************************************
void off()
{
  stateLED = 0;                 // set state to 0 (off)
  digitalWrite(PIN_LED, LOW);   // turn LED off
  smartthing.shieldSetLED(0, 0, 0);
  smartthing.send("off");       // send message to cloud
}


//*****************************************************************************
void setNetworkStateLED()
{
  SmartThingsNetworkState_t tempState = smartthing.shieldGetLastNetworkState();
  if (tempState != stateNetwork)
  {
    switch (tempState)
    {
      case STATE_NO_NETWORK:
        if (isDebugEnabled) Serial.println("NO_NETWORK");
        smartthing.shieldSetLED(2, 0, 0); // red
        break;
      case STATE_JOINING:
        if (isDebugEnabled) Serial.println("JOINING");
        smartthing.shieldSetLED(2, 0, 0); // red
        break;
      case STATE_JOINED:
        if (isDebugEnabled) Serial.println("JOINED");
        smartthing.shieldSetLED(0, 0, 0); // off
        break;
      case STATE_JOINED_NOPARENT:
        if (isDebugEnabled) Serial.println("JOINED_NOPARENT");
        smartthing.shieldSetLED(2, 0, 2); // purple
        break;
      case STATE_LEAVING:
        if (isDebugEnabled) Serial.println("LEAVING");
        smartthing.shieldSetLED(2, 0, 0); // red
        break;
      default:
      case STATE_UNKNOWN:
        if (isDebugEnabled) Serial.println("UNKNOWN");
        smartthing.shieldSetLED(0, 2, 0); // green
        break;
    }
    stateNetwork = tempState; 
  }
}

//*****************************************************************************
// API Functions    | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                  V V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
void setup()
{
  // setup default state of global variables
  isDebugEnabled = true;
  stateLED = 0;                 // matches state of hardware pin set below
  stateNetwork = STATE_JOINED;  // set to joined to keep state off if off

  
  //DHT 11
  
 
  // setup hardware pins 
  pinMode(PIR_MOTION_SENSOR, INPUT);
  digitalWrite(PIR_MOTION_SENSOR, LOW);
  pinMode(PIN_LED, OUTPUT);     // define PIN_LED as an output
  digitalWrite(PIN_LED, LOW);   // set value to LOW (off) to match stateLED=0
  
  
  
  
  dht.begin();
  
  if (isDebugEnabled)
  { // setup debug serial port
    Serial.begin(9600);         // setup serial with a baud rate of 9600
    Serial.println("setup..");  // print out 'setup..' on start
    //give the sensor some time to calibrate
    Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    delay(50);
  }
}

//*****************************************************************************

// Keep Track of Sensor Readings
int   currentLight;
int   currentHumidity;
int   currentTemperature; 


// Intervals 
unsigned long   lightInterval    = (     1 * 1000); 
unsigned long   humidityInterval = (    25 * 1000);
unsigned long   temperatureInterval = (    25 * 1000); 
unsigned long   reportInterval   = (2 * 60 * 1000); 
unsigned long   motionInterval   = (    60 * 1000);

unsigned long   lastHumidityCheckAt  = 0;
unsigned long   lastTempCheckAt      = 0;
unsigned long   lastLightCheckAt     = 0; 
unsigned long 	lastReportAt         = 0; 
unsigned long   motionArmed          = 0;


void loop()
{
  // run smartthing logic
  smartthing.run();
  
  // Timing code 
	unsigned long currentMillis = millis();
        
	// First and at checkInterval 
	if (currentMillis - lastHumidityCheckAt > humidityInterval || lastHumidityCheckAt == 0)
	{
		lastHumidityCheckAt = currentMillis;

		// Do a check 
		checkHumidity();

	}
        if (currentMillis - lastTempCheckAt > temperatureInterval || lastTempCheckAt == 0)
	{
		lastTempCheckAt = currentMillis;

		// Do a check 
		checkTemperature();

	}

	// First and at checkInterval 
	if (currentMillis - lastLightCheckAt > lightInterval || lastLightCheckAt == 0)
	{
		lastLightCheckAt = currentMillis;

		// Do a check 
		checkLight();
               

	}

	// First and at reportInterval 
	if (currentMillis - lastReportAt > reportInterval || lastReportAt == 0)
	{
		lastReportAt = currentMillis;

		// Do a report  
		reportData();

	}
  detectMotion();
  setNetworkStateLED();
  

               
 
		


 
}
void checkData()
{
	checkHumidity();
        checkTemperature();
	checkLight();
        
}

void checkLight() 
{
	Serial.println("Checking light...");

	//int light = ( (int) ( ( (float) analogRead(0)) * (100.0 / 1023.0) ) * 2) / 2; 
        int light = ((float) analogRead(0)); 
	// If notice a large change, we let them know immediately.
	if (light > currentLight + 10 || light < currentLight - 10)
	{
		currentLight = light; 
		reportData();
	}

	currentLight = light; 
	
	Serial.println(currentLight);

}


void detectMotion() 
{
	if(digitalRead(PIR_MOTION_SENSOR) == HIGH)//if the sensor value is HIGH?
	{
         
          String motionMessage = "active"; 
	  
         if(lockLow){  
         //makes sure we wait for a transition to LOW before any further output is made:
         lockLow = false;            
         Serial.println("---");
         Serial.print("motion detected at ");
         Serial.print(millis()/1000);
         Serial.println(" sec"); 
         smartthing.send(motionMessage);
         Serial.println("Motion Detected");
         delay(50);
         }         
         takeLowTime = true;
      
                
                
	}
	if (digitalRead(PIR_MOTION_SENSOR) == LOW)
	{
          
          String motionMessage = "inactive";
	  
        if(takeLowTime){
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }
       //if the sensor is low for more than the given pause, 
       //we assume that no more motion is going to happen
       if(!lockLow && millis() - lowIn > pause){  
           //makes sure this block of code is only executed again after 
           //a new motion sequence has been detected
           smartthing.send(motionMessage);
           lockLow = true;                        
           Serial.print("motion ended at ");      //output
           Serial.print((millis() - pause)/1000);
           Serial.println(" sec");
           delay(500);
           }
       
          
	}
}

// Keep a hot cache with data, ready to deliver to respond to a poll instantly 
void checkHumidity()   
{
	
        
        Serial.println("Checking humidity...");

	// Read humidity
	float h = dht.readHumidity();
        
        
        	
        
	// Discard any data that is NaN
	if (isnan(h)) 
	{
		Serial.println("Failed to read from DHT");
	} else {
                
                Serial.print("Humidity:");
                Serial.println(h);
		currentHumidity = h;
	}
	
	


}

void checkTemperature()
{
  Serial.println("Checking temperature...");

	// Read temperature 
	
        float t = dht.readTemperature(true);
        
        	
        
	// Discard any data that is NaN
	if (isnan(t)) 
	{
		Serial.println("Failed to read from DHT");
	} else {
                Serial.print("Temp:");
                Serial.println(t);
		currentTemperature = t;
           
	}
  
}

void reportData()
{
	Serial.println("Reporting data...");

	// We must insist on actually having some data to send 
	if (isnan(currentHumidity) || isnan(currentLight)|| isnan(currentTemperature)) {
		
		

		Serial.println("We're not in a loop are we?");

		checkData(); 
		reportData();
		return; 
	}

	

	// Report humidity 
	String humidityMessage = "h";
	humidityMessage.concat(currentHumidity); 
	smartthing.send(humidityMessage);
        Serial.println(humidityMessage);
        
        // Report Temp 
	String tempMessage = "t";
	tempMessage.concat(currentTemperature); 
	smartthing.send(tempMessage);

	// Report light 
	String msg2 = "l";
	msg2 += currentLight; 
	smartthing.send(msg2);

	
	

}


//*****************************************************************************
void messageCallout(String message)
{
  // if debug is enabled print out the received message
  if (isDebugEnabled)
  {
    Serial.print("Received message: '");
    Serial.print(message);
    Serial.println("' ");
  }

  // if message contents equals to 'on' then call on() function
  // else if message contents equals to 'off' then call off() function
  if (message.equals("on"))
  {
    on();
  }
  else if (message.equals("off"))
  {
    off();
  }
   
  else if (message.equals("poll"))
  {
    reportData();
  }
  
  
}


