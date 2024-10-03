#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "display.h"

#define GET_TTYD(super) (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
#define ROW_FACTOR 32

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

// Scroll Down and Up are reversed from xterm documentation.
static void scroll_down(int fd)
{
	write(fd, "\e[1S",4);
}

static void scroll_up(int fd)
{
	write(fd, "\e[1T",4);
}

static void tty_scroll_down(struct display *super)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	scroll_down(ttyd->fd);
}

static void tty_scroll_up(struct display *super)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	scroll_up(ttyd->fd);
}

static void tty_set_scroll_window(struct display *super, int top_row, int bot_row)
{
	struct tty_display *ttyd = GET_TTYD(super);
	char buf[32];
	int l = sprintf(buf,"\e[%d;%dr", top_row, bot_row);
	write(ttyd->fd, buf, l);
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

static char *get_row_buffer(struct tty_display *ttyd, int row)
{
	return ttyd->buffer + ROW_FACTOR*row * ttyd->width;
}

static void allocate_buffers(struct tty_display *ttyd)
{
	ttyd->buffer = (char *)malloc(ROW_FACTOR*ttyd->height*ttyd->width); // multiple of 32 for escape sequences.
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
	ttyd->scrollwin_og[0] = 0;
	ttyd->scrollwin_og[1] = ws.ws_row;
	ttyd->current_text_color = color_default;

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
	super->set_scroll_window(super, ttyd->scrollwin_og[0], ttyd->scrollwin_og[1]); 
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

static int put_rowbuf(char * buffer, const char *str)
{
	int i = 0;
	while(str[i] != '\0'){
		buffer[i] = str[i];
		i++;
	}
	return i;
}

static void tty_put_line(struct display *super, const codepoint *line, int row)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	char *rowbuf = get_row_buffer(ttyd, row);
	memset(rowbuf, 0, ROW_FACTOR*ttyd->width);
	int j = 0; //byte index into row buffer
	for(int i = 0; i < ttyd->width; i++){	
		if(ttyd->current_text_color != line[i].text_color){
			if(line[i].text_color == color_default){
				j += put_rowbuf(rowbuf + j, "\e[39m");
			} else {
				char temp[32];
				sprintf(temp, "\e[38;5;%dm", ((int)line[i].text_color - 1));
				j += put_rowbuf(rowbuf + j, temp);
			}
			ttyd->current_text_color = line[i].text_color;
		}
		
		j += put_next_codepoint(rowbuf + j, &line[i]);
		if(line[i].line_status == end_line){
			ttyd->line_len[row] = j;
			break;
		}
	}
	return;
}

static void tty_display_line(struct display *super, int row)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	int cr = ttyd->cursor_row;
	int cc = ttyd->cursor_col;
	hide_cursor(ttyd->fd);
	super->set_cursor(super, row, 0);
	clear_line(ttyd->fd);
	write(ttyd->fd, get_row_buffer(ttyd, row), ttyd->line_len[row]); 
	super->set_cursor(super, cr, cc);
	show_cursor(ttyd->fd);
}

static void tty_clear_display(struct display *super)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	clear_screen(ttyd->fd);
	memset(ttyd->buffer, 0, ROW_FACTOR*ttyd->height*ttyd->width);
	memset(ttyd->line_len, 0, ttyd->height*sizeof(int));
}

static void tty_clear_row(struct display *super, int row)
{
	struct tty_display *ttyd = (struct tty_display *)((char *)super - offsetof(struct tty_display, super));	
	tty_set_cursor(super, 0, row);	
	clear_line(ttyd->fd);
	tty_set_cursor(super, ttyd->cursor_col, ttyd->cursor_row);	
	memset(get_row_buffer(ttyd, row), 0, ROW_FACTOR*ttyd->width);
	ttyd->line_len[row] = 0;
}

struct display *make_tty_display(struct tty_display *ttyd)
{
	ttyd->super.open_display = tty_open_display;
	ttyd->super.close_display = tty_close_display;
	ttyd->super.set_cursor = tty_set_cursor;
	ttyd->super.put_line = tty_put_line;
	ttyd->super.display_line = tty_display_line;
	ttyd->super.get_size = tty_get_size;
	ttyd->super.clear_display = tty_clear_display;
	ttyd->super.scroll_up = tty_scroll_up;
	ttyd->super.scroll_down = tty_scroll_down;
	ttyd->super.set_scroll_window = tty_set_scroll_window;
	return &(ttyd->super);
}
