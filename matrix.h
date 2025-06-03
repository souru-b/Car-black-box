#ifndef MATRIX_H
#define MATRIX_H
	
#define MAX_ROW				4
#define MAX_COL				3

#define STATE_CHANGE				1
#define LEVEL_CHANGE				0
#define MATRIX_KEYPAD_PORT			PORTB
#define ROW3					PORTBbits.RB7
#define ROW2					PORTBbits.RB6
#define ROW1					PORTBbits.RB5
#define COL4					PORTBbits.RB4
#define COL3					PORTBbits.RB3
#define COL2					PORTBbits.RB2
#define COL1					PORTBbits.RB1


#define MK_SW1					1
#define MK_SW2					2
#define MK_SW3					3
#define MK_SW4					4
#define MK_SW5					5
#define MK_SW6					6
#define MK_SW7					7
#define MK_SW8					8
#define MK_SW9					9
#define MK_SW10				10
#define MK_SW11				11
#define MK_SW12				12

#define ALL_RELEASED	0xFF

#define HI				1
#define LO				0



unsigned char scan_key1(void)       // For matrix keypad switch1
{
   RB5=0;
   RB6=RB7=1;
   if(RB1==0)
   {
       return 1;
   }
   if(RB2==0)
   {
       return 4;
   }
   return 0xFF;
}

unsigned char read_switches1()       // Edge triggering the Switch1
{
   static unsigned char once=1;
   unsigned char key;
       key=scan_key1();
       if((key!=0xFF)&&once)
       {
           once=0;
           return key;
       }
       else if(key==0xFF)
       {
           once=1;
       }
    return 0xFF;
}

unsigned char scan_key2(void)       // For matrix keypad switch2
{
   RB6=0;
   RB5=RB7=1;
   if(RB1==0)
   {
       return 2;
   }
   if(RB4==0)
   {
       return 11;
   }
   return 0xFF;
}

unsigned char read_switches2()       // Edge triggering the Switch2
{
   //unsigned char key;
   static unsigned char once=1;
   unsigned char key;
       key=scan_key2();
       if((key!=0xFF)&&once)
       {
           once=0;
           return key;
       }
       else if(key==0xFF)
       {
           once=1;
       }
   return 0xFF;
}

unsigned char scan_key3(void)       // For matrix keypad switch2
{
   RB7=0;
   RB5=RB6=1;
   if(RB1==0)
   {
       return 3;
   }
   if(RB4==0)
   {
       return 12;
   }
   return 0xFF;
}

unsigned char read_switches3()       // Edge triggering the Switch2
{
   //unsigned char key;
   static unsigned char once=1;
   unsigned char key;
       key=scan_key3();
       if((key!=0xFF)&&once)
       {
           once=0;
           return key;
       }
       else if(key==0xFF)
       {
           once=1;
       }
   return 0xFF;
}

unsigned char scan_key(void)
{
	ROW1 = LO;
	ROW2 = HI;
	ROW3 = HI;

	if (COL1 == LO)
	{
		return 1;
	}
	else if (COL2 == LO)
	{
		return 4;
	}
	else if (COL3 == LO)
	{
		return 7;
	}
	else if (COL4 == LO)
	{
		return 10;
	}

	ROW1 = HI;
	ROW2 = LO;
	ROW3 = HI;

	if (COL1 == LO)
	{
		return 2;
	}
	else if (COL2 == LO)
	{
		return 5;
	}
	else if (COL3 == LO)
	{
		return 8;
	}
	else if (COL4 == LO)
	{
		return 11;
	}

	ROW1 = HI;
	ROW2 = HI;
	ROW3 = LO;
	/* TODO: Why more than 2 times? */
	ROW3 = LO;

	if (COL1 == LO)
	{
		return 3;
	}
	else if (COL2 == LO)
	{
		return 6;
	}
	else if (COL3 == LO)
	{
		return 9;
	}
	else if (COL4 == LO)
	{
		return 12;
	}

	return 0xFF;
}

unsigned char read_switches(unsigned char detection_type)
{
	static unsigned char once = 1, key;

	if (detection_type == STATE_CHANGE)
	{
		key = scan_key();
		if(key != 0xFF && once  )
		{
			once = 0;
			return key;
		}
		else if(key == 0xFF)
		{
			once = 1;
		}
	}
	else if (detection_type == LEVEL_CHANGE)
	{
		return scan_key();
	}

	return 0xFF;
}


#endif