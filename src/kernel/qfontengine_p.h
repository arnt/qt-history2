#ifndef QFONTENGINE_P_H
#define QFONTENGINE_P_H

#include <qglobal.h>

#ifdef Q_WS_WIN
#include <qt_windows.h>
#endif

#include "qtextengine_p.h"

class QPaintDevice;

struct glyph_metrics_t;
class QChar;
typedef unsigned short glyph_t;
struct offset_t;
typedef int advance_t;
class QOpenType;

#if defined( Q_WS_X11 ) || defined( Q_WS_WIN) || defined( Q_WS_MAC )
class QFontEngine : public QShared
{
public:
    enum Error {
	NoError,
	OutOfMemory
    };

    enum Type {
	// X11 types
	Box,
	XLFD,
	Xft,

	// MS Windows types
	Win,
	Uniscribe,

	// Apple MacOS types
	Mac,

	// Trolltech QWS types
	QWS
    };

    virtual ~QFontEngine();

    /* returns 0 as glyph index for non existant glyphs */
    virtual Error stringToCMap( const QChar *str, int len, glyph_t *glyphs,
				advance_t *advances, int *nglyphs ) const = 0;

#ifdef Q_WS_X11
    virtual QOpenType *openType() const { return 0; }
#endif

    virtual void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
		       const advance_t *advances, const offset_t *offsets,
		       int numGlyphs, bool reverse ) = 0;

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
					 const advance_t *advances,
					 const offset_t *offsets, int numGlyphs ) = 0;
    virtual glyph_metrics_t boundingBox( glyph_t glyph ) = 0;

    virtual int ascent() const = 0;
    virtual int descent() const = 0;
    virtual int leading() const = 0;
    virtual int maxCharWidth() const = 0;

    virtual const char *name() const = 0;

    virtual bool canRender( const QChar *string,  int len ) = 0;

    virtual void setScale( double ) {}
    virtual int scale() const { return 1; }

    virtual Type type() const = 0;

    QFontDef fontDef;
    int cache_cost;

#ifdef Q_WS_WIN
    HDC dc() const;
    void getGlyphIndexes( const QChar *ch, int numChars, glyph_t *glyphs ) const;
    void getCMap();

    QCString _name;
    HDC		hdc;
    HFONT	hfont;
    uint	stockFont   : 1;
    uint	paintDevice : 1;
    uint        useTextOutA : 1;
    uint        ttf         : 1;
    union {
	TEXTMETRICW	w;
	TEXTMETRICA	a;
    } tm;
    int		lw;
    unsigned char *cmap;
    void *script_cache;
#endif // Q_WS_WIN

};
#elif defined( Q_WS_QWS )
class QGfx;

class QFontEngine : public QShared
{
public:
    QFontEngine( const QFontDef& );
   ~QFontEngine();
    /*QMemoryManager::FontID*/ void *handle() const;

    enum Error {
	NoError,
	OutOfMemory
    };
    /* returns 0 as glyph index for non existant glyphs */
    Error stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	       const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse );

    glyph_metrics_t boundingBox( const glyph_t *glyphs,
			       const advance_t *advances, const offset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;
    int minLeftBearing() const;
    int minRightBearing() const;
    int underlinePos() const;
    int lineWidth() const;

    bool canRender( const QChar *string,  int len );

    QFontDef s;
    /*QMemoryManager::FontID*/ void *id;
    int cache_cost;
};
#endif // WIN || X11 || MAC



enum IndicFeatures {
    InitFeature = 0x0001,
    NuktaFeature = 0x0002,
    AkhantFeature = 0x0004,
    RephFeature = 0x0008,
    BelowFormFeature = 0x0010,
    HalfFormFeature = 0x0020,
    PostFormFeature = 0x0040,
    VattuFeature = 0x0080,
    PreSubstFeature = 0x0100,
    BelowSubstFeature = 0x0200,
    AboveSubstFeature = 0x0400,
    PostSubstFeature = 0x0800,
    HalantFeature = 0x1000
};

class QFontEngineBox : public QFontEngine
{
public:
    QFontEngineBox( int size );
    ~QFontEngineBox();

    Error stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	       const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse );

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
				    const advance_t *advances, const offset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;

    int cmap() const;
    const char *name() const;

    bool canRender( const QChar *string,  int len );

    Type type() const;
    inline int size() const { return _size; }

private:
    friend class QFontPrivate;
    int _size;
};

