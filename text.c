#include "text.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define GET_BXT(s) (struct basic_text*)((char *)s - offsetof(struct basic_text, super));

/* Setters and getters */

static void bxt_set_row_width(struct text *xt, int width)
{
	struct basic_text *bxt = GET_BXT(xt);
	bxt->row_width = width;
}

static int bxt_get_row_width(struct text *xt)
{
	struct basic_text *bxt = GET_BXT(xt);
	return bxt->row_width;
}

static void bxt_set_fd(struct text *xt, int fd)
{
	struct basic_text *bxt = GET_BXT(xt);	
	bxt->fd = fd;
}

static int bxt_is_dirty(struct text *xt)
{
	struct basic_text *bxt = GET_BXT(xt);
	return bxt->dirty;
}

static int bxt_get_total_lines(struct text *xt)
{
	struct basic_text *bxt = GET_BXT(xt);
	return bxt->total_lines;
}

/* Reference counting */

static line_id inc_ref(line_id l_id){
	struct line *l = (struct line *)l_id;
	l->refcount++;
	return l_id;
}

static line_id dec_ref(line_id l_id){
	struct line *l = (struct line *)l_id;
	l->refcount--;
	return l_id;
}

/* Load and Save */

// For now assume that all characters – glphys, letters, tabs – are one byte.
static int next_character(int fd, char *c)
{
	return read(fd, c, 1);
}

static struct line *make_line(int width)
{
	struct line *l = malloc(sizeof(struct line));
	memset(l, 0, sizeof(struct line));
	// width + 1 so all strings are null terminated.
	l->text = (char *)malloc(width + 1);
	memset(l->text, 0 , width + 1);
	l->id = (void *)l;
	return l;
}

static void free_line(struct line *l)
{
	free(l->text);
	free(l);
}

// Assume all characters are ascii, no tabs for now, and are less than max width.
static void bxt_load_file(struct text *xt)
{
	struct basic_text *bxt = GET_BXT(xt);	
	bxt->total_lines = 1;
	struct line *current = make_line(bxt->row_width);
	bxt->head = current;
	char c;
	int c_size, l_size = 0;
	lseek(bxt->fd, 0, SEEK_SET);
	while((c_size = next_character(bxt->fd, &c))){
		if(c == '\n' || l_size >= bxt->row_width){
			current->len = l_size;
			struct line *n = make_line(bxt->row_width);
			current->next = n;
			n->prev = current;
			bxt->total_lines++;
			current = n;
			l_size = 0;
			if(c == '\n')
				continue;
		}
		l_size += c_size;
		current->text[l_size - 1] = c;
	}
	current->len = l_size;
	bxt->tail = current;		
}

static void bxt_save_file(struct text *xt)
{
	struct basic_text *bxt = GET_BXT(xt);
	size_t file_len = 0;
	struct line *current = bxt->head;
	lseek(bxt->fd, 0, SEEK_SET);
	while(current){
		write(bxt->fd, current->text, current->len);
		file_len = file_len + current->len;	
		if(current->next){
			write(bxt->fd, "\n", 1);
			file_len++;
		}
		current = current->next;
	}
	ftruncate(bxt->fd, file_len);
	bxt->dirty = 0;
}

static void bxt_unload_file(struct text *xt)
{
	struct basic_text *bxt = GET_BXT(xt);

	struct line *current = bxt->head;
	while(current){
		struct line *tmp = current;
		current = current->next;
		free_line(tmp);
	}
}

/* Line Access */


static line_id bxt_get_first_line(struct text *xt)
{
	struct basic_text *bxt = GET_BXT(xt);
	return inc_ref(bxt->head->id);
}

static line_id bxt_get_last_line(struct text *xt)
{
	struct basic_text *bxt = GET_BXT(xt);
	return inc_ref(bxt->tail->id);
}

static line_id bxt_next_line(struct text *xt, line_id id)
{	
	dec_ref(id);	
	if(((struct line *)id)->next){
		return inc_ref(((struct line *)id)->next->id);	
	} else {
		return 0;
	}
}

static line_id bxt_prev_line(struct text *xt, line_id id)
{
	dec_ref(id);	
	if(((struct line *)id)->prev){
		return inc_ref(((struct line *)id)->prev->id);	
	} else {
		return 0;
	}
}

