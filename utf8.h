

struct _utf8{
	char x[4];
};

typedef struct _utf8 utf8;

int utf8_size(utf8);

utf8 utf8_next(char *);

int utf8_compare(utf8, char);

int utf8_empty(utf8);

