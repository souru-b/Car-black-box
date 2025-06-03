/*
 * File:   main.c
 * Author: Sourabh bahubali baragale
 */


#include <xc.h>
#include <string.h>
#include "main.h"
#include "clcd.h"
#include "matrix.h"
#include "adc.h"
#include "external_eeprom.h"
#include "i2c.h"    //first we need to communicate then connect to RTC(ds1307)
#include "ds1307.h"
#include "uart.h"

#define _XTAL_FREQ 20000000 //Non_blocking delay
#define CHANNEL4 0x04   //address for ADC

unsigned char add=0x00;     //add var is used to store event/gear in external EEPROM
unsigned char event_count=0;    //gear_count is used to count the no. of events changed
unsigned char addr=0x00;    //addr is used to read the stored events from external EEPROM to string
unsigned char str[10][15];  //string used store the data read
unsigned char count=0;      //count is index used to change the event/gear
unsigned char state;        //state is enum used select the specific operation
unsigned char speed[3];     //speed is used to store the speed of car(ADC)
unsigned short sp=00;   //sp is speed in integer type
unsigned char clock_reg[3];
unsigned char time[9];      //used to store the time in string type
unsigned char gear[9][3]={"ON","GN","G1","G2","G3","G4","G5","GR","C_"};    //array of event/gear
unsigned char key;      //key to store the return value of matrix keypad
unsigned char flag=0;   //used for event change
unsigned char opt_count=0;   //index for menu
unsigned char read_ev=1;    //it acts as a flag for storing events or not
unsigned char ec=0;     //index for view log
unsigned char opt[4][15]={"VIEW LOG","CLEAR LOG","DOWNLOAD LOG","SET TIME"};    //array for menu
unsigned char set_once=1,clear_once=1,once=1,d_once=1;      //flag to do a specific task once
unsigned char field_select=0;   //For setting/updating the time
unsigned char l=0,t1=1,wait=0;

void config()
{
    TRISB=0x1E;         // Configure PORTB pins with RB1-RB4 as inputs (0001 1110)
    RBPU=0;             // Enable pull-up resistors on PORTB
    init_adc();         // Initialize Analog-to-Digital Converter
    init_clcd();        // Initialize Character LCD display
    init_i2c();         // Initialize I2C communication bus
    init_ds1307();      // Initialize DS1307 Real-Time Clock
    init_uart();        // Initialize UART communication
}

void store_event()
{
    for(unsigned int i=0;i<8;i++)
    {
        write_external_eeprom(add++,time[i]);    // Store each character of time string to EEPROM
    }
    write_external_eeprom(add++,gear[count][0]); // Store first character of gear state
    write_external_eeprom(add++,gear[count][1]); // Store second character of gear state
    write_external_eeprom(add++,speed[0]);       // Store tens digit of speed
    write_external_eeprom(add++,speed[1]);       // Store units digit of speed
    
    event_count++;                              // Increment event counter
}

void dashboard(void)
{
    static unsigned char key,event;
    static unsigned char flag=0,once=1;
    
    clcd_print("TIME",LINE1(2));                // Display "TIME" label on first line
    clcd_print("EV",LINE1(10));                 // Display "EV" (event) label
    clcd_print("SP",LINE1(14));                 // Display "SP" (speed) label
    clcd_print(gear[count],LINE2(10));          // Display current gear state
    speed[0]='0'+(sp/10);                       // Convert speed tens digit to ASCII
    speed[1]='0'+(sp%10);                       // Convert speed units digit to ASCII
    speed[2]='\0';                              // Null-terminate speed string
    clcd_print(speed,LINE2(14));                // Display current speed
    clcd_print(time,LINE2(0));                  // Display current time
    
    read_ev=1;                                  // Set flag to read events when needed
    
    if(once)
    {
        store_event();                          // Store initial event on first run
        once=0;                                 // Clear first-run flag
    }
    event=read_switches(1);                     // Read user input
    if(event==1)                                // If UP button pressed
    {
        if(flag)
        {
            flag=0;                             // Clear cruise control flag
            count=0;                            // Reset to first gear
        }
        if(count<7)
            count++;                            // Increment gear if not at maximum
        store_event();                          // Store gear change event
    }
    else if(event==2)                           // If DOWN button pressed
    {
        if(flag)
        {
            flag=0;                             // Clear cruise control flag
            count=1;                            // Set to first gear
        }
        if(count>1)
            count--;                            // Decrement gear if not at minimum
        store_event();                          // Store gear change event
    }
    else if(event==3)                           // If CRUISE button pressed
    {
        flag=1;                                 // Set cruise control flag
        count=8;                                // Set to cruise mode (C_)
        store_event();                          // Store cruise control event
    }
    else if(event==11)                          // If MENU button pressed
    {
        clear_once=1;                           // Set flag to clear display
        state=e_menu;                           // Change state to menu
    }
    clcd_print(gear[count],LINE2(10));          // Update gear display
    sp=read_adc(CHANNEL4)/10.23;                // Read speed from ADC and convert to 0-99 range
    if(sp>=99)
        sp=99;                                  // Limit speed to maximum of 99
}

