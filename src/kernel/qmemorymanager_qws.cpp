/****************************************************************************
** $Id: $
**
** Implementation of Qt/Embedded memory management routines
**
** Created : 000101
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qmemorymanager_qws.h"
#include "qfontmanager_qws.h"
#include "qgfx_qws.h"
#include "qpaintdevice.h"
#include "qfontdata_p.h"
#include "qfile.h"
#include "qdir.h"

#if !defined(Q_OS_QNX6) && !defined(Q_OS_QNX4)
#define QT_USE_MMAP
#endif

#include <stdlib.h>

#ifdef QT_USE_MMAP
// for mmap
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#endif

#include "qgfx_qws.h"

#define FM_SMOOTH 1

static void getProcessedGlyph(QGlyph& tmp, QGlyph& g, bool smooth)
{
    if ( tmp.metrics->width && tmp.metrics->height &&
	 (qt_screen->isTransformed() ) ) {
	int depth = 1;
	int cols = 0;
	QImage::Endian end = QImage::BigEndian;
	if ( smooth ) {
	    depth = 8;
	    cols = 256;
	    end = QImage::IgnoreEndian;
	}
	QImage img( tmp.data, tmp.metrics->width, tmp.metrics->height,
		    depth, tmp.metrics->linestep, 0, cols, end );
	img = qt_screen->mapToDevice( img );
	g.metrics = tmp.metrics;
	g.data = new uchar [img.numBytes()];

	memcpy( g.data, img.bits(), img.numBytes() );
	g.metrics->linestep = img.bytesPerLine();
	delete [] tmp.data;
    } else {

#ifndef QT_NO_QWS_INTERLACE
	if ( tmp.metrics->width && tmp.metrics->height &&
	     qt_screen->isInterlaced() ) {
	    int height = tmp.metrics->height + 1;
	    int width = tmp.metrics->width;
	    int pitch = tmp.metrics->linestep;
	    if ( !smooth ) {
		pitch=(width+3) & -4;
	    }
	    uchar *newdata = new uchar[height*pitch];
	    uchar *olddata = tmp.data;
	    memset(newdata,0,height*pitch);

	    if (smooth) {
		for(int y=0; y<height; y++) {
		    unsigned char *src1 = y == height-1 ? 0 :olddata+y*pitch;
		    unsigned char *src2 = y == 0 ? 0 :olddata+(y-1)*pitch;
		    unsigned char *dst = newdata+y*pitch;

		    for(int x=0;x<width;x++) {
			*dst++ = (2*(src1?*src1++:0) + (src2?*src2++:0))/3;
		    }
		}
	    } else {
		QRgb colTable[2] = {0, 1};
		QImage img( tmp.data, width, height,
			    1, tmp.metrics->linestep, 
			    colTable, 2, QImage::BigEndian);
		for(int y=0; y<height; y++) {
		    unsigned char *dst = newdata+y*pitch;

		    for(int x=0;x<width;x++) {
			*dst++ = ( y!=height-1 && img.pixel(x,y)? 170 :0)
			 + ( y!=0 && img.pixel(x,y-1)? 85 :0);
		    }
		}
	    
	
	    }
	
	    delete tmp.data;
	    tmp.data = newdata;
	    tmp.metrics->height = height;
	    tmp.metrics->linestep = pitch;
	}	      
#endif //QT_NO_QWS_INTERLACE	    
	g = tmp;
    }
}

class QGlyphTree {
    /* Builds up a tree like this:

       root
       /
       Q
       / \
       /   \
       G     l
       / \
       /   \
       /     \
       T       y
       \     /
       e   p
       / \
       /   \
       h     r

       etc.

       Which can be compressed into contiguous spans (when fuller):

       A-Z
       /   \
       0-9   a-z

       etc.

       such a compressed tree could then be stored in ROM.
    */
    QChar min,max;
    QGlyphTree* less;
    QGlyphTree* more;
    QGlyph* glyph;
public:
#ifdef QT_USE_MMAP
    QGlyphTree(uchar*& data)
    {
	read(data);
    }
#else
    QGlyphTree(QIODevice& f)
    {
	read(f);
    }
