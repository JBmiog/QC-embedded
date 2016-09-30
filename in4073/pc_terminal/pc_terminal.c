/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include "../protocol/protocol.h"
#include "globals.h"
#include "keyboard.h"
#include "joystick.h"


#define NANO_SECOND_MULTIPLIER 1000000
/*------------------------------------------------------------
* joystick I/O
*------------------------------------------------------------
*/

void push_packet(char direction, char value)
{
    switch (direction)
    {
    case LIFT:
        js_lift = value;
        //printf ("LIFT: %d \n\n", js_lift);
        break;
    case PITCH:
        js_pitch = value;
        //printf ("PITCH: %d \n\n", js_pitch);
        break;
    case ROLL:
        js_roll = value;
        //printf ("ROLL: %d \n\n", js_roll);
        break;
    case YAW:
        js_yaw = value;
        //printf ("YAW: %d \n\n", js_yaw);
        break;
    }
}


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
    push_packet(direction, send_value);

}

int read_js(int fd)
{
    struct js_event js;
    //unsigned int t;

    //mon_delay_ms(50);
    //t = mon_time_ms();


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
    if(axis[2] < -JS_MIN_VALUE || axis[2] > JS_MIN_VALUE)
    {
        set_js_packet(YAW, axis[2], JS_STEP_DIVISION_SMALL);
    }
    else
    {
        set_js_packet(YAW, 0, JS_STEP_DIVISION_SMALL);
    }

    //lift
    if (axis[3] < -JS_MIN_VALUE || axis[3] > JS_MIN_VALUE)
    {
        set_js_packet(LIFT, axis[3], JS_STEP_DIVISION_SMALL);
    }
    else
    {
        set_js_packet(LIFT, 0, JS_STEP_DIVISION_SMALL);
    }
    return 0;
}


/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */
struct termios 	savetty;

void joystick_init()
{

    if ((fd = open(JS_DEV, O_RDONLY)) < 0)
    {
        perror("jstest");
    }
    // non-blocking mode
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

void	term_initio()
{
    struct termios tty;

    tcgetattr(0, &savetty);
    tcgetattr(0, &tty);

    tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 0;

    tcsetattr(0, TCSADRAIN, &tty);
}

void	term_exitio()
{
    tcsetattr(0, TCSADRAIN, &savetty);
}

void	term_puts(char *s)
{
    fprintf(stderr,"%s",s);
}

void	term_putchar(char c)
{
    putc(c,stderr);
}

int	term_getchar_nb()
{
    static unsigned char 	line [2];

    if (read(0,line,1)) // note: destructive read
        return (int) line[0];

    return -1;
}

int	term_getchar()
{
    int    c;

    while ((c = term_getchar_nb()) == -1)
        ;
    return c;
}

/*------------------------------------------------------------
 * Serial I/O
 * 8 bits, 1 stopbit, no parity,
 * 115,200 baud
 *------------------------------------------------------------
 */
#include <termios.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

int serial_device = 0;
int fd_RS232;

void rs232_open(void)
{
    char 		*name;
    int 		result;
    struct termios	tty;

    fd_RS232 = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);  // Hardcode your serial port here, or request it as an argument at runtime

    assert(fd_RS232>=0);

    result = isatty(fd_RS232);
    assert(result == 1);

    name = ttyname(fd_RS232);
    assert(name != 0);

    result = tcgetattr(fd_RS232, &tty);
    assert(result == 0);

    tty.c_iflag = IGNBRK; /* ignore break condition */
    tty.c_oflag = 0;
    tty.c_lflag = 0;

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; /* 8 bits-per-character */
    tty.c_cflag |= CLOCAL | CREAD; /* Ignore model status + read input */

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 1; // added timeout

    tty.c_iflag &= ~(IXON|IXOFF|IXANY);

    result = tcsetattr (fd_RS232, TCSANOW, &tty); /* non-canonical */

    tcflush(fd_RS232, TCIOFLUSH); /* flush I/O buffer */
}


void 	rs232_close(void)
{
    int 	result;

    result = close(fd_RS232);
    assert (result==0);
}


