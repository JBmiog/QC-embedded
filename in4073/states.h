//declare functions
void calculate_rpm(int Z, int L, int M, int N);
void safe_mode();
void panic_mode();
void manual_mode();
void calibration_mode();
void yaw_control_mode();
void full_control_mode();
void check_connection();
void battery_check();
void process_input();
char restore_num(char num);

//state pointer
void (*statefunc)();

//variable to hold current mode
char cur_mode;

//p controller value
char p_ctrl;
char p1_ctrl;
char p2_ctrl;

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
int32_t p_off;
int32_t q_off;
int32_t r_off;
int32_t theta_off;
int32_t phi_off;

//counters to take care of exiting when communication breaks down
uint32_t time_latest_packet_us, current_time_us;

//flags indicating that there is still connection and battery
bool connection;
bool battery;

//flag indicating that a new message has arrived
bool msg;

//flag to print in safe mode
bool safe_print;

bool decoupled_from_drone;
uint32_t counter;
