/*
 * main.c
 *
 *  Created on: Mar 8, 2020
 *      Author: patrick
 */


/*
 * File:   main.c
 * Author: patrick
 *
 * Created on February 25, 2020, 12:44 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include "io.h"
#include "bit.h"
#include "scheduler.h"
#include "timer.h"
#include "queue.h"
#include <string.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

//https://embedds.com/interfacing-analog-joystick-with-avr/
void ADC_INIT(void)
{
    ADMUX|=(1<<REFS0);
    ADCSRA|=(1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); //ENABLE ADC, PRESCALER 128
}
//https://embedds.com/interfacing-analog-joystick-with-avr/
uint16_t readadc(uint8_t ch)
{
    ch&=0b00000111;         //ANDing to limit input to 7
    ADMUX = (ADMUX & 0xf8)|ch;  //Clear last 3 bits of ADMUX, OR with ch
    ADCSRA|=(1<<ADSC);        //START CONVERSION
    while((ADCSRA)&(1<<ADSC));    //WAIT UNTIL CONVERSION IS COMPLETE
    return(ADC);        //RETURN ADC VALUE
}



const unsigned char tasksNum=6;
unsigned long tasksPeriodGCD;

task tasks[6];
//globals
unsigned char second, min, hour, display, asecond, amin, ahour, alarm;
Queue alarmWait;


void LCD_Custom_Char (unsigned char loc, unsigned char *msg)
{
	unsigned char j;
	if(loc<8)
	{
		LCD_DisplayString(1,"Wake up!");
		LCD_WriteCommand (0x40);
        /* Command 0x40 and onwards forces the device to point CGRAM address */
		for(j=0;j<8;j++){	/* Write 8 byte for generation of 1 character */
            LCD_WriteData(msg[j]);
        }
        LCD_WriteCommand(0x80);
        LCD_WriteCommand(0xC0);
        LCD_Cursor(9);
        LCD_WriteData(0);
        LCD_Cursor(0);



	}
}

enum joystickInput{
	up,down,left,right,none;
	}joyIn;
joyIn getJoystick(){
	joyIn ji;
	uint16_t x,y;


	x = readadc(0);
	x = x-512;
	y = readadc(1);
	y = y - 512;
	if(x < 100){

				}
		else if(x < 700){
			return right;
		}

		else{
			return left;
		}

	if(y <= 250){

			}
	else if(y < 500){
		return up;
	}

	else{
		return down;
	}
	return none;
}
unsigned char getLastDigit(unsigned char foo){

    unsigned char ret = '0';
    foo = foo%10;
    if(foo == 0){
        ret =  '0';
    }
    else if(foo == 1){
        ret =  '1';
    }
    else if(foo == 2){
        ret =  '2';
    }
    else if(foo == 3){
        ret =  '3';
    }
    else if(foo == 4){
        ret =  '4';
    }
    else if(foo == 5){
        ret =  '5';
    }
    else if(foo == 6){
        ret =  '6';
    }
    else if(foo == 7){
        ret =  '7';
    }
    else if(foo == 8){
        ret =  '8';
    }
    else if(foo == 9){
        ret =  '9';
    }



    return ret;



}

unsigned char getFirstDigit(unsigned char foo){

    unsigned char ret = '0';
    foo = foo/10;
    if(foo == 0){
        ret =  '0';
    }
    else if(foo == 1){
        ret =  '1';
    }
    else if(foo == 2){
        ret =  '2';
    }
    else if(foo == 3){
        ret =  '3';
    }
    else if(foo == 4){
        ret =  '4';
    }
    else if(foo == 5){
        ret =  '5';
    }
    else if(foo == 6){
        ret =  '6';
    }
    else if(foo == 7){
        ret =  '7';
    }
    else if(foo == 8){
        ret =  '8';
    }
    else if(foo == 9){
        ret =  '9';
    }



    return ret;



}

enum sstates{s_init, s_cnt}sstate;

int s_tick(int sstate){
    unsigned char snstate;
    switch(sstate){
        case s_init:
            snstate = s_cnt;
            break;
        case s_cnt:
            snstate = s_cnt;
            break;

        default:
            snstate = s_init;
            break;
    }

    switch(sstate){
        case s_init:
            //second = 0;
            break;
        case s_cnt:
            second++;
            break;

        default:

            break;
    }
    return snstate;
}

