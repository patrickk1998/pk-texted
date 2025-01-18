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
#include "span.h"

#define MIN(a,b) (a < b) ? a : b

/* New span stuff implemented here */

/* Initialises the display */
void init_display(struct stext *xt, struct display *dis, struct dstate *state)
{	
	xt->lock(xt);
	
	int tab_width = 4;
	int h = state->size.height;
	int w = state->size.width;

	// first we get the spans for each row
	span **spans = malloc(sizeof(span *)*h);
	int offset = 0; // utf8 offset
	for(int row = 0; row < h; row++){
		spans[row] = xt->get_span(xt, offset);
		if(spans[row] == NULL)
			break;
		for(int col = 0; col < w; col++){
			utf8 uchar = xt->peek_span(spans[row]);		
			if(utf8_empty(uchar))
				break;
			if(utf8_compare(uchar, '\n')){
				offset++;
				break;
			}	

			// When a line wraps to a new span, there can never be a space at
			// the begining due to tab expansion (unlike in vim) this simplifes things a bit.
			if(utf8_compare(uchar, '\t'))
				col = MIN(w, (col + tab_width - (col % tab_width))); 

			xt->grow_span(spans[row]);
			offset++;
		}		
	}


	// Then we convert the spans into code points;
	display_str str;
	str.codepoints = malloc(sizeof(codepoint) * w);
	for(int row = 0; row < h; row++){
		if(spans[row] == NULL)
			break;
		int index = 0;
		for(int col = 0; col < w;){
			if(col == (spans[row]->end - spans[row]->start))
				break;
			utf8 uchar = xt->index_span(spans[row], index);		
			
			// to do tab expansion or not
			if(utf8_compare(uchar, '\t')){
				int l = MIN(w - col, tab_width - (col % tab_width)); 
				for(int i = 0; i < l; i++){
					codepoint_from_uchar(&str.codepoints[col], utf8_from_char(' '));
					col++;
				}
			} else {
				codepoint_from_uchar(&str.codepoints[col], uchar);
				col++;
			}
			str.num = col + 1;
			index++;
		}
		// check if span is empty
		if(spans[row]->end - spans[row]->start == 0)
			str.num = 0;
		dis->put_str(dis, &str, row);
		dis->display_line(dis, row);
	}
	free(str.codepoints);

	dstate->spans = spans;

	xt->unlock(xt);
}

void update_display()
{


}
