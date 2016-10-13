/*------------------------------------------------------------------
 *  in4073.c -- test QR engines and sensors
 *
 *  reads ae[0-3] uart rx queue
 *  (q,w,e,r increment, a,s,d,f decrement)
 *
 *  prints timestamp, ae[0-3], sensors to uart tx queue
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  June 2016
 *------------------------------------------------------------------
 */
#include "in4073.h"
#include "protocol/protocol.h"
#include "states.h"
#include "logging.c"

#define MAXZ 4000000
#define MAXL 1000000
#define MAXM 1000000
#define MAXN 2000000
#define BAT_THRESHOLD  1050

#define MIN_RPM 179200
#define MAX_RPM 1000000

#define int_to_fixed_point(a) (((int16_t)a)<<8)
#define divide_fixed_points(a,b) (int)((((int32_t)a<<8)+(b/2))/b)
#define fixed_point_to_int(a) (int)(a>>8)

bool decoupled_from_drone = true;

//calculate the Z force requested makis
int calculate_Z(char lift)
{
	int Z;
	int8_t l;
	l=lift;
	Z=divide_fixed_points(int_to_fixed_point(l),int_to_fixed_point(63));	
	Z=Z*MAXZ;
	Z=fixed_point_to_int(Z);
	return Z;

	
}
//calculate the L moment requested makis
int calculate_L(char roll)
{
	int L;
	int8_t r;
	
	if((roll&0x40)==0x40)
	{
		r=roll|0x80;
	}
	else
	{	
		r=roll;	
	}

	L=divide_fixed_points(int_to_fixed_point(r),int_to_fixed_point(63));
	L=L*MAXL;	
	L=fixed_point_to_int(L);

	return L;


}

//calculate the M moment requested makis
int calculate_M(char pitch)
{

	int M;
	int8_t p;

	if((pitch&0x40)==0x40)
	{
		p=pitch|0x80;
	}
	else
	{	
		p=pitch;	
	}

	M=divide_fixed_points(int_to_fixed_point(p),int_to_fixed_point(63));
	M=M*MAXM;
	M=fixed_point_to_int(M);

	return M;
}
//calculate the N moment requested makis
int calculate_N(char yaw)
{
	int N;
	int8_t y;

	if((yaw&0x40)==0x40)
	{
		y=yaw|0x80;
	}
	else
	{	
		y=yaw;	
	}


	N=divide_fixed_points(int_to_fixed_point(y),int_to_fixed_point(63));
	N=N*MAXN;	
	N=fixed_point_to_int(N);

	return N;	
}
//in this function calculate the values for the ae[] array makis
void calculate_rpm(int Z, int L, int M, int N)
{
	int ae1[4],i;
	//if there is lift force calculate ae[] array values
	if(Z>0)
	{		
		//calculate the square of each motor rpm
		ae1[0]=(2*M-N+Z)/4+MIN_RPM;
		ae1[1]=(2*L+N+Z)/4-L+MIN_RPM;
		ae1[2]=(2*M-N+Z)/4-M+MIN_RPM;
		ae1[3]=(2*L+N+Z)/4+MIN_RPM;

		//minimum rpm
		for(i=0;i<4;i++)
		{	
			if(ae1[i]<MIN_RPM)
			{
				ae1[i]=MIN_RPM;
			}
		}
		//maximum rpm
		for(i=0;i<4;i++)
		{	
			if(ae1[i]>MAX_RPM)
			{
				ae1[i]=MAX_RPM;
			}
		}

		//get the final motor values	
		ae[0]=ae1[0]>>10;
		ae[1]=ae1[1]>>10;
		ae[2]=ae1[2]>>10;
		ae[3]=ae1[3]>>10;
	}
	//if there is no lift force everything should be shut down
	else if(Z<=0)
	{
		ae[0]=0;
		ae[1]=0;
		ae[2]=0;
		ae[3]=0;
	}
	//update motors
	run_filters_and_control();
}