#ifdef Q_WS_X11
#include "qt_x11.h"

#ifndef QT_NO_XFTFREETYPE
#include <freetype/freetype.h>
#include "ftxopen.h"
#endif


#ifndef QT_NO_XFTFREETYPE
class QTextCodec;

class QFontEngineXft : public QFontEngine
{
public:
    QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap );
    ~QFontEngineXft();

    QOpenType *openType() const;

    Error stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	       const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse );

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
				    const advance_t *advances, const offset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;

    int cmap() const;
    const char *name() const;

    bool canRender( const QChar *string,  int len );

    Type type() const;

private:
    friend class QFontPrivate;
    XftFont *_font;
    XftPattern *_pattern;
    FT_Face _face;
    QOpenType *_openType;
    int _cmap;
};
#endif

class QFontEngineXLFD : public QFontEngine
{
public:
    QFontEngineXLFD( XFontStruct *fs, const char *name, const char *encoding, int cmap );
    ~QFontEngineXLFD();

    Error stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	       const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse );

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
				    const advance_t *advances, const offset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;

    int cmap() const;
    const char *name() const;

    bool canRender( const QChar *string,  int len );

    void setScale( double scale );
    Type type() const;

    Qt::HANDLE handle() const { return (Qt::HANDLE) _fs->fid; }

private:
    friend class QFontPrivate;
    XFontStruct *_fs;
    QCString _name;
    QTextCodec *_codec;
    float _scale; // needed for printing, to correctly scale font metrics for bitmap fonts
    int _cmap;
};

class QScriptItem;

#ifndef QT_NO_XFTFREETYPE
class QOpenType
{
public:
    QOpenType( FT_Face face );
    ~QOpenType();

    bool supportsScript( unsigned int script );

    void apply( unsigned int script, unsigned short *featuresToApply, QScriptItem *item, int stringLength );

private:
    bool loadTables( FT_ULong script);

    FT_Face face;
    TTO_GDEF gdef;
    TTO_GSUB gsub;
    TTO_GPOS gpos;
    FT_UShort script_index;
    FT_ULong current_script;
    unsigned short found_bits;
    unsigned short always_apply;
    bool hasGDef : 1;
    bool hasGSub : 1;
    bool hasGPos : 1;
};
#endif // QT_NO_XFTFREETYPE

#elif defined( Q_WS_MAC )

#include "qt_mac.h"
class QFontEngineMac : public QFontEngine
{
    QFontDef fdef;
#if 0
    ATSFontMetrics *info;
#else
    FontInfo *info;
#endif
    short fnum;
    int psize;
    QMacFontInfo *internal_fi;
    friend class QGLContext;
    friend class QFontPrivate;
    friend class QMacSetFontInfo;

public:
    QFontEngineMac(const QFontDef &f) : QFontEngine(), fdef(f), info(NULL), fnum(-1),
					internal_fi(NULL) { cache_cost = 1; }

    Error stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	       const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse );

    glyph_metrics_t boundingBox( const glyph_t *glyphs,
			       const advance_t *advances, const offset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const { return (int)info->ascent; }
    int descent() const { return (int)info->descent; }
    int leading() const { return (int)info->leading; }
#if 0
    int maxCharWidth() const { return (int)info->maxAdvanceWidth; }
#else
    int maxCharWidth() const { return info->widMax; }
#endif

    const char *name() const { return "ATSUI"; }

    bool canRender( const QChar *string,  int len );

    Type type() const { return QFontEngine::Mac; }

    enum { WIDTH=0x01, DRAW=0x02, EXISTS=0x04 };
    int doTextTask(const QChar *s, int pos, int use_len, int len, uchar task, int =-1, int y=-1,
		   QPaintDevice *dev=NULL, const QRegion *rgn=NULL) const;
};

#elif defined( Q_WS_WIN )

class QFontEngineWin : public QFontEngine
{
public:
    QFontEngineWin( const char *name, HDC, HFONT, bool );

    Error stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	       const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse );

    glyph_metrics_t boundingBox( const glyph_t *glyphs,
			       const advance_t *advances, const offset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;

    const char *name() const;

    bool canRender( const QChar *string,  int len );

    Type type() const;
};

#if 0
class QFontEngineUniscribe : public QFontEngineWin
{
public:
    void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	       const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse );
    bool canRender( const QChar *string,  int len );

    Type type() const;
};
#endif

#endif // Q_WS_WIN

#endif
