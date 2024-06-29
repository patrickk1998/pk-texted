#include <termios.h>
#include <unistd.h>

#ifndef TEXTED_H
#define TEXTED_H

#define RBUFFER_SIZE 256

extern char to_debug;

struct line_item{
	struct line_item *next, *prev;
	char flag; // used to store size of text, and if line is overflows or not.
	char *text;	
	size_t length;
};

struct line_list{
	struct line_item *head, *end;
	int width;
	int lines;
};

/* LIST MANIPULATION */

int create_line_list(char*, struct line_list*);

void proccess_rbuffer(char *, int, struct line_list*);

void merge_lists(struct line_list*, struct line_list*);

void add_line(struct line_list*, struct line_item*);

void add_line_at(struct line_list*, struct line_item*, struct line_item*);

void remove_line(struct line_list*, struct line_item*);

void traverse_list(struct line_list*, int (*callback)(struct line_item*, long), long);

/* ITEM MANIPULATION */

struct line_item* new_line(int);

void free_line(struct line_item*);

void dec_length(struct line_item*);

void inc_length(struct line_item*);

/* HELPER FUNCTIONS */

void read_envs();

void cleanup(int);

/* DISPLAY FUNCTIONS */

#define HIDE_CURSOR() write(STDOUT_FILENO,"\e[?25l",6)
#define SHOW_CURSOR() write(STDOUT_FILENO,"\e[?25h",6)
#define CLEAR_SCREEN() write(STDOUT_FILENO,"\e[2J",4)
#define ALT_BUFFER() write(STDOUT_FILENO,"\e[?1049h",8);

struct displayState{
	int cursorRow;
	int cursorColumn;
	struct line_item* cursorLine;
	struct line_item* displayStart;
	unsigned short winRows;
	unsigned short winColumns;
	struct line_list llist;
};

void get_size(struct displayState*);

extern struct termios termset_orig;

void disable_raw();

void enable_raw();

// event loop, read from input and update data; return after that.

void clean_screen();

void move_cursor(int, int);

void write_line(char*, int);

void display_list(struct displayState*);

/* INPUT FUNCTIONS */

// Seperate keyboard input from user actions.
enum inputAction{
	noop,
	quit,
	up,
	down,
	left,
	right,
	backspace,
};

enum inputAction escape_handle();

enum inputAction get_action();

void update_state(enum inputAction, struct displayState*);

void display_state(struct displayState*);

#endif /* TEXTED_H */


