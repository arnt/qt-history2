/****************************************************************************
**
** Definition of QRubberBand class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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

protected:
    virtual void drawRubberBandMask(QPainter *);
    virtual void drawRubberBand(QPainter *);
    void paintEvent(QPaintEvent *);
    void changeEvent(QEvent *);
private:
    void updateMask();
};

#endif /* __QRUBBERBAND_H__ */
