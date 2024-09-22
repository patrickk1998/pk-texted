#include <termios.h>
#include <unistd.h>
#include "display.h"
#include "input.h"
#include "text.h"

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

struct _rowmap{
	line_id line;
	int index;
	int width;
	int changed;
	// char* text stores tab subsituted text ready to put into the display.
	char *text;  
};

typedef struct _rowmap _rowmap;

// The displayed text area is [start, end) 
struct displayState{
	int cursorRow;
	int cursorCharacter;
	int savedCursorCharacter;
	int csaved;
	int cursorColumn;
	int endRow;
	char *changed;
	struct text *xt;
	int rows;
	int width;
	int viewRows; // Bottom row is command line, so viewRows = rows - 1;
	line_id start;
	line_id end;
	enum _mode mode;
	int changed_control;
	struct _rowmap *row_mapping;	
};

// event loop, read from input and update data; return after that.

void make_state(struct displayState *s, struct text *xt,  int h, int w);

void update_state(struct displayState *s, struct input_action *a);

void render_state(struct displayState *s, struct display *dis);

#endif /* TEXTED_H */