enum mstates{m_init, m_cnt}mstate;
int m_tick(int mstate){
    unsigned char mnstate;
    switch(mstate){
        case m_init:
            mnstate = m_cnt;
            break;
        case m_cnt:
            mnstate = m_cnt;
            break;

        default:

            break;
    }

    switch(mstate){
        case m_init:

            break;
        case m_cnt:
            if(second >= 60){
             min++;
             second = 0;
            }
            break;

        default:

            break;
    }

    return mnstate;
}

enum hstates{h_init, h_cnt}hstate;

int h_tick(int hstate){
    unsigned char hnstate;
    switch(hstate){
        case h_init:
            hnstate = h_cnt;
            break;
        case h_cnt:
            hnstate = h_cnt;
            break;

        default:
            hnstate = h_init;
            break;
    }

    switch(hstate){
        case h_init:
            //hour = 12;
            break;
        case h_cnt:
            if(min >= 60){
                hour++;
                min = 0;
            }
            if(hour >= 13){
                hour = 1;
            }
            break;

        default:

            break;
    }
    return hnstate;
}


enum ostates{o_init, o_write, o_wait}ostate;

int o_tick(int ostate){

    unsigned char onstate;
    switch(ostate){
        case o_init:
            onstate = o_write;
            break;
        case o_write:
            if(display == 0){
                onstate = o_wait;
            }
            else onstate = o_write;
            break;
        case o_wait:
            if(display == 0 || alarm == 1){
                onstate = o_wait;
            }
            else onstate = o_write;
            break;
        default:
            onstate = o_init;
            break;
    }

    switch(ostate){
        case o_init:


            display = 1;

            LCD_ClearScreen();
            break;
        case o_write:

            LCD_ClearScreen();
            LCD_Cursor(1);
            LCD_WriteData(getFirstDigit(hour));
            LCD_Cursor(2);
            LCD_WriteData(getLastDigit(hour));
            LCD_Cursor(3);
            LCD_WriteData(':');
            LCD_Cursor(4);
            LCD_WriteData(getFirstDigit(min));
            LCD_Cursor(5);
            LCD_WriteData(getLastDigit(min));
            LCD_Cursor(6);
            LCD_WriteData(':');
            LCD_Cursor(7);
            LCD_WriteData(getFirstDigit(second));
            LCD_Cursor(8);
            LCD_WriteData(getLastDigit(second));
            LCD_Cursor(9);
            LCD_WriteData(' ');
            LCD_Cursor(0);


            break;

        case o_wait:
            break;
        default:

            break;
    }
    return onstate;
}

enum istates{i_init, i_wait, i_noin, i_0, i_1, i_2, i_noin1, i_noin2}istate;

