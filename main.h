#ifndef MAIN_H
#define MAIN_H

typedef enum
{
    e_dashboard,e_menu,e_view,e_clear,e_download,e_set
}State_t;

 unsigned char ch;

void config();

void store_event();

void dashboard(void);

static void get_time(void);

void read_event(void);

void menu(void);

void view_log();

void main(void);

void set_time();

void clear_log();

void download_log();

#endif