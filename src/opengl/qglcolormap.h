/****************************************************************************
**
** Definition of QGLColormap class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGLCOLORMAP_H
#define QGLCOLORMAP_H

#ifndef QT_H
#include "qcolor.h"
#include "qmemarray.h"
#include "qshared.h"
#endif // QT_H

#if !defined( QT_MODULE_OPENGL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_OPENGL
#else
#define QM_EXPORT_OPENGL Q_EXPORT
#endif

class QWidget;
class QM_EXPORT_OPENGL QGLColormap
{
public:
    QGLColormap();
    QGLColormap( const QGLColormap & );
    ~QGLColormap();
    
    QGLColormap &operator=( const QGLColormap & );
    
    bool   isEmpty() const;
    int    size() const;
    void   detach();

    void   setEntries( int count, const QRgb * colors, int base = 0 );
    void   setEntry( int idx, QRgb color );
    void   setEntry( int idx, const QColor & color );
    QRgb   entryRgb( int idx ) const;
    QColor entryColor( int idx ) const;
    int    find( QRgb color ) const;
    int    findNearest( QRgb color ) const;
    
private:
    class Private : public QShared
    {
    public:
	Private() {
	    cells.resize( 256 ); // ### hardcoded to 256 entries for now
	    cmapHandle = 0;
	}

	~Private() {
	}

	QMemArray<QRgb> cells;
	Qt::HANDLE      cmapHandle;
    };
    
    Private * d;

    friend class QGLWidget;
};

#endif