int i_tick(istate){
    unsigned char instate, tmpB = ~PINB;
    static unsigned char prevB, index, th, tm, ts, bcnt;
    joyIn ji = getJoystick();


    switch(istate){
        case i_init:
            instate = i_wait;
            break;
        case i_wait:
            if(alarmWait->num_objects > 0){
                if(bcnt < 20){
                    bcnt++;
                }
                else{
                    QueueMakeEmpty(alarmWait);
                    bcnt=0;
                }
                instate = i_wait;

            }
            else if(tmpB != 0){
                prevB = tmpB;
                instate = i_noin;
            }
            else instate = i_wait;
            if(alarm == 1){
                                instate = i_wait;
                            }
            break;
        case i_noin:
            if(tmpB != 0){
                instate = i_noin;
            }
            else {
                if(prevB == 0x01){
                    instate = i_0;
                }
                else if(prevB == 0x02){
                    LCD_ClearScreen();
                    instate = i_1;
                    th = hour;
                    tm = min;
                    ts = second;
                }
                else{
                	instate = i_2;
                	index = 0;
                }
                if(alarm == 1){
                    instate = i_wait;
                }
            }
            break;
        case i_0:
            instate = i_wait;
            break;
        case i_1:
            display = 0;

            if(tmpB == 0x02){
				instate = i_noin2;
				display =1;
				hour = th;
				min = tm;
				second = ts;
			}
			else {
				instate = i_1;

			}


            break;

        case i_2:
            display = 0;
            if(tmpB == 0x04){
            	instate = i_noin2;
            	display =1;
            }
            else instate = i_2;

            break;
        case i_noin2:
        	if(tmpB != 0){
        		instate = i_noin2;
        	}
        	else instate = i_wait;
        	break;

        default:
            instate = i_init;
            break;
    }

    switch(istate){
        case i_init:
            bcnt = 0;
            index = 0;
            break;
        case i_wait:

            break;
        case i_noin:

            break;
        case i_0:

            eeprom_write_byte((uint8_t)56, 0x01);
            eeprom_write_byte((uint8_t)8, hour);
            eeprom_write_byte((uint8_t)16,min);
            eeprom_write_byte((uint8_t)24,second);
            eeprom_write_byte((uint8_t)32, ahour);
            eeprom_write_byte((uint8_t)40, amin);
            eeprom_write_byte((uint8_t)48, asecond);
            break;
        case i_1:


        	if(index < 2 && ji != none && ji == right){
        	    index++;
        	}
			else if(index > 0 && ji != none && ji == left){
				index--;
			}

			if(ji == up ){
				switch(index){
					case 0:

						th++;
						if(th >= 13){
							th = 1;
						}
						break;
					case 1:
						tm++;
						if(tm >= 60){
							tm = 0;
						}
						break;
					case 2:
						ts++;
						if(ts >= 60){
							ts = 0;
						}
						break;
					default:
						break;
				}
			}
			else if( ji == down ){
						switch(index){
							case 0:

								th--;
								if(th < 1){
									th = 11;
								}
								break;
							case 1:
								tm--;
								if(tm == 0){
									tm = 59;
								}
								break;
							case 2:
								ts--;
								if(ts == 0){
									ts = 59;
								}
								break;
							default:
								break;
						}

			}

            LCD_Cursor(1);
            LCD_WriteData(getFirstDigit(th));
            LCD_Cursor(2);
            LCD_WriteData(getLastDigit(th));
            LCD_Cursor(3);
            LCD_WriteData(':');
            LCD_Cursor(4);
            LCD_WriteData(getFirstDigit(tm));
            LCD_Cursor(5);
            LCD_WriteData(getLastDigit(tm));
            LCD_Cursor(6);
            LCD_WriteData(':');
            LCD_Cursor(7);
            LCD_WriteData(getFirstDigit(ts));
            LCD_Cursor(8);
            LCD_WriteData(getLastDigit(ts));
            LCD_Cursor(0);
            break;
        case i_2:
        	if(index < 2 && ji != none && ji == right){
        		index++;
        	}
        	else if(index > 0 && ji != none && ji == left){
        		index--;
        	}

            if(ji == up){
                switch(index){
                    case 0:

                        ahour++;
                        if(ahour >= 13){
                            ahour = 1;
                        }
                        break;
                    case 1:
                        amin++;
                        if(amin >= 60){
                            amin = 0;
                        }
                        break;
                    case 2:
                        asecond++;
                        if(asecond >= 60){
                            asecond = 0;
                        }
                        break;
                    default:
                        break;
                }
            }
            else if( ji == down){
						switch(index){
							case 0:

								ahour--;
								if(ahour < 1){
									ahour = 11;
								}
								break;
							case 1:
								amin--;
								if(amin == 0){
									amin = 59;
								}
								break;
							case 2:
								asecond--;
								if(asecond == 0){
									asecond = 59;
								}
								break;
							default:
								break;
						}

            }

            LCD_Cursor(1);
            LCD_WriteData(getFirstDigit(ahour));
            LCD_Cursor(2);
            LCD_WriteData(getLastDigit(ahour));
            LCD_Cursor(3);
            LCD_WriteData(':');
            LCD_Cursor(4);
            LCD_WriteData(getFirstDigit(amin));
            LCD_Cursor(5);
            LCD_WriteData(getLastDigit(amin));
            LCD_Cursor(6);
            LCD_WriteData(':');
            LCD_Cursor(7);
            LCD_WriteData(getFirstDigit(asecond));
            LCD_Cursor(8);
            LCD_WriteData(getLastDigit(asecond));
            LCD_Cursor(0);
            break;

        default:
            instate = i_init;
            break;
    }

    return instate;
}

