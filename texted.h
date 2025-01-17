#include <termios.h>
#include <unistd.h>
#include "display.h"
#include "input.h"
#include "text.h"
#include "span.h"

#ifndef TEXTED_H
#define TEXTED_H




/* HELPER FUNCTIONS */


/* DISPLAY STATE */

enum _mode{
	normal_mode,
	insert_mode,
};

struct _command{
	enum {
		no_command,
		move_cursor,
		change_mode,
	} type;
	union {
		char c;
		enum {
			move_up,
			move_down,
			move_left,
			move_right,
		} direction;	
		enum _mode new_mode;
	};
};

typedef struct _command command;

typedef struct _rowmap _rowmap;

// The displayed text area is [start, end) 
struct dstate{
	struct {
		int height;
		int width;
	} size;
	struct {
		int row;
		int offset; // unicode character offset into row
	} cursor;
	enum _mode mode;
};

// event loop, read from input and update data; return after that.

/*
void make_state(struct displayState *s, struct text *xt,  int h, int w);

void update_state(struct displayState *s, struct input_action *a);

void render_state(struct displayState *s, struct display *dis);
*/


/* new span things */

void init_display(struct stext *, struct display *, int h, int w);

#endif /* TEXTED_H */


