/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgfxvga16_qws.cpp $
**
** Implementation of QGfxVga16 (graphics context) class for VGA cards
*
** Created : 
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

#include <qapplication.h>

#include "qgfxraster_qws.h"
#include "qgfxlinuxfb_qws.h"


#define SINGLE_STATIC_APP
#ifndef SINGLE_STATIC_APP
unsigned char screen_double_buffer[640*480/2];
unsigned char vga_register_values[512];
#else
unsigned char *screen_double_buffer = NULL; // [640*480/2];
unsigned char *vga_register_values = NULL; // [512]
//unsigned char **fb_line_ptrs;
//unsigned char **db_line_ptrs;
#endif

unsigned char *fb_line_ptrs[480];
unsigned char *db_line_ptrs[480];


#define UNREFERENCED_PARAMETER(x)   ((x) = (x))


#ifndef QT_NO_QWS_CURSOR

class QVga16Cursor : public QScreenCursor
{
public:
    QVga16Cursor();
    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
    void drawCursor(QRect &r);
    virtual bool supportsAlphaCursor() { return false; }
    static bool enabled() { return true; }
private:
    void restoreUnder();
};


static QVga16Cursor *qt_vga16cursor;


inline QVga16Cursor::QVga16Cursor()
{
    qt_vga16cursor = this;
}

#endif // QT_NO_QWS_CURSOR


#ifndef QT_NO_QWS_CURSOR

QRect	Vga16DrawArea;

# define VGA16_GFX_START(r)	if(is_screen_gfx && qt_sw_cursor) \
				    Vga16DrawArea = r, beginDraw();
# define VGA16_GFX_END		if(is_screen_gfx && qt_sw_cursor) \
				    qt_vga16cursor->drawCursor(Vga16DrawArea), endDraw();
		
#else //QT_NO_QWS_CURSOR

# define VGA16_GFX_START(r)	if(is_screen_gfx) \
				    beginDraw();
# define VGA16_GFX_END		if(is_screen_gfx) \
				    endDraw();
			    
#endif //QT_NO_QWS_CURSOR


bool set_back_buffer = true;
bool force_set_pixel = false;


class VGA16_CriticalSection    // Mutex for serialising VGA16 access
{
    public:

	VGA16_CriticalSection()
	{
#ifndef SINGLE_STATIC_APP
	    QWSDisplay::grab();
#endif
	}

	~VGA16_CriticalSection()
	{
#ifndef SINGLE_STATIC_APP
	    QWSDisplay::ungrab();
#endif
	}
};


//
// The idea of this macro is to generically make functions that will
// output to the VGA ports only if the cached value of what the port
// is set to is different.
//
#define MAKE_VGA16_FUNC(funcname, reg, port) \
    inline void set_##funcname##_vga16(unsigned char value, bool force = true) \
    { \
	if ((force) || (vga_register_values[((port - 0x3C0) * 16) + reg] != value)) { \
	outw((value << 8) | reg, port); \
	vga_register_values[((port - 0x3C0) * 16) + reg] = value; \
    } \
}


// make some VGA16 helper functions
MAKE_VGA16_FUNC(write_planes,	0x02, 0x3C4)
MAKE_VGA16_FUNC(memory_mode,	0x04, 0x3C4)
MAKE_VGA16_FUNC(set_reset,	0x00, 0x3CE)
MAKE_VGA16_FUNC(enable_set_reset, 0x01, 0x3CE)
MAKE_VGA16_FUNC(rotate,		0x03, 0x3CE)
MAKE_VGA16_FUNC(read_map,	0x04, 0x3CE)
MAKE_VGA16_FUNC(mode,		0x05, 0x3CE)
MAKE_VGA16_FUNC(bit_mask,	0x08, 0x3CE)



unsigned char vga16_getpixel(unsigned int x, unsigned int y)
{
    return (*(db_line_ptrs[y] + (x >> 1)) >> ((x & 1) << 2)) & 0x0F; 
}

void vga16_setpixel(unsigned int x, unsigned int y, unsigned char col)
{
    unsigned char shift;

    if (vga16_getpixel(x, y) == col)
	return;

    shift = (x & 1) << 2;
    col &= 0x0F;

    *(db_line_ptrs[y] + (x >> 1)) &= 0xF0 >> shift;
    *(db_line_ptrs[y] + (x >> 1)) |= col << shift;

    // do once
    set_enable_set_reset_vga16(0x0F);
    set_rotate_vga16(0x00);
    set_mode_vga16(0x00);
    set_write_planes_vga16(0x0F);

    // do for each pixel
    set_set_reset_vga16(col);
    set_bit_mask_vga16(0x80 >> (x & 0x07));

    volatile unsigned char *fb_ptr = fb_line_ptrs[y] + (x >> 3); 
    *fb_ptr |= 0xFE;
}


void vga16_hline(unsigned int x1, unsigned int x2, unsigned int y, unsigned char col)
{
}

void vga16_blt(unsigned int x1, unsigned int x2, 
	unsigned int width, unsigned int height,
	unsigned char *src) 
{
}


//
// This is fairly well optimised
//
void vga16_drawrect(unsigned int x1, unsigned int y1, 
	unsigned int x2, unsigned int y2, unsigned char col)
{
    unsigned char shift;
    unsigned int i, j;
    unsigned char *db_line_ptr = db_line_ptrs[y1];
    unsigned char *fb_line_ptr = fb_line_ptrs[y1];
    unsigned char *first_db_line_ptr = db_line_ptr;
    
    unsigned char middle_char = ((col & 0x0F) << 4) | (col & 0x0F);
    unsigned int tx1 = x1, tx2 = x2;

    if (x1 > x2)
	x1 = tx2, x2 = tx1;
    tx1 = x1, tx2 = x2;


    // ### needs optimisation
    // this is a special case for narrow vertical strips that will
    // be in the same byte and need a special mask.
    // this needs to be optimised from what it is now
    if (x1 + 8 > x2) {
	for (j = y1; j <= y2; j++)
	    for (i = x1; i <= x2; i++)
		vga16_setpixel(i, j, col);
	return;
    }
    
    

    if (tx1 & 1) {
    	db_line_ptr = first_db_line_ptr + (tx1 >> 1);
	for (j = y1; j <= y2; j++, db_line_ptr += 320)
	    *db_line_ptr = (col << 4) | (*db_line_ptr & 0x0F);
	tx1++;
    }

    tx2++; // I didn't originally make it inclusive

    if (tx2 & 1) {
    	db_line_ptr = first_db_line_ptr + (tx2 >> 1);
	for (j = y1; j <= y2; j++, db_line_ptr += 320)
	    *db_line_ptr = (col & 0x0F) | (*db_line_ptr & 0xF0);
	tx2--;
    }

    if (tx1 < tx2) {
	unsigned int width = (tx2 - tx1) >> 1;
	db_line_ptr = first_db_line_ptr + (tx1 >> 1);
	for (j = y1; j <= y2; j++, db_line_ptr += 320) {
	    memset(db_line_ptr, middle_char, width);
	}
    }

    set_enable_set_reset_vga16(0x0F);
    set_rotate_vga16(0x00);
    set_mode_vga16(0x00);
    set_write_planes_vga16(0x0F);
    set_set_reset_vga16(col & 0x0F);
   
    unsigned char *fb_ptr = fb_line_ptr + (x1 >> 3); 
    set_bit_mask_vga16(0xFF >> (x1 & 7));
    for (i = y1; i <= y2; i++, fb_ptr += 80)
        *fb_ptr |= 0xFE;
    x1 += 8 - (x1 & 7);

    fb_ptr = fb_line_ptr + (x2 >> 3);
    set_bit_mask_vga16(0xFF << (7 - (x2 & 7)));
    for (i = y1; i <= y2; i++, fb_ptr += 80)
        *fb_ptr |= 0xFE;
    x2 -= x2 & 7;
 
    set_bit_mask_vga16(0xFF);
    if (x1 < x2) {
	unsigned int width = (x2 - x1) >> 3;
	fb_ptr = fb_line_ptr + (x1 >> 3);
        for (i = y1; i <= y2; i++, fb_ptr += 80) {
	    memset(fb_ptr, 0, width);
	}
    }

}




