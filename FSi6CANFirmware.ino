/**
   Firmware for the FSI6CAN CAN interface for the Flysky FSi6 transmitter.
   Version: 2.2.

   Receives messages via CAN bus and generates analog outputs.

   By Chris Fryer (chris.fryer78@gmail.com) and Jonathan Oxer (jon@oxer.com.au)
   www.superhouse.tv
   
   In the Arduino IDE, select board profile:
    -> Arduino Pro or Pro Mini
      -> ATmega328P (3.3V, 8MHz)

   External dependency, available in the Arduino library manager:
     "CAN" by Sandeep Mistry
*/

/*--------------------------- Configuration ------------------------------*/
// Configuration should be done in the included file:
#include "config.h"

/*--------------------------- Libraries ----------------------------------*/
#include <CAN.h>    // CAN bus: https://github.com/sandeepmistry/arduino-CAN

/*--------------------------- Global Variables ---------------------------*/
/* Mode 1 */
#if JOYSTICK_MODE == 1
const uint8_t g_output_rudder   = X1_OUTPUT_PIN;  // Rudder
const uint8_t g_output_elevator = Y1_OUTPUT_PIN;  // Elevator
const uint8_t g_output_ailerons = X2_OUTPUT_PIN;  // Ailerons
const uint8_t g_output_throttle = Y2_OUTPUT_PIN;  // Throttle
#endif

/* Mode 2 */
#if JOYSTICK_MODE == 2
const uint8_t g_output_rudder   = X1_OUTPUT_PIN;  // Rudder
const uint8_t g_output_elevator = Y2_OUTPUT_PIN;  // Elevator
const uint8_t g_output_ailerons = X2_OUTPUT_PIN;  // Ailerons
const uint8_t g_output_throttle = Y1_OUTPUT_PIN;  // Throttle
#endif

/* Mode 3 */
#if JOYSTICK_MODE == 3
const uint8_t g_output_rudder   = X2_OUTPUT_PIN;  // Rudder
const uint8_t g_output_elevator = Y1_OUTPUT_PIN;  // Elevator
const uint8_t g_output_ailerons = X1_OUTPUT_PIN;  // Ailerons
const uint8_t g_output_throttle = Y2_OUTPUT_PIN;  // Throttle
#endif

/* Mode 4 */
#if JOYSTICK_MODE == 4
const uint8_t g_output_rudder   = X2_OUTPUT_PIN;  // Rudder
const uint8_t g_output_elevator = Y2_OUTPUT_PIN;  // Elevator
const uint8_t g_output_ailerons = X1_OUTPUT_PIN;  // Ailerons
const uint8_t g_output_throttle = Y1_OUTPUT_PIN;  // Throttle
#endif

uint8_t  g_xAxis1Output      =   0;
uint8_t  g_yAxis1Output      =   0;
uint8_t  g_xAxis2Output      =   0;
uint8_t  g_yAxis2Output      = MAX_OUTPUT_VALUE;  // Maximum output is stick full down (throttle off)
uint8_t  g_throttle_position = 255;               // Maximum output is stick full down (throttle off)
char     g_incoming_message_buffer[12];           // Buffer for incoming CAN messages

// Serial
uint32_t g_last_serial_report_time = 0;           // Timestamp of last report to serial console

/*--------------------------- Function Signatures ---------------------------*/
void onReceive(int16_t packetSize);

/*--------------------------- Instantiate Global Objects --------------------*/
// None

/*--------------------------- Program ---------------------------------------*/
/**
  Setup
*/
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("FSi6 CAN bus interface v2.2");
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
  pinMode(g_output_rudder,   OUTPUT);
  pinMode(g_output_elevator, OUTPUT);
  pinMode(g_output_ailerons, OUTPUT);
  pinMode(g_output_throttle, OUTPUT);

  analogWrite(g_output_rudder,   0);
  analogWrite(g_output_elevator, 0);
  analogWrite(g_output_ailerons, 0);
  analogWrite(g_output_throttle, MAX_OUTPUT_VALUE);  // This is to hold right stick down to allow Tx to start

  delay(100);
}

/**
  Loop
*/
void loop() {
  g_yAxis2Output = g_throttle_position;

  analogWrite(g_output_rudder,   g_xAxis1Output);
  analogWrite(g_output_elevator, g_yAxis1Output);
  analogWrite(g_output_ailerons, g_xAxis2Output);
  analogWrite(g_output_throttle, g_yAxis2Output);

  if (OUTPUT_DEBUG)
  {
    uint32_t current_time = millis();
    if (current_time > (g_last_serial_report_time + g_serial_telemetry_period))
    {
      //Serial.print(xAxis1ReportValue);
      //Serial.print(" | ");
      Serial.print("X1:");
      Serial.print(g_xAxis1Output);

      Serial.print(" |Y1:");
      Serial.print(g_yAxis1Output);

      Serial.print(" |X2:");
      Serial.print(g_xAxis2Output);

      Serial.print(" |Y2:");
      Serial.print(g_yAxis2Output);

      Serial.println(" |");
      g_last_serial_report_time = current_time;
    }
  }
}

