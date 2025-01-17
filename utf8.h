
#ifndef UTF8_H
#define UTF8_H

struct _utf8{
	char x[4];
};

typedef struct _utf8 utf8;

int utf8_size(utf8);

utf8 utf8_next(char *);

utf8 utf8_from_char(char);

int utf8_compare(utf8, char);

int utf8_empty(utf8);


#endif /* UTF8_H */
