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
#include "math.h"

#define MAXZ 4000000
#define MAXL 1000000
#define MAXM 1000000
#define MAXN 2000000
#define BAT_THRESHOLD  1050

#define int_to_fixed_point(a) (((int16_t)a)<<8)
#define divide_fixed_points(a,b) (int)((((int32_t)a<<8)+(b/2))/b)
#define fixed_point_to_int(a) (int)(a>>8)

packet pc_packet;
//calculate the Z force requested makis
int calculate_Z(char lift)
{
	int Z;
	int8_t l;
	l=0;

	if((lift&0x40)==0x40)
	{
		lift=(64-(lift&0x3f));
		l=0-lift;
	}
	else
	{	
		l=lift;	
	}

	if(l>=0)
	{
		Z=divide_fixed_points(int_to_fixed_point(l),int_to_fixed_point(63));	
		Z=Z*(MAXZ-start_z);
		Z=fixed_point_to_int(Z)+start_z;
		return Z;
	}
	else
	{
		Z=divide_fixed_points(int_to_fixed_point(l),int_to_fixed_point(63));
		Z=Z*start_z;
		Z=fixed_point_to_int(Z)+start_z;
		return Z;
	}


}
//calculate the L moment requested makis
int calculate_L(char roll)
{
	int L;
	int8_t r;
	
	if((roll&0x40)==0x40)
	{
		roll=(64-(roll&0x3f));
		r=0-roll;
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
		pitch=(64-(pitch&0x3f));
		p=0-pitch;
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
		yaw=(64-(yaw&0x3f));
		y=0-yaw;
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
	int a1,a2,a3,a4;
	//if there is no lift force don't calculate anything
	if(Z>0)
	{		
		//calculate the square of each motor rpm
		a1=(2*M-N+Z)/4;
		a2=(2*L+N+Z)/4-L;
		a3=(2*M-N+Z)/4-M;
		a4=(2*L+N+Z)/4;

		//if movement is not possible, do what is possible				
		if(a1<=0)
		{
			a1=50;
		}
		if(a2<=0)
		{
			a2=50;
		}
		if(a3<=0)
		{
			a3=50;
		}
		if(a4<=0)
		{
			a4=50;
		}
		if(a1>1000000)
		{
			a1=1000000;
		}
		if(a2>1000000)
		{
			a2=1000000;
		}
		if(a3>1000000)
		{
			a3=1000000;
		}
		if(a4>1000000)
		{
			a4=1000000;
		}
	}
	//if there is no lift force everything should be shut down
	else if(Z<=0)
	{
		a1=0;
		a2=0;
		a3=0;
		a4=0;
	}
	//get the final motor values	
	ae[0]=a1>>10;
	ae[1]=a2>>10;
	ae[2]=a3>>10;
	ae[3]=a4>>10;
	//printf("%d - %d - %d - %d\n ",ae[0], ae[1], ae[2], ae[3]);
	//nrf_delay_ms(300);	
	


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
		counter1++;
		//printf("%d - %d - %d - %d\n",pc_packet.roll,pc_packet.pitch,pc_packet.yaw,pc_packet.lift);
		//nrf_delay_ms(50);
	}

	//printf("head:%x, mod:%x,padjust:%x,lift:%x,pitch:%x,roll:%x,yaw:%x,check:%x\n" ,pc_packet.header,pc_packet.mode,pc_packet.p_adjust,pc_packet.lift,pc_packet.pitch,pc_packet.roll,pc_packet.yaw,pc_packet.checksum);
	//nrf_delay_ms(100);

	/*flush rest of queue*/
	while (rx_queue.count >= 1) {
		dequeue(&rx_queue);
	}

}

