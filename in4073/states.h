#define SAFE_MODE 0x00
#define PANIC_MODE 0x01
#define MANUAL_MODE 0x02
#define CALIBRATION_MODE 0x03
#define YCTRL_MODE 0x04
#define FCTRL_MODE 0x05
#define RAW_MODE 0x06
#define HCTRL_MODE 0x07
#define WLESS_MODE 0x08

//struct to hold values sent by pc
struct pc_packet
{
	char mode;
	char p_adjust;
	char lift;
	char pitch;
	char roll;
	char yaw;
};

//declare functions
void actuate(char lift, char pitch, char roll, char yaw);
void read_msg();
void manual_mode();
void safe_mode();
void panic_mode();

//our struct
struct pc_packet pcpacket;

//state pointer
void (*statefunc)() = safe_mode;

//variable to hold current mode
char cur_mode;

//variable to hold current mode
char cur_lift;
char cur_pitch;
char cur_roll;
char cur_yaw;

//global variable to hold temp_msg
char temp_msg[8];

//variable to hold old rotational velocity of motors
char old_lift;
char old_pitch;
char old_roll;
char old_yaw;

//counter for reading message
int cnt;

//int counter1=0;
//int counter2;
//counter2=counter1;
