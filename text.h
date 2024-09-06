#include <stddef.h>

typedef long line_id;

/* Generic Text Interface */

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
	line_id (*next_line)(struct text *, line_id);
	line_id (*prev_line)(struct text *, line_id);
	void (*delete_line)(struct text *, line_id);
	line_id (*insert_after)(struct text *, line_id, char *);
	line_id (*insert_before)(struct text *, line_id, char *);
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
