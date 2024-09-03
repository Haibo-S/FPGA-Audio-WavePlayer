#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <system.h>
#include <sys/alt_alarm.h>
#include <io.h>


#include "alt_types.h"
#include "fatfs.h"
#include "diskio.h"

#include "ff.h"
#include "monitor.h"
#include "uart.h"


#include <altera_up_avalon_audio.h>
#include <altera_up_avalon_audio_and_video_config.h>

#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"


/*=========================================================================*/
/*  DEFINE: All Structures and Common Constants                            */
/*=========================================================================*/

/*=========================================================================*/
/*  DEFINE: Macros                                                         */
/*=========================================================================*/

#define PSTR(_a)  _a

/*=========================================================================*/
/*  DEFINE: Prototypes                                                     */
/*=========================================================================*/

/*=========================================================================*/
/*  DEFINE: Definition of all local Data                                   */
/*=========================================================================*/
static alt_alarm alarm;
static unsigned long Systick = 0;
static volatile unsigned short Timer;   /* 1000Hz increment timer */

/*=========================================================================*/
/*  DEFINE: Definition of all local Procedures                             */
/*=========================================================================*/

/***************************************************************************/
/*  TimerFunction                                                          */
/*                                                                         */
/*  This timer function will provide a 10ms timer and                      */
/*  call ffs_DiskIOTimerproc.                                              */
/*                                                                         */
/*  In    : none                                                           */
/*  Out   : none                                                           */
/*  Return: none                                                           */
/***************************************************************************/
static alt_u32 TimerFunction (void *context)
{
   static unsigned short wTimer10ms = 0;

   (void)context;

   Systick++;
   wTimer10ms++;
   Timer++; /* Performance counter for this module */

   if (wTimer10ms == 10)
   {
      wTimer10ms = 0;
      ffs_DiskIOTimerproc();  /* Drive timer procedure of low level disk I/O module */
   }

   return(1);
} /* TimerFunction */

/***************************************************************************/
/*  IoInit                                                                 */
/*                                                                         */
/*  Init the hardware like GPIO, UART, and more...                         */
/*                                                                         */
/*  In    : none                                                           */
/*  Out   : none                                                           */
/*  Return: none                                                           */
/***************************************************************************/
static void IoInit(void)
{
   uart0_init(115200);

   /* Init diskio interface */
   ffs_DiskIOInit();

   //SetHighSpeed();

   /* Init timer system */
   alt_alarm_start(&alarm, 1, &TimerFunction, NULL);

} /* IoInit */

#define ESC 27
#define CLEAR_LCD_STRING "[2J"

uint32_t acc_size;                 /* Work register for fs command */
uint16_t acc_files, acc_dirs;
FILINFO Finfo;
#if _USE_LFN
char Lfname[512];
#endif

char Line[256];                 /* Console input buffer */

FATFS Fatfs[_VOLUMES];          /* File system object for each logical drive */
FIL File1, File2;               /* File objects */
DIR Dir;                        /* Directory object */
uint8_t Buff[512] __attribute__ ((aligned(4)));  /* Working buffer */

FILE* lcd;


char content[20][20];
long contentSize[50];
int totalContent = 0;

volatile u_int button0;
volatile u_int button1;
volatile u_int button2;
volatile u_int button3;


static void handle_timer_interrupt(void *context, alt_32 id);
static void btn_interrupt(void *context, alt_32 id);
static int button_value = 0xF;
static int counter = 0;
static int button_pressed = 0;


static
FRESULT scan_files(char *path)
{
    DIR dirs;
    FRESULT res;
    uint8_t i;
    char *fn;


    if ((res = f_opendir(&dirs, path)) == FR_OK) {
        i = (uint8_t)strlen(path);
        while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
            if (_FS_RPATH && Finfo.fname[0] == '.')
                continue;
#if _USE_LFN
            fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
            fn = Finfo.fname;
#endif
            if (Finfo.fattrib & AM_DIR) {
                acc_dirs++;
                *(path + i) = '/';
                strcpy(path + i + 1, fn);
                res = scan_files(path);
                *(path + i) = '\0';
                if (res != FR_OK)
                    break;
            } else {
                //      xprintf("%s/%s\n", path, fn);
                acc_files++;
                acc_size += Finfo.fsize;
            }
        }
    }

    return res;
}


