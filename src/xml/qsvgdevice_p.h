/****************************************************************************
**
** Definition of the QSvgDevice class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the xml module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSVGDEVICE_P_H
#define QSVGDEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QPicture class. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qpaintdevice.h"
#include "qrect.h"
#include "qdom.h"
#endif // QT_H

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_SVG
#else
#define QM_EXPORT_SVG Q_XML_EXPORT
#endif

#ifndef QT_NO_SVG

class QPainter;
class QPaintEngine;
class QDomNode;
class QDomNamedNodeMap;
struct QSvgDeviceState;
class QSvgDevicePrivate;

class QM_EXPORT_SVG QSvgDevice : public QPaintDevice
{
public:
    QSvgDevice();
    ~QSvgDevice();

    bool play(QPainter *p);

    QString toString() const;

    bool load(QIODevice *dev);
    bool save(QIODevice *dev);
    bool save(const QString &fileName);

    QRect boundingRect() const;
    void setBoundingRect(const QRect &r);

    QPaintEngine *paintEngine() const;

protected:
    virtual int         metric(int) const;

private:
    // reading
    bool play(const QDomNode &node);
    void saveAttributes();
    void restoreAttributes();
    QColor parseColor(const QString &col);
    double parseLen(const QString &str, bool *ok=0, bool horiz=true) const;
    int lenToInt(const QDomNamedNodeMap &map, const QString &attr,
                  int def=0) const;
    double lenToDouble(const QDomNamedNodeMap &map, const QString &attr,
                  int def=0) const;
    void setStyleProperty(const QString &prop, const QString &val,
                           QPen *pen, QFont *font, int *talign);
    void setStyle(const QString &s);
    void setTransform(const QString &tr);
    void drawPath(const QString &data);

    // writing
    void appendChild(QDomElement &e, int c);
    void applyStyle(QDomElement *e, int c) const;
    void applyTransform(QDomElement *e) const;

    // reading
    QRect brect;                        // bounding rectangle
    QDomDocument doc;                        // document tree
    QDomNode current;
    QPoint curPt;
    QSvgDeviceState *curr;
    QPainter *pt;                        // used by play() et al

    // writing
    bool dirtyTransform, dirtyStyle;

    QSvgDevicePrivate *d;
};

inline QRect QSvgDevice::boundingRect() const
{
    return brect;
}

#endif // QT_NO_SVG

#endif // QSVGDEVICE_P_H
