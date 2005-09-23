/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QSVGTINYDOCUMENT_H
#define QSVGTINYDOCUMENT_H

#include "qsvgstructure_p.h"

#include "QtCore/qrect.h"
#include "QtCore/qlist.h"
#include "QtCore/qhash.h"

class QPainter;
class QByteArray;

class Q_SVG_EXPORT QSvgTinyDocument : public QSvgStructureNode
{
public:
    static QSvgTinyDocument * load(const QString &file);
    static QSvgTinyDocument * load(const QByteArray &contents);
public:
    QSvgTinyDocument();
    Type type() const;

    QSize size() const;
    void setWidth(int len, bool percent);
    void setHeight(int len, bool percent);
    int width() const;
    int height() const;
    bool widthPercent() const;
    bool heightPercent() const;

    bool preserveAspectRatio() const;

    QRect viewBox() const;
    void setViewBox(const QRect &rect);

    void draw(QPainter *p);
private:
    QSize m_size;
    bool  m_widthPercent;
    bool  m_heightPercent;

    QRect m_viewBox;
};

inline QSize QSvgTinyDocument::size() const
{
    return m_size;
}

inline int QSvgTinyDocument::width() const
{
    return m_size.width();
}

inline int QSvgTinyDocument::height() const
{
    return m_size.height();
}

inline bool QSvgTinyDocument::widthPercent() const
{
    return m_widthPercent;
}

inline bool QSvgTinyDocument::heightPercent() const
{
    return m_heightPercent;
}

inline QRect QSvgTinyDocument::viewBox() const
{
    return m_viewBox;
}

inline bool QSvgTinyDocument::preserveAspectRatio() const
{
    return false;
}

#endif