// private QPolygonScanner
class QGfxVga16 : public QGfxRasterBase , private QPolygonScanner
{

    public:

	QGfxVga16(unsigned char *,int w,int h);
	virtual ~QGfxVga16();

	
	unsigned char getPixel( int,int );
	virtual void  setPixel( int,int,unsigned char );
	
	virtual void drawPoint( int,int );
	virtual void drawPoints( const QPointArray &,int,int );
	virtual void drawLine( int,int,int,int );
	virtual void drawRect( int,int,int,int );
	virtual void drawPolyline( const QPointArray &,int,int );
	virtual void drawPolygon( const QPointArray &,bool,int,int );
	virtual void blt( int,int,int,int );
	virtual void scroll( int,int,int,int,int,int );
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
	virtual void stretchBlt( int,int,int,int,int,int );
#endif
	virtual void tiledBlt( int,int,int,int );
	virtual int bitDepth() { return 4; }
	virtual void setSource(const QImage *);
	virtual void setSource(const QPaintDevice *);

    protected:

	virtual void drawThickLine( int,int,int,int );

    private:

	unsigned int get_pixel_4_32(volatile unsigned char *l, unsigned int x);
	unsigned char get_pixel_4(volatile unsigned char *l, unsigned int x);
	void set_pixel_4(volatile unsigned char *l, unsigned int x, unsigned char col);
	void hline_4(volatile unsigned char *l, unsigned int x1, unsigned int x2, unsigned char col);
	void himageline_4(volatile unsigned char *l, unsigned int x1, unsigned int x2, unsigned char *srcdata, bool reverse = false);

	GFX_INLINE unsigned int get_value_32(int sdepth, unsigned char **srcdata, bool reverse = false);
	GFX_INLINE void calcPacking(void * m,int x1,int x2, int & frontadd,int & backadd,int & count);
	GFX_INLINE void drawPointUnclipped( int x, unsigned char* l);
	GFX_INLINE void hline( int x1,int x2,int y);
	GFX_INLINE void hlineUnclipped( int x1,int x2,unsigned char* l);
	GFX_INLINE void hImageLineUnclipped( int x1,int x2,unsigned char *l,unsigned char *srcdata,bool reverse);
	GFX_INLINE void hAlphaLineUnclipped( int x1,int x2,unsigned char *l,unsigned char *srcdata,unsigned char *alphas);
	void processSpans( int n, QPoint* point, int* width );
	void paintCursor(const QImage& image, int hotx, int hoty, QPoint cursorPos);
	void buildSourceClut(QRgb * cols,int numcols);

    private:

	int four_bit_nibble;

};


QGfxVga16::QGfxVga16(unsigned char * a,int b,int c) : QGfxRasterBase(a,b,c)
{
//    setLineStep((depth*width+7)/8);
    UNREFERENCED_PARAMETER(a);
    UNREFERENCED_PARAMETER(b);
    UNREFERENCED_PARAMETER(c);
}


QGfxVga16::~QGfxVga16()
{
}


void QGfxVga16::setPixel( int x, int y, unsigned char color )
{
    set_pixel_4( scanLine(y), x, color );
}


unsigned char QGfxVga16::getPixel( int x, int y )
{
    return get_pixel_4( scanLine(y), x );
}


unsigned int QGfxVga16::get_pixel_4_32(volatile unsigned char *l, unsigned int x)
{
    unsigned char val = get_pixel_4(l, x);

    if ((l >= buffer) && (l <= (buffer + (640*480/2))))
	return clut[val];
    else
	return srcclut[val];
}


unsigned char QGfxVga16::get_pixel_4(volatile unsigned char *l, unsigned int x)
{
    // this gives a value of 4 or 0 if x is odd or even. Handy for shifting bits accordingly
    unsigned char shift = (x & 1) << 2;

    VGA16_CriticalSection cs();

    // translation of l[x] -> pixel_addr
    int linear_address = (l + (x >> 1)) - buffer;

    return ((screen_double_buffer[linear_address] >> shift) & 0x0F);
}


void QGfxVga16::set_pixel_4(volatile unsigned char *l, unsigned int x, unsigned char col)
{
    // don't bother setting the pixel if it's already the right colour

    if (!force_set_pixel)
    if (get_pixel_4(l, x) == col) {
	return;
    }

    // this gives a value of 4 or 0 if x is odd or even. Handy for shifting bits accordingly
    unsigned char shift = (x & 1) << 2;
    col &= 0x0F;

    VGA16_CriticalSection cs();

    // translation of l[x] -> pixel_addr
    int linear_address = (l + (x >> 1)) - buffer;
    int real_y = linear_address / 320;
    int real_x = ((linear_address % 320) << 1) + (x & 0x01);
    int real_offset = (real_y * (640/8)) + (real_x >> 3);

    // have to make buffer volatile for the compiler
    volatile unsigned char *pixel_addr = buffer + real_offset;

    if (set_back_buffer) {
    screen_double_buffer[linear_address] &= 0xF0 >> shift;
    screen_double_buffer[linear_address] |= col << shift;
    }

    // do once
    set_enable_set_reset_vga16(0x0F, 0);
    set_rotate_vga16(0x00, 0);
    set_mode_vga16(0x00, 0);
    set_write_planes_vga16(0x0F, 0);
    set_set_reset_vga16(col, 0);
    set_bit_mask_vga16(0x80 >> (real_x & 0x07), 0);
    *pixel_addr |= 0; //set_reset_temp;
}


