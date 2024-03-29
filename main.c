#include <windows.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

/* defines */
#define LEN_LONG  256
#define LEN_MID   64
#define LEN_SHORT 32
#define TARGET_BAUDRATE 80000
#define MODE_UNDEF     0
#define MODE_NORMAL40K 1
#define MODE_NORMAL60K 2
#define MODE_TEST      4


/* subroutines */
void	print_usage( char* COMMAND_NAME ); // error message
void	check_args(int , char**, char* , char*, unsigned long*);
void	assume_comport(char*, char*);
void	abort_handler(int);
HANDLE	open_comport(unsigned char* COMPORT_NAME ,unsigned long COMPORT_BAUD );
void	send_1ms();
void	send_200ms();
void	send_500ms();
void	send_800ms();
void	getTimecode(char* , unsigned int);
void	wait_clock_00sec(char *);
void	wait_clock_nextsec();
void	send_TIMECODE(char*);


/* vars */
HANDLE	COMPORT_h;
unsigned char MODE = MODE_UNDEF;



int main( int argc, char *argv[] ){
	unsigned char COMMAND_NAME[LEN_LONG]={ '\0' };
	unsigned char COMPORT_NAME[LEN_MID];
	unsigned long  COMPORT_BAUD=0;
	unsigned char TIMECODE_str[LEN_MID] = { '\0' };
	unsigned char TIME_str[LEN_SHORT]   = { '\0' };
	
	/* setting interrupt */
	signal(SIGINT, abort_handler);
	
	
	/* check arg & get COMMAND_NAME,COMPORT_NAME,COMPORT_BAUD */
	check_args( argc , argv , COMMAND_NAME , COMPORT_NAME , &COMPORT_BAUD);
	
	/* check 60kHz mode */
	if ( 0 != (MODE & MODE_NORMAL60K)  ){
		printf("Warning:\t60kHz mode enabled\n");
	}
	
	/* check Test mode */
	if( 0 != (MODE & MODE_TEST) ){
		printf("Warning:\tTest mode enabled\n");
		COMPORT_h = open_comport(COMPORT_NAME,COMPORT_BAUD);
		printf("TESTING:\tsend_800ms\t");
		send_800ms();
		printf("done\n");
		CloseHandle(COMPORT_h);
		return(0);
	}
	
	/* open COM port & setting */
	COMPORT_h = open_comport(COMPORT_NAME,COMPORT_BAUD);
	
	
	/* timecode sending LOOP */
	while(1){
		
		/* get "next minutes" TIMECODE */
		getTimecode(TIMECODE_str, 1);
		
		/* wait clock 00sec */
		printf("Waiting:\tclock 00sec\t");
		wait_clock_00sec( TIME_str );
		printf("done(%s)\n",TIME_str);
		
		/* send TIMECODE */
		send_TIMECODE(TIMECODE_str);
		
	}
	
	
	CloseHandle(COMPORT_h);
	return(0);
}








/* ********** subroutines ************* */
void send_TIMECODE(char *TIMECODE_str){
	unsigned int i = 0;
	
	printf("TIMECODE:\t%s\n",TIMECODE_str);
	printf("Sending:\t");
	
	while( '\0' != TIMECODE_str[i] ){
		
		switch( TIMECODE_str[i] ){
			
			case 'M' :
				printf("M");
				send_200ms();
				if( '\0' != TIMECODE_str[i+1] ) { wait_clock_nextsec(); }
				break;
			
			case '1' :
				printf("1");
				send_500ms();
				wait_clock_nextsec();
				break;
			
			case '0' :
				printf("0");
				send_800ms();
				wait_clock_nextsec();
				break;
			
			default :
				printf("Error:\tinvalid TIMECODE (%s)\n",TIMECODE_str);
				exit(1);
				
		}// end switch
		
		i++;
	}
	printf("\n");
	
	
}


void wait_clock_00sec(char *result){
	struct tm *timeptr;
	time_t t;
	
	while( TRUE ){
		t = time(NULL);
		if( (t % 60) == 0 ) break;
		Sleep(1);
	}
	t = time(NULL);
	
	timeptr = localtime(&t);
	strftime(result , LEN_SHORT , "%Hh%Mm%Ss" , timeptr);
}

