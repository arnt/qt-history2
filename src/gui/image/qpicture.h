/****************************************************************************
**
** Definition of QPicture class.
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

#ifndef QPICTURE_H
#define QPICTURE_H

#ifndef QT_H
#include "qpaintdevice.h"
#include "qstringlist.h"
#include "qbuffer.h"
#endif // QT_H

#ifndef QT_NO_PICTURE

class Q_GUI_EXPORT QPicture : public QPaintDevice		// picture class
{
public:
    QPicture( int formatVersion = -1 );
    QPicture( const QPicture & );
   ~QPicture();

    bool	isNull() const;

    uint	size() const;
    const char* data() const;
    virtual void setData( const char* data, uint size );

    bool	play( QPainter * );

    bool	load( QIODevice *dev, const char *format = 0 );
    bool	load( const QString &fileName, const char *format = 0 );
    bool	save( QIODevice *dev, const char *format = 0 );
    bool	save( const QString &fileName, const char *format = 0 );

    QRect boundingRect() const;
    void setBoundingRect( const QRect &r );

    QPicture& operator= (const QPicture&);

    friend Q_GUI_EXPORT QDataStream &operator<<( QDataStream &, const QPicture & );
    friend Q_GUI_EXPORT QDataStream &operator>>( QDataStream &, QPicture & );

#ifndef QT_NO_IMAGEIO
    static const char* pictureFormat( const QString &fileName );
    static QList<QByteArray> inputFormats();
    static QList<QByteArray> outputFormats();
#ifndef QT_NO_STRINGLIST
    static QStringList inputFormatList();
    static QStringList outputFormatList();
#endif
#endif

    QPaintEngine *engine() const;// { Q_ASSERT(!"QPicture::engine() is not implemented!"); return 0; }

protected:
    int		metric( int ) const;
    void	detach();
    QPicture	copy() const;

private:
    bool	exec( QPainter *, QDataStream &, int );

    struct QPicturePrivate : public QShared {
#if 0 // ### port
	bool	cmd( int, QPainter *, QPDevCmdParam * );
#endif
	bool	checkFormat();
	void	resetFormat();

	QBuffer	pictb;
	int	trecs;
	bool	formatOk;
	int	formatMajor;
	int	formatMinor;
	QRect	brect;
	QPaintEngine *paintEngine;
    } *d;

        enum PDevCmd {
	PdcNOP = 0, //  <void>
	PdcDrawPoint = 1, // point
	PdcDrawFirst = PdcDrawPoint,
	PdcMoveTo = 2, // point
	PdcLineTo = 3, // point
	PdcDrawLine = 4, // point,point
	PdcDrawRect = 5, // rect
	PdcDrawRoundRect = 6, // rect,ival,ival
	PdcDrawEllipse = 7, // rect
	PdcDrawArc = 8, // rect,ival,ival
	PdcDrawPie = 9, // rect,ival,ival
	PdcDrawChord = 10, // rect,ival,ival
	PdcDrawLineSegments = 11, // ptarr
	PdcDrawPolyline = 12, // ptarr
	PdcDrawPolygon = 13, // ptarr,ival
	PdcDrawCubicBezier = 14, // ptarr
	PdcDrawText = 15, // point,str
	PdcDrawTextFormatted = 16, // rect,ival,str
	PdcDrawPixmap = 17, // rect,pixmap
	PdcDrawImage = 18, // rect,image
	PdcDrawText2 = 19, // point,str
	PdcDrawText2Formatted = 20, // rect,ival,str
	PdcDrawTextItem = 21,
	PdcDrawLast = PdcDrawTextItem,

	// no painting commands below PdcDrawLast.

	PdcBegin = 30, //  <void>
	PdcEnd = 31, //  <void>
	PdcSave = 32, //  <void>
	PdcRestore = 33, //  <void>
	PdcSetdev = 34, // device - PRIVATE
	PdcSetBkColor = 40, // color
	PdcSetBkMode = 41, // ival
	PdcSetROP = 42, // ival
	PdcSetBrushOrigin = 43, // point
	PdcSetFont = 45, // font
	PdcSetPen = 46, // pen
	PdcSetBrush = 47, // brush
	PdcSetTabStops = 48, // ival
	PdcSetTabArray = 49, // ival,ivec
	PdcSetUnit = 50, // ival
	PdcSetVXform = 51, // ival
	PdcSetWindow = 52, // rect
	PdcSetViewport = 53, // rect
	PdcSetWXform = 54, // ival
	PdcSetWMatrix = 55, // matrix,ival
	PdcSaveWMatrix = 56,
	PdcRestoreWMatrix = 57,
	PdcSetClip = 60, // ival
	PdcSetClipRegion = 61, // rgn

	PdcReservedStart = 0, // codes 0-199 are reserved
	PdcReservedStop = 199 //   for Qt
    };
};


inline bool QPicture::isNull() const
{
    return d->pictb.buffer().isNull();
}

inline uint QPicture::size() const
{
    return d->pictb.buffer().size();
}

inline const char* QPicture::data() const
{
    return d->pictb.buffer();
}


#ifndef QT_NO_PICTUREIO
class QIODevice;
class QPictureIO;
typedef void (*picture_io_handler)( QPictureIO * ); // picture IO handler

struct QPictureIOData;

class Q_GUI_EXPORT QPictureIO
{
public:
    QPictureIO();
    QPictureIO( QIODevice	 *ioDevice, const char *format );
    QPictureIO( const QString &fileName, const char* format );
   ~QPictureIO();

    const QPicture &picture()	const;
    int		status()	const;
    const char *format()	const;
    QIODevice  *ioDevice()	const;
    QString	fileName()	const;
    int		quality()	const;
    QString	description()	const;
    const char *parameters()	const;
    float gamma() const;

    void	setPicture( const QPicture & );
    void	setStatus( int );
    void	setFormat( const char * );
    void	setIODevice( QIODevice * );
    void	setFileName( const QString & );
    void	setQuality( int );
    void	setDescription( const QString & );
    void	setParameters( const char * );
    void	setGamma( float );

    bool	read();
    bool	write();

    static QByteArray pictureFormat( const QString &fileName );
    static QByteArray pictureFormat( QIODevice * );
    static QList<QByteArray> inputFormats();
    static QList<QByteArray> outputFormats();

    static void defineIOHandler( const char *format,
				 const char *header,
				 const char *flags,
				 picture_io_handler read_picture,
				 picture_io_handler write_picture );

private:
    void	init();

    QPictureIOData *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPictureIO( const QPictureIO & );
    QPictureIO &operator=( const QPictureIO & );
#endif
};

#endif //QT_NO_PICTUREIO


/*****************************************************************************
  QPicture stream functions
 *****************************************************************************/

Q_GUI_EXPORT QDataStream &operator<<( QDataStream &, const QPicture & );
Q_GUI_EXPORT QDataStream &operator>>( QDataStream &, QPicture & );

#endif // QT_NO_PICTURE

#endif // QPICTURE_H
