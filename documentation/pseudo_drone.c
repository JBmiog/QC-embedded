#define SAFE_MODE
#define MANUAL_MODE

struct pc_packet
{
	char mode=SAFE_MODE;
	char direction=0x0;
	char value=0x0;
	char checksum;

}


actuate(direction,value)
{
	switch(direction)
		case .... 
		//according to each case
		//update the ae[] array 
		//taking into consideration the pc_packet->value field as well
	
}

read_msg(mode)
{
	do checksum check;
	if(checksum is ok)
		copy msg values to struct pc packet;	
}

safe_mode()
{
	mode=SAFE_MODE;
	if(rcv_qeue)
	{
		pc_packet=read_msg(mode);
	}	

	switch pc_packet->mode
		case MANUAL_MODE:
			mode_function_pointer=manual_mode();
		default:
			break;

	actuate(direction,value);
}

manual_mode()
{
	mode=MANUAL_MODE;
	if(rcv_qeue)
	{
		pc_packet=read_msg(mode);
	}	
	switch pc_packet->mode
		case SAFE_MODE:
			mode_function_pointer=safe_mode();
		default:
			break;
	actuate(direction,value);
}


while(1)
{
	mode_function_pointer=safe_mode();
}
