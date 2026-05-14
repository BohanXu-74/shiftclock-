#include <stc15.h>
//#define DHT P5_4
#define AB P1_1
#define rtcClk P3_7
#define rtcData P3_6
#define ce P3_3
#define buzzer P3_2
#define modeButton P5_5
#def ne startButton P5_4

/*************************************************/
#define clk P1_0

/*************************************************/

#define u8 unsigned char

u8 min, sec, hour, beforeHour;
u8 tempMin, tempSec, tempHour;
u8 mode, alarmMin[16], alarmHour[16], alarmStatus[16];
u8 digitOne, digitTwo, digitThree, digitFour;
u8 g = 0,x = 0;
u8 buzzerReset = 0;
u8 buzzer2;
u8 modeDown = 0, startDown = 0;
u8 timerRunning = 0;
unsigned int timerEnd, timerNow, timerShow;
u8 timerMin, timerSec;

enum stateMachine {timeDisplay, timeSet, changeMin, changeHour, alarmTitle, alarmMenu, alarmState, alarmSet, timerMenu,timerSet,timerRun,timerPause};

enum stateMachine state = timeDisplay;

const u8 bcd[21][8] = {{0,0,1,1,1,1,1,1}, // 0
							  {0,0,0,0,0,1,1,0},// 1
							  {0,1,0,1,1,0,1,1},// 2
							  {0,1,0,0,1,1,1,1},// 3
							  {0,1,1,0,0,1,1,0},// 4
							  {0,1,1,0,1,1,0,1},// 5
							  {0,1,1,1,1,1,0,1},// 6
							  {0,0,0,0,0,1,1,1},// 7
							  {0,1,1,1,1,1,1,1},// 8
							  {0,1,1,0,1,1,1,1},// 9
							  {0,0,0,0,0,0,0,0},// blank
							  {0,1,1,1,0,1,1,1},// A
							  {0,1,1,1,1,1,0,0},// b
							  {0,0,1,1,1,0,0,1},// C
							  {0,1,1,1,1,0,0,1},// E
							  {0,1,1,1,0,0,0,1},// F
							  {0,1,1,1,0,1,1,0},// H
							  {0,0,1,1,0,0,0,0},// I
							  {0,1,1,0,1,1,1,1},// g
							  {0,1,1,1,1,0,0,0},// t
							  {0,0,1,1,1,0,0,0},// L
							  };