void QGfxVga16::hline_4(volatile unsigned char *l, unsigned int x1, unsigned int x2, unsigned char col)
{
    if ((x2 - x1) < 17) {
	for (unsigned int x = x1; x <= x2; x++)
	    set_pixel_4( l, x, col );
	return;
    }

    // this gives a value of 4 or 0 if x is odd or even. Handy for shifting bits accordingly
    // unsigned char shift = (x1 & 1) << 2;
    col &= 0x0F;

    VGA16_CriticalSection cs();
    // translation of l[x] -> pixel_addr
    unsigned int linear_address1 = (l + (x1 >> 1)) - buffer;
    unsigned int real_y = linear_address1 / 320;
    // unsigned int real_x1 = ((linear_address1 % 320) << 1) + (x1 & 0x01);
    // unsigned int linear_address2 = (l + (x2 >> 1)) - buffer;
    // unsigned int real_x2 = ((linear_address2 % 320) << 1) + (x2 & 0x01);

    for (unsigned int x = x1; x <= x2; x++) {
	unsigned int linear_address = (l + (x >> 1)) - buffer;
	screen_double_buffer[linear_address] &= 0xF0 >> ((x & 1) << 2);
	screen_double_buffer[linear_address] |= col << ((x & 1) << 2);
    }

    // do once
    set_enable_set_reset_vga16(0x0F, 0);
    set_rotate_vga16(0x00, 0);
    set_mode_vga16(0x00, 0);
    set_write_planes_vga16(0x0F, 0);

    // do for each pixel
    set_set_reset_vga16(col, 0);

    unsigned char mask1 = 0xFF >> (x1 & 0x07);
    unsigned char mask2 = 0xFF << (0x07 - (x2 & 0x07));

    // if (x1 < (x1 & ~0x07) + 8) {
    {
	set_bit_mask_vga16(mask1, 0);
	unsigned int linear_address = (l + (x1 >> 1)) - buffer;
	unsigned int real_x = ((linear_address % 320) << 1) + (x1 & 0x01);
	unsigned int real_offset = (real_y * (640/8)) + (real_x >> 3);
	volatile unsigned char *pixel_addr = buffer + real_offset;
	*pixel_addr |= 0;
	
/*
	for (unsigned int x = x1; x <= ((x1 >> 3) << 3) + 8; x++) {
	    int linear_address = (l + (x >> 1)) - buffer;
	    int real_x = ((linear_address % 320) << 1) + (x & 0x01);
	    int real_offset = (real_y * (640/8)) + (real_x >> 3);
	    volatile unsigned char *pixel_addr = buffer + real_offset;
	    *pixel_addr |= 0; //set_reset_temp;
	}
*/
    }

    if ( ((x1 & ~0x07) + 8) <= ((x2 & ~0x07) - 8) ) {

	unsigned int x = (x1 & (~0x07)) + 8;
	set_bit_mask_vga16(0xFF, 0);
	unsigned int linear_address = (l + (x >> 1)) - buffer;
	unsigned int real_x = (linear_address % 320) << 1;
	unsigned int real_offset = (real_y * (640/8)) + (real_x >> 3);
	volatile unsigned char *pixel_addr = buffer + real_offset;
	
//	memset(pixel_addr, 0, ((x2 & ~0x07) - x) >> 3);

	for (; x <= ((x2 >> 3) << 3) - 8; x+=8) {
	    *pixel_addr |= 0; //set_reset_temp;
	    pixel_addr++;
	}


    }

    // if ((x2 & ~0x07) < x2) {
    {   	
	set_bit_mask_vga16(mask2, 0);
	unsigned int linear_address = (l + (x2 >> 1)) - buffer;
	unsigned int real_x = ((linear_address % 320) << 1) + (x2 & 0x01);
	unsigned int real_offset = (real_y * (640/8)) + (real_x >> 3);
	volatile unsigned char *pixel_addr = buffer + real_offset;
	*pixel_addr |= 0;

    /*
	for (unsigned int x = (x2 & ~0x07) - 8; x <= x2; x++) {
	    int linear_address = (l + (x >> 1)) - buffer;
	    int real_x = ((linear_address % 320) << 1) + (x & 0x01);
	    int real_offset = (real_y * (640/8)) + (real_x >> 3);
	    volatile unsigned char *pixel_addr = buffer + real_offset;
	    set_bit_mask_vga16(0x80 >> (real_x & 0x07), 0);
	    *pixel_addr |= 0; //set_reset_temp;
	}
    */
    }
}


extern unsigned int closestMatchUsingTable(int r,int g,int b);


void QGfxVga16::himageline_4(volatile unsigned char *l, unsigned int x1, unsigned int x2, unsigned char *srcdata, bool reverse = false)
{
    unsigned char SavedRow[640];
    // unsigned char ScreenRow[640];
    unsigned char MemCpyPixels[640];

//    if (srcdepth == 0) printf("image src depth = 0\n");

    unsigned int x = (!reverse) ? x1 : x2;
    unsigned int inc = (!reverse) ? 1 : -1;
    unsigned char gv = srccol;
    int w = x2 - x1 + 1;

    w = (w > 640) ? 640 : w;

    if ((srcdepth==4) && (!ismasking))
    {
	unsigned int val;
	unsigned int r, g, b;
	unsigned char shift = (four_bit_nibble == true) ? 4 : 0;

	if ((srcdata >= buffer) && (srcdata <= (buffer + (640*480/2)))) {

	    // This is the fast case for screen->screen blting
	
	    srcdata += screen_double_buffer - buffer;
	    while ( w-- ) {
		SavedRow[x] = ((*srcdata >> shift) & 0x0F);
		x += inc;
		shift = 4 - shift;
		srcdata += four_bit_nibble - reverse;
		four_bit_nibble = 1 - four_bit_nibble;
	    }

	    
	} else {
	    while ( w-- ) {
		val = srcclut[((*srcdata >> shift) & 0x0F)];
		r = (val & 0x00FF0000) >> 16;
		g = (val & 0x0000FF00) >>  8;
		b = (val & 0x000000FF) >>  0;
		SavedRow[x] = closestMatchUsingTable(r,g,b);
		//SavedRow[x] = QColor(r,g,b).alloc();
		x += inc;
		shift = 4 - shift;
		srcdata += four_bit_nibble - reverse;
		four_bit_nibble = 1 - four_bit_nibble;
	    }
	}
    } else if ((srcdepth==16) && (!ismasking)) {
	unsigned int r,g,b;
	while ( w-- ) {
	    unsigned short int hold=*((unsigned short int *)srcdata);
	    r=((hold & 0xf800) >> 11) << 3;
	    g=((hold & 0x07e0) >>  5) << 2;
	    b=((hold & 0x001f) >>  0) << 3;
	    srcdata += 2;
	    SavedRow[x] = closestMatchUsingTable(r,g,b);
	    x += inc;
	}
    } else if (srcdepth==8) {
	if ( !ismasking ) {
	    while ( w-- ) {
		if (src_normal_palette)
		    SavedRow[x] = *srcdata;
		else
		    SavedRow[x] = transclut[*srcdata];
		//SavedRow[x] = *srcdata;
		srcdata++;
		x += inc;
	    }
	} else {
	    while ( w-- ) {
		if ( srctype==SourceImage ) {
		    if (src_normal_palette)
		        gv = *srcdata;
		    else
		        gv = transclut[*srcdata];
		    // gv = *srcdata;
		    srcdata++;
		}
		bool masked = TRUE;
		GET_MASKED(reverse);
		SavedRow[x] = (!masked) ? gv : get_pixel_4( l, x );
		// SavedRow[x] = gv;
		x += inc;
	    }
	}
    } else {
	if ( !ismasking ) {
	    while ( w-- ) {
		unsigned int val = get_value_32(srcdepth,&srcdata,reverse);
		unsigned int r, g, b;
		r = (val & 0x00FF0000) >> 16;
		g = (val & 0x0000FF00) >>  8;
		b = (val & 0x000000FF) >>  0;
		// SavedRow[x] = QColor(r,g,b).alloc();
		SavedRow[x] = closestMatchUsingTable(r,g,b);
		x += inc;
	    }
	} else {
	    while ( w-- ) {
		if ( srctype==SourceImage ) {
		    unsigned int val = get_value_32(srcdepth,&srcdata,reverse);
		    unsigned int r, g, b;
		    r = (val & 0x00FF0000) >> 16;
		    g = (val & 0x0000FF00) >>  8;
		    b = (val & 0x000000FF) >>  0;
		    // gv = QColor(r,g,b).alloc();
		    gv = closestMatchUsingTable(r,g,b);
		}
		bool masked = TRUE;
		GET_MASKED(reverse);
		SavedRow[x] = (!masked) ? gv : get_pixel_4( l, x );
		x += inc;
	    }
	}
    }


    VGA16_CriticalSection cs();

    int new_x1 = x1 & (~0x07);
    int new_x2 = (x2 & (~0x07)) + 7;

    new_x1 = (new_x1 <   0) ?   0 : new_x1;
    new_x2 = (new_x2 > 639) ? 639 : new_x2;

    if (new_x1 < (int)x1)
	for (unsigned int x = new_x1; x < x1; x++) {
	    int linear_address = (l + (x >> 1)) - buffer;
	    SavedRow[x] = (screen_double_buffer[linear_address] >> ((x & 1) << 2)) & 0x0F;
	}

    if (new_x2 > (int)x2)
	for (int x = x2 + 1; x <= new_x2; x++) {
	    int linear_address = (l + (x >> 1)) - buffer;
	    SavedRow[x] = (screen_double_buffer[linear_address] >> ((x & 1) << 2)) & 0x0F;
	}

    if (new_x1 <= new_x2)
    {
	if (set_back_buffer)
	for (int x = new_x1; x <= new_x2; x++) {

	    unsigned int linear_address = (l + (x >> 1)) - buffer;
	    screen_double_buffer[linear_address] &= 0xF0 >> ((x & 1) << 2);
	    screen_double_buffer[linear_address] |= (SavedRow[x]&0x0F) << ((x & 1) << 2);
	}

	set_enable_set_reset_vga16(0x00, 0); // disable it
	set_rotate_vga16(0x00, 0);
	set_mode_vga16(0x00, 0);
	set_bit_mask_vga16(0xFF, 0);

	for (unsigned int plane = 0; plane < 4; plane++ ) {

	    unsigned char plane_mask = 0x01 << plane;

	    set_write_planes_vga16(plane_mask, 0);

	    volatile unsigned char *pixel_addr = buffer + ((l - buffer + (new_x1 >> 1)) >> 2);

#define MyFastShift(x, y) \
    (((x) << (y)) | ((x) >> -(y)))

	    unsigned int i = 0;
	    unsigned int src_pixels, temp_pixel;
	    for (int x = new_x1; x <= new_x2; x += 8) {
		src_pixels = 0;
		src_pixels |= (SavedRow[x+0] & plane_mask) << 7 - plane;
		src_pixels |= (SavedRow[x+1] & plane_mask) << 6 - plane;
		src_pixels |= (SavedRow[x+2] & plane_mask) << 5 - plane; // these shifts are always positive
		src_pixels |= (SavedRow[x+3] & plane_mask) << 4 - plane;
		src_pixels |= (SavedRow[x+4] & plane_mask) << 3 - plane;
		temp_pixel = SavedRow[x+5] & plane_mask;
		src_pixels |= MyFastShift(temp_pixel, 2 - plane);
		temp_pixel = SavedRow[x+6] & plane_mask;
		src_pixels |= MyFastShift(temp_pixel, 1 - plane); // these change
		temp_pixel = SavedRow[x+7] & plane_mask;
		src_pixels |= MyFastShift(temp_pixel, 0 - plane);
		MemCpyPixels[i] = src_pixels;
		i++;
	    }
	    memcpy((unsigned char *)pixel_addr, MemCpyPixels, i);
	}
    }
}