//calibration mode state makis
void calibration_mode()
{
	cur_mode=CALIBRATION_MODE;
	
	nrf_gpio_pin_write(RED,1);
	nrf_gpio_pin_write(GREEN,0);
	int clb;
	clb=0;
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
	p_off=p_off/200;
	q_off=q_off/200;
	r_off=r_off/200;

	process_input();
	switch (pc_packet.mode)	
	{
		case SAFE_MODE:
			//printf("%d, %d, %d\n",p_off,q_off,r_off);
			//nrf_delay_ms(300);
			statefunc=safe_mode;
			break;
		default:
			break;
	}
}


//manual mode state makis
void manual_mode()
{
	cur_mode=MANUAL_MODE;

	nrf_gpio_pin_write(RED,1);
	nrf_gpio_pin_write(YELLOW,0);

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
	}	
	
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

		default:
			break;
	}
}

//panic mode state makis
void panic_mode()
{
	cur_mode=PANIC_MODE;

	nrf_gpio_pin_write(RED,0);
	nrf_gpio_pin_write(YELLOW,0);

	ae[0]=50;
	ae[1]=50;
	ae[2]=50;
	ae[3]=50;
	run_filters_and_control();

	cur_lift=0;
	cur_pitch=0;
	cur_roll=0;
	cur_yaw=0;

	old_lift=0;
	old_pitch=0;
	old_roll=0;
	old_yaw=0;

	nrf_delay_ms(1000);

	statefunc=safe_mode;
}

//safe mode state makis 
void safe_mode()
{
	cur_mode=SAFE_MODE;
		
	nrf_gpio_pin_write(RED,0);
	nrf_gpio_pin_write(YELLOW,1);
	nrf_gpio_pin_write(GREEN,1);

	ae[0]=0;
	ae[1]=0;
	ae[2]=0;
	ae[3]=0;
	run_filters_and_control();

	process_input();
	switch (pc_packet.mode)
	{
		case MANUAL_MODE:
			cur_lift=pc_packet.lift;
			cur_pitch=pc_packet.pitch;
			cur_roll=pc_packet.roll;
			cur_yaw=pc_packet.yaw;
			statefunc=manual_mode;
			break;
		case CALIBRATION_MODE:
			p_off=0;
			q_off=0;
			r_off=0;
			statefunc=calibration_mode;
			break;
		default:
			break;
	}
}

void initialize()
{
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

	/*makis init*/ 
	pc_packet.mode = SAFE_MODE;
	pc_packet.lift = 0;
	pc_packet.pitch = 0;
	pc_packet.roll = 0;
	pc_packet.yaw = 0;	

	cur_mode=SAFE_MODE;
	cur_lift=0;
	cur_pitch=0;
	cur_roll=0;
	cur_yaw=0;
	
	old_lift=0;
	old_pitch=0;
	old_roll=0;
	old_yaw=0;

	cnt=0;

	ae[0]=0;
	ae[1]=0;
	ae[2]=0;
	ae[3]=0;

	//later create a function for start_z
	start_z=ae[0]*ae[0]+ae[1]*ae[1]+ae[2]*ae[2]+ae[3]*ae[3];
}
//if nothing received for over 400ms approximately go to panic mode and exit, makis
void check_connection(uint32_t time)
{
	if(get_time_us()-time>400000)
	{
		if(counter1==counter2)
		{
			statefunc=panic_mode;
			pc_packet.mode = SAFE_MODE;
			pc_packet.lift = 0;
			pc_packet.pitch = 0;
			pc_packet.roll = 0;
			pc_packet.yaw = 0;
		}
		counter2=counter1;
	}	

}

/*------------------------------------------------------------------
 * main -- do the test
 * edited by jmi
 *------------------------------------------------------------------
 */
int main(void)
{
	uint32_t time;
	initialize();
	time = get_time_us();
	counter2=counter1;
	while (!demo_done)
	{	
		(*statefunc)();
		//check battery voltage every now and then	
		if (check_timer_flag()) 
		{
			adc_request_sample();
			clear_timer_flag();
			//check battery voltage threshold			
			//if (bat_volt < BAT_THRESHOLD)
			//{
			//	statefunc=panic_mode;
			//}	
		}
		check_connection(time);
		time=get_time_us();
	}	
	
	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