static const char *bxt_get_text(struct text *xt, line_id line)
{
	return ((struct line *)line)->text;
}

static void bxt_set_text(struct text *xt, line_id line, char *str)
{
	struct basic_text *bxt = GET_BXT(xt); 
	struct line *l = (struct line*)line;
	l->len =  stpncpy(l->text, str, bxt->row_width) - l->text;
	bxt->dirty = 1;
}

/* control */

static line_id bxt_delete_line(struct text *xt, line_id id)
{
	struct basic_text *bxt = GET_BXT(xt); 
	struct line *line = (struct line *)id;
	// Delete nothing if this is the only line.
	if(line->prev == NULL && line->next == NULL){
		return id;
	}
	bxt->total_lines--;
	if(line->prev == NULL){
		line->next->prev = NULL;
		bxt->head = line->next;
		return bxt->head->id;
	} else {
		line->next->prev = line->prev;
	}

	if(line->next == NULL){
		line->prev = NULL;
	} else {
		line->prev->next = line->next;
	}
	line_id new_id = line->prev->id;
	free_line(line);
	bxt->dirty = 1;
	return inc_ref(new_id);
}

static line_id bxt_insert_after(struct text *xt, line_id id, char *str)
{
	struct basic_text *bxt = GET_BXT(xt); 
	struct line *line = (struct line *)id;
	struct line *new_line = make_line(bxt->row_width);			
	bxt_set_text(xt, new_line->id, str);
	if(line->next){
		line->next->prev = new_line;
		new_line->next = line->next;
	} else {
		bxt->tail = new_line;
		new_line->next = NULL;
	}
	line->next = new_line;
	new_line->prev = line;
	bxt->dirty = 1;
	bxt->total_lines++;
	dec_ref(id);
	return inc_ref(new_line->id);
}

static line_id bxt_insert_before(struct text *xt, line_id id, char *str)
{
	struct basic_text *bxt = GET_BXT(xt); 
	struct line *line = (struct line *)id;
	struct line *new_line = make_line(bxt->row_width);			
	bxt_set_text(xt, new_line->id, str);
	if(line->prev){
		line->prev->next = new_line;
		new_line->prev = line->prev;
	} else {
		bxt->head= new_line;			
		new_line->prev = NULL;
	}
	line->prev = new_line;
	new_line->next = line;
	bxt->dirty = 1;
	bxt->total_lines++;
	dec_ref(id);
	return inc_ref(new_line->id);
}

static line_id bxt_get_line(struct text *xt, line_id id)
{
	return inc_ref(id);
}

static void bxt_put_line(struct text *xt, line_id id)
{
	dec_ref(id);
}

struct text *make_basic_text(struct basic_text *bxt)
{
	bxt->super.set_row_width = bxt_set_row_width;
	bxt->super.get_row_width = bxt_get_row_width;
	bxt->super.set_fd = bxt_set_fd;
	bxt->super.is_dirty = bxt_is_dirty;
	bxt->super.get_total_lines = bxt_get_total_lines;
	bxt->super.load_file = bxt_load_file;
	bxt->super.save_file = bxt_save_file;
	bxt->super.unload_file = bxt_unload_file;
	bxt->super.get_first_line = bxt_get_first_line;
	bxt->super.get_last_line = bxt_get_last_line;
	bxt->super.next_line = bxt_next_line;
	bxt->super.prev_line = bxt_prev_line;
	bxt->super.delete_line = bxt_delete_line;
	bxt->super.insert_after = bxt_insert_after;
	bxt->super.insert_before =  bxt_insert_before;
	bxt->super.get_text = bxt_get_text;
	bxt->super.set_text = bxt_set_text;
	bxt->super.get_line = bxt_get_line;
	bxt->super.put_line = bxt_put_line;
	return &(bxt->super);	
}

int basic_refcount_zero(struct text *xt)
{
		struct basic_text *bxt = GET_BXT(xt); 
		struct line *current_line = (struct line *)bxt->head;
		do{
			if(current_line->refcount != 0)
				return -1;
			current_line = (struct line *)current_line->next;		
		}while(current_line != bxt->tail);
		
		return 0;
}