// Convert between pixel values for different depths
// reverse can only be true if sdepth == depth
GFX_INLINE unsigned int QGfxVga16::get_value_32(
		       int sdepth, unsigned char **srcdata, bool reverse = false)
{
    unsigned int ret;
    if(sdepth==32) {
	ret = *((unsigned int *)(*srcdata));
	if(reverse) {
	    (*srcdata)-=4;
	} else {
	    (*srcdata)+=4;
	}
    } else if(sdepth==16) {
	unsigned int r,g,b;
	unsigned short int hold=*((unsigned short int *)(*srcdata));
	r=(hold & 0xf800) >> 11;
	g=(hold & 0x07e0) >> 5;
	b=(hold & 0x001f);
	r=r << 3;
	g=g << 2;
	b=b << 3;
	ret = 0;
	unsigned char * tmp=(unsigned char *)&ret;
	*(tmp+2)=r;
	*(tmp+1)=g;
	*(tmp+0)=b;
	(*srcdata)+=2;
#ifndef QT_NO_QWS_DEPTH_15
    } else if(sdepth==15) {
	unsigned int r,g,b;
	unsigned short int hold=*((unsigned short int *)(*srcdata));
	r=(hold & 0x7c00) >> 11;
	g=(hold & 0x03e0) >> 5;
	b=(hold & 0x001f);
	r=r << 3;
	g=g << 3;
	b=b << 3;
	ret=(r << 16) | (g << 8) | b;
	(*srcdata)+=2;
#endif
    } else if(sdepth==8) {
	unsigned char val=*((*srcdata));
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	if(src_normal_palette) {
	    ret=((val >> 5) << 16)  | ((val >> 6) << 8) | (val >> 5);
	} else {
#else
	if(TRUE) {
#endif
	    ret = srcclut[val];
	}
	(*srcdata)++;
    } else if(sdepth==4) {
	ret = get_pixel_4_32((*srcdata), (four_bit_nibble == true) ? 1 : 0);
	if (!reverse) {
	    (*srcdata) += (four_bit_nibble == true) ? 1 : 0;
	} else {
	    (*srcdata) -= (four_bit_nibble == false) ? 1 : 0;
	}
	four_bit_nibble = (four_bit_nibble == true) ? false : true;
    } else if(sdepth==1) {
	if(monobitcount<8) {
	    monobitcount++;
	} else {
	    monobitcount=1;	// yes, 1 is correct
	    (*srcdata)++;
	    monobitval=*((*srcdata));
	}
	if(src_little_endian) {
	    ret=monobitval & 0x1;
	    monobitval=monobitval >> 1;
	} else {
	    ret=(monobitval & 0x80) >> 7;
	    monobitval=monobitval << 1;
	    monobitval=monobitval & 0xff;
	}
	ret=srcclut[ret];
    } else {
	qDebug("Odd source depth %d!",sdepth);
	ret=0;
    }

    return ret;
}


void QGfxVga16::paintCursor(const QImage& image, int hotx, int hoty, QPoint cursorPos)
{
#ifndef QT_NO_QWS_CURSOR
    if (QScreenCursor::enabled())
    {
	setSource(&image);
	setAlphaType(QGfx::InlineAlpha);
	blt(cursorPos.x()-hotx-1,
		cursorPos.y()-hoty-1,
		image.width(), image.height());
    }
#endif
}


GFX_INLINE void QGfxVga16::calcPacking(
			  void * m,int x1,int x2,
			  int & frontadd,int & backadd,int & count)
{
    UNREFERENCED_PARAMETER(m);
    
    frontadd = x2-x1+1;
    backadd = 0;
    count = 0;
    if(frontadd<0)
    	frontadd=0;
    return;
}

void QGfxVga16::setSource(const QPaintDevice * p)
{
    QPaintDeviceMetrics qpdm(p);
    srclinestep=((QPaintDevice *)p)->bytesPerLine();
    srcdepth=qpdm.depth();
    if(srcdepth==0)
	abort();
    srcbits=((QPaintDevice *)p)->scanLine(0);
    srctype=SourceImage;
    setAlphaType(IgnoreAlpha);
    if ( p->devType() == QInternal::Widget ) {
	QWidget * w=(QWidget *)p;
	srcwidth=w->width();
	srcheight=w->height();
	QPoint hold;
	hold=w->mapToGlobal(hold);
	setSourceWidgetOffset( hold.x(), hold.y() );
	if ( srcdepth == 1 ) {
	    buildSourceClut(0, 0);
	} else if(srcdepth <= 8) {
	    src_normal_palette=TRUE;
	}
    } else if ( p->devType() == QInternal::Pixmap ) {
	//still a bit ugly
	QPixmap *pix = (QPixmap*)p;
	setSourceWidgetOffset( 0, 0 );
	srcwidth=pix->width();
	srcheight=pix->height();
	if ( srcdepth == 1 ) {
	    buildSourceClut(0, 0);
	} else if(srcdepth <= 8) {
	    src_normal_palette=TRUE;
	}
    } else {
	// This is a bit ugly #### I'll say!
	//### We will have to find another way to do this
	setSourceWidgetOffset( 0, 0 );
	buildSourceClut(0,0);
    }

    src_little_endian=TRUE;
}

void QGfxVga16::setSource(const QImage * i)
{
    srctype=SourceImage;
    srclinestep=i->bytesPerLine();
    srcdepth=i->depth();
    if(srcdepth==0)
	abort();
    srcbits=i->scanLine(0);
    src_little_endian=(i->bitOrder()==QImage::LittleEndian);
    setSourceWidgetOffset( 0, 0 );
    srcwidth=i->width();
    srcheight=i->height();
    src_normal_palette=FALSE;
    if ( srcdepth == 1 )
	buildSourceClut( 0, 0 );
    else  if(srcdepth<=8)
	buildSourceClut(i->colorTable(),i->numColors());
}

void QGfxVga16::buildSourceClut(QRgb * cols,int numcols)
{
    if (!cols) {
	useBrush();
	srcclut[0]=pixel;
	transclut[0]=pixel;
	usePen();
	srcclut[1]=pixel;
	transclut[1]=pixel;
	return;
    }

    int loopc;

    // Copy clut
    for(loopc=0;loopc<numcols;loopc++)
	srcclut[loopc] = cols[loopc];

    for(loopc=0;loopc<numcols;loopc++) {
	int r = qRed(srcclut[loopc]);
	int g = qGreen(srcclut[loopc]);
	int b = qBlue(srcclut[loopc]);
	transclut[loopc] = QColor(r,g,b).alloc();
    }
}


GFX_INLINE void QGfxVga16::drawPointUnclipped( int x, unsigned char* l)
{
    if (myrop==XorROP) {
	    set_pixel_4(l, x, get_pixel_4(l, x) ^ pixel);
    } else if (myrop==NotROP) {
	    set_pixel_4(l, x, ~(get_pixel_4(l, x)));
    } else {
	    set_pixel_4(l, x, pixel & 0x0F);
    }
}


void QGfxVga16::drawPoint( int x, int y )
{
    if(cpen.style()==NoPen)
	return;
    x += xoffs;
    y += yoffs;
    if (inClip(x,y)) {
	usePen();
    VGA16_GFX_START(QRect(x,y,2,2))
	drawPointUnclipped( x, scanLine(y) );
    VGA16_GFX_END
    }
}


void QGfxVga16::drawPoints( const QPointArray & pa, int index, int npoints )
{
    if(cpen.style()==NoPen)
	return;
    usePen();
    QRect cr;
    bool in = FALSE;

    VGA16_GFX_START(clipbounds);
    while (npoints--) {
	int x = pa[index].x() + xoffs;
	int y = pa[index].y() + yoffs;
	if ( !cr.contains(x,y) ) {
	    in = inClip(x,y,&cr);
	}
	if ( in ) {
	    drawPointUnclipped( x, scanLine(y) );
	}
	++index;
    }
    VGA16_GFX_END
}

void QGfxVga16::drawLine( int x1, int y1, int x2, int y2 )
{
    if(cpen.style()==NoPen)
	return;

    if (cpen.width() > 1) {
	drawThickLine( x1, y1, x2, y2 );
	return;
    }

    usePen();
    x1+=xoffs;
    y1+=yoffs;
    x2+=xoffs;
    y2+=yoffs;

    if(x1>x2) {
	int x3;
	int y3;
	x3=x2;
	y3=y2;
	x2=x1;
	y2=y1;
	x1=x3;
	y1=y3;
    }

    int dx=x2-x1;
    int dy=y2-y1;

    VGA16_GFX_START(QRect(x1, y1 < y2 ? y1 : y2, dx+1, QABS(dy)+1))

#ifdef QWS_EXPERIMENTAL_FASTPATH
    // Fast path
    if (y1 == y2 && !dashedLines && ncliprect == 1) {
	if ( x1 > cliprect[0].right() || x2 < cliprect[0].left()
	     || y1 < cliprect[0].top() || y1 > cliprect[0].bottom() ) {
	    VGA16_GFX_END
	    return;
	}
	x1 = x1 > cliprect[0].left() ? x1 : cliprect[0].left();
	x2 = x2 > cliprect[0].right() ? cliprect[0].right() : x2;
	unsigned char *l = scanLine(y1);
	hlineUnclipped(x1,x2,l);
	VGA16_GFX_END
	return;
    }
#endif
    // Bresenham algorithm from Graphics Gems

    int ax=QABS(dx)*2;
    int ay=QABS(dy)*2;
    int sx=dx>0 ? 1 : -1;
    int sy=dy>0 ? 1 : -1;
    int x=x1;
    int y=y1;

    int d;

    QRect cr;
    bool inside = inClip(x,y,&cr);
    if(ax>ay && !dashedLines) {
	unsigned char* l = scanLine(y);
	d=ay-(ax >> 1);
	int px=x;
	#define FLUSH(nx) \
		if ( inside ) \
		    if ( sx < 1 ) \
			hlineUnclipped(nx,px,l); \
		    else \
			hlineUnclipped(px,nx,l); \
		px = nx+sx;
	for(;;) {
	    if(x==x2) {
		FLUSH(x);
		VGA16_GFX_END
		return;
	    }
	    if(d>=0) {
		FLUSH(x);
		y+=sy;
		d-=ax;
		l = scanLine(y);
		if ( !cr.contains(x+sx,y) )
		    inside = inClip(x+sx,y, &cr);
	    } else if ( !cr.contains(x+sx,y) ) {
		FLUSH(x);
		inside = inClip(x+sx,y, &cr);
	    }
	    x+=sx;
	    d+=ay;
	}
    } else if (ax > ay) {
	// cannot use hline for dashed lines
	int di = 0;
	int dc = dashedLines ? dashes[0] : 0;
	d=ay-(ax >> 1);
	for(;;) {
	    if ( !cr.contains(x,y))
		inside = inClip(x,y, &cr);
	    if ( inside && (di&0x01) == 0) {
		drawPointUnclipped( x, scanLine(y) );
	    }
	    if(x==x2) {
		VGA16_GFX_END
		return;
	    }
	    if (dashedLines && --dc <= 0) {
		if (++di >= numDashes)
		    di = 0;
		dc = dashes[di];
	    }
	    if(d>=0) {
		y+=sy;
		d-=ax;
	    }
	    x+=sx;
	    d+=ay;
	}
    } else {
	int di = 0;
	int dc = dashedLines ? dashes[0] : 0;
	d=ax-(ay >> 1);
	for(;;) {
	    // y is dominant so we can't optimise with hline
	    if ( !cr.contains(x,y))
		inside = inClip(x,y, &cr);
	    if ( inside && (di&0x01) == 0)
		drawPointUnclipped( x, scanLine(y) );
	    if(y==y2) {
		VGA16_GFX_END
		return;
	    }
	    if (dashedLines && --dc <= 0) {
		if (++di >= numDashes)
		    di = 0;
		dc = dashes[di];
	    }
	    if(d>=0) {
		x+=sx;
		d-=ay;
	    }
	    y+=sy;
	    d+=ax;
	}
    }
    VGA16_GFX_END
}


void QGfxVga16::drawThickLine( int x1, int y1, int x2, int y2 )
{
    QPointArray pa(5);
    int w = cpen.width() - 1;
    double a = atan2( y2 - y1, x2 - x1 );
    double ix = cos(a) * w / 2;
    double iy = sin(a) * w / 2;

    // No cap.
    pa[0].setX( x1 + iy );
    pa[0].setY( y1 - ix );
    pa[1].setX( x2 + iy );
    pa[1].setY( y2 - ix );
    pa[2].setX( x2 - iy );
    pa[2].setY( y2 + ix );
    pa[3].setX( x1 - iy );
    pa[3].setY( y1 + ix );

    pa[4] = pa[0];

    usePen();

    VGA16_GFX_START(clipbounds)
    scan(pa, FALSE, 0, 5);
    QPen savePen = cpen;
    cpen = QPen( cpen.color() );
    drawPolyline(pa, 0, 5);
    cpen = savePen;
    VGA16_GFX_END
}

//screen coordinates, clipped, x1<=x2
GFX_INLINE void QGfxVga16::hline( int x1,int x2,int y)
{
    unsigned char *l=scanLine(y);
    QRect cr;
    bool plot=inClip(x1,y,&cr);
    int x=x1;
    for (;;) {
	int xr = cr.right();
	if ( xr >= x2 ) {
	    if (plot)
		vga16_drawrect(x,y,x2,y,pixel);
//		hlineUnclipped(x,x2,l);
	    break;
	} else {
	    if (plot)
		vga16_drawrect(x,y,xr,y,pixel);
//		hlineUnclipped(x,xr,l);
	    x=xr+1;
	    plot=inClip(x,y,&cr,plot);
	}
    }
}


GFX_INLINE void QGfxVga16::hlineUnclipped( int x1,int x2,unsigned char* l)
{
    if( (myrop!=XorROP) && (myrop!=NotROP) ) {
    	hline_4( l, x1, x2, pixel );
    } else if(myrop==XorROP) {
    	for (int x = x1; x <= x2; x++)
    	    drawPointUnclipped( x, l );
    } else if(myrop==NotROP) {
    	for (int x = x1; x <= x2; x++)
    	    drawPointUnclipped( x, l );
    }
}

GFX_INLINE void QGfxVga16::hImageLineUnclipped( int x1,int x2,
						    unsigned char *l,
						    unsigned char *srcdata,
						    bool reverse)
{
	himageline_4( l, x1, x2, srcdata, reverse);
}

GFX_INLINE void QGfxVga16::hAlphaLineUnclipped( int x1,int x2,
						    unsigned char* l,
						    unsigned char * srcdata,
						    unsigned char * alphas)
{
	// First read in the destination line
	unsigned char *avp = alphas;
	unsigned char *srcptr = srcdata;
	unsigned int srcval = 0;
//	int loopc, i;
	unsigned int av;

	// SourcePen
	if (srctype != SourceImage)
	    srcval = clut[srccol];

	for (int loopc = 0, i = x1; i <= x2; loopc++, i++) {
	    if (srctype == SourceImage)
		srcval = get_value_32(srcdepth, &srcptr);

	    if (alphatype == InlineAlpha)
		av = srcval >> 24;
	    else if (alphatype == SolidAlpha)
		av = calpha;
	    else
		av = *(avp++);

	    if (av == 255)
		alphabuf[loopc] = srcval;
	    else if (av == 0)
		alphabuf[loopc] = get_pixel_4_32(l, i);
	    else {
		unsigned int screen_val = get_pixel_4_32(l, i);
	        int r2 = (screen_val & 0xff0000) >> 16,
	            g2 = (screen_val & 0x00ff00) >>  8,
	            b2 = (screen_val & 0x0000ff) >>  0;
		int r = (srcval & 0xff0000) >> 16,
		    g = (srcval & 0x00ff00) >>  8,
		    b = (srcval & 0x0000ff) >>  0;
		r = (((r - r2) * av) >> 8) + r2;
		g = (((g - g2) * av) >> 8) + g2;
		b = (((b - b2) * av) >> 8) + b2;
		alphabuf[loopc] = (r << 16) | (g << 8) | b;
	    }
	}

	int tmpdepth = srcdepth;
	srcdepth = 32;
	himageline_4(l,x1,x2,(unsigned char *)alphabuf,false);
	srcdepth = tmpdepth;
}

void QGfxVga16::drawRect( int rx,int ry,int w,int h )
{
    setAlphaType(IgnoreAlpha);
    ismasking=FALSE;
    if ( w <= 0 || h <= 0 ) return;

    VGA16_GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))

    if(cpen.style()!=NoPen) {
	drawLine(rx,ry,rx+(w-1),ry);
	drawLine(rx+(w-1),ry,rx+(w-1),ry+(h-1));
	drawLine(rx,ry+(h-1),rx+(w-1),ry+(h-1));
	drawLine(rx,ry,rx,ry+(h-1));
	rx++;
	ry++;
	w-=2;
	h-=2;
    }