void wait_clock_nextsec(){
	time_t t_begin = time(NULL);
	time_t t_now;
	while( TRUE ){
		t_now = time(NULL);
		if( t_now != t_begin ) break;
		Sleep(1);
	}
}






void getTimecode(char *result, unsigned int DIFF_MINUTES){
	char buff[LEN_SHORT] = { '\0' };
	struct tm *timeptr;
	time_t t = time(NULL);
	unsigned int  YEAR,DATE;
	unsigned char WEEK,HOUR,MIN;
	unsigned char YEAR10,YEAR01;
	unsigned char DATE100,DATE010,DATE001;
	unsigned char HOUR10,HOUR01;
	unsigned char MIN10 , MIN01;
	unsigned char           Y80,Y40,Y20,Y10,Y8,Y4,Y2,Y1;
	unsigned char                              W4,W2,W1;
	unsigned char D200,D100,D80,D40,D20,D10,D8,D4,D2,D1;
	unsigned char                   H20,H10,H8,H4,H2,H1;
	unsigned char               M40,M20,M10,M8,M4,M2,M1;
	unsigned char LS2,LS1, PA2,PA1, SU2,SU1, ST6,ST5,ST4,ST3,ST2,ST1;
	
	/* calc DIFF MINUTES */
	t += 60 * DIFF_MINUTES;

	/* get time */
	timeptr  = localtime(&t);
	strftime(buff   , LEN_SHORT, "%Y" , timeptr); YEAR = atoi(buff);
	strftime(buff   , LEN_SHORT, "%w" , timeptr); WEEK = atoi(buff);
	strftime(buff   , LEN_SHORT, "%j" , timeptr); DATE = atoi(buff);
	strftime(buff   , LEN_SHORT, "%H" , timeptr); HOUR = atoi(buff);
	strftime(buff   , LEN_SHORT, "%M" , timeptr); MIN  = atoi(buff);

	/* forming digit */
	YEAR10 = ( YEAR / 10 ) % 10;
	YEAR01 =   YEAR        % 10;
	DATE100= ( DATE / 100) % 10;
	DATE010= ( DATE / 10 ) % 10;
	DATE001=   DATE        % 10;
	HOUR10 = ( HOUR / 10 ) % 10;
	HOUR01 =   HOUR        % 10;
	MIN10  = ( MIN  / 10 ) % 10;
	MIN01  =   MIN         % 10;

	/* make timecode bit */
	Y80 = ( YEAR10  & 8 ) >> 3;
	Y40 = ( YEAR10  & 4 ) >> 2;
	Y20 = ( YEAR10  & 2 ) >> 1;
	Y10 = ( YEAR10  & 1 ) >> 0;
	Y8  = ( YEAR01  & 8 ) >> 3;
	Y4  = ( YEAR01  & 4 ) >> 2;
	Y2  = ( YEAR01  & 2 ) >> 1;
	Y1  = ( YEAR01  & 1 ) >> 0;
	W4  = ( WEEK    & 4 ) >> 2;
	W2  = ( WEEK    & 2 ) >> 1;
	W1  = ( WEEK    & 1 ) >> 0;
	D200= ( DATE100 & 2 ) >> 1;
	D100= ( DATE100 & 1 ) >> 0;
	D80 = ( DATE010 & 8 ) >> 3;
	D40 = ( DATE010 & 4 ) >> 2;
	D20 = ( DATE010 & 2 ) >> 1;
	D10 = ( DATE010 & 1 ) >> 0;
	D8  = ( DATE001 & 8 ) >> 3;
	D4  = ( DATE001 & 4 ) >> 2;
	D2  = ( DATE001 & 2 ) >> 1;
	D1  = ( DATE001 & 1 ) >> 0;
	H20 = ( HOUR10  & 2 ) >> 1;
	H10 = ( HOUR10  & 1 ) >> 0;
	H8  = ( HOUR01  & 8 ) >> 3;
	H4  = ( HOUR01  & 4 ) >> 2;
	H2  = ( HOUR01  & 2 ) >> 1;
	H1  = ( HOUR01  & 1 ) >> 0;
	M40 = ( MIN10   & 4 ) >> 2;
	M20 = ( MIN10   & 2 ) >> 1;
	M10 = ( MIN10   & 1 ) >> 0;
	M8  = ( MIN01   & 8 ) >> 3;
	M4  = ( MIN01   & 4 ) >> 2;
	M2  = ( MIN01   & 2 ) >> 1;
	M1  = ( MIN01   & 1 ) >> 0;

	/* set LS bit (fixied value) */
	LS1 = 0;
	LS2 = 0;
	
	/* Calc. pari */
	PA1=(H20+H10+H8+H4+H2+H1)     % 2;
	PA2=(M40+M20+M10+M8+M4+M2+M1) % 2;
	
	/* set SU bit (fixed value) */
	SU1 = 0;
	SU2 = 0;
	
	/* set ST bit (fixed value) */
	ST1 = 0;
	ST2 = 0;
	ST3 = 0;
	ST4 = 0;
	ST5 = 0;
	ST6 = 0;
	
	/* build TimeCode */
	// M = marker/Position marker
	sprintf(result,"M%d%d%d0%d%d%d%dM00%d%d0%d%d%d%dM00%d%d0%d%d%d%dM%d%d%d%d00%d%d%dM%d%d%d%d%d%d%d%d%dM%d%d%d%d%d0000M",
		M40,M20,M10,M8,M4,M2,M1,
		H20,H10,H8,H4,H2,H1,
		D200,D100,D80,D40,D20,D10,D8,D4,D2,D1,
		PA1,PA2,SU1,SU2,
		Y80,Y40,Y20,Y10,Y8,Y4,Y2,Y1,
		W4,W2,W1,LS1,LS2);

#if 0
	/* for debug */
	printf("YEAR :\t%d%d\t:    %d%d%d%d_%d%d%d%d\n",YEAR10,YEAR01,Y80,Y40,Y20,Y10,Y8,Y4,Y2,Y1);
	printf("WEEK :\t%d\t:          %d%d%d\n",WEEK,W4,W2,W1);
	printf("DATE :\t%d%d%d\t: %d%d_%d%d%d%d_%d%d%d%d\n",DATE100,DATE010,DATE001,D200,D100,D80,D40,D20,D10,D8,D4,D2,D1);
	printf("HOUR :\t%d%d\t:      %d%d_%d%d%d%d\n",HOUR10,HOUR01,H20,H10,H8,H4,H2,H1);
	printf("MIN  :\t%d%d\t:     %d%d%d_%d%d%d%d\n",MIN10,MIN01,M40,M20,M10,M8,M4,M2,M1);
#endif


}




