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


#define MILLION 1000000
#define BAT_THRESHOLD  1050
packet pc_packet;

/*------------------------------------------------------------------
 * process incomming packets
 * edited by jmi
 *------------------------------------------------------------------
 */

void actuate(char lift, char pitch, char roll, char yaw)
{
	float Z,M,L,N;
	float t1,t2,t3,t4;



	//first restore the sign bit to the msb!!!
	lift=lift<<1;
	pitch=pitch<<1;
	roll=roll<<1;
	yaw=yaw<<1;

	//actuate motors depending on the values received
	L=(roll/127)*(MILLION);
	M=(pitch/127)*(MILLION);
	N=(yaw/127)*(2*MILLION);
	Z=(-1)*(lift/127)*(4*MILLION);

	t1=(2*M-N-Z)/4;
	t2=(2*L+N-Z)/4 - L;
	t3=(2*M-N-Z)/4 - M;
	t4=(2*L+N-Z)/4;
	
	if(t1<0)
	{
		t1=10000;
	}

	if(t2<0)
	{
		t2=10000;
	}

	if(t3<0)
	{
		t3=10000;
	}

	if(t4<0)
	{
		t4=10000;
	}

	ae[0]=(int16_t)sqrt(t1);
	ae[1]=(int16_t)sqrt(t2);
	ae[2]=(int16_t)sqrt(t3);
	ae[3]=(int16_t)sqrt(t4);
	
	//ae[0]=200;
	//ae[1]=200;
	//ae[2]=0;
	//ae[3]=0;
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

//panic mode state makis
void panic_mode()
{
	nrf_gpio_pin_write(RED,0);
	nrf_gpio_pin_write(YELLOW,0);
	cur_mode=PANIC_MODE;
	ae[0]=ae[0]/2;
	ae[1]=ae[1]/2;
	ae[2]=ae[2]/2;
	ae[3]=ae[3]/2;
	run_filters_and_control();
	nrf_delay_ms(1000);
	
	statefunc=safe_mode;
}

//manual mode state makis
void manual_mode()
{
	cur_mode=MANUAL_MODE;
	actuate(cur_lift,cur_pitch,cur_roll,cur_yaw);
	process_input();
	nrf_gpio_pin_write(RED,1);
	nrf_gpio_pin_write(YELLOW,0);
	switch (pc_packet.mode)
	{
		case PANIC_MODE:
			cur_lift=0;
			cur_pitch=0;
			cur_roll=0;
			cur_yaw=0;
			old_lift=0;
			old_pitch=0;
			old_roll=0;
			old_yaw=0;
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

//safe mode state makis 
void safe_mode()
{
	cur_mode=SAFE_MODE;	
	ae[0]=0;
	ae[1]=0;
	ae[2]=0;
	ae[3]=0;
	run_filters_and_control();
	process_input();
	nrf_gpio_pin_write(RED,0);
	nrf_gpio_pin_write(YELLOW,1);
	switch (pc_packet.mode)
	{
		case MANUAL_MODE:
			cur_lift=pc_packet.lift;
			cur_pitch=pc_packet.pitch;
			cur_roll=pc_packet.roll;
			cur_yaw=pc_packet.yaw;
			statefunc=manual_mode;
			break;
		default:
			break;
	}
}

/*------------------------------------------------------------------
 * main -- do the test
 * edited by jmi
 *------------------------------------------------------------------
 */
int main(void)
{
	uart_init();
	gpio_init();
	timers_init();
	adc_init();
	twi_init();
	imu_init(true, 100);	
	baro_init();
	spi_flash_init();
//	ble_init();

	//uint32_t counter = 0;
	demo_done = false;

	/*makis init*/ 
	pc_packet.mode = SAFE_MODE;
	pc_packet.lift = 0;
	pc_packet.pitch = 0;
	pc_packet.roll = 0;
	pc_packet.yaw = 0;	
	cur_lift=0;
	cur_pitch=0;
	cur_roll=0;
	cur_yaw=0;
	old_lift=0;
	old_pitch=0;
	old_roll=0;
	old_yaw=0;	
	cnt=0;
	cur_mode=SAFE_MODE;


	while (!demo_done)
	{	
		(*statefunc)();	
		if (check_timer_flag()) 
		{
			adc_request_sample();
			clear_timer_flag();
			//printf("%4d\n", bat_volt);
			
			/*if (bat_volt < BAT_THRESHOLD) {
				statefunc=panic_mode;
			}*/	
		}
		
	}	
	
	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
