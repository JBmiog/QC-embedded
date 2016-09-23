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
packet pc_packet;
/*------------------------------------------------------------------
 * process incomming packets
 * edited by jmi
 *------------------------------------------------------------------
 */
void process_input(char c) 
{
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
	printf("h:%x, m:%x,..,c:%x\n",pc_packet.header,pc_packet.mode,pc_packet.checksum);
	if(pc_packet.mode == 0x03){
		nrf_gpio_pin_write(YELLOW,0);
	} else {
		nrf_gpio_pin_write(YELLOW,1);
	}

	/*flush rest of queue*/
	while (rx_queue.count >= 1) {
		dequeue(&rx_queue);
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

	uint32_t counter = 0;
	demo_done = false;

	while (!demo_done)
	{	
		if (rx_queue.count) process_input( dequeue(&rx_queue) ); 
		
		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) { 
				nrf_gpio_pin_toggle(BLUE);
			}
			//adc_request_sample();
			//read_baro();

			clear_timer_flag();
		}
 
		if (check_sensor_int_flag()) 
		{
			//get_dmp_data();
			//run_filters_and_control();

			clear_sensor_int_flag();
		}
		
	}	
	
	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
