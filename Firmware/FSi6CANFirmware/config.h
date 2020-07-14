/* ----------------- General config -------------------------------- */
/* Joystick arrangement */
#define JOYSTICK_MODE       3         // 1, 2, 3, or 4
#define INVERT_X1        true
#define INVERT_Y1        true
#define INVERT_X2       false
#define INVERT_Y2        true

/* Ratcheting throttle */
#define THROTTLE_INCREMENT 30
//#define THROTTLE_STALL_POINT 135   // Values above this, the motor won't turn properly

/* Serial */
#define   SERIAL_BAUD_RATE          115200       // Speed for USB serial console
uint32_t  g_serial_telemetry_period  = 500;      // Serial report interval in milliseconds

/* Debugging */
#define CAN_DEBUG       false
#define OUTPUT_DEBUG    false

/* ----------------- Hardware-specific config ---------------------- */
/* Analog outputs to Flysky transmitter */
const uint8_t X1_OUTPUT_PIN = 3;  // Red
const uint8_t Y1_OUTPUT_PIN = 6;  // Orange
const uint8_t X2_OUTPUT_PIN = 9;  // Black
const uint8_t Y2_OUTPUT_PIN = 5;  // Brown

// Don't output more than this using PWM. Scale to make this 100% output
#define MAX_OUTPUT_VALUE  255
#define MID_OUTPUT_VALUE  127
#define MIN_OUTPUT_VALUE    0