int	rs232_getchar_nb()
{
    int 		result;
    unsigned char 	c;

    result = read(fd_RS232, &c, 1);

    if (result == 0)
        return -1;

    else
    {
        assert(result == 1);
        return (int) c;
    }
}


int rs232_getchar()
{
    int 	c;
    while ((c = rs232_getchar_nb()) == -1)
        ;
    return c;
}


int rs232_putchar(char c)
{
    int result;

    do
    {
        result = (int) write(fd_RS232, &c, 1);
    }
    while (result == 0);

    assert(result == 1);
    return result;
}

/* handles the input of the keyboard, Jeffrey Miog */
void kb_input_handler(char pressed_key)
{
    switch(pressed_key)
    {
    case ESC:
        mode = PANIC_MODE;
        break;
    case ZERO:
        mode = SAFE_MODE;
        term_puts("changed mode to safe mode\n");
        break;
    case ONE:
        mode = PANIC_MODE;
        break;
    case TWO:
        mode = MANUAL_MODE;
        break;
    case THREE:
        mode = CALIBRATION_MODE;
        break;
    case FOUR:
        mode = YAW_CONTROLLED_MODE;
        break;
    case FIVE:
        mode = FULL_CONTROL_MODE;
        break;
    case SIX:
        mode = RAW_MODE;
        break;
    case SEVEN:
        mode = HEIGHT_CONTROL_MODE;
        break;
    case EIGHT:
        mode = WIRELESS_MODE;
        break;
    case 'a':
        if((lift_offset+UP)>=126 || lift_offset==127 )
        {
            lift_offset=127;
        }
        else
        {
            lift_offset+=UP;
        }
        break;
    case 'z':
        if((lift_offset<-120) && (lift_offset+DOWN>0))
        {
            lift_offset=-127;
        }
        else
        {
            lift_offset+=DOWN;
        }
        break;
    case 'q':
        if((yaw_offset<-120) && (yaw_offset+DOWN>0))
        {
            yaw_offset=-127;
        }
        else
        {
            yaw_offset+=DOWN;
        }
        break;
    case 'w':
        if((yaw_offset+UP)>=126 || yaw_offset==127 )
        {
            yaw_offset=127;
        }
        else
        {
            yaw_offset+=UP;
        }
        break;
    //control loop values adjusting
    case 'u':
        yaw_offset_p = UP;
        break;
    case 'j':
        yaw_offset_p = DOWN;
        break;
    case 'i':
        roll_pitch_offset_p1 = UP;
        break;
    case 'k':
        roll_pitch_offset_p1 = DOWN;
        break;
    case 'o':
        roll_pitch_offset_p2 = UP;
        break;
    case 'l':
        roll_pitch_offset_p2 = DOWN;
        break;
    //arrow up
    case 'A':
        if((pitch_offset<-120) && (pitch_offset+DOWN>0))
        {
            pitch_offset=-127;
        }
        else
        {
            pitch_offset+=DOWN;
        }
        break;
    //arrow down
    case 'B':
        if((pitch_offset+UP)>=126 || pitch_offset==127 )
        {
            pitch_offset=127;
        }
        else
        {
            pitch_offset+=UP;
        }
        break;
    //arrow right
    case 'C':
        if((roll_offset<-120) && (roll_offset+DOWN>0))
        {
            roll_offset=-127;
        }
        else
        {
            roll_offset+=DOWN;
        }
        break;
    //arrow left
    case 'D':
        if((roll_offset+UP)>=126 || roll_offset==127 )
        {
            roll_offset=127;
        }
        else
        {
            roll_offset+=UP;
        }
        break;

    //own implementation
    case 't':
        kb_pitch = UP;
    case 'g':
        kb_pitch = DOWN;
    case 'f':
        kb_roll = UP;
    case 'h':
        kb_roll = DOWN;
    case 'v':
        kb_lift = UP;
    case 'b':
        kb_lift = DOWN;
    default:
        return;
        break;
    }


}

/* jmi */
void print_static_offsets()
{
    printf("mode = %d\t y_offset = %d\t p_offset = %d\t, r_offset = %d\t, l_offset = %d\t p = %d\t P1 = %d\t, P2 = %d\n",mode, yaw_offset, pitch_offset, roll_offset, lift_offset, yaw_offset_p, roll_pitch_offset_p1, roll_pitch_offset_p2);
}