enum astates{a_init, a_wait, a_alert, a_noin}astate;

int a_tick(int astate){
	static unsigned char count = 0;
    unsigned char anstate, atmpB = ~PINB, temp;
    static unsigned char aprevB;
    unsigned char Char1[8] = { 0x01, 0x03, 0x07, 0x1F, 0x1F, 0x07, 0x03, 0x01 };

    switch(astate){
        case a_init:
            anstate = a_wait;
            break;
        case a_wait:
        	count = 0;
            if(hour == ahour && min == amin && second == asecond){
                anstate = a_alert;
                alarm = 1;
                display = 0;
                LCD_ClearScreen();
            }
            else anstate = a_wait;
            break;
        case a_alert:
           anstate = a_alert;
           if(atmpB == 0x02){
               anstate = a_wait;
               alarm = 0;
               display = 1;
               //istate = i_init;
               QueueEnqueue(alarmWait, 5);
               LCD_ClearScreen();
           }
           else if(atmpB == 0x04){
               anstate = a_wait;
               alarm = 0;
               display =1;
               if(asecond < 40){
            	   asecond+= 20;
               }
               else {
            	   temp = 60 - asecond;
            	   amin++;
            	   asecond = temp;
               }
               QueueEnqueue(alarmWait, 5);
               LCD_ClearScreen();

            }
            break;

        default:
            anstate = a_init;
            break;
    }

    switch(astate){
        case a_init:
            alarm = 0;
            break;
        case a_wait:

            break;
        case a_alert:
            //LCD_ClearScreen();
        	//alarm = 1;
        	if(count < 10){
             LCD_Custom_Char(1,Char1);
        	}
        	count++;




             //LCD_DisplayString(16,"Wake up!");
            break;
        case a_noin:
            break;
        default:

            break;
    }

    return anstate;
}

int main(int argc, char** argv) {
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0x00; PORTB = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    LCD_init();
    ADC_INIT();
    static task task1,task2,task3,task4,task5,task6;
    task *tasks[] = {&task1,&task2,&task3,&task4,&task5,&task6};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    alarmWait = QueueInit(1);
    if( eeprom_read_byte((uint8_t)56) == 0x01){


        hour = eeprom_read_byte((uint8_t)8);
        min = eeprom_read_byte((uint8_t)16);
        second = eeprom_read_byte((uint8_t)24);
        ahour = eeprom_read_byte((uint8_t)32);
        amin = eeprom_read_byte((uint8_t)40);
        asecond = eeprom_read_byte((uint8_t)48);
    }
    else{
        ahour = hour = 1;
        amin = min = 0;
        second = 0;
        asecond = 15;
    }


    task1.state = s_init;
    task1.period = 1000;
    task1.elapsedTime = task1.period;
    task1.TickFct = &s_tick;

    task2.state = m_init;
    task2.period = 1000;
    task2.elapsedTime = task2.period;
    task2.TickFct = &m_tick;

    task3.state = h_init;
    task3.period = 1000;
    task3.elapsedTime = task3.period;
    task3.TickFct = &h_tick;

    task4.state = o_init;
    task4.period = 1000;
    task4.elapsedTime = task4.period;
    task4.TickFct = &o_tick;

    task5.state = i_init;
    task5.period = 300;
    task5.elapsedTime = task5.period;
    task5.TickFct = &i_tick;

    task6.state = a_init;
    task6.period = 100;
    task6.elapsedTime = task6.period;
    task6.TickFct = &a_tick;


    for(int i = 0; i < tasksNum; i++){
        tasksPeriodGCD = findGCD(tasksPeriodGCD, tasks[i]->period);

    }
    TimerSet(tasksPeriodGCD);
    TimerOn();

    while(1){
        for(int i = 0; i < numTasks; i++){
            if(tasks[i]->elapsedTime == tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime= 0;
            }
            tasks[i]->elapsedTime += tasksPeriodGCD;

        }

        while(!TimerFlag);
        TimerFlag = 0;
    }
    return (EXIT_SUCCESS);

}

