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

#ifndef QRUBBERBAND_H
#define QRUBBERBAND_H

#include <QtGui/qwidget.h>

class QRubberBandPrivate;

class Q_GUI_EXPORT QRubberBand : public QWidget
{
    Q_OBJECT

public:
    enum Shape { Line, Rectangle };
    explicit QRubberBand(Shape, QWidget * =0);
    ~QRubberBand();

    Shape shape() const;

    void setGeometry(const QRect &r);

    inline void setGeometry(int x, int y, int w, int h)
        { setGeometry(QRect(x, y, w, h)); }
    inline void move(int x, int y)
    { setGeometry(x + geometry().x() - QWidget::x(),
		  y - geometry().y() - QWidget::y(),
		  width(), height()); }
    inline void move(const QPoint &p)
    { move(p.x(), p.y()); }
    inline void resize(int w, int h)
    { setGeometry(geometry().x(), geometry().y(), w, h); }
    inline void resize(const QSize &s)
    { resize(s.width(), s.height()); }


protected:

    void paintEvent(QPaintEvent *);
    void changeEvent(QEvent *);

private:
    Q_DECLARE_PRIVATE(QRubberBand)
};

#endif // QRUBBERBAND_H
