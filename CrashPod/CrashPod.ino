//includes
#include <SparkFun_LTE_Shield_Arduino_Library.h>
#include <Wire.h> //Needed for I2C to GNSS
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <MPU6050.h>
#include <String.h>

//Definitions
#define BUTTON_LED 2
#define BUTTON 3
#define MPU_INT 11
#define SPEAKER A1
#define SerialMonitor Serial
#define LTEShieldSerial Serial1


const mobile_network_operator_t MOBILE_NETWORK_OPERATOR = MNO_SW_DEFAULT;
const String MOBILE_NETWORK_STRINGS[] = {"Default", "SIM_ICCD", "AT&T", "VERIZON", 
  "TELSTRA", "T-Mobile", "CT"};

// APN -- Access Point Name. Gateway between GPRS MNO
// and another computer network. E.g. "hologram
const String APN = "hologram";
#define MAX_OPERATORS 5
//#define DEBUG_PASSTHROUGH_ENABLED

//LTE Globals
LTE_Shield lte;

//GNSS (gps) Globals
SFE_UBLOX_GNSS myGNSS;
long lastTime = 0; //timer to stop gps congestion (calls take a while)
long latitude, longitude, altitude;
byte SIV;

//mpu6050 Globals
MPU6050 mpu;
boolean freefallDetected = false;

//button Global
boolean buttonPressed = false;

void setup() {
  Serial.begin(9600);
  //while(!Serial);//wait for serial to open
  Wire.begin();
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(BUTTON_LED, OUTPUT);
  pinMode(SPEAKER, OUTPUT);

  gnss_setup();
  lte_setup();
  mpu_setup();
  attachInterrupt(BUTTON, buttonInterrupt, RISING); 
  
  tone(SPEAKER, 440);
  delay(500);
  tone(SPEAKER, 550);
  delay(500);
  noTone(SPEAKER);  
}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - lastTime > 5000)
  {
    lastTime = millis(); //Update the timer
    updateGNSS();
    //Serial.println(freefallDetected, DEC);
  }
  if(freefallDetected){
    manageAlert();
  }
}

void lte_setup(void){
  int opsAvailable;
  struct operator_stats ops[MAX_OPERATORS];
  String currentOperator = "";
  bool newConnection = true;

  //SerialMonitor.begin(9600);
  //while (!SerialMonitor) ; // For boards with built-in USB

  Serial.println(F("Initializing the LTE Shield..."));
  Serial.println(F("...this may take ~25 seconds if the shield is off."));
  Serial.println(F("...it may take ~5 seconds if it just turned on."));
  
  // Call lte.begin and pass it your Serial/SoftwareSerial object to 
  // communicate with the LTE Shield.
  // Note: If you're using an Arduino with a dedicated hardware serial
  // port, you may instead slide "Serial" into this begin call.
  if ( lte.begin(LTEShieldSerial, 9600) ) { //expects software serial, but hw serial works despite warning
    Serial.println(F("LTE Shield connected!\r\n"));
  } else {
    Serial.println("Unable to initialize the shield.");
    while(1) ;
  }

  // First check to see if we're already connected to an operator:
  if (lte.getOperator(&currentOperator) == LTE_SHIELD_SUCCESS) {
    Serial.print(F("Connected to: "));
    Serial.println(currentOperator);
    return;
  }

  if (newConnection) {
    // Set MNO to either Verizon, T-Mobile, AT&T, Telstra, etc.
    // This will narrow the operator options during our scan later
    Serial.println(F("Setting mobile-network operator"));
    if (lte.setNetwork(MOBILE_NETWORK_OPERATOR)) {
      Serial.print(F("Set mobile network operator to "));
      Serial.println(MOBILE_NETWORK_STRINGS[MOBILE_NETWORK_OPERATOR] + "\r\n");
    } else {
      Serial.println(F("Error setting MNO. Try cycling power to the shield/Arduino."));
      while (1) ;
    }
    
    // Set the APN -- Access Point Name -- e.g. "hologram"
    Serial.println(F("Setting APN..."));
    if (lte.setAPN(APN) == LTE_SHIELD_SUCCESS) {
      Serial.println(F("APN successfully set.\r\n"));
    } else {
      Serial.println(F("Error setting APN. Try cycling power to the shield/Arduino."));
      while (1) ;
    }
    /*
    // Wait for user to press button before initiating network scan.
    SerialMonitor.println(F("Press any key scan for networks.."));
    serialWait();
    */
    Serial.println(F("Scanning for operators...this may take up to 3 minutes\r\n"));
    // lte.getOperators takes in a operator_stats struct pointer and max number of
    // structs to scan for, then fills up those objects with operator names and numbers
    opsAvailable = lte.getOperators(ops, MAX_OPERATORS); // This will block for up to 3 minutes

    if (opsAvailable > 0) {
      // Pretty-print operators we found:
      Serial.println("Found " + String(opsAvailable) + " operators:");
      printOperators(ops, opsAvailable);

      // Wait until the user presses a key to initiate an operator connection
      //SerialMonitor.println("Press 1-" + String(opsAvailable) + " to select an operator.");
      /*
      char c = 0;
      bool selected = false;
      */
      char c = 1;
      bool selected = true;
      //while (!selected) {
        //while (!SerialMonitor.available()) ;
        c = Serial.read();
        int selection = c - '0';
        if ((selection >= 1) && (selection <= opsAvailable)) {
          selected = true;
          Serial.println("Connecting to option " + String(selection));
          if (lte.registerOperator(ops[selection - 1]) == LTE_SHIELD_SUCCESS) {
            Serial.println("Network " + ops[selection - 1].longOp + " registered\r\n");
          } else {
            Serial.println(F("Error connecting to operator. Reset and try again, or try another network."));
          }
        }
      //}
    } else {
      Serial.println(F("Did not find an operator. Double-check SIM and antenna, reset and try again, or try another network."));
      while (1) ;
    }
  }

  // At the very end print connection information
  printInfo();
}