#endif

    QGlyphTree(const QChar& from, const QChar& to, QRenderedFont* renderer) :
	min(from),
	max(to),
	less(0),
	more(0)
    {
	int n = max.unicode()-min.unicode()+1;
	glyph = new QGlyph[n];
	for (int i=0; i<n; i++) {
	    QChar ch(min.unicode()+i);
	    QGlyph tmp = renderer->render(ch);
	    getProcessedGlyph(tmp,glyph[i],renderer->smooth);
	}
    }

    ~QGlyphTree()
    {
	// NOTE: does not delete glyph[*].metrics or .data.
	//       the caller does this (only they know who owns
	//	 the data).  See clear().
	delete less;
	delete more;
	delete [] glyph;
    }

    void clear()
    {
	if ( less ) less->clear();
	if ( more ) more->clear();
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    if ( glyph[i].data && !(glyph[i].metrics->flags & QGlyphMetrics::RendererOwnsData) ) {
		delete [] glyph[i].data;
	    }
	    delete glyph[i].metrics;
	}
    }

    bool inFont(const QChar& ch) const
    {
	if ( ch < min ) {
	    if ( !less )
		return FALSE;
	    return less->inFont(ch);
	} else if ( ch > max ) {
	    if ( !more )
		return FALSE;
	    return more->inFont(ch);
	}
	return TRUE;
    }

    QGlyph* get(const QChar& ch, QRenderedFont* renderer)
    {
	if ( ch < min ) {
	    if ( !less ) {
		if ( !renderer || !renderer->inFont(ch) )
		    return 0;
		less = new QGlyphTree(ch,ch,renderer);
	    }
	    return less->get(ch,renderer);
	} else if ( ch > max ) {
	    if ( !more ) {
		if ( !renderer || !renderer->inFont(ch) )
		    return 0;
		more = new QGlyphTree(ch,ch,renderer);
	    }
	    return more->get(ch,renderer);
	}
	return &glyph[ch.unicode()-min.unicode()];
    }
    int totalChars() const
    {
	if ( !this ) return 0;
	return max.unicode()-min.unicode()+1 + less->totalChars() + more->totalChars();
    }
    int weight() const
    {
	if ( !this ) return 0;
	return 1 + less->weight() + more->weight();
    }
    static void balance(QGlyphTree*& root)
    {
	int x,y;
	(void)balance(root,x,y);
    }
    static int balance(QGlyphTree*& root, int& l, int& m)
    {
	if ( root ) {
	    int ll, lm, ml, mm;
	    l = balance(root->less,ll,lm);
	    m = balance(root->more,ml,mm);

	    if ( root->more ) {
		if ( l + ml + 1 < mm ) {
		    // Shift less-ward
		    QGlyphTree* b = root;
		    QGlyphTree* c = root->more;
		    root = c;
		    b->more = c->less;
		    c->less = b;
		}
	    }
	    if ( root->less ) {
		if ( m + lm + 1 < ll ) {
		    // Shift more-ward
		    QGlyphTree* c = root;
		    QGlyphTree* b = root->less;
		    root = b;
		    c->less = b->more;
		    b->more = c;
		}
	    }
	    return 1 + l + m;
	} else {
	    l = m = 0;
	    return 0;
	}
    }
    void compress();

    void write(QIODevice& f)
    {
	writeNode(f);
	writeMetrics(f);
	writeData(f);
    }


    void dump(int indent=0)
    {
	for (int i=0; i<indent; i++) printf(" ");
	printf("%d..%d",min.unicode(),max.unicode());
	//if ( indent == 0 )
	printf(" (total %d)",totalChars());
	printf("\n");
	if ( less ) less->dump(indent+1);
	if ( more ) more->dump(indent+1);
    }