//                put_rc(f_mount((uint8_t) p1, &Fatfs[p1]));

static
void put_rc(FRESULT rc)
{
    const char *str =
        "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
        "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
        "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
        "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
    FRESULT i;

    for (i = 0; i != rc && *str; i++) {
        while (*str++);
    }
    xprintf("rc=%u FR_%s\n", (uint32_t) rc, str);
}

static
void display_help(void)
{
    xputs("dd <phy_drv#> [<sector>] - Dump sector\n"
          "di <phy_drv#> - Initialize disk\n"
          "ds <phy_drv#> - Show disk status\n"
          "bd <addr> - Dump R/W buffer\n"
          "be <addr> [<data>] ... - Edit R/W buffer\n"
          "br <phy_drv#> <sector> [<n>] - Read disk into R/W buffer\n"
          "bf <n> - Fill working buffer\n"
          "fc - Close a file\n"
          "fd <len> - Read and dump file from current fp\n"
          "fe - Seek file pointer\n"
          "fi <log drv#> - Force initialize the logical drive\n"
          "fl [<path>] - Directory listing\n"
          "fo <mode> <file> - Open a file\n"
    	  "fp -  (to be added by you) \n"
          "fr <len> - Read file\n"
          "fs [<path>] - Show logical drive status\n"
          "fz [<len>] - Get/Set transfer unit for fr/fw commands\n"
          "h view help (this)\n");
}

//************************************************************************************************//





void disk_initialization(){
//	disk_initialize((uint8_t) 0);
	xprintf("rc=%d\n", (uint16_t) disk_initialize((uint8_t) 0));
}

void force_initialization(){
	put_rc(f_mount((uint8_t) 0, &Fatfs[0]));
}

void file_list(FATFS *fs){
    uint8_t res = f_opendir(&Dir, 0);
    if (res)
    {
        put_rc(res);
        return;
    }
    long p1, s1, s2 = 0; // otherwise initialize the pointers and proceed.
    for (;;)
    {
        res = f_readdir(&Dir, &Finfo);
        if ((res != FR_OK) || !Finfo.fname[0])
            break;
        if (Finfo.fattrib & AM_DIR)
        {
            s2++;
        }
        else
        {
            s1++;
            p1 += Finfo.fsize;
        }
        xprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
                (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                (Finfo.fattrib & AM_HID) ? 'H' : '-',
                (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                (Finfo.fattrib & AM_ARC) ? 'A' : '-',
                (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
                (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63, Finfo.fsize, &(Finfo.fname[0]));

//        xprintf("\n%s", &(Finfo.fname[0]));
        if(isWav(&(Finfo.fname[0]))){
        	strcpy(content[totalContent], &(Finfo.fname[0]));
			contentSize[totalContent] = Finfo.fsize;
			totalContent++;
        }

#if _USE_LFN
        for (p2 = strlen(Finfo.fname); p2 < 14; p2++)
            xputc(' ');
        xprintf("%s\n", Lfname);
#else
        xputc('\n');
#endif
    }
    xprintf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
    res = f_getfree(0, (uint32_t *) & p1, &fs);
    if (res == FR_OK)
        xprintf(", %10lu bytes free\n", p1 * fs->csize * 512);
    else
        put_rc(res);
}

void normal_speed(int len, alt_up_audio_dev * audio_dev){
	int index = 0;

	while (len > 0) {
		uint32_t cnt = (len > sizeof(Buff)) ? sizeof(Buff) : len;
		uint8_t res = f_read(&File1, Buff, cnt, &cnt);

		for (index = 0; index < cnt; index += 4) {
			while (alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_RIGHT) < 4 &&
				   alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_LEFT) < 4) {}

			unsigned int l_buf = (Buff[index + 3] << 8) | (Buff[index + 2] & 0xFF);
			unsigned int r_buf = (Buff[index + 1] << 8) | (Buff[index] & 0xFF);

			alt_up_audio_write_fifo(audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
			alt_up_audio_write_fifo(audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);

			len -= 4;
		}
	}
}

void half_speed(int len, alt_up_audio_dev * audio_dev) {
    int index = 0;

    while (len > 0) {
    	uint32_t cnt = (len > sizeof(Buff)) ? sizeof(Buff) : len;
    	uint8_t res = f_read(&File1, Buff, cnt, &cnt);

        for (index = 0; index < cnt; index += 4) {
            while (alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_RIGHT) < 4 &&
                   alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_LEFT) < 4) {}

            unsigned int l_buf = (Buff[index + 3] << 8) | (Buff[index + 2] & 0xFF);
            unsigned int r_buf = (Buff[index + 1] << 8) | (Buff[index] & 0xFF);

            alt_up_audio_write_fifo(audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
            alt_up_audio_write_fifo(audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);

            while (alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_RIGHT) < 4 &&
                   alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_LEFT) < 4) {}

            alt_up_audio_write_fifo(audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
            alt_up_audio_write_fifo(audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);

            len -= 4;
        }
    }
}