//calibration mode state makis
void calibration_mode()
{
	cur_mode=CALIBRATION_MODE;
	logging_enabled = true;
	print_pc_enabled = true;

	//indicate that you are in calibration mode
	nrf_gpio_pin_write(RED,1);
	nrf_gpio_pin_write(GREEN,0);
	//counter
	int clb;
	clb=0;
	//take 200 samples 
	while(clb<200)
	{
		if (check_sensor_int_flag())
		{
			get_dmp_data();
			clear_sensor_int_flag();
			clb++;
			p_off=p_off+sp;
			q_off=q_off+sq;
			r_off=r_off+sq;	
		}	
	}
	//calculate the offset
	p_off=p_off/200;
	q_off=q_off/200;
	r_off=r_off/200;
	
	//check the messages
	process_input();
	switch (pc_packet.mode)	
	{
		case SAFE_MODE:
			statefunc=safe_mode;
			break;
		default:
			break;
	}
}


void yaw_control_mode()
{
	
	cur_mode=YAW_CONTROLLED_MODE;
	logging_enabled = true;
	print_pc_enabled = true;

	nrf_gpio_pin_write(RED,0);
	nrf_gpio_pin_write(YELLOW,1);
	nrf_gpio_pin_write(GREEN,0);

	if(old_lift!=cur_lift || old_pitch!=cur_pitch || old_roll!=cur_roll || old_yaw!=cur_yaw)	
	{
		lift_force=calculate_Z(cur_lift);
		roll_moment=calculate_L(cur_roll);
		pitch_moment=calculate_M(cur_pitch);
		yaw_moment=calculate_N(cur_yaw);
		calculate_rpm(lift_force,roll_moment,pitch_moment,yaw_moment);
		old_lift=cur_lift;
		old_roll=cur_roll;
		old_pitch=cur_pitch;
		old_yaw=cur_yaw;
		run_filters_and_control();
		print_to_pc();
	}	
	

	while(msg==false && connection==true)
	{
		check_connection();
		if (check_sensor_int_flag())
		{
			get_dmp_data();				
			clear_sensor_int_flag();
			calculate_rpm(lift_force,roll_moment,pitch_moment,yaw_moment - (yaw_moment-sr*32)*p_ctrl);
			//print_to_pc();
		}
	}
	
	process_input();
	switch (pc_packet.mode)	
	{
		case PANIC_MODE:
			statefunc=panic_mode;
			break;
		case YAW_CONTROLLED_MODE:
			cur_lift=pc_packet.lift;
			cur_pitch=pc_packet.pitch;
			cur_roll=pc_packet.roll;
			cur_yaw=pc_packet.yaw;
			if(pc_packet.p_adjust==1)
			{
				p_ctrl=p_ctrl+1;
			}
			if(pc_packet.p_adjust==2)
			{
				p_ctrl=p_ctrl-1;
				if(p_ctrl<=1)
				{
					p_ctrl=1;
				}
			}
		default:
			break;
	}
}


//manual mode state makis
void manual_mode()
{
	cur_mode=MANUAL_MODE;
	logging_enabled = true;
	print_pc_enabled = true;

	//indicate that you are in manual mode
	nrf_gpio_pin_write(RED,1);
	nrf_gpio_pin_write(YELLOW,0);

	//if there is a new command do the calculations
	if(old_lift!=cur_lift || old_pitch!=cur_pitch || old_roll!=cur_roll || old_yaw!=cur_yaw)	
	{
		lift_force=calculate_Z(cur_lift);
		roll_moment=calculate_L(cur_roll);
		pitch_moment=calculate_M(cur_pitch);
		yaw_moment=calculate_N(cur_yaw);
		calculate_rpm(lift_force,roll_moment,pitch_moment,yaw_moment);
		old_lift=cur_lift;
		old_roll=cur_roll;
		old_pitch=cur_pitch;
		old_yaw=cur_yaw;
	
	}	

	//while there is no message received wait here and check your connection	
	while(msg==false && connection==true)
	{
		check_connection();
	}

	//read the new messages to come
	process_input();
	switch (pc_packet.mode)	
	{
		case PANIC_MODE:
			statefunc=panic_mode;
			break;
		case MANUAL_MODE:
			cur_lift=pc_packet.lift;
			cur_pitch=pc_packet.pitch;
			cur_roll=pc_packet.roll;
			cur_yaw=pc_packet.yaw;
			break;
		case SAFE_MODE:
			statefunc=safe_mode;
		default:
			break;
	}
}

