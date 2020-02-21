// Debug options
#define CAN_DEBUG false
#define OUTPUT_DEBUG false

#define THROTTLE_INCREMENT 10
//#define THROTTLE_STALL_POINT 135   // Values above this, the motor won't turn properly

// Don't output more than this using PWM. Scale to make this 100% output
#define MAX_OUTPUT_VALUE 255
#define MID_OUTPUT_VALUE 127
#define MIN_OUTPUT_VALUE  0

#define INVERT_X1 true
#define INVERT_Y1 true
#define INVERT_X2 false
#define INVERT_Y2 true

#define REFERENCE_OFFSET_MULTIPLIER 0.013 // Magic number!
