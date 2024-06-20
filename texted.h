#include <termios.h>
#include <unistd.h>

#ifndef TEXTED_H
#define TEXTED_H

#define RBUFFER_SIZE 256

extern char to_debug;

struct line_item{
	struct line_item *next, *prev;
	char flag; // used to store size of text, and if line is full or not.
	char *text;	
};

struct line_list{
	struct line_item *head, *end;
	int width;
	int lines;
};

int create_line_list(char*, struct line_list*);

void proccess_rbuffer(char *, int, struct line_list*);

void merge_lists(struct line_list*, struct line_list*);

struct line_item* new_line(int);

void add_line(struct line_list*, struct line_item*, int);

void remove_line(struct line_list*, int);

void traverse_list(struct line_list*, int (*callback)(struct line_item*, long), long);

void read_envs();

void cleanup(int);

/* DISPLAY FUNCTIONS */

#define HIDE_CURSOR() write(STDOUT_FILENO,"\e[?25l",6)
#define SHOW_CURSOR() write(STDOUT_FILENO,"\e[?25h",6)
#define CLEAR_SCREEN() write(STDOUT_FILENO,"\e[2J",4)
#define ALT_BUFFER() write(STDOUT_FILENO,"\e[?1049h",8);

extern struct termios termset_orig;

void disable_raw();

void enable_raw();

// event loop, read from input and update data; return after that.
void after_display();

void clean_screen();

void move_cursor(int, int);

void write_line(char*, int);

#endif /* TEXTED_H */