void printInfo(void) {
  String currentApn = "";
  IPAddress ip(0, 0, 0, 0);
  String currentOperator = "";

  Serial.println(F("Connection info:"));
  // APN Connection info: APN name and IP
  if (lte.getAPN(&currentApn, &ip) == LTE_SHIELD_SUCCESS) {
    Serial.println("APN: " + String(currentApn));
    Serial.print("IP: ");
    Serial.println(ip);
  }

  // Operator name or number
  if (lte.getOperator(&currentOperator) == LTE_SHIELD_SUCCESS) {
    Serial.print(F("Operator: "));
    Serial.println(currentOperator);
  }

  // Received signal strength
  Serial.println("RSSI: " + String(lte.rssi()));
  Serial.println();
}

void printOperators(struct operator_stats * ops, int operatorsAvailable) {
  for (int i = 0; i < operatorsAvailable; i++) {
    Serial.print(String(i + 1) + ": ");
    Serial.print(ops[i].longOp + " (" + String(ops[i].numOp) + ") - ");
    switch (ops[i].stat) {
    case 0:
      Serial.println(F("UNKNOWN"));
      break;
    case 1:
      Serial.println(F("AVAILABLE"));
      break;
    case 2:
      Serial.println(F("CURRENT"));
      break;
    case 3:
      Serial.println(F("FORBIDDEN"));
      break;
    }
  }
  Serial.println();
}

void serialWait() {
  while (!Serial.available()) ;
  while (Serial.available()) Serial.read();
}

void gnss_setup(){
  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }
  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
}

void updateGNSS(){
    latitude = myGNSS.getLatitude();
    Serial.print(F("Lat: "));
    Serial.print(latitude);

    longitude = myGNSS.getLongitude();
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));

    altitude = myGNSS.getAltitude();
    Serial.print(F(" Alt: "));
    Serial.print(altitude);
    Serial.print(F(" (mm)"));

    SIV = myGNSS.getSIV();
    Serial.print(F(" SIV: "));
    Serial.print(SIV);

    Serial.println();
}

void fallInterrupt()
{
  freefallDetected = true;  
}

void mpu_setup(){
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
  
  mpu.setIntFreeFallEnabled(true);
  mpu.setIntZeroMotionEnabled(false);
  mpu.setIntMotionEnabled(false);
  
  mpu.setDHPFMode(MPU6050_DHPF_5HZ);

  mpu.setFreeFallDetectionThreshold(17);
  mpu.setFreeFallDetectionDuration(2); 
  attachInterrupt(MPU_INT, fallInterrupt, RISING); 
}

void manageAlert(){
  delay(600); //in case something jostles on impact
  mpu.readActivites(); //reads int regs to reset interrupt  
  long count = millis();
  buttonPressed = 0;
  freefallDetected = 0;  
  tone(SPEAKER, 550);
  digitalWrite(BUTTON_LED, HIGH);
  while(millis() < count + 30000){ //give biker 30 seconds to respond 
    if(buttonPressed){
      buttonPressed = 0;
      noTone(SPEAKER);
      digitalWrite(BUTTON_LED, LOW);
      return;
    }else{
      delay(50);
    }
  }
  sendAlertText();
  noTone(SPEAKER);  
  while(1){
    digitalWrite(BUTTON_LED, LOW);
    noTone(SPEAKER);
    delay(1000);
    digitalWrite(BUTTON_LED, HIGH);
    tone(SPEAKER, 550);
    delay(1000);
  }
}

void sendAlertText(){
  String f_latitude = String(float(latitude)/10000000, 10);
  String f_longitude = String(float(longitude)/10000000, 10);  
  String msg = "http://maps.google.com/maps?q=" + f_latitude + "," + f_longitude;
  msg = "Zachary may have been in a bike accident. Last known location: \n" + msg;
  lte.sendSMS("16502857655", msg);
  Serial.println(msg);
}

void buttonInterrupt(){
  buttonPressed = 1;
}