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

/* 
 * Spans Are **Special**:
 * 
 * * Contigous range; no \n to exclude.
 *
 * * Must wrap around, meaning that a character (\t) 
 *   can occupy multiple rows.
 * 
 * * Editing a span change all the rows that belong to it.
 * 
 * * Cursor must wrap around with it, meaning that a span and (utf8 char) 
 *   offset are a natural choice for cursor cordinates.
 */

// The key here is that a span **must** takeup
// a contiguous range of text. Also always starts at a new line.
// Assume for now that each column is one utf8 char
struct span_buf{
	utf8 *v;
	int size; // number of 
};

void print_spanbuf(struct span_buf *b){
	write(2, "span: ", 6);
	for(int i = 0; i < b->size; i++){
		write(2, b->v[i].x, utf8_size(b->v[i])); 
	}
	write(2, "\n", 1);
}

/* Initialises the display */
void init_display(struct stext *xt, struct display *dis, struct dstate *state)
{	
	xt->lock(xt);
	
	int tab_width = 4;
	int h = state->size.height;
	int w = state->size.width;

//
	//first we get the buffers
	span **spans = malloc(sizeof(span *)*h);
	struct span_buf *bvec = malloc(sizeof(struct span_buf)*h);
	int offset = 0; // utf8 offset
	int total_spans = 0;
	int total_size = 0;
	int tmp = 0;
	for(int i = 0; i < h; i++){		
		if(total_size >= h*w)
				break;
		spans[i] = xt->get_span(xt, offset);
		if(spans[i] == NULL)
			break;
		bvec[i].v = malloc(sizeof(utf8)*h*w);
		total_spans++;
		for(int j = 0; j < h*w; j++){
			if(total_size + j >= h*w){
				total_size += j;
				break; // jumps to if-statement that will break outer loop, avoids using goto.
			}
			utf8 uchar = xt->peek_span(spans[i]);		
			if(utf8_empty(uchar))
				break;
			if(utf8_compare(uchar, '\n')){
				offset++;
				break;
			}	
			if(utf8_compare(uchar, '\t')){
				int t = j + tab_width - ((j+1) % tab_width);
				for(; j < t && j < h*w; j++)
					bvec[i].v[j] = utf8_from_char(' ');	
				j--; // otherwise j will be double incremented by the two for-loops.
				goto grow;
			}
			bvec[i].v[j] = uchar;	
grow:
			xt->grow_span(spans[i]);
			offset++;
			tmp = j;
		}		
		bvec[i].size = tmp + 1;
		total_size += (tmp + w - tmp % w); // round up to nearest w multiple	
		tmp = 0;
	}
		

	display_str str;
	str.codepoints = malloc(sizeof(codepoint) * w);
	int row = 0;
	int col = 0;
	for(int i = 0; i < total_spans; i++){
		//print_spanbuf(&bvec[i]);
		for(int j = 0; j < bvec[i].size; j++){
			if(col == w){
				str.num = col;
				dis->put_str(dis, &str, row);	
				dis->display_line(dis, row);
				row++;
				col = 0;
			}
			codepoint_from_uchar(&str.codepoints[col], bvec[i].v[j]);
			col++;		
		}
		str.num = col;
		dis->put_str(dis, &str, row);	
		dis->display_line(dis, row);
		row++;
		col = 0;
	}

/*
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
*/
	state->spans = spans;

	xt->unlock(xt);
}

/*
void update_display(struct stext *xt, struct display *dis, struct dstate *state, )
{


}
*/