/**
  Callback when a CAN packet arrives
*/
void onReceive(int16_t packetSize) {
  // received a packet

  int integerValue = 0;    // throw away previous integerValue
  char incomingByte;
  byte is_negative = 0;
  uint8_t i = 0;
  if (CAN_DEBUG)
  {
    Serial.print("ID: 0x");
    Serial.print(CAN.packetId(), HEX);
    Serial.print(" Value: ");
  }
  while (CAN.available()) {
    incomingByte = (char)CAN.read();
    if (CAN_DEBUG)
    {
      Serial.print(incomingByte - 48);
    }
    g_incoming_message_buffer[i] = incomingByte;
    i++;
    if (incomingByte == '-')
    {
      is_negative = true;
    } else {
      integerValue *= 10;  // shift left 1 decimal place
      // convert ASCII to integer, add, and shift left 1 decimal place
      integerValue = ((incomingByte - 48) + integerValue);
    }
  }
  if (CAN_DEBUG)
  {
    Serial.println("");
  }


  if (is_negative)
  {
    integerValue = integerValue * -1;
  }

  int axis_output = 0;
  // only print packet data for non-RTR packets
  if (CAN.packetId() == 0x12) // Packet ID 0x12 for X axis
  {
    if (CAN_DEBUG)
    {
      Serial.print("X=");
      Serial.print(integerValue, DEC);
    }
    if (INVERT_X1)
    {
      axis_output = map(integerValue, +75, -75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
    } else {
      axis_output = map(integerValue, -75, +75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
    }
    g_xAxis1Output = constrain(axis_output, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
  }

  if (CAN.packetId() == 0x13) // Packet ID 0x13 for Y axis
  {
    if (CAN_DEBUG)
    {
      Serial.print("     Y=");
      Serial.println(integerValue, DEC);
    }
    //g_yAxis1Output = integerValue;
    if (INVERT_Y1)
    {
      axis_output = map(integerValue, +75, -75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
    } else {
      axis_output = map(integerValue, -75, +75, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
    }
    g_yAxis1Output = constrain(axis_output, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE);
  }

  if (CAN.packetId() == 0x14) // Packet ID 0x14 for buttons
  {
    if (CAN_DEBUG)
    {
      Serial.print("CAN key: ");
      Serial.println(g_incoming_message_buffer[0]);
    }
    // Check for specific keys to use for throttle up and down
    if (g_incoming_message_buffer[0] == 'w' && g_incoming_message_buffer[1] == '+') // Throttle up
    {
      if (INVERT_Y2)
      {
        if (g_throttle_position < THROTTLE_INCREMENT) // Bounds checking to prevent <0
        {
          g_throttle_position = MIN_OUTPUT_VALUE;
        } else {
          g_throttle_position = g_throttle_position - THROTTLE_INCREMENT;
        }
      } else {
        if (g_throttle_position > (MAX_OUTPUT_VALUE - THROTTLE_INCREMENT)) // Bounds checking to prevent >255
        {
          g_throttle_position = MAX_OUTPUT_VALUE;
        } else {
          g_throttle_position = g_throttle_position + THROTTLE_INCREMENT;
        }
      }
    }

    if (g_incoming_message_buffer[0] == 'v' && g_incoming_message_buffer[1] == '+') // Throttle down
    {
      if (INVERT_Y2)
      {
        if (g_throttle_position > (MAX_OUTPUT_VALUE - THROTTLE_INCREMENT)) // Bounds checking to prevent >255
        {
          g_throttle_position = MAX_OUTPUT_VALUE;
        } else {
          g_throttle_position = g_throttle_position + THROTTLE_INCREMENT;
        }
      } else {
        if (g_throttle_position < THROTTLE_INCREMENT) // Bounds checking to prevent <0
        {
          g_throttle_position = MIN_OUTPUT_VALUE;
        } else {
          g_throttle_position = g_throttle_position - THROTTLE_INCREMENT;
        }
      }
    }

    if (OUTPUT_DEBUG)
    {
      Serial.print("New throttle position: ");
      Serial.println(g_throttle_position);
    }
  }
}
