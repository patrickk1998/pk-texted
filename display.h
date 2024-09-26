#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>

#ifndef DISPLAY_H
#define DISPLAY_H

/*
	GENERIC DISPLAY INTERFACE
*/

struct resize_event{
	struct display *dis;
	void (*resize)(struct resize_event *);
};

struct display{
	int  (*open_display)(struct display *, int *, int *);
	void (*close_display)(struct display *);
	void (*put_line)(struct display *, const char *, int);
	void (*display_line)(struct display*, int);
	void (*set_cursor)(struct display *, int, int);
	void (*get_size)(struct display *, int *, int *);
	void (*clear_display)(struct display *);
	void (*clear_line)(struct display *, int);
	void (*scroll_up)(struct display *);
	void (*scroll_down)(struct display *);
	void (*set_scroll_window)(struct display *, int, int);
	struct resize_event *resize_callback;
};

/*
	TTY DISPLAY
	assume one character is one column unit wide for now.
*/

#define HIDE_CURSOR() write(STDOUT_FILENO,"\e[?25l",6)
#define SHOW_CURSOR() write(STDOUT_FILENO,"\e[?25h",6)
#define CLEAR_SCREEN() write(STDOUT_FILENO,"\e[2J",4)
#define ALT_BUFFER() write(STDOUT_FILENO,"\e[?1049h",8);

struct tty_display{
	struct termios termset_orig;
	int fd;
	char *buffer;
	int *line_len;
	int width;	
	int height;
	int cursor_row;
	int cursor_col;
	struct display super;
	int scrollwin_og[2];
};

struct display *make_tty_display(struct tty_display *);

#endif /* DISPLAY_H */