#ifdef QWS_EXPERIMENTAL_FASTPATH
    // ### fix for 8bpp
    // This seems to be reliable now, at least for 16bpp

    if (ncliprect == 1 && cbrush.style()==SolidPattern) {
	// Fast path
	    useBrush();
	    int x1,y1,x2,y2;
	    rx+=xoffs;
	    ry+=yoffs;
	    x2=rx+w-1;
	    y2=ry+h-1;
	    if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
	       x2<cliprect[0].left() || y2<cliprect[0].top()) {
		VGA16_GFX_END
	        return;
	    }
	    x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
	    y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
	    x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
	    y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();

	    if((x2<x1) || (y2<y1)) {
		VGA16_GFX_END
		return;
	    }

	    vga16_drawrect(x1,y1,x2,y2,pixel);
/*
	    for(int loopc=y1;loopc<=y2;loopc++)
		hlineUnclipped(x1,x2,scanLine(loopc));
*/
	    VGA16_GFX_END
	    return;
    }
#endif // QWS_EXPERIMENTAL_FASTPATH

    if( (cbrush.style()!=QBrush::NoBrush) &&
	(cbrush.style()!=QBrush::SolidPattern) ) {
	srcwidth=cbrushpixmap->width();
	srcheight=cbrushpixmap->height();
	if(cbrushpixmap->depth()==1) {
	    if(opaque) {
		setSource(cbrushpixmap);
		setAlphaType(IgnoreAlpha);
		useBrush();
		srcclut[0]=pixel;
		QBrush tmp=cbrush;
		cbrush=QBrush(backcolor);
		useBrush();
		srcclut[1]=pixel;
		cbrush=tmp;
	    } else {
		useBrush();
		srccol=pixel;
		srctype=SourcePen;
		setAlphaType(LittleEndianMask);
		setAlphaSource(cbrushpixmap->scanLine(0),
			       cbrushpixmap->bytesPerLine());
	    }
	} else {
	    setSource(cbrushpixmap);
	    setAlphaType(IgnoreAlpha);
	}
//	setSourceOffset(0,0);
	tiledBlt(rx,ry,w,h);
    } else if(cbrush.style()!=NoBrush) {
	useBrush();
	rx += xoffs;
	ry += yoffs;
	// Gross clip
	if ( rx < clipbounds.left() ) {
	    w -= clipbounds.left()-rx;
	    rx = clipbounds.left();
	}
	if ( ry < clipbounds.top() ) {
	    h -= clipbounds.top()-ry;
	    ry = clipbounds.top();
	}
	if ( rx+w-1 > clipbounds.right() )
	    w = clipbounds.right()-rx+1;
	if ( ry+h-1 > clipbounds.bottom() )
	    h = clipbounds.bottom()-ry+1;
	if ( w > 0 && h > 0 ) {
//	    vga16_drawrect(rx,ry,rx+w-1,ry+h,pixel);
	    for (int j=0; j<h; j++,ry++)
		hline(rx,rx+w-1,ry);
	}
    }
    VGA16_GFX_END
}


