#include "qmemorymanager_qws.h"
#include "qfontmanager_qws.h"
#include "qpaintdevice.h"
#include "qfontdata_p.h"
#include "qfile.h"

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
    QGlyphTree(QIODevice& f)
    {
	read(f);
    }

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
	    glyph[i] = renderer->render(ch);
	}
    }

    ~QGlyphTree()
    {
	delete [] glyph;
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
		if ( !renderer )
		    return 0;
		less = new QGlyphTree(ch,ch,renderer);
	    }
	    return less->get(ch,renderer);
	} else if ( ch > max ) {
	    if ( !more ) {
		if ( !renderer )
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
    void compress()
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
	f.writeBlock((char*)&min, sizeof(QChar));
	f.writeBlock((char*)&max, sizeof(QChar));
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
	    uint datasize = glyph[i].metrics->linestep * glyph[i].metrics->height;
	    f.writeBlock((char*)glyph[i].data, datasize);
	}
	if ( less ) less->writeData(f);
	if ( more ) more->writeData(f);
    }

    void read(QIODevice& f)
    {
	// All node data first
	readNode(f);
	// Then all non-video data
	readMetrics(f);
	// Then all video data
	readData(f);
    }

    void readNode(QIODevice& f)
    {
	f.readBlock((char*)&min, sizeof(QChar));
	f.readBlock((char*)&max, sizeof(QChar));
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

    void readData(QIODevice& f)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    uint datasize = glyph[i].metrics->linestep * glyph[i].metrics->height;
	    glyph[i].data = new uchar[datasize];
	    f.readBlock((char*)glyph[i].data, datasize);
	}
	if ( less )
	    less->readData(f);
	if ( more )
	    more->readData(f);
    }

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


class QMemoryManagerFont {
    QGlyph* default_glyph;
public:
    QMemoryManagerFont() : default_glyph(0) { }
    ~QMemoryManagerFont()
    {
	delete default_glyph->metrics;
	delete [] default_glyph->data;
	delete default_glyph;
    }

    QGlyph* defaultGlyph()
    {
	if ( !default_glyph ) {
	    QGlyphMetrics* m = new QGlyphMetrics;
	    m->width = fm.maxwidth;
	    m->linestep = fm.smooth ? m->width : (m->width+7)/8;
	    m->height = fm.ascent;
	    m->padding = 0;
	    m->bearingx = 0;
	    m->advance = m->width+1+m->width/8;
	    m->bearingy = 0;
	    uchar* d = new uchar[m->linestep*m->height];
	    memset(d,255,m->linestep*m->height);
	    default_glyph = new QGlyph(m,d);
	}
	return default_glyph;
    }

    QFontDef def;
    QGlyphTree* tree;
    QRenderedFont* renderer; // ==0 for QPFs

    struct {
	Q_UINT8 ascent,descent;
	Q_INT8 leftbearing,rightbearing;
	Q_UINT8 maxwidth;
	Q_UINT8 leading;
	uint smooth:1;
    } fm;
};

QMemoryManager::QMemoryManager(
    void* vram, int vramsize,
    void* fontrom
    //, ...
) :
    next_pixmap_id(1000),
    next_font_id(1000)
{
}

// Pixmaps
QMemoryManager::PixmapID QMemoryManager::newPixmap(int w, int h, int d)
{
    uchar* data;
    PixmapID id;

    int xoffset;

    // Aggressively find space in vram.
    if ( 0 ) {
	data = 0; // XXX
	xoffset = 0; // XXX

	// use an ODD next_pixmap_id
	id = ++next_pixmap_id;
	next_pixmap_id++; // stay even
    } else {
	// No vram left - use main memory

	const int test_offset=0; // for testing - should be 0
	xoffset = test_offset; // for testing

	int siz = ((w+test_offset)*d+7)/8*h;
	data = new uchar[siz];

	// even id
	id = next_pixmap_id;
	next_pixmap_id += 2; // stay even
    }

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
    delete [] (*it).data;
    pixmap_map.remove(it);
}

