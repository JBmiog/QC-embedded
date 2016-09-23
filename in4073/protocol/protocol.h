#ifndef _protocol_h
#define _protocol_h

/* protcol header file, JMI  */

// mode
#define SAFE_MODE               0x00
#define PANIC_MODE              0x01
#define MANUAL_MODE             0x02
#define CALIBRATION_MODE        0x03
#define YAW_CONTROLLED_MODE     0x04
#define FULL_CONTROL_MODE       0x05
#define RAW_MODE                0x06
#define HEIGHT_CONTROL_MODE     0x07
#define WIRELESS_MODE           0x08

#define p_up					0x01
#define p_down					0x02
#define p1_up					0x04
#define p1_down					0x08
#define p2_up					0x10
#define p2_down					0x80

// data
#define UP           			0x9
#define DOWN         			0xF7
#define HEADER_VALUE 			0x80
typedef struct {
	char header;
	char mode;
	char p_adjust;
	char lift;
	char pitch;
	char roll;
	char yaw;
	char checksum;
} packet;


#endif
