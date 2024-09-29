#ifndef CODEPOINT_H
#define CODEPOINT_H

// Just two colors for now.
enum color{
	color_default,
	color_blue,
	color_red,
};

// Each code point represents both a column on screen and a utf-8 character.
struct _codepoint{
	char content[4];
	// The following variable determines if the cursor can stop at the code point, 
	// used for when tab characters are expanded into spaces.
	enum {
		positional,
		non_positional,
	} disposition;
	enum color text_color;
	enum color back_ground;
};

typedef struct _codepoint codepoint;

int get_next_codepoint(char *, codepoint *);

int put_next_codepoint(char *, codepoint *);

int get_length(codepoint *);

#endif /* CODEPOINT_H */