void QGfxVga16::drawPolyline( const QPointArray &a,int index, int npoints )
{
    //int m=QMIN( index+npoints-1, int(a.size())-1 );
    VGA16_GFX_START(clipbounds)
    int loopc;
    int end;
    end=(index+npoints) > (int)a.size() ? a.size() : index+npoints;
    for(loopc=index+1;loopc<end;loopc++) {
	    drawLine(a[loopc-1].x(),a[loopc-1].y(),
		     a[loopc].x(),a[loopc].y());
    }
    // XXX beware XOR mode vertices
    VGA16_GFX_END
}


void QGfxVga16::drawPolygon( const QPointArray &pa, bool winding, int index, int npoints )
{
    useBrush();
    VGA16_GFX_START(clipbounds)
    if ( cbrush.style()!=QBrush::NoBrush )
	scan(pa,winding,index,npoints);
    drawPolyline(pa, index, npoints);
    if (pa[index] != pa[index+npoints-1]) {
	drawLine(pa[index].x(), pa[index].y(),
		pa[index+npoints-1].x(),pa[index+npoints-1].y());
    }
    VGA16_GFX_END
}


void QGfxVga16::processSpans( int n, QPoint* point, int* width )
{
    while (n--) {
	int x=point->x()+xoffs;
	if ( *width > 0 ) {
	    if(patternedbrush) {
		// XXX
	    } else {
//		vga16_drawrect(x,point->y()+yoffs,x+*width,point->y()+yoffs,pixel);
		hline(x,x+*width-1,point->y()+yoffs);
	    }
	}
	point++;
	width++;
    }
}

