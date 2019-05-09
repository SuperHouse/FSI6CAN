/**
 * Firmware for the FSI6CAN CAN interface for the Flysky FSi6 transmitter.
 * 
 * Receives messages via CAN bus and generates analog outputs.
 * 
 * By Chris Fryer (chris.fryer78@gmail.com) and Jonathan Oxer (jon@superhouse.tv)
 * www.superhouse.tv
 */

#include <CAN.h>    // CAN bus communication: https://github.com/sandeepmistry/arduino-CAN (available in library manager)

// Configuration should be done in the included file:
#include "config.h"

// Settings for analog output
const int outputX1 = 9;  // Red
const int outputY1 = 6;  // Orange
const int outputX2 = 5;  // Black
const int outputY2 = 3;  // Brown

/*
outputX2 = 9;  // Red
outputY2 = 6;  // Orange
outputX1 = 5;  // Black
outputY1 = 3;  // Brown
*/

int xAxis1Output = 0;
int yAxis1Output = 0;
int xAxis2Output = 0;
int yAxis2Output = 0;

long last_report_time = 0;
unsigned int serial_report_interval = 500;

/*
 * 
 */
void setup() {
  Serial.begin(19200);

  Serial.println("FSi6 CAN bus interface v2.0");
  
  Serial.println("Starting CAN Receiver");
  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  } else {
    Serial.println("Started CAN OK");
  }

  // register the receive callback
  CAN.onReceive(onReceive);

  // Set up analog outputs
  pinMode(outputX1, OUTPUT);
  pinMode(outputY1, OUTPUT);
  pinMode(outputX2, OUTPUT);
  pinMode(outputY2, OUTPUT);

  analogWrite(outputX1, 0);
  analogWrite(outputY1, 0);
  analogWrite(outputX2, 0);
  analogWrite(outputY2, MAX_OUTPUT_VALUE);  // This is to hold right stick down to allow Tx to start
  
  delay(100);
}

/**
 * Loop
 */
void loop() {
  analogWrite(outputX1, xAxis1Output);
  analogWrite(outputY1, yAxis1Output);
  analogWrite(outputX2, xAxis2Output);
  analogWrite(outputY2, yAxis2Output);
  
  if(OUTPUT_DEBUG)
  {
  long current_time = millis();
  if(current_time > (last_report_time + serial_report_interval))
  {
    //Serial.print(xAxis1ReportValue);
    //Serial.print(" | ");
    Serial.print(xAxis1Output);
    Serial.print(" | ");
    Serial.print(yAxis1Output);
    Serial.print(" | ");
    Serial.print(xAxis2Output);
    Serial.print(" | ");
    Serial.print(yAxis2Output);
    Serial.println(" | ");
    last_report_time = current_time;
  }
  }
}

/**
 * Callback when a CAN packet arrives
 */
void onReceive(int packetSize) {
  // received a packet

    int integerValue = 0;    // throw away previous integerValue
    char incomingByte;
    byte is_negative = 0;
    while (CAN.available()) {
      //Serial.print((char)CAN.read());
      incomingByte = (char)CAN.read();
      if(incomingByte == '-')
      {
        is_negative = true;
      } else {
        integerValue *= 10;  // shift left 1 decimal place
        // convert ASCII to integer, add, and shift left 1 decimal place
        integerValue = ((incomingByte - 48) + integerValue);
      }
    }
  
    if(is_negative)
    {
      integerValue = integerValue * -1;
    }

    int axis_output = 0;
    // only print packet data for non-RTR packets
    if(CAN.packetId() == 0x12)  // Packet ID 0x12 for X axis
    {
      if(CAN_DEBUG)
      {
        Serial.print("X=");
        Serial.print(integerValue, DEC);
      }
      axis_output = map(integerValue, -75, +75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
      xAxis1Output = constrain(axis_output, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
    }
    if(CAN.packetId() == 0x13)  // Packet ID 0x13 for Y axis
    {
      if(CAN_DEBUG)
      {
        Serial.print("     Y=");
        Serial.println(integerValue, DEC);
      }
      //yAxis1Output = integerValue;
      axis_output = map(integerValue, -75, +75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
      yAxis1Output = constrain(axis_output, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
    }
}


void onReceiveOriginal(int packetSize) {
  // received a packet

  /*
  if (CAN.packetRtr()) {
    Serial.print(" and requested length ");
    Serial.println(CAN.packetDlc());
  } else {
    Serial.print(" and length ");
    Serial.println(packetSize);
  */
  int integerValue = 0;         // throw away previous integerValue
  char incomingByte;
  byte is_negative = 0;
  while (CAN.available()) {
    //Serial.print((char)CAN.read());
    incomingByte = (char)CAN.read();
    if(incomingByte == '-')
    {
      is_negative = true;
    } else {
      integerValue *= 10;  // shift left 1 decimal place
      // convert ASCII to integer, add, and shift left 1 decimal place
      integerValue = ((incomingByte - 48) + integerValue);
    }
  }
  if(is_negative)
  {
    integerValue = integerValue * -1;
  }

  int axis_output = 0;
  // only print packet data for non-RTR packets
  if(CAN.packetId() == 0x12)  // Packet ID 0x12 for X axis
  {
    //Serial.print("X=");
    //Serial.print(integerValue, DEC);
    axis_output = map(integerValue, -110, +110, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
    xAxis1Output = constrain(axis_output, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
  }
  if(CAN.packetId() == 0x13)  // Packet ID 0x13 for Y axis
  {
    //Serial.print("     Y=");
    //Serial.println(integerValue, DEC);
    //yAxis1Output = integerValue;
    axis_output = map(integerValue, -110, +110, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
    yAxis1Output = constrain(axis_output, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
  }
    
  //  while (CAN.available()) {
  //    Serial.print((char)CAN.read());
  //  }
  //Serial.println();
}
