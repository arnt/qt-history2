/****************************************************************************
**
** Definition of the QSimpleRichText class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSIMPLERICHTEXT_H
#define QSIMPLERICHTEXT_H

#ifndef QT_H
#include "qnamespace.h"
#include "qstring.h"
#include "qregion.h"
#include "qcolor.h"
#endif // QT_H

#ifndef QT_NO_RICHTEXT

class QPainter;
class QWidget;
class QStyleSheet;
class QBrush;
class QMimeSourceFactory;
class QSimpleRichTextData;

class Q_COMPAT_EXPORT QSimpleRichText
{
public:
    QSimpleRichText(const QString& text, const QFont& fnt,
                     const QString& context = QString::null, const QStyleSheet* sheet = 0);
    QSimpleRichText(const QString& text, const QFont& fnt,
                     const QString& context,  const QStyleSheet* sheet,
                     const QMimeSourceFactory* factory, int pageBreak = -1,
                     const QColor& linkColor = Qt::blue, bool linkUnderline = true);
    ~QSimpleRichText();

    void setWidth(int);
    void setWidth(QPainter*, int);
    void setDefaultFont(const QFont &f);
    int width() const;
    int widthUsed() const;
    int height() const;
    void adjustSize();

    void draw(QPainter* p,  int x, int y, const QRect& clipRect,
               const QPalette& pal, const QBrush* paper = 0) const;

    // obsolete
    void draw(QPainter* p,  int x, int y, const QRegion& clipRegion,
               const QPalette& pal, const QBrush* paper = 0) const {
        draw(p, x, y, clipRegion.boundingRect(), pal, paper);
    }

    QString context() const;
    QString anchorAt(const QPoint& pos) const;

    bool inText(const QPoint& pos) const;

private:
    QSimpleRichTextData* d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSimpleRichText(const QSimpleRichText &);
    QSimpleRichText &operator=(const QSimpleRichText &);
#endif
};

#endif // QT_NO_RICHTEXT

#endif // QSIMPLERICHTEXT_H