void print_packet()
{
    printf("lift=%d, pitch=%d, roll=%d, yaw=%d \n",mypacket.lift,mypacket.pitch,mypacket.roll,mypacket.yaw);

}

/*jmi*/
char get_checksum()
{
    char checksum = (mypacket.header^mypacket.mode^mypacket.p_adjust^mypacket.lift^mypacket.pitch^mypacket.roll^mypacket.yaw) >> 1;
    return checksum;
}

char inspect_overflow(char offset, char js, char kb)
{

    char temp_sum;

    if ((offset + js + kb) > 127)
    {
        temp_sum = ((127)>>1) & 0x7F;;
    }
    else if ((offset + js + kb) < -127)
    {
        temp_sum = ((-127)>>1) & 0x7F;
    }
    else
    {
        temp_sum = ((offset + js + kb)>>1) & 0x7F;
    }

    return temp_sum;
}



/* jmi
the &0x7F is for savety only, so we know MSB is only set in the
header
*/
void create_packet()
{
    mypacket.header = HEADER_VALUE;
    mypacket.mode = mode;
    //mypacket.p_adjust =
    /*here i need the joystick...?*/
    mypacket.lift = inspect_overflow(lift_offset, js_lift, kb_lift);
    mypacket.pitch = inspect_overflow(pitch_offset, js_pitch, kb_pitch);
    mypacket.roll = inspect_overflow(roll_offset, js_roll, kb_roll);
    mypacket.yaw = inspect_overflow(yaw_offset, js_yaw, kb_yaw);
    mypacket.checksum = get_checksum(mypacket) & 0x7F;
}


/* jmi */
void tx_packet()
{
    //term_puts("tx packet to FCB\n");
    rs232_putchar(mypacket.header);
    rs232_putchar(mypacket.mode);
    rs232_putchar(mypacket.p_adjust);
    rs232_putchar(mypacket.lift);
    rs232_putchar(mypacket.pitch);
    rs232_putchar(mypacket.roll);
    rs232_putchar(mypacket.yaw);
    rs232_putchar(mypacket.checksum);
}

/*----------------------------------------------------------------
 * main -- execute terminal
 * edited by jmi
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
    char	c;

    term_puts("\nTerminal program - Embedded Real-Time Systems\n");

    term_initio();
    rs232_open();

    /* display keyboard mapping */
    term_puts("keyboard mapping:\n");
    term_puts("a:\t	lift_offset up\n 'z':\t	lift_offset down\n");
    term_puts("q:\t	yaw up\n 'w':\t	yaw down\n");
    term_puts("up:\t	pitch_offset up\n 'down':\t	ptich_offset down\n");
    term_puts("right:\t	roll_offset up\n 'right':	roll_offset down \n");
    term_puts("P CONTROLLERS TO BE ADDED \n");

    term_puts("\nType ^C to exit\n");


    /* needed for sleeping mode */
    struct timespec tim, tim2;
    const long interval_ms = 100 * NANO_SECOND_MULTIPLIER;
    tim.tv_sec  = 0;
    tim.tv_nsec = interval_ms;

    //joystick_init();

    while(1)
    {
        /*delay +/- 100 ms*/
        if(nanosleep(&tim, &tim2) < 0 )
        {
            printf("Nano sleep system call failed \n");
        }

        //	printf("creating apcket\n");
        create_packet();
        tx_packet();

        //read_js(fd);

        while ((c = term_getchar_nb()) != -1)
        {
            kb_input_handler(c);
            term_putchar('>');
            term_putchar(c);
            term_putchar('\n');
            print_static_offsets();
            //rs232_putchar(pressed_key);
        }

        /*read until return is given*/
        if((c = rs232_getchar_nb()) != -1)
        {
            term_putchar('<');
            term_putchar(c);
            while ((c = rs232_getchar_nb()) != '\n')
            {
                term_putchar(c);
            }
            term_putchar('\n');

            while ((c = rs232_getchar_nb()) != -1)
            {
                term_putchar(c);
            }

        }
    }

    term_exitio();
    rs232_close();
    term_puts("\n<exit>\n");

    return 0;
}