void abort_handler(int sig){
	printf("\n\nSignal Interrupt:\tclose COMPORT\n\n");
	CloseHandle(COMPORT_h);
	exit(1);
}



void check_args(int argc, char *argv[], char *COMMAND_NAME , char *COMPORT_NAME, unsigned long *COMPORT_BAUD){
	unsigned int i = 0;
	unsigned int j = 0;
	
	/* get command name */
	j=0;i=0;
	while( '\0' != argv[0][i] ){
		COMMAND_NAME[j] = argv[0][i];
		if( '\\' == argv[0][i] ){ j=0; }else{ j++; }
		i++;
	}
	COMMAND_NAME[j]=0x00; // null mark
	
	
	
	/* check arg num */
	// argc==1 : COMPORT assume mode, and default BAUDRATE
	// argc==2 : use default BAUDRATE
	// argc==3 : full specified
	// other   : arg error
	if( argc != 3 && argc != 2 && argc != 1 ){
		printf("Error:\tinvalid args\n");
		print_usage(COMMAND_NAME);
		exit(1);
	}
	
	
	
	/* check arg format COMPORT */
	// argc==1 : assume from filename
	// other   : check&get from argv[1]
	if( argc == 1 ){
		assume_comport(COMPORT_NAME , COMMAND_NAME);	// COMPORT assume mode
	}else{
		j=0;i=3;
		if( argv[1][3] == '\0' ){ j=1; }
		while( '\0' != argv[1][i] ){
			if( 0x30 > argv[1][i] || 0x39 < argv[1][i] ){ j=1; }
			i++;
		}
		if( argv[1][0] != 'C' ||
		    argv[1][1] != 'O' ||
		    argv[1][2] != 'M' ||
		    j          !=  0     ){
			printf("Error:\tinvalid args <COMPORT>=%s\n",argv[1]);
			print_usage(COMMAND_NAME);
			exit(1);
		}
		strncpy(COMPORT_NAME,argv[1],LEN_MID-1);
	}
	
	
	/* check arg format BAUDRATE */
	// argc==1 : default BAUDRATE
	// argc==2 : default BAUDRATE
	// too long argv[2].len : default BAUDRATE
	// other   : check&get from argv[2]
	if( argc == 2 || argc == 1 ){
		*COMPORT_BAUD=TARGET_BAUDRATE;	// default BAUDRATE
	}else if( strlen(argv[2]) > 9){
		*COMPORT_BAUD=TARGET_BAUDRATE;	// default BAUDRATE
		printf("Warning:\tbaudrate ignored, using %d\n",TARGET_BAUDRATE);
	}else{
		j=0;i=0;
		while( '\0' != argv[2][i] ){
			if( i >= 1             &&
			    ( 'T'  == argv[2][i] || 't'  == argv[2][i] ) ){
				MODE |= MODE_TEST;
				argv[2][i]='\0';
				break;
			}
			if( 0x30 > argv[2][i] || 0x39 < argv[2][i] ){ j=1; }
			i++;
		}
		if( j != 0 ){
			printf("Error:\tinvalid args <BAUDRATE>=%s\n",argv[2]);
			print_usage(COMMAND_NAME);
			exit(1);
		}
		*COMPORT_BAUD=atol(argv[2]);	// custom BAUDRATE
	}
	if ( *COMPORT_BAUD <= (unsigned long)100000 ){
		MODE |= MODE_NORMAL40K;
	}else{
		MODE |= MODE_NORMAL60K;
	}

}