//panic mode state makis
void panic_mode()
{
	cur_mode=PANIC_MODE;
	logging_enabled = true;
	print_pc_enabled = true;

	//indicate that you are in panic mode
	nrf_gpio_pin_write(RED,0);
	nrf_gpio_pin_write(YELLOW,0);
	
	//fly at minimum rpm
	if(ae[0]>175 || ae[1]>175 || ae[2]>175 || ae[3]>175) 
	{
		ae[0]=175;
		ae[1]=175;
		ae[2]=175;
		ae[3]=175;
		run_filters_and_control();
	}

	//zero down some values
	cur_lift=0;
	cur_pitch=0;
	cur_roll=0;
	cur_yaw=0;
	old_lift=0;
	old_pitch=0;
	old_roll=0;
	old_yaw=0;

	//after 2 seconds get to safe mode
	nrf_delay_ms(2000);

	//fixes a bug, doesn't care to check connection going to safe mode anyway
	time_latest_packet_us=get_time_us();

	//flag to print once in safe mode
	safe_print=true;

	//enters safe mode
	statefunc=safe_mode;
}

//safe mode state makis 
void safe_mode()
{
	cur_mode=SAFE_MODE;	
	logging_enabled = false;
	print_pc_enabled = true;

	//indicate that you are in safe mode	
	nrf_gpio_pin_write(RED,0);
	nrf_gpio_pin_write(YELLOW,1);
	nrf_gpio_pin_write(GREEN,1);

	//motors are shut down
	ae[0]=0;
	ae[1]=0;
	ae[2]=0;
	ae[3]=0;
	run_filters_and_control();


	//while no message is received wait here and check your connection
	while(msg==false && connection==true)
	{
		check_connection();
	}
	
	//if there is battery and the connection is ok read the messages
	if(battery==true && connection==true)
	{
		process_input();
		switch (pc_packet.mode)
		{
			//check for not switching to manual mode with offsets different than zero
			case MANUAL_MODE:
				if(pc_packet.lift==0 && pc_packet.pitch==0 && pc_packet.roll==0 && pc_packet.yaw==0)
				{
					statefunc=manual_mode;
				}
				break;
			case CALIBRATION_MODE:
				p_off=0;
				q_off=0;
				r_off=0;
				statefunc=calibration_mode;
				break;
			case YAW_CONTROLLED_MODE:
				if(pc_packet.lift==0 && pc_packet.pitch==0 && pc_packet.roll==0 && pc_packet.yaw==0)
				{
					statefunc=yaw_control_mode;
				} else {
					printf("offsets do not match 0!\n");
				}
				break;
			case DUMP_FLASH_MODE:
				read_flight_data();
				break;
			case SHUTDOWN_MODE:
				printf("\n\t Goodbye \n\n");
				nrf_delay_ms(100);
				NVIC_SystemReset();
				break;
			default:
				break;
		}
	}
}

/*jmi*/
char get_checksum(packet p) {	
	char checksum = (p.header^p.mode^p.p_adjust^p.lift^p.pitch^p.roll^p.yaw) >> 1;
	return checksum;
}

