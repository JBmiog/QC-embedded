#include <math.h>
#include <time.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "joystick.h"
#include "packet.h"


/* current axis and button readings
 */
int	axis[6];
int	button[12];

/* time */
unsigned int mon_time_ms(void)
{
    unsigned int    ms;
    struct timeval  tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);
    ms = 1000 * (tv.tv_sec % 65); // 65 sec wrap around
    ms = ms + tv.tv_usec / 1000;
    return ms;
}

void mon_delay_ms(unsigned int ms)
{
    struct timespec req, rem;

    req.tv_sec = ms / 1000;
    req.tv_nsec = 1000000 * (ms % 1000);
    assert(nanosleep(&req,&rem) == 0);
}

void set_js_packet(char direction, int axis, int divisor)
{
	float throttle_on_scale = 0, multiplier = 127;
	char send_value;

	throttle_on_scale = (axis + (divisor / 2)) / divisor;

	if(throttle_on_scale > 1000)
        throttle_on_scale = 1000;

	else if(throttle_on_scale < -1000)
		throttle_on_scale = -1000;

	send_value = throttle_on_scale/1000.0*multiplier;

	js_p.mode = EMPTY;
	js_p.type = direction;
	js_p.data = send_value;
	printf ("packet.mode: %d, packet.type: %d, packet.data: %d \n\n", js_p.mode, js_p.type, js_p.data);
}

int read_js(int fd)
{
	struct js_event js;
	unsigned int t;

	mon_delay_ms(50);
	t = mon_time_ms();


	while (read(fd, &js, sizeof(struct js_event)) == sizeof(struct js_event))
	{
		/* register data */
		switch(js.type & ~JS_EVENT_INIT)
		{
			case JS_EVENT_BUTTON:
                        button[js.number] = js.value;
                        break;

			case JS_EVENT_AXIS:
                        axis[js.number] = js.value;
                        break;
		}
	}

	if (errno != EAGAIN)
	{
		perror("\njs: error reading (EAGAIN)");
		exit(1);
	}

	//exit program 
	if (button[0])
		return 1;

	//roll
	if(axis[0] < -JS_MIN_VALUE || axis[0] > JS_MIN_VALUE)
	{
		set_js_packet(ROLL, axis[0], JS_STEP_DIVISION_SMALL);
		
	}
	else
	{
		set_js_packet(ROLL, 0, JS_STEP_DIVISION_SMALL);
		
	}

	//pitch
	if(axis[1] < -JS_MIN_VALUE || axis[1] > JS_MIN_VALUE)
	{
		set_js_packet(PITCH, axis[1], JS_STEP_DIVISION_SMALL);
		
	}
	else
	{
		set_js_packet(PITCH, 0, JS_STEP_DIVISION_SMALL);
		
	}

	//yaw
	if(axis[2] < -JS_MIN_VALUE || axis[2] > JS_MIN_VALUE){
		set_js_packet(YAW, axis[2], JS_STEP_DIVISION_SMALL);

		
	}else{
		set_js_packet(YAW, 0, JS_STEP_DIVISION_SMALL);

		
	}

	//lift

	if (axis[3] < -JS_MIN_VALUE || axis[3] > JS_MIN_VALUE)	
	{
		set_js_packet(LIFT, axis[3], JS_STEP_DIVISION_SMALL);

				
	}else{
		set_js_packet(LIFT, 0, JS_STEP_DIVISION_SMALL);

		
	}


	return 0;
}
/*
int main(){
	int fd, a = 0;
		if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
		perror("jstest");
		exit(1);
	}

	// non-blocking mode
	fcntl(fd, F_SETFL, O_NONBLOCK);	
	while (a == 0){
	a = read_js(fd);
	}
	return 0;
}
*/
