#include "qws.h"
#include "qwsevent.h"
#include "qwscommand.h"
#include "qwsutils.h"
#include "qws_cursor.h"

#include <qapplication.h>
#include <qgfx.h>
#include <qcolor.h>
#include <qpen.h>
#include <qbrush.h>

#include <stdlib.h>
#include <stdio.h>


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "qwsaccel.h"

#include <linux/fb.h>


static struct fb_var_screeninfo fb_vinfo;
static struct fb_fix_screeninfo fb_finfo;
static struct {
  int fbfd;
  long int screensize;
  uchar *fbp;
  int width;
  int height;
  int depth;
} qfb;

static bool fb_open = FALSE;

void QWSServer::openDisplay()
{
  fb_open = TRUE;
  qfb.fbp=0;
 /* Open the file for reading and writing */
  qfb.fbfd = open("/dev/fb0", O_RDWR);
  if (!qfb.fbfd) {
    printf("Error: cannot open framebuffer device.\n");
    exit(1);
  }
  printf("The framebuffer device was opened successfully.\n");

  /* Get fixed screen information */
  if (ioctl(qfb.fbfd, FBIOGET_FSCREENINFO, &fb_finfo)) {
    printf("Error reading fixed information.\n");
    exit(2);
  }

  /* Get variable screen information */
  if (ioctl(qfb.fbfd, FBIOGET_VSCREENINFO, &fb_vinfo)) {
    printf("Error reading variable information.\n");
    exit(3);
  }

  /* Figure out the size of the screen in bytes */
  qfb.screensize = fb_vinfo.xres * fb_vinfo.yres * fb_vinfo.bits_per_pixel / 8;

  qfb.width = fb_vinfo.xres;
  qfb.height = fb_vinfo.yres;
  qfb.depth = fb_vinfo.bits_per_pixel;
  /* Map the device to memory */
  qfb.fbp = (uchar *)mmap(0, qfb.screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
		     qfb.fbfd, 0);
  if ((int)qfb.fbp == -1) {
    printf("Error: failed to map framebuffer device to
memory.\n");
    exit(4);
  }
  printf("The framebuffer device was mapped to memory successfully.\n");

    // ### Is this the correct place for this?
    if (getenv("QWS_NOACCEL")==0 && probed_card==0) {
	probe_bus(fb_vinfo.xres,fb_vinfo.yres,fb_vinfo.bits_per_pixel);
    }

    if ( swidth || sheight ) {
	swidth = QMIN( swidth, qfb.width );
	sheight = QMIN( sheight, qfb.height );
    } else {
	swidth = qfb.width;
	sheight = qfb.height;
    }

#ifdef Q_WS_QWS
    gfx = QGfx::createRasterGfx( qfb.depth, qfb.fbp, qfb.width, qfb.height );
    Q_ASSERT( gfx );
#endif
}


void QWSServer::closeDisplay()
{
  munmap(qfb.fbp, qfb.screensize);
  close(qfb.fbfd);
}


//### should use QGfx
void QWSServer::paintServerRegion()
{

    if ( shmid == -1 ) {
	Q_ASSERT( fb_open );
/*	
	QRegion sr = serverRegion.intersect( QRegion(0,0,fb_vinfo.xres,
						     fb_vinfo.yres ));
	
	
	//### testcode - should paint properly
	uint col = fb_vinfo.bits_per_pixel == 32 ? 0x003000 : 0x0200;
	
	QMemArray<QRect> reg = sr.rects();
	
	for ( int i = 0;  i < int(reg.count());  i++ ) {
	    QRect r = reg[i];
	    for ( int y = r.top(); y <= r.bottom() ; y++ )
		for ( int x = r.left(); x <= r.right() ; x++ ) {
		    int l = (x+fb_vinfo.xoffset) * (fb_vinfo.bits_per_pixel/8)
			    + (y+fb_vinfo.yoffset) * fb_finfo.line_length;

		    if ( fb_vinfo.bits_per_pixel == 16 ) {
			ushort *fbp = (ushort*)qfb.fbp;
			*(fbp+l/sizeof(ushort)) = col;
		    } else if ( fb_vinfo.bits_per_pixel == 32 ) {
			uint *fbp = (uint*)qfb.fbp;
			*(fbp+l/sizeof(uint)) = col;
			col +=0x000200;
		    }
		}
	}
*/
	gfx->setSource(&cursor->image());
	gfx->setAlphaType(QGfx::InlineAlpha);
	gfx->blt(cursorPos.x()-cursor->hotSpot().x()-1,
		cursorPos.y()-cursor->hotSpot().y()-1,
                cursor->image().width(), cursor->image().height());
    }
}

void QWSServer::paintBackground( QRegion r )
{
    if ( shmid == -1 ) {
	Q_ASSERT ( fb_open );

	//### testcode - should paint properly
	uint col = fb_vinfo.bits_per_pixel == 32 ? 0x0030e0 : 0x0118;

/*
	QMemArray<QRect> reg = r.rects();
	
	for ( int i = 0;  i < int(reg.count());  i++ ) {
	    QRect r = reg[i];
	    for ( int y = r.top(); y <= r.bottom() ; y++ )
		for ( int x = r.left(); x <= r.right() ; x++ ) {
		    int l = (x+fb_vinfo.xoffset) * (fb_vinfo.bits_per_pixel/8)
			    + (y+fb_vinfo.yoffset) * fb_finfo.line_length;

		    if ( fb_vinfo.bits_per_pixel == 16 ) {
			ushort *fbp = (ushort*)qfb.fbp;
			*(fbp+l/sizeof(ushort)) = col;
		    } else if ( fb_vinfo.bits_per_pixel == 32 ) {
			uint *fbp = (uint*)qfb.fbp;
			*(fbp+l/sizeof(uint)) = col;
		    }
		}
	}
*/
	QMemArray<QRect> reg = r.rects();

	gfx->setPen(QPen::NoPen);
	gfx->setBrush(QBrush(QColor(col, col)));

	for ( int i = 0;  i < int(reg.count());  i++ ) {
	    QRect r = reg[i];
	    gfx->drawRect( r.left(), r.top(), r.width()+1, r.height() );
	}
    }
}
