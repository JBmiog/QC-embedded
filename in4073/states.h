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
void process_input();

//state pointer
void (*statefunc)();

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

//force and moments in drone
int lift_force;
int roll_moment;
int pitch_moment;
int yaw_moment;

//dc offset of gyro sensor
int16_t p_off;
int16_t q_off;
int16_t r_off;

//counters to take care of exiting when communication breaks down
uint32_t time_latest_packet, current_time;

//flags indicating that there is still connection and battery
bool connection;
bool battery;

//flag indicating that a new message has arrived
bool msg;

//flag to print in safe mode
bool safe_print;
