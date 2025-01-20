#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "codepoint.h"

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
	void (*put_line)(struct display *, const codepoint *, int);
	void (*put_str)(struct display *, const display_str *, int);
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
*/

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
	enum color current_text_color;
	// Original windows size to restore scroll window.
	int scrollwin_og[2];
};

struct display *make_tty_display(struct tty_display *);

#endif /* DISPLAY_H */
