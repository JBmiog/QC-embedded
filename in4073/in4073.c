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

void actuate(char lift, char pitch, char roll, char yaw)
{
	int Z,M,L,N,a1,a2,a3,a4;
	int8_t l,p,r,y;

	l=0;
	p=0;
	r=0;
	y=0;

	//first restore the numbers as created in the pc-terminal!!!
	if((lift&0x40)==0x40)
	{
		lift=(64-(lift&0x3f));
		l=0-lift;
	}
	else
	{	
		l=lift;	
	}

	if((pitch&0x40)==0x40)
	{
		pitch=(64-(pitch&0x3f));
		p=0-pitch;
	}
	else
	{	
		p=pitch;	
	}

	if((roll&0x40)==0x40)
	{
		roll=(64-(roll&0x3f));
		r=0-roll;
	}
	else
	{	
		r=roll;	
	}

	if((yaw&0x40)==0x40)
	{
		yaw=(64-(yaw&0x3f));
		y=0-yaw;
	}
	else
	{	
		y=yaw;	
	}

	//calculate forces and moments
	if(l>=0)
	{
		Z=divide_fixed_points(int_to_fixed_point(l),int_to_fixed_point(63));	
		Z=Z*(MAXZ-start_z);
		Z=fixed_point_to_int(Z)+start_z;
	}
	else
	{
		Z=divide_fixed_points(int_to_fixed_point(l),int_to_fixed_point(63));
		Z=Z*start_z;
		Z=fixed_point_to_int(Z)+start_z;
	}

	if(Z>0)
	{
		L=divide_fixed_points(int_to_fixed_point(r),int_to_fixed_point(63));
		L=L*MAXL;	
		L=fixed_point_to_int(L);

		M=divide_fixed_points(int_to_fixed_point(p),int_to_fixed_point(63));
		M=M*MAXM;
		M=fixed_point_to_int(M);

		N=divide_fixed_points(int_to_fixed_point(y),int_to_fixed_point(63));
		N=N*2*MAXN;	
		N=fixed_point_to_int(N);
	//calculate the square of each motor rpm
		a1=(2*M-N+Z)/4;
		a2=(2*L+N+Z)/4-L;
		a3=(2*M-N+Z)/4-M;
		a4=(2*L+N+Z)/4;
	//if the square is too high or too low then assign it to a minimum or maximum value
		if(a1<0)
		{
			a1=2500;
		}

		if(a2<0)
		{
			a2=2500;
		}

		if(a3<0)
		{
			a3=2500;
		}

		if(a4<0)
		{
			a4=2500;
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
		
	ae[0]=(int16_t)sqrt(a1);
	ae[1]=(int16_t)sqrt(a2);
	ae[2]=(int16_t)sqrt(a3);
	ae[3]=(int16_t)sqrt(a4);
	//printf("%d - %d - %d - %d\n ",ae[0], ae[1], ae[2], ae[3]);
	//nrf_delay_ms(300);
	run_filters_and_control();
	
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



//manual mode state makis
void manual_mode()
{
	cur_mode=MANUAL_MODE;

	nrf_gpio_pin_write(RED,1);
	nrf_gpio_pin_write(YELLOW,0);

	actuate(cur_lift,cur_pitch,cur_roll,cur_yaw);

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

	ae[0]=ae[0]/2;
	ae[1]=ae[1]/2;
	ae[2]=ae[2]/2;
	ae[3]=ae[3]/2;
	run_filters_and_control();

	cur_lift=0;
	cur_pitch=0;
	cur_roll=0;
	cur_yaw=0;

	nrf_delay_ms(1000);

	statefunc=safe_mode;
}

//safe mode state makis 
void safe_mode()
{
	cur_mode=SAFE_MODE;
		
	nrf_gpio_pin_write(RED,0);
	nrf_gpio_pin_write(YELLOW,1);

	ae[0]=0;
	ae[1]=0;
	ae[2]=0;
	ae[3]=0;
	run_filters_and_control();

	process_input();
	switch (pc_packet.mode)
	{
		case MANUAL_MODE:
			statefunc=manual_mode;
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
	
	cnt=0;

	ae[0]=0;
	ae[1]=0;
	ae[2]=0;
	ae[3]=0;

	//later create a function for start_z
	start_z=ae[0]*ae[0]+ae[1]*ae[1]+ae[2]*ae[2]+ae[3]*ae[3];
}

/*------------------------------------------------------------------
 * main -- do the test
 * edited by jmi
 *------------------------------------------------------------------
 */
int main(void)
{
	initialize();
	while (!demo_done)
	{	
		(*statefunc)();	
		if (check_timer_flag()) 
		{
			adc_request_sample();
			clear_timer_flag();
			//printf("%4d\n", bat_volt);
			
			//if (bat_volt < BAT_THRESHOLD) {
			//	statefunc=panic_mode;
			//}	
		}
		
	}	
	
	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
