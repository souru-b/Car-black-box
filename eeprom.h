#ifndef EEPROM_H
#define EEPROM_H

unsigned char read(unsigned char adr)
{
    EEADR=adr;
    EEPGD=0;
    CFGS=0;
    WREN=0;
    RD=1;
    return EEDATA;
}

void write(unsigned char adr,unsigned char data)
{
    EEADR=adr;
    EEDATA=data;
    EEPGD=0;
    CFGS=0;
    WREN=1;
    GIE=0;
    EECON2=0X55;
    EECON2=0XAA;
    WR=1;
    GIE=1;
    while(WR);
}

#endif