void QGfxVga16::scroll( int rx,int ry,int w,int h,int sx, int sy )
{
    if (!w || !h)
	return;

    int dy = sy - ry;
    int dx = sx - rx;

    if (dx == 0 && dy == 0)
	return;

    VGA16_GFX_START(QRect(QMIN(rx+xoffs,sx+xoffs), QMIN(ry+yoffs,sy+yoffs), w+QABS(dx)+1, h+QABS(dy)+1))

    srcbits=buffer;
    src_normal_palette = TRUE;
    srclinestep=linestep();
    srcdepth=4; // source is screen ??
    if(srcdepth==0)
	abort();
    srctype=SourceImage;
    setAlphaType(IgnoreAlpha);
    ismasking=FALSE;
    setSourceWidgetOffset( xoffs, yoffs );
    setSourceOffset(sx,sy);
    blt(rx,ry,w,h);

    VGA16_GFX_END
}

void QGfxVga16::blt( int rx,int ry,int w,int h )
{
    if ( !w || !h ) return;
    int osrcdepth=srcdepth;
    if(srctype==SourcePen) {
	srclinestep=0;//w;
	srcdepth=0;
	usePen();
    }

    rx += xoffs;
    ry += yoffs;
    QRect cr;

    QRect cursRect(rx, ry, w+1, h+1);

    VGA16_GFX_START(cursRect);

    int dl = linestep();
    int sl = srclinestep;
    int dj = 1;
    int dry = 1;
    int tj;
    int j;
    if(srcoffs.y() < ry) {
	// Back-to-front
	dj = -dj;
	dl = -dl;
	sl = -sl;
	dry = -dry;
	j = h-1;
	ry=(ry+h)-1;
	tj = -1;
    } else {
	j = 0;
	tj = h;
    }

    unsigned char *l = scanLine(ry);
    unsigned char *srcline = srcScanLine(j+srcoffs.y());

	if ( alphatype == InlineAlpha || alphatype == SolidAlpha ||
	     alphatype == SeparateAlpha ) {
	    alphabuf = new unsigned int[w];
	}

	// reverse will only ever be true if the source and destination
	// are the same buffer.
	bool reverse = srcoffs.y()==ry && rx>srcoffs.x() &&
			srctype==SourceImage && srcbits == buffer;

	if ( alphatype == LittleEndianMask || alphatype == BigEndianMask ) {
	    // allows us to optimise GET_MASK a little
	    amonolittletest = FALSE;
	    if( (alphatype==LittleEndianMask && !reverse) ||
		(alphatype==BigEndianMask && reverse) ) {
		amonolittletest = TRUE;
	    }
	}

	for (; j!=tj; j+=dj,ry+=dry,l+=dl,srcline+=sl) {
	    bool plot=inClip(rx,ry,&cr);
	    int x=rx;
	    for (;;) {
		int x2 = cr.right();
		if ( x2 > rx+w-1 ) {
		    x2 = rx+w-1;
		    if ( x2 < x ) break;
		}
		if (plot) {
		    unsigned char *srcptr;
		    if ( srctype == SourceImage ) {
			if ( srcdepth == 1) {
			    srcptr=find_pointer(srcbits,(x-rx)+srcoffs.x(),
					 j+srcoffs.y(), x2-x, srclinestep,
					 monobitcount, monobitval,
					 !src_little_endian, reverse);
			} else if ( reverse ) {
			    srcptr = srcline + (x2-rx+srcoffs.x())*srcdepth/8;
			    four_bit_nibble = (x2 - rx + srcoffs.x()) & 0x01;
			} else {
			    srcptr = srcline + (x-rx+srcoffs.x())*srcdepth/8;
			    four_bit_nibble = (x - rx + srcoffs.x()) & 0x01;
			}
		    }

		    switch ( alphatype ) {
		      case LittleEndianMask:
		      case BigEndianMask:
			maskp=find_pointer(alphabits,(x-rx)+srcoffs.x(),
					   j+srcoffs.y(), x2-x, alphalinestep,
					   amonobitcount,amonobitval,
					   alphatype==BigEndianMask, reverse);
			// Fall through
		      case IgnoreAlpha:
			hImageLineUnclipped(x,x2,l,srcptr,reverse);
			break;
		      case InlineAlpha:
		      case SolidAlpha:
			hAlphaLineUnclipped(x,x2,l,srcptr,0);
			break;
		      case SeparateAlpha:
			// Separate alpha table
			unsigned char * alphap=alphabits+(j*alphalinestep)
					       +(x-rx);
			hAlphaLineUnclipped(x,x2,l,srcptr,alphap);
		    }

		}
		if ( x >= rx+w-1 )
		    break;
		x=x2+1;
		plot=inClip(x,ry,&cr,plot);
	    }
	}
	if ( alphabuf ) {
	    delete [] alphabuf;
	    alphabuf = 0;
	}

    srcdepth=osrcdepth;
    VGA16_GFX_END
}


#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
void QGfxVga16::stretchBlt( int rx,int ry,int w,int h,int sw,int sh )
{
    UNREFERENCED_PARAMETER(rx);
    UNREFERENCED_PARAMETER(ry);
    UNREFERENCED_PARAMETER(w);
    UNREFERENCED_PARAMETER(h);
    UNREFERENCED_PARAMETER(sw);
    UNREFERENCED_PARAMETER(sh);
    qDebug("Can't cope with stretchblt in VGA16");
}
#endif