private:
    QGlyphTree()
    {
    }

    void writeNode(QIODevice& f)
    {
	char t[4];
	t[0] = min.row(); t[1]=min.cell();
	t[2] = max.row(); t[3]=max.cell();
	f.writeBlock(t,4);
	int flags = 0;
	if ( less ) flags |= 1;
	if ( more ) flags |= 2;
	f.putch(flags);
	if ( less ) less->writeNode(f);
	if ( more ) more->writeNode(f);
    }

    void writeMetrics(QIODevice& f)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++)
	    f.writeBlock((char*)glyph[i].metrics, sizeof(QGlyphMetrics));
	if ( less ) less->writeMetrics(f);
	if ( more ) more->writeMetrics(f);
    }

    void writeData(QIODevice& f)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
	    s = qt_screen->mapToDevice( s );
	    uint datasize = glyph[i].metrics->linestep * s.height();
	    f.writeBlock((char*)glyph[i].data, datasize);
	}
	if ( less ) less->writeData(f);
	if ( more ) more->writeData(f);
    }

#ifdef QT_USE_MMAP
    void read(uchar*& data)
    {
	// All node data first
	readNode(data);
	// Then all non-video data
	readMetrics(data);
	// Then all video data
	readData(data);
    }
#else
    void read(QIODevice& f)
    {
	// All node data first
	readNode(f);
	// Then all non-video data
	readMetrics(f);
	// Then all video data
	readData(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readNode(uchar*& data)
    {
	uchar rw = *data++;
	uchar cl = *data++;
	min.unicode() = (rw << 8) | cl;
	rw = *data++;
	cl = *data++;
	max.unicode() = (rw << 8) | cl;
	int flags = *data++;
	if ( flags & 1 )
	    less = new QGlyphTree;
	else
	    less = 0;
	if ( flags & 2 )
	    more = new QGlyphTree;
	else
	    more = 0;
	int n = max.unicode()-min.unicode()+1;
	glyph = new QGlyph[n];

	if ( less )
	    less->readNode(data);
	if ( more )
	    more->readNode(data);
    }
#else
    void readNode(QIODevice& f)
    {
	uchar rw = f.getch();
	uchar cl = f.getch();
	min.unicode() = (rw << 8) | cl;
	rw = f.getch();
	cl = f.getch();
	max.unicode() = (rw << 8) | cl;
	int flags = f.getch();
	if ( flags & 1 )
	    less = new QGlyphTree;
	else
	    less = 0;
	if ( flags & 2 )
	    more = new QGlyphTree;
	else
	    more = 0;
	int n = max.unicode()-min.unicode()+1;
	glyph = new QGlyph[n];

	if ( less )
	    less->readNode(f);
	if ( more )
	    more->readNode(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readMetrics(uchar*& data)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    glyph[i].metrics = (QGlyphMetrics*)data;
	    data += sizeof(QGlyphMetrics);
	}
	if ( less )
	    less->readMetrics(data);
	if ( more )
	    more->readMetrics(data);
    }
#else
    void readMetrics(QIODevice& f)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    glyph[i].metrics = new QGlyphMetrics;
	    f.readBlock((char*)glyph[i].metrics, sizeof(QGlyphMetrics));
	}
	if ( less )
	    less->readMetrics(f);
	if ( more )
	    more->readMetrics(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readData(uchar*& data)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
	    s = qt_screen->mapToDevice( s );
	    uint datasize = glyph[i].metrics->linestep * s.height();
	    glyph[i].data = data; data += datasize;
	}
	if ( less )
	    less->readData(data);
	if ( more )
	    more->readData(data);
    }
#else
    void readData(QIODevice& f)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
	    s = qt_screen->mapToDevice( s );
	    uint datasize = glyph[i].metrics->linestep * s.height();
	    glyph[i].data = new uchar[datasize]; // ### deleted?
	    f.readBlock((char*)glyph[i].data, datasize);
	}
	if ( less )
	    less->readData(f);
	if ( more )
	    more->readData(f);
    }
#endif

    static QGlyph* concatGlyphs(QGlyphTree* a, QGlyphTree* b, QChar& min, QChar& max)
    {
	int n = b->max.unicode()-a->min.unicode()+1;
	int n_a = a->max.unicode()-a->min.unicode()+1;
	QGlyph *newglyph = new QGlyph[n];
	int i=0;
	for (; i<n_a; i++)
	    newglyph[i] = a->glyph[i];
	for (; i<n; i++)
	    newglyph[i] = b->glyph[i-n_a];
	min = a->min;
	max = b->max;
	return newglyph;
    }
};