u8 BCDToDec(u8 val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

u8 decToBCD(u8 val) {
    return ((val / 10) << 4) | (val % 10);
}

void writeByte(u8 dat) {
    u8 i;
    for (i = 0; i < 8; i++) {
        rtcData = dat & 0x01;
        rtcClk = 1; 
        rtcClk = 0;         
        dat >>= 1;
    }
}

u8 readByte() {
    u8 i, dat = 0;
    rtcData = 1;
    for (i = 0; i < 8; i++) {
        dat >>= 1;
        if (rtcData) dat |= 0x80;
        rtcClk = 1;
        rtcClk = 0;
    }
    return dat;
}


void write(u8 addr, u8 dat) {
    ce = 0;
    rtcClk = 0;
    ce = 1; 
    writeByte(addr); 
    writeByte(dat);
    ce = 0;
}


u8 read(u8 addr) {
    u8 dat;
    ce = 0;
    rtcClk = 0;
    ce = 1;
    writeByte(addr | 0x01);
    dat = readByte();
    ce = 0;
    return dat;
}

void rtcInit() {
    write(0x8E, 0x00); // Disables Write Protect
    // Ensure Clock Halt (CH) bit is 0 to start the oscillator
    // This reads the current seconds and clears the 7th bit
    u8 sec = read(0x81);
    write(0x80, sec & 0x7F); 
    write(0x8E, 0x80); // Re-enable Write Protect
}

void rtcSetTime(u8 h, u8 m, u8 s) {
    write(0x8E, 0x00);            // WP Off
    write(0x84, decToBCD(h));   // Hours
    write(0x82, decToBCD(m));   // Minutes
    write(0x80, decToBCD(s));   // Seconds
    write(0x8E, 0x80);            // WP On
}

void rtcReadTime() {
    // Read from DS1302 and store in our designated registers
    sec = BCDToDec(read(0x81));
    min = BCDToDec(read(0x83));
    hour = BCDToDec(read(0x85));
}

//********************************************************* actual functions for code *******************************************************//

void delay(u8 b)
{
	u8 a,z;
	for (a = 0; a < b; a++);
		for (z = 0; z < 254; z++);
}

void delayLong(unsigned int b)
{
	unsigned int a,z;
	for (a = 0; a < b; a++)
		for (z = 0; z < 10000; z++);
}


void data(u8 data)
{
	u8 i = 0;
	for(i = 0; i < 8; i++)
	{
		AB = bcd[data][i];
		clk = 1;
		clk = 0;
	}

}

void wordData(u8 a, u8 b, u8 c, u8 d)
{
	digitOne = a;
	digitTwo = b;
	digitThree = c;
	digitFour = d;
}

void buzzerOn(u8 pitch)
{
	buzzerReset = pitch;
}

void timer0() __interrupt 12
{
	g++;
	if(g == 4)
	{
		g = 0;
	}
	if(g == 0)
	{
		P1_2 = 0;
		data(digitOne);
		P1_5 = 1;
	}
	else if(g == 1)
	{
		P1_5 = 0;
		data(digitTwo);
		P1_4 = 1;

	}
	else if(g == 2)
	{
		P1_4 = 0;
		data(digitThree);
		P1_3 = 1;
	}
	else if(g == 3)
	{
		P1_3 = 0;
		data(digitFour);
		P1_2 = 1;
	}
}

void buzzerSound() __interrupt 1
{
	TR0 = 0;
	TF0 = 0;
	TH0 = 0xFE;
	TL0 = 0x00;
	TR0 = 1;
	x++;
	if(buzzerReset == 255)
	{
		buzzer = 0;
		return;
	}
	else if(x > buzzerReset)
	{
		buzzer2 = ~buzzer2;
		x = 0;
	}
	buzzer = buzzer2;
}

void main()
{
	/*****************Setup************************/
	u8 a = 0,f = 1, b = 0;
	u8 simpleMode = 0;
	unsigned int i = 0;
	u8 h = 0;
	P3M1 = 0x00;
	P3M0 = 0xFF;
	P1M1 = 0x00;
	P1M0 = 0B00000011;
	mode = 0;
	rtcInit();
	for(i = 0; i < 15; i++)
	{
		alarmMin[i] = 0;
		alarmHour[i] = 0;
		alarmStatus[i] = 0;
	}
	buzzerOn(255);
	TMOD = 0x01;
	EA = 1;
	IE |= 0x02;
	TH0 = 0xFE;
	TL0 = 0x00;
	TR0 = 1;
	IE2 |= 0x04;
	T2L = 0x00;
	T2H = 0xFA;
	AUXR |= 0x10;
	P1_5 = 0;
	P1_4 = 0;
	P1_3 = 0;
	P1_2 = 0;
	/**********************Main Loop**************************/
	while (1)
	{
		rtcReadTime();
		buzzerOn(255);
		/************************Button Press*********************/
		if(!modeButton)
		{
			buzzerOn(1);
			delay(255);
			if(!modeButton)	
			{
				while(!modeButton);
				modeDown = 1;
			}
			buzzerOn(255);
			delay(255);
		}
		else if(!startButton)
		{
			buzzerOn(1);
			delay(255);
			if(!startButton)	
			{
				while(!startButton);
				startDown = 1;
			}
			buzzerOn(255);
			delay(255);
		}
		/*****************************state that displays time***********************/
		if(state == timeDisplay)
		{
			wordData(hour/10, hour%10, min/10, min%10);
			if(startDown)
			{
				state = changeMin;
				tempSec = 0;
				tempMin = min;
				tempHour = hour;
				i = 0;
				simpleMode = 0;
			}
			if(modeDown)
			{
				state = alarmTitle;
			}
		}
		/*****************************sstate that changes min for either alarm or time***********************/
		else if(state == changeMin)
		{
			i++;
			if(i> 500)
			{
				wordData(tempHour/10, tempHour%10, tempMin/10, tempMin%10);
			}
			else if(i < 500)
			{
				wordData(tempHour/10, tempHour%10, 10, 10);
			}
			if(i > 1000)
			{
				i = 0;
			}
			if(modeDown)
			{
				tempMin++;
				if(tempMin > 59)
				{
					tempMin = 0;
				}
			}
			if(startDown)
			{
				state = changeHour;
				i = 0;
			}
		}
		/*****************************state that changed hour for time or alarm***********************/
		else if(state == changeHour)
		{
			i++;
			if(i> 500)
			{
				wordData(tempHour/10, tempHour%10, tempMin/10, tempMin%10);
			}
			else if(i < 500)
			{
				wordData(10,10, tempMin/10, tempMin%10);
			}
			if(i > 1000)
			{
				i = 0;
			}
			if(modeDown)
			{
				tempHour++;
				if(simpleMode != 2)
				{
					if(tempHour > 23)
					{
						tempHour = 0;
					}
				}
				else
				{
					if(tempHour > 59)
					{
						tempHour = 0;
					}
				}
			}
			if(startDown)
			{
				if(simpleMode == 1)
				{
					state = alarmSet;
				}
				else if(simpleMode == 0)
				{
					state = timeSet;
				}
				else if(simpleMode == 2)
				{
					state = timerSet;
				}
			}
		}
		/*****************************state that sets time and puts temptime into real time;***********************/
		else if(state == timeSet)
		{
			rtcSetTime(tempHour,tempMin, 0);
			state = timeDisplay;
		}
		/*****************************state that sets alarm and puts temptime into real alarm;***********************/
		else if(state == alarmSet)
		{
			alarmMin[f-1] = tempMin;
			alarmHour[f-1] = tempHour;
			alarmStatus[f-1] = 1;
			b = 0;
			f = 1;
			state = timeDisplay;
		}
		/*****************************state that shows the AL to show that it is in alarm mode***********************/
		else if(state == alarmTitle)
		{
			wordData(11,21,10,10);
			if(startDown)
			{
				state = alarmMenu;
			}
			if(modeDown)
			{
				state = timerMenu;
			}
		}
		/*****************************state that shows your alarms that you can choose from***********************/
		else if(state == alarmMenu)
		{
			wordData(0,0,f/10,f%10);
			if(modeDown)
			{
				f++;
				if(f > 16)
				{
					f = 1;
				}
			}
			if(startDown)
			{
				state = alarmState;
				tempHour = alarmHour[f-1];
				tempMin = alarmMin[f-1];
				simpleMode = 1;
			}
		}
		/*****************************state that sets if the alarm is on or of***********************/
		else if(state == alarmState)
		{
			if(b == 0)
			{
				wordData(0,0,0,0);
			}
			else if(b == 1)
			{
				wordData(1,1,1,1);
			}
			else if(b >= 2)
			{
				b = 0;
			}
			if(modeDown)
			{
				b++;
			}
			else if(startDown)
			{
				if(b)
				{
					state = changeMin;
				}
				else
				{
					state = timeDisplay;
				}
			}	
		}
		/*****************************state that shows the TI for timer mode***********************************/
		else if(state == timerMenu)
		{
			if(timerRunning == 1)
			{
				state = timerRun;
			}
			if(timerRunning == 2)
			{
				state = timerPause;
			}
			wordData(19, 17, 10, 10);
			if(startDown)
			{
				simpleMode = 2;
				tempHour = 0;
				tempMin = 0;
				state = changeMin;
			}
			if(modeDown)
			{
				state = timeDisplay;
			}
		}
		/*****************************state that sets the timer and starts it**********************************/
		else if(state == timerSet)
		{
			timerNow = (tempHour*60) + tempMin;
			timerEnd = ((min*60)+sec)+ timerNow;
			timerRunning = 1;
			state = timerRun;
		}
		/**************code to lighten up space for timer******************/
		timerNow = (min*60+sec);
		timerMin = timerShow/60;
		timerSec = timerShow%60;
		/******************************state that shows the timer running*************************************/
		if(state == timerRun)
		{
			timerShow = timerEnd - timerNow;
			wordData(timerMin/10, timerMin%10, timerSec/10, timerSec%10);
			if(startDown)
			{
				state = timerPause;
				timerRunning = 2;
			}
			if(modeDown)
			{
				mode = timeDisplay;
			}
		}
		else if(state == timerPause)
		{
			timerEnd = timerShow+timerNow;
			wordData(timerMin/10, timerMin%10, timerSec/10, timerSec%10);
			if(startDown)
			{
				state = timerRun;
				timerRunning = 1;
			}
			if(modeDown)
			{
				state = timeDisplay;
				timerRunning = 2;
			}
		}
		modeDown = 0;
		startDown = 0;
		/**********************alarm handling***************************/
		for(h = 0; h < 16; h++)
		{
			if(alarmStatus[h])
			{
				if(min == alarmMin[h] && hour == alarmHour[h] && sec == 0)
				{
					while(1)
					{
						wordData(0,0,(h+1)/10,(h+1)%10);
						buzzerOn(1);
						for(a = 0; a < 255; a++)
						{							
							delay(50);
						}
						buzzerOn(255);
						for(a = 0; a < 255; a++)
						{							
							delay(10);
						}
						buzzerOn(5);
						for(a = 0; a < 255; a++)
						{							
							delay(50);
						}
						buzzerOn(255);
						for(a = 0; a < 255; a++)
						{							
							delay(10);
						}
						wordData(10,10,10,10);
						delayLong(10);
						if(!startButton)
						{
							buzzerOn(2);
							delay(255);
							if(!startButton)	
							{
								while(!startButton);
								buzzerOn(255);
								break;
							}
							buzzerOn(255);
						}
					}	
				}
			}
		}
		/***********************Timer Handling****************************/
		if(timerShow == 0 && timerRunning == 1)
		{
			timerRunning = 0;
			timerEnd = 0;
			timerShow = 0;
			timerNow = 0;
			while(1)
			{
				wordData(19,17,0,0);
				buzzerOn(4);
				for(a = 0; a < 255; a++)
				{							
					delay(100);
				}
				buzzerOn(255);
				for(a = 0; a < 255; a++)
				{							
					delay(100);
				}
				buzzerOn(0);
				for(a = 0; a < 255; a++)
				{							
					delay(100);
				}
				buzzerOn(255);
				for(a = 0; a < 255; a++)
				{							
					delay(100);
				}
				wordData(10,10,10,10);
				delayLong(10);
				if(!startButton)
				{
					buzzerOn(2);
					delay(255);
					if(!startButton)	
					{
						while(!startButton);
						buzzerOn(255);
						state = timeDisplay;
						break;
					}
					buzzerOn(255);
				}
			}
		}
	}
}