void double_speed(int len, alt_up_audio_dev * audio_dev) {
    int index = 0;

    while (len > 0) {
    	uint32_t cnt = (len > sizeof(Buff)) ? sizeof(Buff) : len;
		uint8_t res = f_read(&File1, Buff, cnt, &cnt);

        for (index = 0; index < cnt; index += 8) {
            while (alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_RIGHT) < 8 &&
                   alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_LEFT) < 8) {}

            unsigned int l_buf = (Buff[index + 3] << 8) | (Buff[index + 2] & 0xFF);
            unsigned int r_buf = (Buff[index + 1] << 8) | (Buff[index] & 0xFF);

            alt_up_audio_write_fifo(audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
            alt_up_audio_write_fifo(audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);

            len -= 8;
        }
    }
}


void mono(int len, alt_up_audio_dev * audio_dev) {
    int index = 0;

    while (len > 0) {
        uint32_t cnt = (len > sizeof(Buff)) ? sizeof(Buff) : len;
        uint8_t res = f_read(&File1, Buff, cnt, &cnt);

        for (index = 0; index < cnt; index += 4) {
            while (alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_RIGHT) < 4 &&
                   alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_LEFT) < 4) {}

            unsigned int r_buf = (Buff[index + 1] << 8) | (Buff[index] & 0xFF);

            alt_up_audio_write_fifo(audio_dev, &(r_buf), 1, ALT_UP_AUDIO_LEFT);
			alt_up_audio_write_fifo(audio_dev, &(r_buf), 1, ALT_UP_AUDIO_RIGHT);

			len -= 4;
        }
    }
}


int isWav(char *filename){
	int x = (int)strlen(filename);
	return (filename[x-4] == '.' && filename[x-3] == 'W' && filename[x-2] == 'A' &&  filename[x-1] == 'V');
}

static void handle_timer_interrupt(void* context, alt_32 id){
	int current_value = IORD(BUTTON_PIO_BASE, 0);
	IOWR(LED_PIO_BASE, 0, 0x1);

	if (button_pressed == 0 && current_value != 0xF){
		button_value = current_value;
		button_pressed = 1;
	}

	if(current_value == 0xF){
		counter += 1;
		if (counter < 20){
			// restart timer
			IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x5);
        }
	}

	if (counter > 20){
        IOWR(LED_PIO_BASE, 0, 0x0);
        IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x8);
        IOWR(BUTTON_PIO_BASE, 2, 0xF);
        // reset timer variables
        counter = 0;
        button_pressed = 0;
	}
}

void fo(char* ptr){
	put_rc(f_open(&File1, ptr, (uint8_t) 1));
}

static void btn_interrupt(void *context, alt_32 id){
	IOWR(BUTTON_PIO_BASE, 3, 0);
	IOWR(BUTTON_PIO_BASE, 2, 0);
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x5);
}

