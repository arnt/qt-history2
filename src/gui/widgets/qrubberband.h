/****************************************************************************
**
** Definition of QRubberBand class.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __QRUBBERBAND_H__
#define __QRUBBERBAND_H__

#ifndef QT_H
# include <qwidget.h>
#endif

class QRubberBandPrivate;

class Q_GUI_EXPORT QRubberBand : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QRubberBand);
public:
    enum Shape { Line, Rectangle };
    QRubberBand(Shape, QWidget * =0);
    ~QRubberBand();

    Shape shape() const;

    void move(const QPoint &);
    void move(int, int);
    void resize(const QSize &);
    void resize(int, int);
    void setGeometry(const QRect &);
    void setGeometry(int x, int y, int w, int h);

protected:
    virtual void drawRubberBandMask(QPainter *);
    virtual void drawRubberBand(QPainter *);

    void paintEvent(QPaintEvent *);
    void changeEvent(QEvent *);
private:
    void updateMask();
};

inline void QRubberBand::move(const QPoint &p)
{ move( p.x(), p.y() ); }

inline void QRubberBand::move(int x, int y)
{ setGeometry(x + geometry().x() - QWidget::x(), y - geometry().y() - QWidget::y(), width(), height()); }

inline void QRubberBand::resize(const QSize &s)
{ resize( s.width(), s.height()); }

inline void QRubberBand::resize(int w, int h)
{ setGeometry(geometry().x(), geometry().y(), w, h); }

inline void QRubberBand::setGeometry(const QRect &r)
{ setGeometry( r.left(), r.top(), r.width(), r.height() ); }


#endif /* __QRUBBERBAND_H__ */