void QGlyphTree::compress()
{
    // XXX Does not compress as much as is possible

    if ( less ) {
	less->compress();
	if (less->max.unicode() == min.unicode()-1) {
	    // contiguous with me.
	    QGlyph *newglyph = concatGlyphs(less,this,min,max);
	    QGlyphTree* t = less->less; less->less = 0;
	    delete less; less = t;
	    delete [] glyph;
	    glyph = newglyph;
	}
    }

    if ( more ) {
	more->compress();
	if (more->min.unicode() == max.unicode()+1) {
	    // contiguous with me.
	    QGlyph *newglyph = concatGlyphs(this,more,min,max);
	    QGlyphTree* t = more->more; more->more = 0;
	    delete more; more = t;
	    delete [] glyph;
	    glyph = newglyph;
	}
    }
}

class QMemoryManagerFont : public QShared {
    QGlyph* default_glyph;
public:
    QMemoryManagerFont() : default_glyph(0) { }
    ~QMemoryManagerFont()
    {
	if ( default_glyph ) {
	    delete default_glyph->metrics;
	    if ( default_glyph->data )
		delete [] default_glyph->data;
	    delete default_glyph;
	}
	if ( tree ) {
	    if ( renderer ) {
		tree->clear();
		qt_fontmanager->uncache(renderer);
	    }
	    delete tree;
	}
    }

    QGlyph* defaultGlyph()
    {
	if ( !default_glyph ) {
	    QGlyphMetrics* m = new QGlyphMetrics;
	    memset((char*)m, 0, sizeof(QGlyphMetrics));
	    m->width = fm.maxwidth;
	    m->linestep = (fm.flags & FM_SMOOTH) ? m->width : (m->width+7)/8;
	    m->height = fm.ascent;
	    m->bearingy = fm.ascent;
	    m->advance = m->width+1+m->width/8;
	    uchar* d = new uchar[m->linestep*m->height];
	    memset(d,255,m->linestep);
	    memset(d+m->linestep*(m->height-1),255,m->linestep);
	    for (int i=1; i<m->height-1; i++) {
		int j=m->linestep*i;
		memset(d+j,0,m->linestep);
		if ( fm.flags & FM_SMOOTH ) {
		    d[j]=255;
		    d[j+m->linestep-1]=255;
		} else {
		    d[j]=128;
		    d[j+m->linestep-1]|=128>>((m->width+7)%8);
		}
	    }
	    QGlyph g(m,d);
	    default_glyph = new QGlyph;
	    getProcessedGlyph(g,*default_glyph,fm.flags & FM_SMOOTH);
	}
	return default_glyph;
    }

    QFontDef def;
    QGlyphTree* tree;
    QRenderedFont* renderer; // ==0 for QPFs

    struct Q_PACKED {
	Q_INT8 ascent,descent;
	Q_INT8 leftbearing,rightbearing;
	Q_UINT8 maxwidth;
	Q_INT8 leading;
	Q_UINT8 flags;
	Q_UINT8 underlinepos;
	Q_UINT8 underlinewidth;
	Q_UINT8 reserved3;
    } fm;
};

// ####### We don't use this info yet
void* qt_unused_vram;
int qt_unused_vramsize;
void* qt_unused_fontrom;

QMemoryManager::QMemoryManager(
    void* vram, int vramsize,
    void* fontrom
    //, ...
) :
    next_pixmap_id(1000),
    next_font_id(1000)
{
    qt_unused_vram = vram;
    qt_unused_vramsize = vramsize;
    qt_unused_fontrom = fontrom;
}


//### must follow QWS_PACKING_4BYTE in qgfxraster_qws.cpp
#ifdef __i386__
static const int memAlign = 64;
#else
static const int memAlign = 32;
#endif

inline static const int calcLineStep( int w, int d, bool vram )
{
    int align = vram ? qt_screen->pixmapLinestepAlignment() : memAlign;
    return ((w*d+align-1)/align)*align/8;
}