int main()
{
	alt_irq_register(TIMER_0_IRQ, (void*)0, handle_timer_interrupt);
	alt_irq_register(BUTTON_PIO_IRQ, (void*)0, btn_interrupt);
	IOWR(BUTTON_PIO_BASE, 3, 0);
	IOWR(BUTTON_PIO_BASE, 2, 0xF);
	IOWR(TIMER_0_BASE, 0, 0); // status
	IOWR(TIMER_0_BASE, 1, 0x8); // control (stop is on, ITO is on)
	IOWR(TIMER_0_BASE, 2, 0x8F9C); // periodl
	IOWR(TIMER_0_BASE, 3, 1); // periodh
	int fifospace;
	char *ptr, *ptr2;
	long p1, p2, p3;
	uint8_t res, b1, drv = 0;
	uint16_t w1;
	uint32_t s1, s2, cnt, blen = sizeof(Buff);
	static const uint8_t ft[] = { 0, 12, 16, 32 };
	uint32_t ofs = 0, sect = 0, blk[2];
	FATFS *fs;                  /* Pointer to file system object */
	lcd = fopen("/dev/lcd_display", "w");

	alt_up_audio_dev * audio_dev;
	/* used for audio record/playback */
	unsigned int l_buf;
	unsigned int r_buf;
	// open the Audio port
	audio_dev = alt_up_audio_open_dev ("/dev/Audio");
	if ( audio_dev == NULL)
	alt_printf ("Error: could not open audio device \n");
	else
	alt_printf ("Opened audio device \n");

	IoInit();

	IOWR(SEVEN_SEG_PIO_BASE,1,0x0007);

	xputs(PSTR("FatFs module test monitor\n"));
	xputs(_USE_LFN ? "LFN Enabled" : "LFN Disabled");
	xprintf(", Code page: %u\n", _CODE_PAGE);

	display_help();

	#if _USE_LFN
	    Finfo.lfname = Lfname;
	    Finfo.lfsize = sizeof(Lfname);
	#endif

	disk_initialization();
	force_initialization();

	file_list(fs);


//	for (int i = 0; i < totalContent; i++) {
//		xprintf("Content: %s, Size: %d\n", content[i], contentSize[i]);
//	}

//	while(1){
//
//		int switch_0 = IORD(SWITCH_PIO_BASE, 0) & 0x1;
//		int switch_1 = IORD(SWITCH_PIO_BASE, 0) & 0x2;
//
//
//	}
//	fo(content[totalContent - 4]);
//	mono(contentSize[totalContent - 4], audio_dev);

	int currentSound = 0;
	fprintf(lcd, "%s\n", content[0]);

	while(1){
		switch(button_value){
			case 14:
//				fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING);
//				fprintf(lcd, "\n%s\n", "button 0");
				currentSound++;
				fprintf(lcd, "%s\n", content[currentSound % totalContent]);
				button_value = 0xF;
				break;
			case 13:
				fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING);

				int switch_0 = IORD(SWITCH_PIO_BASE, 0) & 0x1;
				int switch_1 = IORD(SWITCH_PIO_BASE, 0) & 0x2;

				fo(content[currentSound % totalContent]);
				if(!switch_0 && !switch_1){
					fprintf(lcd, "\n%s\n", "PBACK-NORM SPD");
					normal_speed(contentSize[currentSound % totalContent], audio_dev);
				}else if(!switch_0 && switch_1){
					fprintf(lcd, "\n%s\n", "PBACK–HALF SPD");
					half_speed(contentSize[currentSound % totalContent], audio_dev);
				}else if(switch_0 && !switch_1){
					fprintf(lcd, "\n%s\n", "PBACK–DBL SPD");
					double_speed(contentSize[currentSound % totalContent], audio_dev);
				}else{
					fprintf(lcd, "\n%s\n", "PBACK–MONO");
					mono(contentSize[currentSound % totalContent], audio_dev);
				}

				button_value = 0xF;
				break;
			case 11:
				fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING);
				fprintf(lcd, "\n%s\n", "button 2");
				break;
			case 7:
//				fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING);
//				fprintf(lcd, "\n%s\n", "button 3");
				if(!--currentSound)currentSound = totalContent - 1;
				fprintf(lcd, "%s\n", content[currentSound % totalContent]);
				button_value = 0xF;
				break;
			default:
				continue;
		}
	}


//	lcd = fopen("/dev/lcd_display", "w");

//	char* str = "Hello";
//
//	fprintf(lcd, "%s\n", str);
//	fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING);


  return 0;
}