static void get_time(void)
{
	clock_reg[0] = read_ds1307(HOUR_ADDR);     // Read hours from RTC
	clock_reg[1] = read_ds1307(MIN_ADDR);      // Read minutes from RTC
	clock_reg[2] = read_ds1307(SEC_ADDR);      // Read seconds from RTC

	if (clock_reg[0] & 0x40)                   // Check if in 12-hour mode (bit 6 set)
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);  // Convert BCD hours tens digit to ASCII
		time[1] = '0' + (clock_reg[0] & 0x0F);         // Convert BCD hours units digit to ASCII
	}
	else                                        // In 24-hour mode
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);  // Convert BCD hours tens digit to ASCII
		time[1] = '0' + (clock_reg[0] & 0x0F);         // Convert BCD hours units digit to ASCII
	}
	time[2] = ':';                              // Add colon separator
	time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);  // Convert BCD minutes tens digit to ASCII
	time[4] = '0' + (clock_reg[1] & 0x0F);         // Convert BCD minutes units digit to ASCII
	time[5] = ':';                              // Add colon separator
	time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);  // Convert BCD seconds tens digit to ASCII
	time[7] = '0' + (clock_reg[2] & 0x0F);         // Convert BCD seconds units digit to ASCII
	time[8] = '\0';                             // Null-terminate time string
}

void read_event(void)
{
    unsigned char addr=0x00;                    // Start from beginning of EEPROM
    for(unsigned int j=0;j<event_count;j++)     // For each stored event
    {
        for(unsigned char k=0;k<15;k++)         // For each character in event string
        {
            if(k==8 || k==11)                   // Add spaces after time and gear
                str[j][k]=' ';
            else if(k==14)                      // Add null terminator at the end
                str[j][k]='\0';
            else                                // Read data from EEPROM
                str[j][k]=read_external_eeprom(addr++);
        }
    }
    if(event_count>9)
    {
        l=event_count-10;
    }
    else
        l=0;
}

void menu(void)
{
    static unsigned char ev=0,j=0;             // ev tracks menu position, j tracks cursor line
    if(j){                                      // If cursor on second line
        clcd_putch('*',LINE2(0));               // Show cursor on second line
        clcd_putch(' ',LINE1(0));               // Clear cursor from first line
    }
    else{                                       // If cursor on first line
        clcd_putch('*',LINE1(0));               // Show cursor on first line
        clcd_putch(' ',LINE2(0));               // Clear cursor from second line
    }
    clcd_print(opt[ev],LINE1(1));               // Display first menu option
    clcd_print(opt[ev+1],LINE2(1));             // Display second menu option
    if(key==2)                                  // If DOWN button pressed
    {
        if(j==0)                                // If cursor on first line
        {
            clcd_putch(' ',LINE1(0));           // Clear cursor from first line
            j=1;                                // Move cursor to second line
        }
        else                                    // If cursor on second line
        {
            if(ev<2){                           // If not at end of menu
                ev++;                           // Move to next pair of options
                clear_once=1;                   // Flag to clear display
            }
        }
        if(opt_count<3)                         // If not at last option
            opt_count++;                        // Increment option counter
    }
    if(key==1)                                  // If UP button pressed
    {
        if(j==1)                                // If cursor on second line
        {
            clcd_putch(' ',LINE2(0));           // Clear cursor from second line
            j=0;                                // Move cursor to first line
        }
        else                                    // If cursor on first line
        {
            if(ev>0){                           // If not at start of menu
                ev--;                           // Move to previous pair of options
                clear_once=1;                   // Flag to clear display
            }
        }
        if(opt_count>0)                         // If not at first option
            opt_count--;                        // Decrement option counter
    }
    if(key==12)                                 // If BACK button pressed
    {
        opt_count=0;
        ev=0,j=0;                              // Reset menu position
        clear_once=1;                          // Flag to clear display
        state=e_dashboard;                     // Return to dashboard state
    }
    else if(key==11)                           // If ENTER button pressed
    {
        ev=0,j=0;                              // Reset menu position
        clear_once=1;                          // Flag to clear display
        if(opt_count==0)                       // If VIEW LOG selected
            state=e_view;
        else if(opt_count==1)                  // If CLEAR LOG selected
            state=e_clear;
        else if(opt_count==2)                  // If DOWNLOAD LOG selected
            state=e_download;
        else                                   // If SET TIME selected
            state=e_set;
        opt_count=0;
    }
}

