/****************************************************************************
**
** Definition of QPixmap class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPIXMAP_H
#define QPIXMAP_H

#ifndef QT_H
#include "qcolor.h" // char*->QColor conversion
#include "qnamespace.h"
#include "qpaintdevice.h"
#include "qstring.h" // char*->QString conversion
#include "qimage.h"
#endif // QT_H

class QGfx;
class QPixmapPrivate;

#if defined(Q_WS_WIN)
// Internal pixmap memory optimization class for Windows 9x
class QMultiCellPixmap;
#endif
#if defined(Q_WS_X11)
class QX11Info;
#endif

class Q_GUI_EXPORT QPixmap : public QPaintDevice, public Qt
{
public:
    enum ColorMode { Auto, Color, Mono };
    enum Optimization { DefaultOptim, NoOptim, MemoryOptim=NoOptim,
			NormalOptim, BestOptim };

    QPixmap();
    QPixmap( const QImage& image );
    QPixmap( int w, int h,  int depth = -1, Optimization = DefaultOptim );
    QPixmap( const QSize &, int depth = -1, Optimization = DefaultOptim );
#ifndef QT_NO_IMAGEIO
    QPixmap( const QString& fileName, const char *format=0,
	     ColorMode mode=Auto );
    QPixmap( const QString& fileName, const char *format,
	     int conversion_flags );
    explicit QPixmap( const char * const xpm[] );
#endif
    QPixmap( const QPixmap & );
   ~QPixmap();

    QPixmap    &operator=( const QPixmap & );
    QPixmap    &operator=( const QImage	 & );

    inline bool isNull() const;

    int		width()		const { return data->w; }
    int		height()	const { return data->h; }
    QSize	size()		const { return QSize(data->w,data->h); }
    QRect	rect()		const { return QRect(0,0,data->w,data->h); }
    int		depth()		const { return data->d; }
    static int	defaultDepth();

    void	fill( const QColor &fillColor = Qt::white );
    void	fill( const QWidget *, int xofs, int yofs );
    void	fill( const QWidget *, const QPoint &ofs );
    void	resize( int width, int height );
    void	resize( const QSize & );

    const QBitmap *mask() const;
    void	setMask( const QBitmap & );
    bool	selfMask() const;
    bool	hasAlpha() const;
    bool	hasAlphaChannel() const;
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
    QBitmap	createHeuristicMask( bool clipTight = TRUE ) const;
#endif
#ifndef QT_NO_MIME
    static QPixmap fromMimeSource( const QString& abs_name );
#endif
    static  QPixmap grabWindow( WId, int x=0, int y=0, int w=-1, int h=-1 );
    static  QPixmap grabWidget( QWidget * widget,
				int x=0, int y=0, int w=-1, int h=-1 );

#ifndef QT_NO_PIXMAP_TRANSFORMATION
    QPixmap	    xForm( const QWMatrix & ) const;
    static QWMatrix trueMatrix( const QWMatrix &m, int w, int h );
#endif

    QImage	convertToImage() const;
    bool	convertFromImage( const QImage &, ColorMode mode=Auto );
    bool	convertFromImage( const QImage &, int conversion_flags );
#ifndef QT_NO_IMAGEIO
    static const char* imageFormat( const QString &fileName );
    bool	load( const QString& fileName, const char *format=0,
		      ColorMode mode=Auto );
    bool	load( const QString& fileName, const char *format,
		      int conversion_flags );
    bool	loadFromData( const uchar *buf, uint len,
			      const char* format=0,
			      ColorMode mode=Auto );
    bool	loadFromData( const uchar *buf, uint len,
			      const char* format,
			      int conversion_flags );
    bool	loadFromData( const QByteArray &data,
			      const char* format=0,
			      int conversion_flags=0 );
    bool	save( const QString& fileName, const char* format, int quality = -1 ) const;
    bool	save( QIODevice* device, const char* format, int quality = -1 ) const;
#endif

#if defined(Q_WS_WIN)
    HBITMAP	hbm()		const;
#endif

#if defined(Q_WS_MAC)
    virtual Qt::HANDLE      macCGHandle() const;
#endif

    int		serialNumber()	const;

    Optimization	optimization() const;
    void		setOptimization( Optimization );
    static Optimization defaultOptimization();
    static void		setDefaultOptimization( Optimization );

    virtual void detach();

    bool	isQBitmap() const;

#if defined(Q_WS_WIN)
    // These functions are internal and used by Windows 9x only
    bool	isMultiCellPixmap() const;
    HDC		multiCellHandle() const;
    HBITMAP	multiCellBitmap() const;
    int		multiCellOffset() const;
    int		allocCell();
    void	freeCell( bool = FALSE );
#endif

#if defined(Q_WS_QWS)
    virtual QGfx * graphicsContext(bool clip_children=TRUE) const;
    virtual unsigned char * scanLine(int) const;
    virtual int bytesPerLine() const;
    QRgb * clut() const;
    int numCols() const;
#elif defined(Q_WS_X11)
    static int x11SetDefaultScreen( int screen );
    void x11SetScreen( int screen );
    QX11Info *x11Info() const;
#endif

    inline bool operator!() const { return isNull(); }
    inline operator QImage() const { return convertToImage(); }

#ifndef Q_QDOC
    Q_DUMMY_COMPARISON_OPERATOR(QPixmap)
#endif

protected:
    QPixmap( int w, int h, const uchar *data, bool isXbitmap );
    int metric( int ) const;

#if defined(Q_WS_WIN)
    struct QMCPI {				// mem optim for win9x
	QMultiCellPixmap *mcp;
	int	offset;
    };
#endif

    struct QPixmapData : public QShared {	// internal pixmap data
	QCOORD	w, h;
	short	d;
	uint	uninit	 : 1;
	uint	bitmap	 : 1;
	uint	selfmask : 1;
#if defined(Q_WS_WIN)
	uint	mcp	 : 1;
#endif
	int	ser_no;
	QBitmap *mask;
#if defined(Q_WS_WIN)
	QPixmap *maskpm;
	union {
	    HBITMAP hbm;    // if mcp == FALSE
	    QMCPI  *mcpi;   // if mcp == TRUE
	} hbm_or_mcpi;
	uchar *realAlphaBits;
#ifdef Q_OS_TEMP
	uchar* ppvBits; // Pointer to DIBSection bits
#endif
#elif defined(Q_WS_X11)
	void   *ximage;
	void   *maskgc;
	QPixmap *alphapm;
	QX11Info *xinfo;
#elif defined(Q_WS_MAC)
	ColorTable *clut;
	QPixmap *alphapm;
#elif defined(Q_WS_QWS)
	int id; // ### should use QPaintDevice::hd, since it is there
	QRgb * clut;
	int numcols;
	int rw;
	int rh;
	bool hasAlpha;
#endif
	Optimization optim;
#if defined(Q_WS_WIN)
	HBITMAP old_hbm;
#endif
    } *data;
private:
#ifndef QT_NO_IMAGEIO
    bool doImageIO( QImageIO* io, int quality ) const;
#endif
    QPixmap( int w, int h, int depth, bool, Optimization );
    void	init( int, int, int, bool, Optimization );
    void	deref();
    QPixmap	copy( bool ignoreMask = FALSE ) const;
#if defined(Q_WS_WIN)
    void initAlphaPixmap( uchar *bytes, int length, struct tagBITMAPINFO *bmi );
    void convertToAlphaPixmap( bool initAlpha=TRUE );
    static void bitBltAlphaPixmap( QPixmap *dst, int dx, int dy,
				   const QPixmap *src, int sx, int sy,
				   int sw, int sh, bool useDstAlpha );
#endif
    static Optimization defOptim;
    friend Q_GUI_EXPORT void bitBlt( QPaintDevice *, int, int,
				 const QPaintDevice *,
				 int, int, int, int, RasterOp, bool );
    friend Q_GUI_EXPORT void bitBlt( QPaintDevice *, int, int,
				 const QImage* src,
				 int, int, int, int, int conversion_flags );
    friend Q_GUI_EXPORT void copyBlt( QPixmap *dst, int dx, int dy,
				  const QPixmap *src, int sx, int sy,
				  int sw, int sh );

#if defined(Q_WS_MAC)
    friend void unclippedScaledBitBlt(QPaintDevice *, int, int, int, int,
				      const QPaintDevice *, int, int, int, int,
				      Qt::RasterOp, bool, bool);
#endif

    friend class QBitmap;
    friend class QPaintDevice;
    friend class QPainter;
#if defined(Q_WS_X11)
    friend class QX11GC;
#endif
#if defined(Q_WS_QWS)
    friend class QWSGC;
#endif
};


inline bool QPixmap::isNull() const
{
    return data->w == 0;
}

inline void QPixmap::fill( const QWidget *w, const QPoint &ofs )
{
    fill( w, ofs.x(), ofs.y() );
}

inline void QPixmap::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}

inline const QBitmap *QPixmap::mask() const
{
    return data->mask;
}

inline bool QPixmap::selfMask() const
{
    return data->selfmask;
}

#if defined(Q_WS_WIN)
inline HBITMAP QPixmap::hbm() const
{
    return data->mcp ? 0 : data->hbm_or_mcpi.hbm;
}
#endif

inline int QPixmap::serialNumber() const
{
    return data->ser_no;
}

inline QPixmap::Optimization QPixmap::optimization() const
{
    return data->optim;
}

inline bool QPixmap::isQBitmap() const
{
    return data->bitmap;
}

#if defined(Q_WS_WIN)
inline bool QPixmap::isMultiCellPixmap() const
{
    return data->mcp;
}
#endif

/*****************************************************************************
  QPixmap stream functions
 *****************************************************************************/

#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
Q_GUI_EXPORT QDataStream &operator<<( QDataStream &, const QPixmap & );
Q_GUI_EXPORT QDataStream &operator>>( QDataStream &, QPixmap & );
#endif

/*****************************************************************************
  QPixmap (and QImage) helper functions
 *****************************************************************************/

Q_GUI_EXPORT void copyBlt( QPixmap *dst, int dx, int dy,
		       const QPixmap *src, int sx = 0, int sy = 0,
		       int sw = -1, int sh = -1 );

#endif // QPIXMAP_H
