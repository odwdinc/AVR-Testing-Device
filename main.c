// please see http://frank.circleofcurrent.com/index.php?page=hid_tutorial_1
// the "usb_hid_rpt_desc.hid" file needs to be viewed with the HID Descriptor Tool from USB.org

// required headers
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <avr/eeprom.h> // text file and calibration data is stored in EEPROM
#include <stdio.h> // allows streaming strings
#include <stdint.h>
// V-USB
#include "usbconfig.h"
#include "usbdrv/usbdrv.h"
#define MOD_CONTROL_LEFT    (1<<0)
#define MOD_SHIFT_LEFT      (1<<1)
#define MOD_ALT_LEFT        (1<<2)
#define MOD_GUI_LEFT        (1<<3)
#define MOD_CONTROL_RIGHT   (1<<4)
#define MOD_SHIFT_RIGHT     (1<<5)
#define MOD_ALT_RIGHT       (1<<6)
#define MOD_GUI_RIGHT       (1<<7)

#define KEY_1       30
#define KEY_2       31
#define KEY_3       32
#define KEY_4       33
#define KEY_5       34
#define KEY_6       35
#define KEY_7       36
#define KEY_8       37
#define KEY_9       38
#define KEY_0       39
#define KEY_RETURN  40
#define KEY_TAB		43

#define WHITE_LED 1
#define YELLOW_LED 1
#define LED 3

#define UTIL_BIN4(x)        (uchar)((0##x & 01000)/64 + (0##x & 0100)/16 + (0##x & 010)/4 + (0##x & 1))
#define UTIL_BIN8(hi, lo)   (uchar)(UTIL_BIN4(hi) * 16 + UTIL_BIN4(lo))

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#ifndef NULL
#define NULL    ((void *)0)
#endif

#define clock (PINB & (1<<PINB4))
#define clockbit (PINB & (1<<PINB3))

PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,                    //   REPORT_ID (1)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x03,                    //   REPORT_COUNT (3)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs) ; LED report
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs) ; LED report padding
    
    
    0xc0,                          // END_COLLECTION
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x85, 0x02,                    //     REPORT_ID (2)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0,                          // END_COLLECTION
};

typedef struct
{
	uint8_t report_id;
	uint8_t modifier;
	uint8_t reserved;
	uint8_t keycode[3];
} keyboard_report_t;
static keyboard_report_t keyboard_report;
#define keyboard_report_reset() keyboard_report.report_id=1;keyboard_report.modifier=0;keyboard_report.reserved=0;keyboard_report.keycode[0]=0;keyboard_report.keycode[1]=0;keyboard_report.keycode[2]=0;


typedef struct
{
	uint8_t report_id;
	uint8_t buttons;
	int8_t x;
	int8_t y;
	int8_t wheel;
} mouse_report_t;
static mouse_report_t mouse_report;
#define mouse_report_reset() mouse_report.report_id=2;mouse_report.buttons=0;mouse_report.x=0;mouse_report.y=0;mouse_report.wheel=0;

static uint8_t idle_rate = 500 / 4; // see HID1_11.pdf sect 7.2.4
static uint8_t protocol_version = 0; // see HID1_11.pdf sect 7.2.6



usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	// see HID1_11.pdf sect 7.2 and http://vusb.wikidot.com/driver-api
	usbRequest_t *rq = (void *)data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS)
		return 0; // ignore request if it's not a class specific request

	// see HID1_11.pdf sect 7.2
	switch (rq->bRequest)
	{
		case USBRQ_HID_GET_IDLE:
			usbMsgPtr = &idle_rate; // send data starting from this byte
			return 1; // send 1 byte
		case USBRQ_HID_SET_IDLE:
			idle_rate = rq->wValue.bytes[1]; // read in idle rate
			return 0; // send nothing
		case USBRQ_HID_GET_PROTOCOL:
			usbMsgPtr = &protocol_version; // send data starting from this byte
			return 1; // send 1 byte
		case USBRQ_HID_SET_PROTOCOL:
			protocol_version = rq->wValue.bytes[1];
			return 0; // send nothing
		case USBRQ_HID_GET_REPORT:
			// check for report ID then send back report
			if (rq->wValue.bytes[0] == 1)
			{
				usbMsgPtr = &keyboard_report;
				return sizeof(keyboard_report);
			}
			else if (rq->wValue.bytes[0] == 2)
			{
				usbMsgPtr = &mouse_report;
				return sizeof(mouse_report);
			}
			else
			{
				return 0; // no such report, send nothing
			}
		case USBRQ_HID_SET_REPORT: // no "output" or "feature" implemented, so ignore
		if (rq->wValue.bytes[0] == 1){
				return USB_NO_MSG; // send nothing but call usbFunctionWrite	
			}else{
				
			return 0; // send nothing
			}
		default: // do not understand data, ignore
			return 0; // send nothing
	}
}
static uint8_t LED_state = 0; // see HID1_11.pdf appendix B section 1
int blink_count = 0; // keep track of how many times caps lock have toggled

usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len)
{	 		

	if (data[1] != LED_state)
	{
		// increment count when LED has toggled
		
		LED_state = data[1];
		
		
		if (bit_is_set(LED_state, 1))
		{
			sbi(PORTB, YELLOW_LED);
			blink_count++;
		}
		else
		{
			cbi(PORTB, YELLOW_LED);
		}
	}
	return 1;             // return 1 if we have all data
}
/* ------------------------------------------------------------------------- */
/* ------------------------ Oscillator Calibration ------------------------- */
/* ------------------------------------------------------------------------- */

/* Calibrate the RC oscillator to 8.25 MHz. The core clock of 16.5 MHz is
 * derived from the 66 MHz peripheral clock by dividing. Our timing reference
 * is the Start Of Frame signal (a single SE0 bit) available immediately after
 * a USB RESET. We first do a binary search for the OSCCAL value and then
 * optimize this value with a neighboorhod search.
 * This algorithm may also be used to calibrate the RC oscillator directly to
 * 12 MHz (no PLL involved, can therefore be used on almost ALL AVRs), but this
 * is wide outside the spec for the OSCCAL value and the required precision for
 * the 12 MHz clock! Use the RC oscillator calibrated to 12 MHz for
 * experimental purposes only!
 */
static void calibrateOscillator(void)
{
uchar       step = 128;
uchar       trialValue = 0, optimumValue;
int         x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

    /* do a binary search: */
    do{
        OSCCAL = trialValue + step;
        x = usbMeasureFrameLength();    /* proportional to current real frequency */
        if(x < targetValue)             /* frequency still too low */
            trialValue += step;
        step >>= 1;
    }while(step > 0);
    /* We have a precision of +/- 1 for optimum OSCCAL here */
    /* now do a neighborhood search for optimum value */
    optimumValue = trialValue;
    optimumDev = x; /* this is certainly far away from optimum */
    for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
        x = usbMeasureFrameLength() - targetValue;
        if(x < 0)
            x = -x;
        if(x < optimumDev){
            optimumDev = x;
            optimumValue = OSCCAL;
        }
    }
    OSCCAL = optimumValue;
}
/*
Note: This calibration algorithm may try OSCCAL values of up to 192 even if
the optimum value is far below 192. It may therefore exceed the allowed clock
frequency of the CPU in low voltage designs!
You may replace this search algorithm with any other algorithm you like if
you have additional constraints such as a maximum CPU clock.
For version 5.x RC oscillators (those with a split range of 2x128 steps, e.g.
ATTiny25, ATTiny45, ATTiny85), it may be useful to search for the optimum in
both regions.
*/

void    usbEventResetReady(void)
{
    calibrateOscillator();
    eeprom_write_byte(0, OSCCAL);   /* store the calibrated value in EEPROM */
}

// this function is used to guarantee that the data is sent to the computer once
void usbSendHidReport(uchar * data, uchar len)
{
	while(1)
	{
		usbPoll();
		if (usbInterruptIsReady())
		{
			usbSetInterrupt(data, len);
			break;
		}
	}
}