void view_log()
{
    if(read_ev)                               // If events need to be read
    {
        read_event();                         // Read events from EEPROM
        read_ev=0;                           // Clear read flag

    }
    clcd_putch((ec%10 + '0'),LINE2(0));      // Display current event index    
    clcd_print(str[l],LINE2(2));            // Display current event data
    if(key==1)                               // If UP button pressed
    {
        if(l<event_count-1){                   // If not at last event
            l++;                            // Move to next event
            ec++;
        }
    }
    if(key==2)                               // If DOWN button pressed
    {
        if(l>(event_count-10)){                             // If not at first event
            l--;                            // Move to previous event
            ec--;
        }
    }
    if(key==12)                              // If BACK button pressed
    {
        read_ev=1;
        ec=0;
        clear_once=1;                        // Flag to clear display
        state=e_menu;                        // Return to menu state
    }
    
}

void clear_log()
{
    if(event_count==0)
    {
        clcd_print("NO LOG FOUND  ",LINE1(0));
        __delay_ms(1000);                         // Wait 700ms
        clear_once=1;                           // Flag to clear display
        state=e_menu;                           // Return to menu state
    }
    
    add=addr=0x00;                           // Reset EEPROM address pointers
    event_count=0;                      // Reset event counter
    count=1;                     // Reset event gear position
    clcd_print("CLEARING LOG...",LINE1(0));  // Display clearing message
    __delay_ms(1000);                         // Wait 1000ms
    clcd_print("LOG CLEARED     ",LINE1(0)); // Display cleared message
    __delay_ms(1000);                         // Wait 1000ms
    clear_once=1;                           // Flag to clear display
    state=e_menu;                           // Return to menu state
}

void download_log()
{
    if(d_once)                               // If first time entering download
    {
        read_event();                        // Read all events from EEPROM
        d_once=0;                           // Clear first-time flag
        puts("IN   TIME   EV SP");
        puts("\n\r");
    }
    if(l<event_count)                        // If not all events downloaded
    {
        ec++;
        putch(ec/10 + '0');
        putch(ec%10 + '0');
        putch(' ');
        puts(str[l]);                        // Send event string via UART
        puts("\n\r");                        // Send newline and carriage return
        l++;                                 // Move to next event
        clcd_print("LOG DOWNLOADING...",LINE1(0));   // Display download message
    }
    else
    {
        ec=0;
        clcd_print("LOG DOWNLOADED    ",LINE1(0));   // Display download message
        __delay_ms(1000);                         // Wait 1000ms
        state=e_menu;
        clear_once=1;
        puts("\n\r");
    }
}