/*jmi*/
void process_input() 
{
	//temporary package before checksum validation
	packet tp; 
	char c;
	if(rx_queue.count>0)
	{
		/*skip through all input untill header is found*/
		while ( (c = dequeue(&rx_queue) ) != HEADER_VALUE) {
		}
		tp.header = c;
		tp.mode = dequeue(&rx_queue);		
		tp.p_adjust = dequeue(&rx_queue);
		tp.lift = dequeue(&rx_queue);
		tp.pitch = dequeue(&rx_queue);
		tp.roll = dequeue(&rx_queue);
		tp.yaw = dequeue(&rx_queue);
		tp.checksum = dequeue(&rx_queue);
		//if checksum is correct,copy whole packet into global packet	
		if (tp.checksum == (get_checksum(tp) & 0x7F)) {
			pc_packet.mode = tp.mode;
			pc_packet.p_adjust = tp.p_adjust;
			pc_packet.lift = tp.lift;
			pc_packet.pitch = tp.pitch;
			pc_packet.roll = tp.roll;
			pc_packet.yaw = tp.yaw;
			pc_packet.checksum = tp.checksum;
			time_latest_packet_us = get_time_us();
			msg=false;

		}

		/*flush rest of queue*/
		while (rx_queue.count >= 1) {
			dequeue(&rx_queue);
		}
	}
}


void initialize()
{
	//message flag initialization
	msg=false;
	
	//drone modules initialization
	uart_init();
	gpio_init();
	timers_init();
	adc_init();
	twi_init();
	imu_init(true, 100);	
	baro_init();
	spi_flash_init();
	//ble_init();
	demo_done = false;
	adc_request_sample();

	//initialise the pc_packet struct to safe values, just in case
	pc_packet.mode = SAFE_MODE;
	pc_packet.lift = 0;
	pc_packet.pitch = 0;
	pc_packet.roll = 0;
	pc_packet.yaw = 0;	
	
	//initialise rest of the variables to safe values, just in case
	cur_mode=SAFE_MODE;
	cur_lift=0;
	cur_pitch=0;
	cur_roll=0;
	cur_yaw=0;	
	old_lift=0;
	old_pitch=0;
	old_roll=0;
	old_yaw=0;
	lift_force=0;
	roll_moment=0;
	pitch_moment=0;
	yaw_moment=0;
	ae[0]=0;
	ae[1]=0;
	ae[2]=0;
	ae[3]=0;
	battery=true;
	connection=true;
	safe_print=true;
	p_ctrl=10;
	//first get to safe mode
	statefunc= safe_mode;
	//disable logging on startup
	logging_enabled = false;
	print_pc_enabled = true;
}

//if nothing received for over 500ms approximately go to panic mode and exit
void check_connection()
{
	current_time_us=get_time_us();
	uint32_t diff = current_time_us - time_latest_packet_us;
	if(diff > 500000)
	{	
		connection=false;
		statefunc=panic_mode;
	}
}

//jmi
void print_to_pc(){
	printf("%10ld | ", get_time_us());
	printf("%d | ",cur_mode);
	printf("%3d %3d %3d %3d | ",ae[0],ae[1],ae[2],ae[3]);
	printf("%6d %6d %6d | ", phi, theta, psi);
	printf("%6d %6d %6d | ", sp, sq, sr);
	printf("%4d | %4ld | %6ld \n", bat_volt, temperature, pressure);
}

//jmi
void battery_check(){
	//for testing purpose, the bat is not connected.
	if(decoupled_from_drone) {
		return; 
	}
	adc_request_sample();
	
	if (bat_volt < 1050)
	{
		printf("bat voltage %d below threshold %d",bat_volt,BAT_THRESHOLD);
		battery=false;
		statefunc=panic_mode;
	}
}

/*------------------------------------------------------------------
 * main -- do the test
 * edited by jmi
 *------------------------------------------------------------------
 */
int main(void)
{
	//initialize the drone
	initialize();

	uint32_t counter = 0;	

	while (!demo_done)
	{		
		//get to the state
		(*statefunc)();

		if (check_timer_flag()) {			
			if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);
			battery_check();
			if(print_pc_enabled){
				print_to_pc();
			}
			clear_timer_flag();
		}

		if (check_sensor_int_flag() && logging_enabled ) 
		{
			write_flight_data();	
			clear_sensor_int_flag();
		}

	}	
	
	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
