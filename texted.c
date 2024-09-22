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


/* Functions for Line to row Mappings */

int get_width(char c)
{
	return 1;
}

int	col_of_char(struct text *xt, _rowmap *rowmap, int i, int width)
{
	assert(i <= rowmap->width);
	return i;
}


int get_height(struct text *xt, line_id line, int width){
	int h = 1;
	int i = 0;
	int l = 0;
	const char *text =  xt->get_text(xt, line);
	while(text[i] != '\0'){	
		i++;
		l += get_width(text[i]);
		if( l >= width ){
			h++;
			l = 0;
		}
	}
	return h;
}

int set_rowmap(struct text *xt, _rowmap *rowmap, int width){
	assert(rowmap->index < get_height(xt, rowmap->line, width));
	int j = 0;
	int w = 0;
	const char *text = xt->get_text(xt, rowmap->line);
	for(int i = rowmap->index*width; i < (rowmap->index+1)*width; i++){
		if(text[i] == '\0')
			break;
		rowmap->text[j] = text[i];
		j++;
	}
	rowmap->text[j] = '\0';
	rowmap->width = j;
	return j;
}

int next_rowmap(struct text *xt, _rowmap *old_rowmap, _rowmap *new_rowmap, int width)
{
	new_rowmap->line = xt->get_line(xt, old_rowmap->line);
	if((old_rowmap->index + 1) < get_height(xt, old_rowmap->line, width)){
		new_rowmap->index = new_rowmap->index++;
	} else {
		new_rowmap->line = xt->next_line(xt, new_rowmap->line);
		new_rowmap->index = 0;
		if(new_rowmap->line == NULL){
			return -1;
		}
	}
	set_rowmap(xt, new_rowmap, width);
	return 0;
}

void make_state(struct displayState *s, struct text *xt, int h, int w)
{
	s->xt = xt;
	s->rows = h;
	s->viewRows = s->rows - 1;
	s->width = w;
	s->changed = malloc(s->rows);
	s->mode = normal_mode;

	// allocate row table
	s->row_mapping = (struct _rowmap *)malloc(sizeof(struct _rowmap)*s->viewRows);
	char *tmp = malloc(s->rows*(s->width+1));	
	for(int j = 0; j < s->viewRows; j++){
		s->row_mapping[j].text = (tmp + j*(s->width+1));
		s->row_mapping[j].line = NULL;
	}

	// prime row table
	s->row_mapping[0].line = s->xt->get_first_line(s->xt);
	s->row_mapping[0].index = 0;
	set_rowmap(s->xt, s->row_mapping + 0, s->width);	
	s->row_mapping[0].changed = 1;
	
	// fill row table
	int j = 1;
	while(next_rowmap(xt, s->row_mapping + j - 1, s->row_mapping + j, s->width) != -1){
		s->row_mapping[j].changed = 1;
		j++;
		if(j == s->viewRows)
			break;
	} 

	s->cursorRow = 0;
	s->cursorColumn = 0;

	s->changed_control = 1;
}

command procin_normal(struct displayState *s, struct input_action *a)
{
	command c = { .type = no_command };
	switch(a->type){
		case up:
			c.type = move_cursor;
			c.direction = move_up;
			break;
		case down:
			c.type  = move_cursor;
			c.direction = move_down;
			break;
		case left:
			c.type = move_cursor;
			c.direction = move_left;
			break;
		case right:
			c.type = move_cursor;
			c.direction = move_right;
			break;
		case insert:
			if(a->value == 'i'){
				c.type = change_mode;
				c.new_mode = insert_mode;
			}
			if(a->value == 'j'){
				c.type = move_cursor;
				c.direction = move_down;
			} 	
			if(a->value == 'k'){
				c.type = move_cursor;
				c.direction = move_up;
			} 
			if(a->value == 'h'){
				c.type = move_cursor;
				c.direction = move_left;
			}
			if(a->value == 'l'){
				c.type = move_cursor;
				c.direction = move_right;
			}
			break;
	}
	return c;
}

/* Proccess Input for Different Modes and Return an Command */

command procin_insert(struct displayState *s, struct input_action *a)
{
	command c;
	switch(a->type){
		case up:
			c.type = move_cursor;
			c.direction = move_up;
			break;
		case down:
			c.type  = move_cursor;
			c.direction = move_down;
			break;
		case escape:
			c.type = change_mode;	
			c.new_mode = normal_mode;
			break;
		case left:
			c.type = move_cursor;
			c.direction = move_left;
			break;
		case right:
			c.type = move_cursor;
			c.direction = move_right;
			break;
	}
	return c;
}

command proccess_input(struct displayState *s, struct input_action *a)
{
	switch(s->mode){
		case normal_mode:
			return procin_normal(s, a);
			break;
		case insert_mode:
			return procin_insert(s, a);
			break;
	}	
	command c = { .type = no_command };
	return c;
}

/* Update State Based on Command */

void update_cursor(struct displayState *s, command *c)
{
	switch(c->direction){
		case move_up:
			if(s->cursorRow != 0){
				s->cursorRow--;
				goto check_column;
			}
			break;
		case move_down:
			if(s->row_mapping[s->cursorRow+1].line != NULL ){
				s->cursorRow++;
				goto check_column;
			}	
			break;
		case move_left:
			if(s->cursorCharacter != 0){
				s->csaved = 0;
				s->cursorCharacter--;
				goto update_column;
			}
			break;
		case move_right:
			if(s->cursorCharacter < s->row_mapping[s->cursorRow].width){
				s->csaved = 0;
				s->cursorCharacter++;
				goto update_column;	
			} 
			break;
	}

	return;

	check_column:
	if(!s->csaved && s->cursorCharacter > s->row_mapping[s->cursorRow].width){
		if(!s->csaved){
			s->savedCursorCharacter = s->cursorCharacter;
			s->csaved = 1;
		}
		s->cursorCharacter = s->row_mapping[s->cursorRow].width;
	}
	if(s->csaved){
		if(s->savedCursorCharacter <= s->row_mapping[s->cursorRow].width){
			s->csaved = 0;
			s->cursorCharacter = s->savedCursorCharacter;
		} else {
			s->cursorCharacter = s->row_mapping[s->cursorRow].width;
		}
	}

	update_column:
	s->cursorColumn = col_of_char(s->xt, &s->row_mapping[s->cursorRow],
					s->cursorCharacter, s->width);
}

void update_state(struct displayState *s, struct input_action *a)
{
	command c = proccess_input(s, a);
	switch(c.type){
		case no_command:
			break;
		case move_cursor:
			update_cursor(s, &c);
			break;
		case change_mode:
			s->mode = c.new_mode;
			s->changed_control= 1;
			break;
	}
}

void render_state(struct displayState *s, struct display *dis)
{
	
	for(int i = 0; i < s->viewRows; i++){
		if(s->row_mapping[i].changed){
			dis->put_line(dis, s->row_mapping[i].text, i);
			dis->display_line(dis, i);
			s->row_mapping[i].changed = 0;
		}
	}

	if(s->changed_control){
		switch(s->mode){
			case normal_mode:
				dis->put_line(dis, "-- NORMAL --", s->rows - 1);
				break;
			case insert_mode:
				dis->put_line(dis, "-- INSERT --", s->rows - 1);
				break;
		}
		dis->display_line(dis, s->rows - 1);
		s->changed_control = 0;
	}

	dis->set_cursor(dis, s->cursorRow, s->cursorColumn);
}

