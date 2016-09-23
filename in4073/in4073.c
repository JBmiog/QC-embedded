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


/*

	switch (c) 
	{
		case 'q':
			ae[0] += 10;
			break;
		case 'a':
			ae[0] -= 10;
			if (ae[0] < 0) ae[0] = 0;
			break;
		case 'w':
			ae[1] += 10;
			break;
		case 's':
			ae[1] -= 10;
			if (ae[1] < 0) ae[1] = 0;
			break;
		case 'e':
			ae[2] += 10;
			break;
		case 'd':
			ae[2] -= 10;
			if (ae[2] < 0) ae[2] = 0;
			break;
		case 'r':
			ae[3] += 10;
			break;
		case 'f':
			ae[3] -= 10;
			if (ae[3] < 0) ae[3] = 0;
			break;
		case 27:
			demo_done = true;
			break;
		default:
			nrf_gpio_pin_toggle(RED);
	}
	*/


#include "in4073.h"
#include "protocol/protocol.h"
#include "states.h"
packet pc_packet;
/*------------------------------------------------------------------
 * process incomming packets
 * edited by jmi
 *------------------------------------------------------------------
 */

void actuate(char lift1, char pitch1, char roll1, char yaw1)
{
	char temp, lift, pitch, roll, yaw;

	lift=lift1;
	pitch=pitch1;
	roll=roll1;
	yaw=yaw1;
	
	//first restore the sign bit to the msb!!!
	temp=lift & 0x40;
	if(temp==0x40)
	{
		lift=lift&0x3f;
		lift=lift|0x80;
	}
	
	temp=pitch & 0x40;
	if(temp==0x40)
	{
		pitch=pitch&0x3f;
		pitch=pitch|0x80;
	}

	temp=roll & 0x40;
	if(temp==0x40)
	{
		roll=roll&0x3f;
		roll=roll|0x80;
	}

	temp=yaw & 0x40;
	if(temp==0x40)
	{
		yaw=yaw&0x3f;
		yaw=yaw|0x80;
	}
	

	//actuate motors depending on the values received
	if((lift&0x80)!=0x80)
	{
		if(old_lift!=lift)
		{
			printf("L\n");
			old_lift=lift;
		}
	}
	
	if((lift&0x80)==0x80)
	{
		if(old_lift!=lift)
		{
			printf("l\n");
			old_lift=lift;
		}		
	}	
	
	if((pitch&0x80)!=0x80)
	{
		if(old_pitch!=pitch)
		{
			printf("P\n");
			old_pitch=pitch;			
		}	
	}
	
	if((pitch&0x80)==0x80)
	{
		if(old_pitch!=pitch)
		{
			printf("p\n");
			old_pitch=pitch;			
		}
	}

	if((roll&0x80)!=0x80)
	{
		if(old_roll!=roll)
		{
			printf("R\n");
			old_roll=roll;			
		}
	}
	
	if((roll&0x80)==0x80)
	{
		if(old_roll!=roll)
		{
			printf("r\n");
			old_roll=roll;			
		}		
	}

	if((yaw&0x80)!=0x80)
	{
		if(old_yaw!=yaw)
		{
			printf("Y\n");
			old_yaw=yaw;			
		}
	}
	
	if((yaw&0x80)==0x80)
	{
		if(old_yaw!=yaw)
		{
			printf("y\n");
			old_yaw=yaw;			
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
	nrf_delay_ms(1000);
	
	statefunc=safe_mode;
}

//manual mode state makis
void manual_mode()
{
	cur_mode=MANUAL_MODE;
	//actuate(cur_lift,cur_pitch,cur_roll,cur_yaw);
	process_input();
	nrf_gpio_pin_write(RED,1);
	nrf_gpio_pin_write(YELLOW,0);
	switch (pc_packet.mode)
	{
		case PANIC_MODE:
			//printf("B\n");
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
	process_input();
	nrf_gpio_pin_write(RED,0);
	nrf_gpio_pin_write(YELLOW,1);
	switch (pc_packet.mode)
	{
		case MANUAL_MODE:
			printf("A\n");
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
	}	
	
	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
