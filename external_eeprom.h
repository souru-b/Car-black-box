#ifndef external_eeprom_h
#define external_eeprom_h

#define SLAVE_READ_E		0xA1
#define SLAVE_WRITE_E	0xA0

void write_external_eeprom(unsigned char address1,  unsigned char data);
unsigned char read_external_eeprom(unsigned char address1);
void init_ds1307();

#endif