#ifndef CODEPOINT_H
#define CODEPOINT_H

/*
 *  A codepoint represents not just a utf8 unicode character, but
 *  Someing that is being displayed on screen, hence is must have 
 *  properties associated with it like text and higlight color.
 */

// Subtract 1 from color enum to get ANSI color number.
enum color{
	color_default = 0,
	color_black = 1,
	color_red = 2,
	color_green = 3,
	color_yello = 4,
	color_blue = 5,
	color_purple = 6,
	color_cyan = 7,
	color_gray = 8,
};

// Each code point represents both a column on screen and a utf-8 character.
struct _codepoint{
	char content[4];
	// The following variable determines if the cursor can stop at the code point, 
	// used for when tab characters are expanded into spaces. And to representing 
	// glyphs that are made up of more than one unicode codepoint.
	enum {
		positional,
		non_positional,
		zero_length,
	} disposition;
	enum{
		continue_line,
		end_line,
	} line_status;
	enum color text_color;
	enum color highlight_color;
};

typedef struct _codepoint codepoint;

int get_next_codepoint(const char *, codepoint *);

int put_next_codepoint(char *, const codepoint *);

int get_length(codepoint *);

/* 
	The const char *str argument can be seen as a format string.
	To switch text colors use %c and the text color will change to 
	that of the corresponding argument. Example:

	stocp("%cRed%cWhite%cBlue%c and default.", color_red, color_white, color_blue, color_default);

	If no %c format operator is used, then the text color will be default. The convert 
	a precentage symbol (%), the format operator is %%.
*/
codepoint *stocp(const char * str, ...);

#endif /* CODEPOINT_H */
