
bool erase_flight_data();
bool write_flight_data();
bool read_flight_data();
//the variable to be updated with get_time_us();
uint32_t time;
uint32_t N_BYTES = 24;
/*------------------------------------------------------------------
 * logs the flight data 
 * write timestamp,mode,batteryvoltage,barometer, gyroscope and
 * accelerometer, connection lost?,  data to flash
 * jmi
 *------------------------------------------------------------------
 */
bool write_flight_data(){
	//time = get_time_us();
	//adress between 0 and 131071
	static uint32_t address = 0;
	uint8_t data[nBytes];
	//time = uint32_t 
	data[0] = (time>>24) & 0xFF; 
	data[1] = (time>>16) & 0xFF; 
	data[2] = (time>>8) & 0xFF; 
	data[3] = time & 0xff;	
	//one byte for mode	
	data[4] = cur_mode;
	//bat_volt = uint16_t
	data[5] = (bat_volt>>8) & 0xFF;
	data[6] = bat_volt & 0xFF;
	//pressure = uint32_t
	data[7] = (pressure>>24) & 0xFF; 
	data[8] = (pressure>>16) & 0xFF; 
	data[9] = (pressure>>8) & 0xFF; 
	data[10] = pressure & 0xff;	
	
	//uint16_t sp,sq,sr => gyro p,q,r rate
	data[11] = (sp>>8) & 0xFF;
	data[12] = sp & 0xFF;
	data[13] = (sq>>8) & 0xFF;
	data[14] = sq & 0xFF;
	data[15] = (sr>>8) & 0xFF;
	data[16] = sr & 0xFF;
	
	//uint16_t phi,theta,psi =>  	
	data[17] = (phi>>8) & 0xFF;
	data[18] = phi & 0xFF;
	data[19] = (theta>>8) & 0xFF;
	data[20] = theta & 0xFF;
	data[21] = (psi>>8) & 0xFF;
	data[22] = psi & 0xFF;
	data[23] = 170;

	printf("writing debug flight data\n");	

	if(flash_write_bytes(address, data, nBytes)){
		address = address + nBytes; 		
		if((address + nBytes) >= MAX_FLASH_ADDRESS){
			erase_flight_data();
			address = 0;				
		}
	} else {
		printf("error writing to spi\n");
		return 0;
	} 
	printf("succesfull write\n");
	return 1;
} 

/*------------------------------------------------------------------
 * reads the flight data from memory.
 *------------------------------------------------------------------
 */
bool read_flight_data(){
	//adress between 0 and 131071
	static uint32_t address = 0;
	
	uint8_t buffer[nBytes];
	uint32_t rtime, rpressure;
	uint8_t rmode;
    uint8_t static written_check = 170;
	uint16_t rbat_volt, rsp, rsq, rsr, rphi, rtheta, rpsi; 
	printf("log of previous flight\n");
	nrf_delay_ms(15);		
	printf("address,time_ms,mode,bat_volt,pressure,sp,sq,sr,phi,theta,psi,write_check(=170)\n");
	nrf_delay_ms(15);
	//escape loop if end is reached, or byte 23 does not contain AA
	//else we need to dump complete flash, which is time consuming.
	//170 is an arbitrary picked value
	while(((address + nBytes) < MAX_FLASH_ADDRESS) && (written_check == 170)) {
		if(flash_read_bytes(address, buffer, nBytes)){
			rtime = (buffer[0]<<24) + (buffer[1]<<16) + (buffer[2]<<8) + (buffer[3]);
			rmode = buffer[4];
			rbat_volt = (buffer[5]<<8) + buffer[6];
			rpressure = (buffer[7] <<24) + (buffer[8] << 16) + (buffer[8] << 8) + buffer[10];
	
			rsp = (buffer[11] << 8) + buffer[12];
			rsq = (buffer[13] << 8) + buffer[14];	
			rsr = (buffer[15] << 8) + buffer[16];
			
			rphi = (buffer[17] << 8) + buffer[18]; 
			rtheta = (buffer[19] << 8) + buffer[20];
			rpsi = (buffer[21] << 8) + buffer[22];
			written_check = buffer[23];			
			
			printf("%ld,%ld,%d,%d,%ld,%d,%d,%d,%d,%d,%d,%d\n",address, rtime,rmode,rbat_volt,rpressure,rsp,rsq,rsr,rphi,rtheta,rpsi,written_check);		
			address += nBytes;
			nrf_delay_ms(15);		
		}  else {
			printf("error while reading\n");
			return 0;
		}						
	}	
	return 0;
} 

/*------------------------------------------------------------------
 * erases all the flight data so we can log a new flight
 * call this function after leaving safe mode, so we can 
 * read the previous log in safe mode!	
 * jmi
 *-----------------------------------------------------------------
 */
bool erase_flight_data() { 
	if(flash_chip_erase()){
		return true;
	} else {
		return false;
	}
}
