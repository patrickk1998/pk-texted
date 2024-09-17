#include <termios.h>
#include <unistd.h>
#include "display.h"
#include "input.h"
#include "text.h"

#ifndef TEXTED_H
#define TEXTED_H




/* HELPER FUNCTIONS */


/* DISPLAY STATE */

 
struct displayState{
	int cursorRow;
	int cursorColumn;
	char *changed;
	struct text *xt;
	int rows;
	line_id start;
	line_id end;
};

// event loop, read from input and update data; return after that.

void makeState(struct displayState *s, struct text *xt,  int h);

//void updateState(struct displayState *s, struct input_action *a)

void renderState(struct displayState *s, struct display *dis);

#endif /* TEXTED_H */