// Pixmaps
QMemoryManager::PixmapID QMemoryManager::newPixmap(int w, int h, int d,
						   int optim )
{
    uchar* data;
    PixmapID id;

    int xoffset;

    const int test_offset=0;

    int siz = calcLineStep( w, d, TRUE ) * h;

    // Find space in vram.

    data=qt_screen->cache(siz, optim);

    if ( data ) {
	xoffset = 0; // XXX

	// use an ODD next_pixmap_id
	id = ++next_pixmap_id;
	next_pixmap_id++; // stay even
	//	qDebug( "pixmap in VRAM %d", id );
    } else { 	// No vram left - use main memory
	// Possibly different alignment if it's in main ram
	siz = calcLineStep( w, d, FALSE ) * h;

	xoffset = test_offset; // for testing

	data = (uchar*)malloc(siz);
	if ( !data )
	    return 0; //out of memory
	// even id
	id = next_pixmap_id;
	next_pixmap_id += 2; // stay even
    }

    memset((void *)data,0,siz);

    QMemoryManagerPixmap mmp;
    mmp.data = data;
    mmp.xoffset = xoffset;
    pixmap_map[id]=mmp;

    return id;
}

bool QMemoryManager::inVRAM(PixmapID id) const
{
    return id & 1;
}

void QMemoryManager::deletePixmap(PixmapID id)
{
    if ( !id ) return; // C++-life
    QMap<PixmapID,QMemoryManagerPixmap>::Iterator it = pixmap_map.find(id);
    if(id & 1) {
	qt_screen->uncache((*it).data);
    } else {
	free((*it).data);
    }
    pixmap_map.remove(it);
}

void QMemoryManager::findPixmap(PixmapID id, int width, int depth, uchar** address, int* xoffset, int* linestep)
{
    QMap<PixmapID,QMemoryManagerPixmap>::Iterator it = pixmap_map.find(id);
    *address = (*it).data;
    *xoffset = (*it).xoffset;

    *linestep = calcLineStep( width, depth, id&1 );
}

// Fonts

static QString fontKey(const QFontDef& font)
{
    QString key = font.family.lower();

    key += "_";
    key += QString::number(font.pointSize);
    key += "_";
    key += QString::number(font.weight);
    key += font.italic ? "i" : "";
    if ( qt_screen->isTransformed() ) {
	key += "_t";
	QPoint a = qt_screen->mapToDevice(QPoint(0,0),QSize(2,2));
	QPoint b = qt_screen->mapToDevice(QPoint(1,1),QSize(2,2));
	key += QString::number( a.x()*8+a.y()*4+(1-b.x())*2+(1-b.y()) );
    }
    if ( qt_screen->isInterlaced() ) {
	key += "_I";
    }
    return key;
}

static QFontDef fontFilenameDef( const QString& key )
{
    // font name has the form of
    // (/w)+_(\d)+_(\d)+(i|) the rest of it we don't care about.
    QFontDef result;
    int u0 = key.find('_');
    int u1 = key.find('_',u0+1);

    // next field will be deleminted by either a _, . or i.
    // find the first non-diget
    int u2 = u1+1;
    while(key[u2].isDigit())
	u2++;

    result.family = key.left(u0);
    result.pointSize = key.mid(u0+1,u1-u0-1).toInt();
    result.weight = key.mid(u1+1,u2-u1-1).toInt();
    result.italic = key[u2] == 'i';
    // #### ignores _t and _I fields

    return result;
}

extern QString qws_topdir();

static QString fontDir()
{
    return qws_topdir()+"/lib/fonts/";
}

static QString fontFilename(const QFontDef& font)
{
    return fontDir()+fontKey(font)+".qpf";
}