void QMemoryManager::findPixmap(PixmapID id, int width, int depth, uchar** address, int* xoffset, int* linestep)
{
    QMap<PixmapID,QMemoryManagerPixmap>::Iterator it = pixmap_map.find(id);
    *address = (*it).data;
    *xoffset = (*it).xoffset;
    *linestep = (width*depth+7)/8;
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

    return key;
}
extern QString qws_topdir();
static QString fontFilename(const QFontDef& font)
{
    return qws_topdir()+"/etc/fonts/"+fontKey(font)+".qpf"; // "Qt Prerendered Font"
}

QMemoryManager::FontID QMemoryManager::findFont(const QFontDef& font)
{
    QString key = fontKey(font);

    // look it up...

    QMap<QString,FontID>::ConstIterator i = font_map.find(key);
    QMemoryManagerFont* mmf;
    if ( i == font_map.end() ) {
	mmf = new QMemoryManagerFont;
	mmf->def = font;
	mmf->tree = 0;
	QString filename = fontFilename(font);
	if ( !QFile::exists(filename) ) {
	    mmf->renderer = qt_fontmanager->get(font);
	    if ( !mmf->renderer ) {
		QFontDef d = font;
		d.family = "helvetica";
		filename = fontFilename(d);
		if ( !QFile::exists(filename) ) {
		    d.pointSize = 120;
		    filename = fontFilename(d);
		    if ( !QFile::exists(filename) ) {
			filename = qws_topdir()+"/etc/fonts/helvetica_120_50.qpf";
		    }
		}
	    }
	}
	if ( QFile::exists(filename) ) {
	    qWarning("Using prerendered font \"%s\"",filename.local8Bit().data());
	    QFile f(filename);
	    f.open(IO_ReadOnly);
	    mmf->renderer = 0;
	    f.readBlock((char*)&mmf->fm,sizeof(mmf->fm));
	    mmf->tree = new QGlyphTree(f);
	} else {
	    mmf->fm.ascent = mmf->renderer->fascent;
	    mmf->fm.descent = mmf->renderer->fdescent;
	    mmf->fm.leftbearing = mmf->renderer->fleftbearing;
	    mmf->fm.rightbearing = mmf->renderer->frightbearing;
	    mmf->fm.maxwidth = mmf->renderer->fmaxwidth;
	    mmf->fm.smooth = mmf->renderer->smooth;
	}
	font_map[key] = (FontID)mmf;

	extern bool qws_savefonts;
	if ( qws_savefonts && mmf->renderer )
	    savePrerenderedFont(font);
    } else {
	mmf = (QMemoryManagerFont*)*i;
    }
    return (FontID)mmf;
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
	return font->renderer->inFont(ch);
    else
	return font->tree->inFont(ch);
}

QGlyph QMemoryManager::lockGlyph(FontID id, const QChar& ch)
{
    QMemoryManagerFont* font = (QMemoryManagerFont*)id;
    if ( !font->tree ) {
	QChar c = ch;
	if ( !font->renderer->inFont(c) )
	    c = ' '; // ### Hope this is inFont()
	font->tree = new QGlyphTree(c,c,font->renderer);
    }
    QGlyph* g = font->tree->get(ch,font->renderer);
    if ( !g )
	g = font->defaultGlyph();
    return *g;
}

QGlyphMetrics* QMemoryManager::lockGlyphMetrics(FontID id, const QChar& ch)
{
    QGlyph g = lockGlyph(id,ch);
    return g.metrics;
}

void QMemoryManager::unlockGlyph(FontID id, const QChar& ch)
{
}

void QMemoryManager::savePrerenderedFont(const QFontDef& f, bool all)
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)findFont(f);
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
	    qDebug("Rendering %s",fontFilename(mmf->def).ascii());
	    for (int i=0; i<=mmf->renderer->maxchar; i++) {
		QChar ch((ushort)i);
		if (
			/* XXX BUG IN CYBERBIT OR FREETYPE ??? */i!=9660&&/*XXX*/

			mmf->renderer->inFont(ch) ) {
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
	//mmf->tree->dump();
	QFile f(fontFilename(mmf->def));
	f.open(IO_WriteOnly);
	f.writeBlock((char*)&mmf->fm,sizeof(mmf->fm));
	mmf->tree->write(f);
    }
}

// Perhaps we should just return the struct?
//
bool QMemoryManager::fontSmooth(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.smooth;
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


QMemoryManager* memorymanager=0;
