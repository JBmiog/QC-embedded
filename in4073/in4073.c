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
void process_input() 
{
	char c;
	/*skip through all input untill header is found*/
	while ( (c = dequeue(&rx_queue) ) != HEADER_VALUE) {
	}
	pc_packet.header = c;
	pc_packet.mode = dequeue(&rx_queue);		
	pc_packet.p_adjust = dequeue(&rx_queue);
	pc_packet.lift = dequeue(&rx_queue);
	pc_packet.pitch = dequeue(&rx_queue);
	pc_packet.roll = dequeue(&rx_queue);
	pc_packet.yaw = dequeue(&rx_queue);
	pc_packet.checksum = dequeue(&rx_queue);

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
	//actuate(cur_lift,cur_pitch,cur_roll,cur_yaw);
	process_input();
	nrf_gpio_pin_write(RED,1);
	nrf_gpio_pin_write(YELLOW,0);
	switch (pc_packet.mode)
	{
		case PANIC_MODE:
			printf("B\n");
			cur_lift=0;
			cur_pitch=0;
			cur_roll=0;
			cur_yaw=0;
			old_lift=0;
			old_pitch=0;
			old_roll=0;
			old_yaw=0;
			//statefunc=panic_mode;
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
