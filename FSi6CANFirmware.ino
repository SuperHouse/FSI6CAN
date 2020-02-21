/**
 * Firmware for the FSI6CAN CAN interface for the Flysky FSi6 transmitter.
 * 
 * Receives messages via CAN bus and generates analog outputs.
 * 
 * In the Arduino IDE, select board profile:
 *  -> Arduino Pro or Pro Mini
 *    -> ATmega328P (3.3V, 8MHz)
 * 
 * By Chris Fryer (chris.fryer78@gmail.com) and Jonathan Oxer (jon@superhouse.tv)
 * www.superhouse.tv
 */

#include <CAN.h>    // CAN bus communication: https://github.com/sandeepmistry/arduino-CAN (available in library manager)

// Configuration should be done in the included file:
#include "config.h"

// Settings for analog output
/*
const int outputX1 = 9;  // Red
const int outputY1 = 6;  // Orange
const int outputX2 = 5;  // Black
const int outputY2 = 3;  // Brown
*/

const int outputX1 = 3;  // Red      Rudder
const int outputY1 = 6;  // Orange   Elevator
const int outputX2 = 9;  // Black
const int outputY2 = 5;  // Brown    Throttle

/*
outputX2 = 9;  // Red
outputY2 = 6;  // Orange
outputX1 = 5;  // Black
outputY1 = 3;  // Brown
*/

int xAxis1Output = 0;
int yAxis1Output = 0;
int xAxis2Output = 0;
int yAxis2Output = 255;  // Maximum output is stick full down (throttle off)

long last_report_time = 0;
unsigned int serial_report_interval = 500;

int throttle_position = 255;  // Maximum output is stick full down (throttle off)
char incoming_message_buffer[12];  // Buffer for incoming CAN messages

/*
 * 
 */
void setup() {
  Serial.begin(9600);
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
  yAxis2Output = throttle_position;

  analogWrite(outputX1, xAxis1Output);
  analogWrite(outputY1, yAxis1Output);
  //analogWrite(outputX2, xAxis2Output);
  analogWrite(outputY2, yAxis2Output);
  
  if(OUTPUT_DEBUG)
  {
    long current_time = millis();
    if(current_time > (last_report_time + serial_report_interval))
    {
      //Serial.print(xAxis1ReportValue);
      //Serial.print(" | ");
      Serial.print("X1:");
      Serial.print(xAxis1Output);
      
      Serial.print(" |Y1:");
      Serial.print(yAxis1Output);
      
      Serial.print(" |X2:");
      Serial.print(xAxis2Output);
      
      Serial.print(" |Y2:");
      Serial.print(yAxis2Output);
      
      Serial.println(" |");
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
    int i = 0;
    if(CAN_DEBUG)
    {
      Serial.print("ID: 0x");
      Serial.print(CAN.packetId(), HEX);
      Serial.print(" Value: ");
    }
    while (CAN.available()) {
      incomingByte = (char)CAN.read();
      if(CAN_DEBUG)
      {
        Serial.print(incomingByte - 48);
      }
      incoming_message_buffer[i] = incomingByte;
      i++;
      if(incomingByte == '-')
      {
        is_negative = true;
      } else {
        integerValue *= 10;  // shift left 1 decimal place
        // convert ASCII to integer, add, and shift left 1 decimal place
        integerValue = ((incomingByte - 48) + integerValue);
      }
    }
    if(CAN_DEBUG)
    {
      Serial.println("");
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
      if(INVERT_X1)
      {
        axis_output = map(integerValue, +75, -75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
      } else {
        axis_output = map(integerValue, -75, +75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
      }
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
      if(INVERT_Y1)
      {
        axis_output = map(integerValue, +75, -75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
      } else {
        axis_output = map(integerValue, -75, +75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
      }
      yAxis1Output = constrain(axis_output, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
    }
    
    if(CAN.packetId() == 0x14)  // Packet ID 0x14 for buttons
    {
      if(CAN_DEBUG)
      {
        Serial.print("CAN key: ");
        Serial.println(incoming_message_buffer[0]);
      }
      // Check for specific keys to use for throttle up and down
      if(incoming_message_buffer[0] == 'w' && incoming_message_buffer[1] == '+')  // Throttle up
      {
        if(INVERT_Y2)
        {
          throttle_position = throttle_position - THROTTLE_INCREMENT;
        } else {
          throttle_position = throttle_position + THROTTLE_INCREMENT;
        }
      }

      if(incoming_message_buffer[0] == 'v' && incoming_message_buffer[1] == '+')  // Throttle down
      {
        if(INVERT_Y2)
        {
          throttle_position = throttle_position + THROTTLE_INCREMENT;
        } else {
          throttle_position = throttle_position - THROTTLE_INCREMENT;
        }
      }

      // Enforce limits
      if(throttle_position > 255)
      {
        throttle_position = 255;
      }
      if(throttle_position < 0)
      {
        throttle_position = 0;
      }

      if(OUTPUT_DEBUG)
      {
        Serial.print("New throttle position: ");
        Serial.println(throttle_position);
      }
    }
}