void assume_comport(char *COMPORT_NAME, char *COMMAND_NAME){
	unsigned int i=0;
	unsigned int j=0;
	
	/* check filename length */
	if( strlen(COMMAND_NAME) <= 8 ){ // filename too short
		printf("Error:\tCan't assume <COMPORT> from \"%s\" (maybe too short)\n",COMMAND_NAME);
		print_usage(COMMAND_NAME);
		exit(1);
	}
	
	/* search COMPORT from filename */
	j=0;i=0;
	while( '\0' != COMMAND_NAME[i+4] ){				// filename check loop
		if( COMMAND_NAME[i+0] == 'C'  &&
		    COMMAND_NAME[i+1] == 'O'  &&
		    COMMAND_NAME[i+2] == 'M'  &&
		    0x30 <= COMMAND_NAME[i+3] &&
		    0x39 >= COMMAND_NAME[i+3]     ){
			COMPORT_NAME[0]=COMMAND_NAME[i+0];		// copy to COMPORT_NAME
			COMPORT_NAME[1]=COMMAND_NAME[i+1];		// copy to COMPORT_NAME
			COMPORT_NAME[2]=COMMAND_NAME[i+2];		// copy to COMPORT_NAME
			COMPORT_NAME[3]=COMMAND_NAME[i+3];		// copy to COMPORT_NAME
			j=4;
			while ( 0x30 <= COMMAND_NAME[i+j] &&
			        0x39 >= COMMAND_NAME[i+j] &&
			        LEN_MID-3-1 > j               ){
				COMPORT_NAME[j]=COMMAND_NAME[i+j];	// copy to COMPORT_NAME
				j++;
			}
			COMPORT_NAME[j]='\0';
			break;
		}
		i++;
	}												// filename check loop end
	
	/* check result */
	if ( j == 0 ){									// error: not found from filename
		printf("Error:\tCan't assume <COMPORT> from \"%s\"\n",COMMAND_NAME);
		print_usage(COMMAND_NAME);
		exit(1);
	}

}


void print_usage(char* COMMAND_NAME){
	printf("\n");
	printf("Usage:\n");
	printf("  %s <COMPORT> [BAUDRATE[t|T]]\n",COMMAND_NAME);
	printf("  <COMPORT> can specify in filename (COMPORT assume mode)\n");
	printf("\n");
	printf("Example:\n");
	printf("  %s   COM3    80000  (full specified)\n",COMMAND_NAME);
	printf("  %s   COM3           (use default BAUDRATE)\n",COMMAND_NAME);
	printf("  fakeJJY_COM3.exe             (COMPORT assume mode)\n");
	printf("  %s   COM3    80000t (Test mode)\n",COMMAND_NAME);
	printf("  %s   COM3   120000  (60kHz mode)\n",COMMAND_NAME);
}