QMemoryManager::FontID QMemoryManager::refFont(const QFontDef& font)
{
    QString key = fontKey(font);

    // look it up...

    QMap<QString,FontID>::ConstIterator i = font_map.find(key);
    QMemoryManagerFont* mmf;
    if ( i == font_map.end() ) {
	mmf = new QMemoryManagerFont;
	mmf->def = font;
	mmf->tree = 0;
	mmf->renderer = 0;
	QString filename = fontFilename(font);
	if ( !QFile::exists(filename) ) {
	    filename = QString::null;
            int bestmatch = -999;
	    QDiskFont* qdf = qt_fontmanager->get(font);
#ifndef QT_NO_DIR
	    // QPF close-font matching
	    if ( qdf )
	        bestmatch = QFontManager::cmpFontDef(font, qdf->fontDef());
	    QDir dir(fontDir(),"*.qpf");
	    for (int i=0; i<(int)dir.count(); i++) {
	        QFontDef d = fontFilenameDef(dir[i]);
	        int match = QFontManager::cmpFontDef(font,d);
	        if ( match >= bestmatch ) {
		    QString ff = fontFilename(d);
		    if ( QFile::exists(ff) ) {
		        filename = ff;
		        bestmatch = match;
		    }
	        }
	    }
#endif
	    if ( filename.isNull() ) {
	        if ( qdf ) {
		    mmf->renderer = qdf->load(font);
	        } else {
		    // last-ditch attempt to find a font...
		    QFontDef d = font;
		    d.family = "helvetica";
		    filename = fontFilename(d);
		    if ( !QFile::exists(filename) ) {
		        d.pointSize = 120;
		        filename = fontFilename(d);
		        if ( !QFile::exists(filename) ) {
		            d.italic = FALSE;
		            filename = fontFilename(d);
		            if ( !QFile::exists(filename) ) {
		                d.weight = 50;
		                filename = fontFilename(d);
		            }
		        }
		    }
	        }
	    }
	}
	if ( !mmf->renderer && QFile::exists(filename) ) {
#if defined(QT_USE_MMAP)
	    int f = ::open( QFile::encodeName(filename), O_RDONLY );
	    Q_ASSERT(f>=0);
	    struct stat st;
	    if ( fstat( f, &st ) )
		qFatal("Failed to stat %s",QFile::encodeName(filename).data());
	    uchar* data = (uchar*)mmap( 0, // any address
		    st.st_size, // whole file
                    PROT_READ, // read-only memory
#if !defined(Q_OS_SOLARIS) && !defined(Q_OS_QNX4)
                    MAP_FILE | MAP_PRIVATE, // swap-backed map from file
#else
                    MAP_PRIVATE,
#endif
                    f, 0 ); // from offset 0 of f
#if defined(Q_OS_QNX4) && !defined(MAP_FAILED)
#define MAP_FAILED ((void *)-1)
#endif
	    if ( !data || data == (uchar*)MAP_FAILED )
		qFatal("Failed to mmap %s",QFile::encodeName(filename).data());
	    ::close(f);
	    memcpy((char*)&mmf->fm,data,sizeof(mmf->fm)); data += sizeof(mmf->fm);
	    mmf->tree = new QGlyphTree(data);
#else
	    QFile f(filename);
	    f.open(IO_ReadOnly);
	    f.readBlock((char*)&mmf->fm,sizeof(mmf->fm));
	    mmf->tree = new QGlyphTree(f);
#endif
	} else {
	    memset((char*)&mmf->fm, 0, sizeof(mmf->fm));
	    if(!mmf->renderer) {
		return (FontID)mmf;
	    }
	    mmf->fm.ascent = mmf->renderer->fascent;
	    mmf->fm.descent = mmf->renderer->fdescent;
	    mmf->fm.leftbearing = mmf->renderer->fleftbearing;
	    mmf->fm.rightbearing = mmf->renderer->frightbearing;
	    mmf->fm.maxwidth = mmf->renderer->fmaxwidth;
	    mmf->fm.leading = mmf->renderer->fleading;
	    mmf->fm.underlinepos = mmf->renderer->funderlinepos;
	    mmf->fm.underlinewidth = mmf->renderer->funderlinewidth;
	    mmf->fm.flags = (mmf->renderer->smooth||qt_screen->isInterlaced())
			    ? FM_SMOOTH : 0;
	}
	font_map[key] = (FontID)mmf;
#ifndef QT_NO_QWS_SAVEFONTS
	extern bool qws_savefonts;
	if ( qws_savefonts && mmf->renderer )
	    savePrerenderedFont(font);
#endif
    } else {
	mmf = (QMemoryManagerFont*)*i;
	mmf->ref();
    }
    return (FontID)mmf;
}

