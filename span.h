#include "utf8.h"

/* stext interface */
#ifndef SPAN_H
#define SPAN_H

//struct stext;

struct _span{
	int start; // the utf8 start offset 
	int end;   // the utf8 end offset 
	struct stext *s;
};

typedef struct _span span;

struct stext{
	void (*lock)(struct stext *);
	void (*unlock)(struct stext *);
	void (*set_fd)(struct stext *, int);
	void (*load_file)(struct stext *);
	void (*save_file)(struct stext *);
	void (*unload_file)(struct stext *);	
	span *(*get_span)(struct stext *, int);
	void (*put_span)(span *);
	int  (*grow_span)(span *);
	utf8 (*peek_span)(span *);		
	utf8 (*index_span)(span *, int);
	void (*insert)(span *, int, utf8);
	void (*del)(span *, int);
};


/* mock stext interface implementation for testing */

struct mock_text{	
	int fd;
	char *file;
	int length;
	int size;
	int allocated;
	struct stext super;
};

struct _mock_span{
	char *start;
	char *end;
	span super;
};

typedef struct _mock_span mock_span;

struct stext *make_mock_text(struct mock_text *);

#endif /* SPAN_H */
