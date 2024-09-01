#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "display.h"

// Want to avoid global variables, but no other
// choice for SIGWINCH handler.
struct resize_event *pproc_resize_callback; 

static void enable_alt_buffer(int fd)
{
	write(fd, "\e[?1049h", 8);
}

static void disable_alt_buffer(int fd)
{
	write(fd, "\e[?1049l", 8);
}

static void hide_cursor(int fd)
{
	write(fd,"\e[?25l", 6);
}

static void show_cursor(int fd)
{
	write(fd,"\e[?25h", 6);
}

static void clear_line(int fd)
{
	write(fd, "\e[2K", 4);
}

static void clear_screen(int fd)
{
	write(fd,"\e[2J",4);
}

static void disable_raw(struct termios *termset_orig, int fd)
{
	tcsetattr(fd, TCSAFLUSH, termset_orig);
}

static void enable_raw(struct termios *termset_orig, int fd)
{
	struct termios termset;

	tcgetattr(fd, &termset);

	memcpy(termset_orig, &termset, sizeof(struct termios));

	termset.c_lflag &= ~( ECHO | ICANON | IEXTEN);
	termset.c_oflag &= ~(OPOST);
	termset.c_iflag &= ~(ICRNL | IXON);

	tcsetattr(fd, TCSAFLUSH, &termset);
}

static void allocate_buffers(struct tty_display *ttyd)
{
	ttyd->buffer = (char *)malloc(ttyd->height*ttyd->width);
	ttyd->line_len = (int *)malloc(ttyd->height*sizeof(int));
	memset(ttyd->buffer, 0, ttyd->height*ttyd->width);
	memset(ttyd->line_len, 0, ttyd->height*sizeof(int));
}

static void tty_signal_handler(int sig)
{
	if(sig == SIGWINCH){
		struct tty_display *ttyd = (struct tty_display *)((char *)(pproc_resize_callback->dis)
			- offsetof(struct tty_display, super));	

		struct winsize ws;
		ioctl(ttyd->fd, TIOCGWINSZ, &ws);
		ttyd->height = ws.ws_row;
		ttyd->width = ws.ws_col;

		signal(SIGWINCH, tty_signal_handler);

		clear_screen(ttyd->fd);

		free(ttyd->buffer);
		free(ttyd->line_len);
		allocate_buffers(ttyd);	

		pproc_resize_callback->resize(pproc_resize_callback);	
	}
} 

static void tty_get_size(struct display *super, int *width, int *height)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	

	*height = ttyd->height;
	*width = ttyd->width;
}

static int tty_open_display(struct display *super, int *width, int *height)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	

	struct winsize ws;
	ioctl(ttyd->fd, TIOCGWINSZ, &ws);
	ttyd->height = ws.ws_row;
	ttyd->width = ws.ws_col;
	*height = ws.ws_row;
	*width = ws.ws_col;
	allocate_buffers(ttyd);	

	enable_raw(&(ttyd->termset_orig), ttyd->fd);
	enable_alt_buffer(ttyd->fd);
	clear_screen(ttyd->fd);	
	
	if(ttyd->super.resize_callback){
		pproc_resize_callback = ttyd->super.resize_callback;
		signal(SIGWINCH, tty_signal_handler);
	}

	return 0;
}

static void tty_close_display(struct display *super)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	free(ttyd->buffer);
	disable_raw(&ttyd->termset_orig, ttyd->fd);
	disable_alt_buffer(ttyd->fd);
}

static void tty_set_cursor(struct display *super, int row, int col)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	char cs[32];
	ttyd->cursor_row = row;
	ttyd->cursor_col = col;
	int l = sprintf(cs,"\e[%d;%dH", row+1, col+1);
	write(ttyd->fd, cs, l);
}

static void tty_put_line(struct display *super, char *line, int row)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	for(int i = 0; i < ttyd->width; i++){	
		if(line[i] == '\0'){
			ttyd->line_len[row] = i + 1;
			break;
		}
		ttyd->buffer[i + row * ttyd->width] = line[i];
	}
}

static void tty_display_line(struct display* super, int row)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	int cr = ttyd->cursor_row;
	int cc = ttyd->cursor_col;
	hide_cursor(ttyd->fd);
	super->set_cursor(super, row, 0);
	clear_line(ttyd->fd);
	write(ttyd->fd, ttyd->buffer + (row * ttyd->width), ttyd->line_len[row]); 
	super->set_cursor(super, cr, cc);
	show_cursor(ttyd->fd);
}

struct display *make_tty_display(struct tty_display *ttyd)
{
	ttyd->super.open_display = tty_open_display;
	ttyd->super.close_display = tty_close_display;
	ttyd->super.set_cursor = tty_set_cursor;
	ttyd->super.put_line = tty_put_line;
	ttyd->super.display_line = tty_display_line;
	ttyd->super.get_size = tty_get_size;
	return &(ttyd->super);
}
