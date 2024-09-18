#include <stddef.h>
#include <stdatomic.h>

#ifndef TEXT_H
#define TEXT_H

typedef void *line_id;

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
		removes the line object and frees its memory unless it is the only line, then it does nothing. The 
		ref count of the line returned is incremented by one.

		In general, it is only safe to assume that a line_id is valid and represents a constant, unique line 
		if it is used between a "get" function call and a "put" function call. 

		A generic way to iterate through lines would be something like this:

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

		What cannot be assumed is that the same line will have the same line_id if the reference count is zero:

		line_id start_line_old = xt->get_first_line(xt);
		xt->put_line(xt, start_line);
		
		line_id start_line_new = xt->get_first_line(xt);
		assert(start_line_old == start_line_new); <-- May fail.
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
	line_id (*get_line)(struct text *, line_id);
	void (*put_line)(struct text *, line_id);
	const char *(*get_text)(struct text *, line_id);	
	void (*set_text)(struct text *, line_id, char *);
};

/* Basic Implementation */

/* 
	** Purpose **

	The reason to implement a basic text object that reads and stores 
	the whole file in a linked list is two-fold:

	1. To ensure that the interface that struct text provides is enough to build a working 
	and useful text editor and that there are unit tests that thoroughly test it. 

	2. To test that all the rest of the components of the text editor independently of 
	the specific file-interface backend.
*/

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
	_Atomic int refcount; // Only a relaxed memory order needed for reference counting.
	line_id id;
	size_t len;
	char *text;	
	struct line *prev;
	struct line *next;
};

struct text *make_basic_text(struct basic_text *);

int basic_refcount_zero(struct text*);

#endif /* TEXTED_H */