//-------------Keyboad---------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
void send_report_once()
{
  usbSendHidReport(&keyboard_report, sizeof(keyboard_report));
}

static void addDigit(uchar key,uchar mod)
{
	keyboard_report.report_id=1;
    keyboard_report.keycode[0] =  key;
	keyboard_report.modifier = mod;
	send_report_once();
	keyboard_report_reset(); // release keys
	send_report_once();
}

static void alt_input(int code){
	uchar   digit;
	do{
    	digit = code % 10;
    	code /= 10;
     	if(digit == 0){
     		keyboard_report.keycode[0] = 98;
			keyboard_report.modifier = (1<<6);
        }else{
			keyboard_report.keycode[0] = 88 + digit;
			keyboard_report.modifier = (1<<6);
        }
        send_report_once();
    }while(code != 0);
    
	keyboard_report_reset(); // release keys
	send_report_once();	
}	

// translates ASCII to appropriate keyboard report, taking into consideration the status of caps lock
void ASCII_to_keycode(uint8_t ascii)
{
	keyboard_report.keycode[0] = 0x00;
	keyboard_report.modifier = 0x00;
	
	// see scancode.doc appendix C
	
	if (ascii >= 'A' && ascii <= 'Z')
	{
		keyboard_report.keycode[0] = 4 + ascii - 'A'; // set letter
		if (bit_is_set(LED_state, 1)) // if caps is on
		{
			keyboard_report.modifier = 0x00; // no shift
		}
		else
		{
			keyboard_report.modifier = _BV(1); // hold shift // hold shift
		}
	}
	else if (ascii >= 'a' && ascii <= 'z')
	{
		keyboard_report.keycode[0] = 4 + ascii - 'a'; // set letter
		if (bit_is_set(LED_state, 1)) // if caps is on
		{
			keyboard_report.modifier = _BV(1); // hold shift // hold shift
		}
		else
		{
			keyboard_report.modifier = 0x00; // no shift
		}
	}
	else if (ascii >= '0' && ascii <= '9')
	{
		keyboard_report.modifier = 0x00;
		if (ascii == '0')
		{
			keyboard_report.keycode[0] = 0x27;
		}
		else
		{
			keyboard_report.keycode[0] = 30 + ascii - '1'; 
		}
	}
	else
	{
		switch (ascii) // convert ascii to keycode according to documentation
		{
			case '!':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 1;
				break;
			case '@':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 2;
				break;
			case '#':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 3;
				break;
			case '$':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 4;
				break;
			case '%':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 5;
				break;
			case '^':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 6;
				break;
			case '&':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 7;
				break;
			case '*':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 8;
				break;
			case '(':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 9;
				break;
			case ')':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 0x27;
				break;
			case '~':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '`':
				keyboard_report.keycode[0] = 0x35;
				break;
			case '_':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '-':
				keyboard_report.keycode[0] = 0x2D;
				break;
			case '+':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '=':
				keyboard_report.keycode[0] = 0x2E;
				break;
			case '{':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '[':
				keyboard_report.keycode[0] = 0x2F;
				break;
			case '}':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case ']':
				keyboard_report.keycode[0] = 0x30;
				break;
			case '|':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '\\':
				keyboard_report.keycode[0] = 0x31;
				break;
			case ':':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case ';':
				keyboard_report.keycode[0] = 0x33;
				break;
			case '"':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '\'':
				keyboard_report.keycode[0] = 0x34;
				break;
			case '<':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case ',':
				keyboard_report.keycode[0] = 0x36;
				break;
			case '>':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '.':
				keyboard_report.keycode[0] = 0x37;
				break;
			case '?':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '/':
				keyboard_report.keycode[0] = 0x38;
				break;
			case ' ':
				keyboard_report.keycode[0] = 0x2C;
				break;
			case '\t':
				keyboard_report.keycode[0] = 0x2B;
				break;
			case '\n':
				keyboard_report.keycode[0] = 0x28;
				break;
			default:
				alt_input(ascii);
				return;
		}
	}
	send_report_once();
	keyboard_report_reset(); // release keys
	send_report_once();
}
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//




