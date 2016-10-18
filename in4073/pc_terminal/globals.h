#ifndef  GLOBALS_h__
#define GLOBALS_h__

/*jmi*/

char yaw_offset;
char pitch_offset;
char roll_offset;
char lift_offset;

char kb_yaw;
char kb_pitch;
char kb_roll;
char kb_lift;

char js_yaw;
char js_pitch;
char js_roll;
char js_lift;

char yaw_offset_p_up;
char yaw_offset_p_down;
char roll_pitch_offset_p1_up;
char roll_pitch_offset_p2_up;
char roll_pitch_offset_p1_down;
char roll_pitch_offset_p2_down;

char mode = 0;

int	axis[6];
int	button[12];
int fd = 0;


packet mypacket;

#endif