void send_200ms(){
	unsigned int i=0;
	for(i=1;i<=200;i++){
		send_1ms();
	}
}

void send_500ms(){
	unsigned int i=0;
	for(i=1;i<=500;i++){
		send_1ms();
	}
}

void send_800ms(){
	unsigned int i=0;
	for(i=1;i<=800;i++){
		send_1ms();
	}
}


void send_1ms(){
	unsigned char VALUE40KHZMODE[] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
	unsigned char VALUE60KHZMODE[] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
	unsigned char *OUTPUTVALUE;
	DWORD discard_var; // discard
	DWORD len;

	/* check 40kHz or 60kHz mode */
	if ( 0 != ( MODE & MODE_NORMAL40K) ){
		OUTPUTVALUE=VALUE40KHZMODE;
		len=sizeof( VALUE40KHZMODE)/sizeof(unsigned char);
	}else{
		OUTPUTVALUE=VALUE60KHZMODE;
		len=sizeof( VALUE60KHZMODE)/sizeof(unsigned char);
	}

	/* Write 0x55 */
	if( 0 == WriteFile(COMPORT_h , OUTPUTVALUE , len , &discard_var , NULL) ) {
		printf("\nError:\tWrite Failed\n");
		CloseHandle(COMPORT_h);
		exit(1);
	}
	
}


HANDLE open_comport(unsigned char* COMPORT_NAME ,unsigned long COMPORT_BAUD){
	HANDLE h;
	DCB dcb;
	unsigned char ST[LEN_SHORT]={ '\0' };
	unsigned char PARI=0;
	
	
	/* open COM port */
	printf("Opening:\t%s\t\t",COMPORT_NAME);
	h = CreateFile( COMPORT_NAME, 
		GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		0,
		0 ); 
	if ( h == INVALID_HANDLE_VALUE ) {
		printf("\nError:\tFailed open serial port : %s\n",COMPORT_NAME);
		exit(1);
	}
	printf("done\n");
	
	
	
	/* setting COM port */
	printf("Setting:\t%dN81\t",COMPORT_BAUD);
	GetCommState(h , &dcb);
	dcb.BaudRate			= COMPORT_BAUD;
	dcb.ByteSize			= 8;
	dcb.Parity				= NOPARITY;
	dcb.StopBits			= ONESTOPBIT;
	dcb.fBinary				= TRUE;
	dcb.fParity				= FALSE;
	dcb.fOutxCtsFlow		= FALSE;
	dcb.fOutxDsrFlow		= FALSE;
	dcb.fDtrControl			= DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity		= FALSE;
	dcb.fTXContinueOnXoff	= TRUE;
	dcb.fOutX				= FALSE;
	dcb.fInX				= FALSE;
	dcb.fNull				= FALSE;
	dcb.fRtsControl			= RTS_CONTROL_DISABLE;
	dcb.fAbortOnError		= FALSE;
	if( 0 == SetCommState(h , &dcb) ){
		printf("\nError:\tFailed setting serial port : %s %dN81\n",COMPORT_NAME,COMPORT_BAUD);
		CloseHandle(h);
		exit(1);
	}
	printf("done\n");
	
	GetCommState(h , &dcb);
	if( dcb.BaudRate != COMPORT_BAUD ||
		dcb.ByteSize != 8            ||
		dcb.Parity   != NOPARITY     ||
		dcb.StopBits != ONESTOPBIT       ) {
		if (        dcb.Parity == NOPARITY     ) { PARI='N'; }
		else                                     { PARI=0x30 + dcb.Parity; }
		if (      dcb.StopBits == ONESTOPBIT   ) { ST[0]='1';   }
		else if ( dcb.StopBits == ONE5STOPBITS ) { ST[0]='1';ST[1]='.';ST[2]='5'; }
		else                                     { ST[0]='2';   }
		printf("Warning:\tSetting rejected\trequested(%d,N,8,1)\tsetted(%d,%c,%d,%s)\n",COMPORT_BAUD,dcb.BaudRate,PARI,dcb.ByteSize,ST);
	}
	
	
	return(h);
}