//---------------------Mouse--------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//

void mouse_report_once()
{
  usbSendHidReport(&mouse_report, sizeof(mouse_report));
}

void mouse_move(int8_t x,int8_t y){
	int i;
  
	if (x>0){
    for(i=0;i<x;i++){  /* 300 ms disconnect */
        mouse_report.x=5;
        mouse_report_once();
        usbPoll();
    }
   }
   else
   {
      for(i=x;i<0;i++){  /* 300 ms disconnect */
        mouse_report.x=-5;
        mouse_report_once();
        usbPoll();
      }
   }
   
	
	mouse_report_reset();
	mouse_report_once();
}



//----------------------------------------------------//
//----------------------------------------------------//
int clockstate;

void inputPoll()
{
	if (clockstate != clock)
	{
		clockstate = clock;
		if (!clock){
			sbi(PORTB, YELLOW_LED);
			blink_count++;
		}
		else
		{
			cbi(PORTB, YELLOW_LED);
		}
	}
	
}

//----------------------------------------------------//
//----------------------------------------------------//
int poolcout;
static void Poll(void)
{
	sbi(PORTB, WHITE_LED);
	switch (poolcout++)
      {
         case 1:
			addDigit(0,0);
			addDigit(21,MOD_GUI_RIGHT);
			addDigit(0,0);
			break;
		case 2:
			puts_P(PSTR("notepad.exe"));
			break;
		case 3:
			puts_P(PSTR("+------------------+"));
			puts_P(PSTR("¦   Hello World    ¦"));
			puts_P(PSTR("¦------------------¦"));
      		puts_P(PSTR("¦ (1) USB Keyboad  ¦"));
      		puts_P(PSTR("¦------------------¦"));
      		puts_P(PSTR("¦ (2) USB MOuse    ¦"));
      		puts_P(PSTR("+------------------+"));
			break;
		case 4:
			mouse_move(50,0);
			break;
		case 5:
			mouse_move(-50,0);
			break;
		case 6:
			addDigit(76,20); //ctrl+alt+delt
			break;
		case 7:
			poolcout=0;
			break;
      }
	

	cbi(PORTB, WHITE_LED);
}

// stdio's stream will use this funct to type out characters in a string
void type_out_char(uint8_t ascii, FILE *stream)
{
	ASCII_to_keycode(ascii);
}
static FILE mystdout = FDEV_SETUP_STREAM(type_out_char, NULL, _FDEV_SETUP_WRITE); // setup writing stream
int main()
{
int i;
	wdt_disable(); // no watchdog, just because I'm lazy
	stdout = &mystdout; // set default stream
	DDRB |= 1 << WHITE_LED;
	
	DDRB &= ~(1<<DDB4);
	DDRB &= ~(1<<DDB3);
	
	PORTB |= 1<<DDB3;
	PORTB |= 1<<DDB4;
	
	sbi(PORTB, WHITE_LED);
    for(i=0;i<20;i++){  /* 300 ms disconnect */
        _delay_ms(15);
    }
	cbi(PORTB, WHITE_LED);
	
	sbi(PORTB, YELLOW_LED);
    for(i=0;i<20;i++){  /* 300 ms disconnect */
        _delay_ms(15);
    }
	cbi(PORTB, YELLOW_LED);
	
  usbDeviceDisconnect(); // enforce USB re-enumeration, do this while interrupts are disabled!
	_delay_ms(250);
  usbDeviceConnect();
  usbInit(); // start v-usb
  sei(); // enable interrupts	
	for(;;){
		// set the report IDs manually
		keyboard_report.report_id = 1;
		mouse_report.report_id = 2;

		if(blink_count > 2){
			Poll();
			blink_count =0;
		}
		usbPoll();
		inputPoll();
		
	}
	
	return 0;
}