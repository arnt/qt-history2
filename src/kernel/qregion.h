/****************************************************************************
**
** Definition of QRegion class.
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

#ifndef QREGION_H
#define QREGION_H

#ifndef QT_H
#include "qatomic.h"
#include "qrect.h"
#include "qvector.h"
#endif // QT_H

#if defined(Q_WS_QWS) || defined(Q_WS_X11) || defined(Q_WS_MAC)
struct QRegionPrivate;
#endif

class Q_GUI_EXPORT QRegion
{
public:
    enum RegionType { Rectangle, Ellipse };

    QRegion();
    QRegion(int x, int y, int w, int h, RegionType t = Rectangle);
    QRegion(const QRect &r, RegionType t = Rectangle);
    QRegion(const QPointArray &pa, bool winding=false);
    QRegion(const QRegion &region);
    QRegion(const QBitmap &bitmap);
    ~QRegion();
    QRegion &operator=(const QRegion &);

#ifndef QT_NO_COMPAT
    inline bool isNull() const { return isEmpty(); }
#endif
    bool isEmpty() const;

    bool contains(const QPoint &p) const;
    bool contains(const QRect &r) const;

    void translate(int dx, int dy);
    inline void translate(const QPoint &p) { translate(p.x(), p.y()); }

    QRegion unite(const QRegion &r) const;
    QRegion intersect(const QRegion &r) const;
    QRegion subtract(const QRegion &r) const;
    QRegion eor(const QRegion &r) const;

    QRect boundingRect() const;
    QVector<QRect> rects() const;
    void setRects(const QRect *rect, int num);

    const QRegion operator|(const QRegion &r) const;
    const QRegion operator+(const QRegion &r) const;
    const QRegion operator&(const QRegion &r) const;
    const QRegion operator-(const QRegion &r) const;
    const QRegion operator^(const QRegion &r) const;
    QRegion& operator|=(const QRegion &r);
    QRegion& operator+=(const QRegion &r);
    QRegion& operator&=(const QRegion &r);
    QRegion& operator-=(const QRegion &r);
    QRegion& operator^=(const QRegion &r);

    bool operator==(const QRegion &r) const;
    inline bool operator!=(const QRegion &r) const { return !(operator==(r)); }

#if defined(Q_WS_WIN)
    inline HRGN    handle() const { return d->rgn; }
#elif defined(Q_WS_X11)
    inline Region handle() const { if(!d->rgn) updateX11Region(); return d->rgn; }
#elif defined(Q_WS_MAC)
    inline RgnHandle handle() const { return handle(FALSE); }
    RgnHandle handle(bool require_rgn) const;
#elif defined(Q_WS_QWS)
    // QGfx_QWS needs this for region drawing
    inline void *handle() const { return d->qt_rgn; }
#endif

#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<( QDataStream &, const QRegion & );
    friend Q_GUI_EXPORT QDataStream &operator>>( QDataStream &, QRegion & );
#endif
private:
    QRegion copy() const;   // helper of detach.
    void detach();
#if defined(Q_WS_WIN)
    QRegion winCombine(const QRegion &r, int num) const;
#elif defined(Q_WS_X11)
    void updateX11Region() const;
    void *clipRectangles(int &num) const;
    friend void *qt_getClipRects(const QRegion &r, int &num);
#elif defined(Q_WS_MAC)
    friend QRegion qt_mac_convert_mac_region(RgnHandle rgn);
#endif
    void exec(const QByteArray &ba, int ver = 0);
    struct QRegionData {
	QAtomic ref;
#if defined(Q_WS_WIN)
	HRGN   rgn;
#elif defined(Q_WS_X11)
	Region rgn;
	void *xrectangles;
#elif defined(Q_WS_MAC)
	mutable RgnHandle rgn;
#endif
#if defined(Q_WS_QWS) || defined(Q_WS_X11) || defined(Q_WS_MAC)
	QRegionPrivate *qt_rgn;
#endif
    };
    struct QRegionData *d;
    static struct QRegionData shared_empty;
    static void cleanUp(QRegionData *x);
};


#define QRGN_SETRECT		1		// region stream commands
#define QRGN_SETELLIPSE		2		//  (these are internal)
#define QRGN_SETPTARRAY_ALT	3
#define QRGN_SETPTARRAY_WIND	4
#define QRGN_TRANSLATE		5
#define QRGN_OR			6
#define QRGN_AND		7
#define QRGN_SUB		8
#define QRGN_XOR		9
#define QRGN_RECTS	       10

#ifndef QT_NO_WMATRIX
QRegion operator*(const QWMatrix &, const QRect &);
QRegion operator*(const QWMatrix &, const QRegion &);
#endif

/*****************************************************************************
  QRegion stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<( QDataStream &, const QRegion & );
Q_GUI_EXPORT QDataStream &operator>>( QDataStream &, QRegion & );
#endif

#endif // QREGION_H
