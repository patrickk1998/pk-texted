#include <stddef.h>

#ifndef TEXT_H
#define TEXT_H

typedef long line_id;

/* Generic Text Interface */


/*
		** Reference counting ** 

		The following functions increment the ref count of a line by one.
			get_first_line()
			get_last_line()
			get_line()
			insert_after() 
			insert_before() 

		The following functions decrement the ref count of argument line by one and 
		increment the ref count of the returned line by one.
			next_line()
			prev_line()

		put_line() decrements the ref count of the line in the argument by one. delete_line()
		deletes the line object in memory unless it is the only line, then it does nothing.

		In general, it is only save to assume that a line_id is valid and represents a constant, unique, line 
		if it is used between a "get" function call and a "put" function call. 

		A generic way to iterate through lines would be this:

		line_id start_line = xt->get_last_line(xt);
		line_id end_line = xt->get_first_line(xt);

		line_id current_line = xt->get_line(xt, start_line);
		do{
			DO SOMETHING...
			current_line = xt->next_line(xt, current_line);		
		}while(current_line != end_line);
		xt->put_line(xt, current_line);

		xt->put_line(xt, start_line);
		xt->put_line(xt, end_line);	
*/
struct text{
	void (*set_row_width)(struct text *, int);
	int (*get_row_width)(struct text *);
	void (*set_fd)(struct text *, int);
	void (*load_file)(struct text *);
	void (*save_file)(struct text *);
	void (*unload_file)(struct text *);
	int (*is_dirty)(struct text *);
	int (*get_total_lines)(struct text *);
	line_id (*get_first_line)(struct text *);
	line_id (*get_last_line)(struct text *);
	line_id (*next_line)(struct text *, line_id);
	line_id (*prev_line)(struct text *, line_id);
	line_id (*delete_line)(struct text *, line_id);
	line_id (*insert_after)(struct text *, line_id, char *);
	line_id (*insert_before)(struct text *, line_id, char *);
	line_id *(get_line)(struct text *, line_id);
	void *(put_line)(struct text *, line_id);
	const char *(*get_text)(struct text *, line_id);	
	void (*set_text)(struct text *, line_id, char *);
};

/* Basic Implementation  */

struct basic_text{
	int row_width;	
	int total_lines;
	int fd;
	int dirty;
	struct line *head;
	struct line *tail;
	struct text super;
};


struct line{
	line_id id;
	size_t len;
	char *text;	
	struct line *prev;
	struct line *next;
};

struct text *make_basic_text(struct basic_text *);

#endif /* TEXTED_H */
