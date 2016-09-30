//declare functions
int calculate_Z(char lift);
int calculate_L(char roll);
int calculate_M(char pitch);
int calculate_N(char yaw);
void calculate_rpm(int Z, int L, int M, int N);
void calibration_mode();
void manual_mode();
void safe_mode();
void panic_mode();
void calibration_mode();
void check_connection();

//state pointer, initialised to safe mode
void (*statefunc)() = safe_mode;

//variable to hold current mode
char cur_mode;

//variable to hold current movement
char cur_lift;
char cur_pitch;
char cur_roll;
char cur_yaw;


//variable to hold old movement
char old_lift;
char old_pitch;
char old_roll;
char old_yaw;

//counter for reading message
int cnt;

//start force in drone Z-axis
int start_z;

//force and moments in drone
int lift_force=0;
int roll_moment=0;
int pitch_moment=0;
int yaw_moment=0;

//dc offset of gyro sensor
int16_t p_off=0;
int16_t q_off=0;
int16_t r_off=0;

//counters to take care of exiting when communication breaks down
uint32_t time_latest_packet, current_time;