void QMemoryManager::derefFont(FontID id)
{
    QMemoryManagerFont* font = (QMemoryManagerFont*)id;
    if ( font->renderer ) {
        font->renderer->deref();
    } 
    if ( font->deref() ) {
	QString key = fontKey(font->def);
	font_map.remove(key);
	delete font;
    }
}

QRenderedFont* QMemoryManager::fontRenderer(FontID id)
{
    QMemoryManagerFont* font = (QMemoryManagerFont*)id;
    return font->renderer;
}

bool QMemoryManager::inFont(FontID id, const QChar& ch) const
{
    QMemoryManagerFont* font = (QMemoryManagerFont*)id;
    if ( font->renderer )
	return ch.unicode() <= font->renderer->maxchar && font->renderer->inFont(ch);
    else
	return font->tree->inFont(ch);
}

QGlyph QMemoryManager::lockGlyph(FontID id, const QChar& ch)
{
    QMemoryManagerFont* font = (QMemoryManagerFont*)id;
    if ( !font->tree ) {
	if(!font->renderer) {
	    if ( ch.isSpace() && ch != ' ' )
		return lockGlyph(id,' ');
	    return *(font->defaultGlyph());
	}
	QChar c = ch;
	if ( !font->renderer->inFont(c) )
	    c = ' '; // ### Hope this is inFont()
	font->tree = new QGlyphTree(c,c,font->renderer);
    }
    QGlyph* g = font->tree->get(ch,font->renderer);
    if ( !g ) {
	if ( ch.isSpace() && ch != ' ' )
	    return lockGlyph(id,' ');
	g = font->defaultGlyph();
    }
    return *g;
}

QGlyphMetrics* QMemoryManager::lockGlyphMetrics(FontID id, const QChar& ch)
{
    QGlyph g = lockGlyph(id,ch);
    return g.metrics;
}

void QMemoryManager::unlockGlyph(FontID, const QChar&)
{
}

#ifndef QT_NO_QWS_SAVEFONTS
void QMemoryManager::savePrerenderedFont(const QFontDef& f, bool all)
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)refFont(f);
    savePrerenderedFont((FontID)mmf,all);
}

void QMemoryManager::savePrerenderedFont(FontID id, bool all)
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;

    if ( !mmf->renderer ) {
	qWarning("Already a ROM font");
    } else {
	if ( !mmf->tree )
	    mmf->tree = new QGlyphTree(32,32,mmf->renderer); // 32 = " " - likely to be in the font
	if ( all ) {
	    int j=0;
	    //qDebug("Rendering %s",fontFilename(mmf->def).ascii());
	    for (int i=0; i<=mmf->renderer->maxchar; i++) {
		QChar ch((ushort)i);
		if ( mmf->renderer->inFont(ch) ) {
		    mmf->tree->get(ch,mmf->renderer);
		    if ( !(j++ & 0x3f)  ) {
			// XXX keep it from becoming degenerate - should be in QGlyphTree
			mmf->tree->compress();
			QGlyphTree::balance(mmf->tree);
		    }
		}
	    }
	}
	mmf->tree->compress();
	QGlyphTree::balance(mmf->tree);
	//qDebug("DUMP..."); mmf->tree->dump();
	QFile f(fontFilename(mmf->def));
	f.open(IO_WriteOnly);
	f.writeBlock((char*)&mmf->fm,sizeof(mmf->fm));
	mmf->tree->write(f);
    }
}
#endif
// Perhaps we should just return the struct?
//
bool QMemoryManager::fontSmooth(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.flags & FM_SMOOTH;
}
int QMemoryManager::fontAscent(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.ascent;
}
int QMemoryManager::fontDescent(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.descent;
}
int QMemoryManager::fontMinLeftBearing(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.leftbearing;
}
int QMemoryManager::fontMinRightBearing(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.rightbearing;
}
int QMemoryManager::fontLeading(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.leading;
}
int QMemoryManager::fontMaxWidth(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.maxwidth;
}
int QMemoryManager::fontUnderlinePos(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.underlinepos ? mmf->fm.underlinepos : 1;
}
int QMemoryManager::fontLineWidth(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.underlinewidth ? mmf->fm.underlinewidth : 1;
}


QMemoryManager* memorymanager=0;
