
/* WARNING! THIS IS NOT YET PART OF THE Qt API, AND MAY BE CHANGED WITHOUT
   NOTICE */

#include "qlayoutengine.h"

static inline int toFixed( int i ) { return i * 256; }
static inline int fRound( int i ) {
    return  i % 256 < 128 ? i / 256 : 1 + i / 256;
}
/*
  \internal
  This is the main workhorse of the QGridLayout. It portions out
  available space to the chain's children.

  The calculation is done in fixed point: "fixed" variables are scaled
  by a factor of 256.

  If the layout runs "backwards" (i.e. RightToLeft or Up) the layout
  is computed mirror-reversed, and it is the callers responsibility do
  reverse the values before use.

  chain contains input and output parameters describing the geometry.
  count is the count of items in the chain,
  pos and space give the interval (relative to parentWidget topLeft.)
*/

void qGeomCalc( QArray<QLayoutStruct> &chain, int count, int pos,
		      int space, int spacer )
{
    typedef int fixed;
    int cHint = 0;
    int cMin = 0;
    int cMax = 0;
    int sumStretch = 0;
    int spacerCount = 0;

    bool wannaGrow = FALSE; // anyone who really wants to grow?
    //    bool canShrink = FALSE; // anyone who could be persuaded to shrink?

    int i; //some hateful compilers do not handle for loops correctly
    for ( i = 0; i < count; i++ ) {
	chain[i].done = FALSE;
	cHint += chain[i].sizeHint;
	cMin += chain[i].minimumSize;
	cMax += chain[i].maximumSize;
	sumStretch += chain[i].stretch;
	if ( !chain[i].empty )
	    spacerCount++;
	wannaGrow = wannaGrow ||  chain[i].expansive;
    }

    if ( spacerCount )
	spacerCount -= 1; //only spacers between things
    if ( space < cMin + spacerCount*spacer ) {
	//	debug("not enough space");
	for ( int i = 0; i < count; i++ ) {
	    chain[i].size = chain[i].minimumSize;
	    chain[i].done = TRUE;
	}
    } else if ( space < cHint + spacerCount*spacer ) {
	int n = count;
	int space_left = space - spacerCount*spacer;
	int overdraft = cHint - space_left;
	//first give to the fixed ones:
	for ( i = 0; i < count; i++ ) {
	    if ( !chain[i].done && chain[i].minimumSize >= chain[i].sizeHint) {
		chain[i].size = chain[i].sizeHint;
		chain[i].done = TRUE;
		space_left -= chain[i].sizeHint;
		sumStretch -= chain[i].stretch;
		n--;
	    }
	}
	bool finished = n == 0;
	while ( !finished ) {
	    finished = TRUE;
	    fixed fp_over = toFixed( overdraft );
	    fixed fp_w = 0;

	    for ( i = 0; i < count; i++ ) {
		if ( chain[i].done )
		    continue;
		// if ( sumStretch <= 0 )
		fp_w += fp_over / n;
		// else
		//    fp_w += (fp_space * chain[i].stretch) / sumStretch;
		int w = fRound( fp_w );
		chain[i].size = chain[i].sizeHint - w;
		fp_w -= toFixed( w ); //give the difference to the next
		if ( chain[i].size < chain[i].minimumSize ) {
		    chain[i].done = TRUE;
		    chain[i].size = chain[i].minimumSize;
		    finished = FALSE;
		    overdraft -= chain[i].sizeHint - chain[i].minimumSize;
		    sumStretch -= chain[i].stretch;
		    n--;
		    break;
		}
	    }
	}
    } else { //to much space
	int n = count;
	int space_left = space - spacerCount*spacer;
	//first give to the fixed ones:
	for ( i = 0; i < count; i++ ) {
	    if ( !chain[i].done && ( chain[i].maximumSize <= chain[i].sizeHint
				     || wannaGrow && !chain[i].expansive )) {
		chain[i].size = chain[i].sizeHint;
		chain[i].done = TRUE;
		space_left -= chain[i].sizeHint;
		sumStretch -= chain[i].stretch;
		n--;
	    }
	}
	bool finished = n == 0;
	while ( !finished ) {
	    finished = TRUE;
	    fixed fp_space = toFixed( space_left );
	    fixed fp_w = 0;

	    for ( i = 0; i < count; i++ ) {
		if ( chain[i].done )
		    continue;
		if ( sumStretch <= 0 )
		    fp_w += fp_space / n;
		else
		    fp_w += (fp_space * chain[i].stretch) / sumStretch;
		int w = fRound( fp_w );
		chain[i].size = w;
		fp_w -= toFixed( w ); //give the difference to the next
		if ( w < chain[i].sizeHint ) {
		    chain[i].done = TRUE;
		    chain[i].size = chain[i].sizeHint;
		    finished = FALSE;
		    space_left -= chain[i].sizeHint;
		    sumStretch -= chain[i].stretch;
		    n--;
		    break;
		}
	    }
	}
    }

    int p = pos;
    for ( i = 0; i < count; i++ ) {
	chain[i].pos = p;
	p = p + chain[i].size;
	if ( !chain[i].empty )
	    p += spacer;
    }
}
