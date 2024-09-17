#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "texted.h"
#include "display.h"
#include "text.h"

void makeState(struct displayState *s, struct text *xt, int h)
{
	s->xt = xt;
	s->rows = h;
	int i = 0;
	s->start = s->xt->get_first_line(s->xt);
	line_id current_id = s->start;
	s->changed = malloc(s->rows);
	while(i < s->rows && current_id){
		s->changed[i] = 1;
		s->end = current_id;
		current_id = s->xt->next_line(s->xt, current_id);
		i++;
	}
	s->cursorRow = 0;
	s->cursorColumn = 0;
}

void renderState(struct displayState *s, struct display *dis)
{
	line_id current_id = s->start;
	int row = 0;
	do{
		if(s->changed[row]){
			dis->put_line(dis, s->xt->get_text(s->xt, current_id), row);	
			dis->display_line(dis, row);
			s->changed[row] = 0;
		}		
		current_id = s->xt->next_line(s->xt, current_id);
		row++;
	}while(current_id != s->end);
	dis->set_cursor(dis, s->cursorRow, s->cursorColumn);
}