void set_time()
{
    unsigned char h[3],m[3],s[3],hour,min,sec,key;
    clcd_print("HOUR",LINE1(0));            // Display HOUR label
    clcd_print("MIN",LINE1(5));             // Display MIN label
    clcd_print("SEC",LINE1(9));             // Display SEC label
    
    if(set_once)                            // If first time entering set time
    {
        h[0]=time[0];                      // Initialize hours tens digit from current time
        h[1]=time[1];                      // Initialize hours units digit from current time
        h[2]='\0';                         // Null-terminate hours string

        m[0]=time[3];                      // Initialize minutes tens digit from current time
        m[1]=time[4];                      // Initialize minutes units digit from current time
        m[2]='\0';                         // Null-terminate minutes string

        s[0]=time[6];                      // Initialize seconds tens digit from current time
        s[1]=time[7];                      // Initialize seconds units digit from current time
        s[2]='\0';                         // Null-terminate seconds string
        set_once=0;                        // Clear first-time flag
    }
    
    if(t1)
    {
        if(field_select==2)
        {
            clcd_print(h,LINE2(1));                // Display hours
        }
        else if(field_select==1)
        {
            clcd_print(m,LINE2(5));                // Display minutes
        }
        else{
            clcd_print(s,LINE2(9));                // Display seconds
        }
    }
    else{
        if(field_select==2)
        {
            clcd_putch(INPUT,LINE2(1));
            clcd_putch(INPUT,LINE2(2));
        }
        else if(field_select==1)
        {
            clcd_putch(INPUT,LINE2(5));
            clcd_putch(INPUT,LINE2(6));
        }
        else{
            clcd_putch(INPUT,LINE2(9));
            clcd_putch(INPUT,LINE2(10));
        }
    }
    if(wait++==200)
    {
        t1=!t1;
        wait=0;
    }
    
    if(field_select==0)
    {
        clcd_print(h,LINE2(1));                // Display hours
        clcd_print(m,LINE2(5));                // Display minutes
    }
    else if(field_select==1)
    {
        clcd_print(h,LINE2(1));                // Display hours
        clcd_print(s,LINE2(9));                // Display seconds
    }
    else if(field_select==2)
    {
        clcd_print(m,LINE2(5));                // Display minutes
        clcd_print(s,LINE2(9));                // Display seconds
    }
    
    
    hour = ((h[0]-'0')*10) + (h[1]-'0');   // Convert hours string to integer
    min = ((m[0]-'0')*10) + (m[1]-'0');    // Convert minutes string to integer
    sec = ((s[0]-'0')*10) + (s[1]-'0');    // Convert seconds string to integer
    key=read_switches(1);                  // Read user input
    if(key==1)                             // If UP button pressed
    {
        if(field_select==0)                // If seconds field selected
        {
            if(sec<59)
                sec++;                     // Increment seconds if not at max
            else
                sec=0;                     // Wrap around to 0
        }
        if(field_select==1)                // If minutes field selected
        {
            if(min<59)
                min++;                     // Increment minutes if not at max
            else
                min=0;                     // Wrap around to 0
        }
        if(field_select==2)                // If hours field selected
        {
            if(hour<23)
                hour++;                    // Increment hours if not at max
            else
                hour=0;                    // Wrap around to 0
        }
    }
    if(key==2)                             // If DOWN button pressed (used for field selection)
    {
        if(field_select<2)
            field_select++;               // Move to next field
        else
            field_select=0;               // Wrap around to first field
    }
    else if(key==11)                       // If ENTER button pressed
    {
        CLEAR_DISP_SCREEN;                // Clear display
        clcd_print("SAVING TIME...",LINE1(0));  // Display saving message
        __delay_ms(500);                  // Wait 500ms
        
        write_ds1307(HOUR_ADDR,(((hour/10)<<4)|(hour%10)));
        write_ds1307(MIN_ADDR,(((min/10)<<4)|(min%10)));
        write_ds1307(SEC_ADDR,(((sec/10)<<4)|(sec%10)));
        
        clear_once=1;                     // Flag to clear display
        set_once=1;                       // Reset first-time flag for next use
        field_select=0;                   // Reset field selection
        state = e_dashboard;              // Return to dashboard state
    }
    else if(key==12)                       // If BACK button pressed
    {
        set_once=1;                       // Reset first-time flag for next use
        field_select=0;                   // Reset field selection
        clear_once=1;                     // Flag to clear display
        state = e_menu;                   // Return to menu state
    }
    
    // Convert hours,minutes and seconds digit to ASCII
    h[0] = hour/10 + '0';                 
    h[1] = hour%10 + '0';                 
    m[0] = min/10 + '0';                  
    m[1] = min%10 + '0';                  
    s[0] = sec/10 + '0';                  
    s[1] = sec%10 + '0';                  
}

void main(void)
{
    config();                            // Initialize system configuration
    state = e_dashboard;                 // Set initial state to dashboard
    while(1)                             // Main program loop
    {
        if(clear_once){                  // If display needs clearing
            CLEAR_DISP_SCREEN;           // Clear display
            clear_once=0;                // Reset clear flag
        }
        key = read_switches(1);          // Read user input
        
        get_time();                      // Update time from RTC
        
        switch(state)                    // State machine implementation
        {
            case e_dashboard:            // Dashboard state
                dashboard();
                break;
            case e_menu:                 // Menu state
                menu();
                break;
            case e_view:                 // View log state
                if(event_count==0){
                    clcd_print("                ",LINE2(0));
                    clcd_print("NO LOG FOUND    ",LINE1(0));
                    __delay_ms(1000);
                    clear_once=1;
                    state=e_menu;
                }
                else{
                    clcd_putch('I',LINE1(0));            // Display index indicator
                    clcd_print("TIME",LINE1(4));         // Display TIME header
                    clcd_print("EV",LINE1(11));          // Display EV header
                    clcd_print("SP",LINE1(14));          // Display SP header
                    view_log();
                }
                break;
            case e_clear:                // Clear log state
                clear_log();
                break;
            case e_download:             // Download log state
                if(event_count==0)
                {
                    puts("NO LOG FOUND");
                    puts("\n\r");
                    clcd_print("NO LOG FOUND  ",LINE1(0));
                    __delay_ms(1000);                         // Wait 1000ms
                    clear_once=1;                           // Flag to clear display
                    state=e_menu;                           // Return to menu state
                }
                else{
                    download_log();
                }
                break;
            case e_set:                  // Set time state
                set_time();
                break;
        }
    }
}