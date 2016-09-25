//declare functions
void actuate(char lift, char pitch, char roll, char yaw);
void manual_mode();
void safe_mode();
void panic_mode();

//state pointer, initialised to safe mode
void (*statefunc)() = safe_mode;

//variable to hold current mode
char cur_mode;

//variable to hold current movement
char cur_lift;
char cur_pitch;
char cur_roll;
char cur_yaw;

//counter for reading message
int cnt;

//start force in drones Z-axis
int start_z;