void QGfxVga16::tiledBlt( int rx,int ry,int w,int h )
{
    VGA16_GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))

    useBrush();
    unsigned char * savealphabits=alphabits;

    int offx = srcoffs.x();
    int offy = srcoffs.y();

    // from qpainter_qws.cpp
    if ( offx < 0 )
        offx = srcwidth - -offx % srcwidth;
    else
        offx = offx % srcwidth;
    if ( offy < 0 )
        offy = srcheight - -offy % srcheight;
    else
        offy = offy % srcheight;

    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = ry;
    yOff = offy;
    while( yPos < ry + h ) {
        drawH = srcheight - yOff;    // Cropping first row
        if ( yPos + drawH > ry + h )        // Cropping last row
            drawH = ry + h - yPos;
        xPos = rx;
        xOff = offx;
        while( xPos < rx + w ) {
            drawW = srcwidth - xOff; // Cropping first column
            if ( xPos + drawW > rx + w )    // Cropping last column
                drawW = rx + w - xPos;
	    setSourceOffset(xOff, yOff);
	    blt(xPos, yPos, drawW, drawH);
	    alphabits=savealphabits;
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
    
    VGA16_GFX_END
}


class QVga16Screen : public QLinuxFbScreen {

public:

    QVga16Screen( int display_id );
    virtual ~QVga16Screen();
    virtual bool connect( const QString &spec, char *,
			    unsigned char * config );
    virtual bool initCard();
    virtual int initCursor(void*, bool);
    virtual void shutdownCard();
    virtual bool useOffscreen() { return true; }
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual int alloc(unsigned int, unsigned int, unsigned int);

protected:

    virtual int pixmapOffsetAlignment() { return 128; }
    virtual int pixmapLinestepAlignment() { return 128; }

};


QVga16Screen::QVga16Screen( int display_id )
    : QLinuxFbScreen( display_id )
{
}

static int VGA16DummyOpType;

bool QVga16Screen::connect( const QString &displaySpec, char *,
			    unsigned char * config )
{
    bool ret = TRUE;
    
    UNREFERENCED_PARAMETER(config);
    
    if ( !QLinuxFbScreen::connect( displaySpec ) )
	ret = FALSE;

    optype = &VGA16DummyOpType;

    if (screen_double_buffer == NULL)
	printf("error\n"), exit(0);
    printf("connected\n");

    unsigned char *db_line_ptr = screen_double_buffer;

    for (int i = 0; i < 480; i++)
    {
	db_line_ptrs[i] = db_line_ptr;
	db_line_ptr += 640/2;
    }
    
//    printf("QVga16Screen::connect - optype: %i, *optype: %i\n", optype, *optype);
    return ret;
}

QVga16Screen::~QVga16Screen()
{
}

bool QVga16Screen::initCard()
{
    QLinuxFbScreen::initCard();
    optype = &VGA16DummyOpType;
/*
    if (-1 == ioperm(0x3C0, 0x20, 1))
	    perror("io permissions to VGA registers");
    
    memset(vga_register_values, 0, 512);

    outw(0x0002, 0x3C4);
//	outw(0x0004, 0x3C4);
    outw(0x0604, 0x3C4); vga_register_values[0x44] = 0x06;
    outw(0x0000, 0x3CE);
    outw(0x0001, 0x3CE);
    outw(0x0003, 0x3CE);
    outw(0x0004, 0x3CE);
//	outw(0x0604, 0x3CE); vga_register_values[0xE4] = 0x06;
    outw(0x0005, 0x3CE);
    outw(0x0008, 0x3CE);
*/

//    printf("QVga16Screen::initcard - optype: %i, *optype: %i\n", optype, *optype);
    return true;
}

int QVga16Screen::initCursor(void* e, bool init)
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor=new QVga16Cursor();
    qt_screencursor->init((SWCursorData*)e - 1,init);
    return sizeof(SWCursorData*);
#endif
    return 0;
}

void QVga16Screen::shutdownCard()
{
    QLinuxFbScreen::shutdownCard();
}

int QVga16Screen::alloc(unsigned int r, unsigned int g, unsigned int b)
{
    return QColor(r,g,b).alloc();
}

QGfx * QVga16Screen::createGfx(unsigned char * b,int w,int h,int d_arg,int linestep)
{
/*
    printf("Buffer: %x width: %i, height: %i, depth: %i, linestep: %i\n",
	b, w, h, d_arg, linestep);
*/  
    if (d_arg == 4) {
	d = d_arg;
    	QGfx *ret = new QGfxVga16(b,w,h);

	unsigned char *fb_line_ptr = b;

	for (int i = 0; i < 480; i++)
	{
		fb_line_ptrs[i] = fb_line_ptr;
		fb_line_ptr += 640/8;
	}
	
	if (ret) {
    	    ret->setLineStep(linestep);
    	    return ret;
       	}
    }
    
    return QLinuxFbScreen::createGfx(b,w,h,d_arg,linestep);
}


extern "C" QScreen * qt_get_screen_vga16( int display_id, const char *spec,
					char * slot,unsigned char * config )
{
    if (!qt_screen && slot!=0) {
	QVga16Screen *ret = new QVga16Screen( display_id );
	if(ret->connect( spec, slot, config ))
	    qt_screen=ret;
    }
    if( !qt_screen ) {
	qt_screen=new QLinuxFbScreen( display_id );
	qt_screen->connect( spec );
    }
    return qt_screen;
}

#ifndef QT_NO_QWS_CURSOR


void QVga16Cursor::set(const QImage& image, int hotx, int hoty)
{
    QWSDisplay::grab( TRUE );
    restoreUnder();
    data->hotx = hotx;
    data->hoty = hoty;
    data->width = image.width();
    data->height = image.height();
    memcpy(data->cursor, image.bits(), image.numBytes());
    data->colors = image.numColors();
    int depth = gfx->bitDepth();
    if ( depth <= 8 ) {
	for (int i = 0; i < image.numColors(); i++) {
	    int r = qRed( image.colorTable()[i] );
	    int g = qGreen( image.colorTable()[i] );
	    int b = qBlue( image.colorTable()[i] );
	    data->translut[i] = QColor(r, g, b).pixel();
	}
    }
    for (int i = 0; i < image.numColors(); i++) {
	data->clut[i] = image.colorTable()[i];
    }
    data->bound = QRect( data->x - data->hotx, data->y - data->hoty,
		   data->width+1, data->height+1 );
    drawCursor(data->bound);
    QWSDisplay::ungrab();
}

void QVga16Cursor::move(int x, int y)
{
    QWSDisplay::grab( TRUE );

    restoreUnder();

    data->x = x;
    data->y = y;
    data->bound = QRect( data->x - data->hotx, data->y - data->hoty,
		   data->width+1, data->height+1 );
   
    drawCursor(data->bound);
    
    QWSDisplay::ungrab();
}

void QVga16Cursor::restoreUnder()
{
    if (!data || !data->enable)
	return;

//    QWSDisplay::grab( TRUE );

    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

    if ( data->width && data->height ) {

	QGfxVga16 *gfxVga = (QGfxVga16*)qt_screen->screenGfx();
	
//	set_back_buffer = false;
	force_set_pixel = true;

	if ((x > 0) && (x < 640) && (y > 0) && (y < 480)) {
	    for (int j = 0; j < data->height; j++) {
		for (int i = 0; i < data->width; i++) {
		    gfxVga->setPixel(x + i, y + j, 
	    		    gfxVga->getPixel(x + i, y + j));
		}
	    }
	}
	
	force_set_pixel = false;
//	set_back_buffer = true;
	
    }
}


void QVga16Cursor::drawCursor(QRect &r)
{
    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

    if (!r.intersects(data->bound))
	return;

    if ( data->width != cursor->width() || data->height != cursor->height() ) {
	delete cursor;
	cursor = new QImage( data->cursor, data->width, data->height, 8,
			 data->clut, data->colors, QImage::IgnoreEndian );
    }
    if ( data->width && data->height ) {
	qt_sw_cursor = FALSE;   // prevent recursive call from blt
	gfx->setSource( cursor );
	gfx->setSourceOffset(0, 0);
	gfx->setAlphaType(QGfx::InlineAlpha);

	set_back_buffer = false;
	force_set_pixel = true;
	gfx->blt(x,y,data->width,data->height);
	force_set_pixel = false;
	set_back_buffer = true;
	
	qt_sw_cursor = TRUE;
    }
}

#endif // QT_NO_QWS_CURSOR

