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

#include <qwidget.h>
#include <qpainter.h>

class QAxWidget1 : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{1D9928BD-4453-4bdd-903D-E525ED17FDE5}")
    Q_CLASSINFO("InterfaceID", "{99F6860E-2C5A-42ec-87F2-43396F4BE389}")
    Q_CLASSINFO("EventsID", "{0A3E9F27-E4F1-45bb-9E47-63099BCCD0E3}")

    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor)
public:
    QAxWidget1(QWidget *parent = 0)
        : QWidget(parent), fill_color(Qt::red)
    {
    }

    QColor fillColor() const
    {
	return fill_color;
    }
    void setFillColor(const QColor &fc)
    {
	fill_color = fc;
	repaint();
    }

protected:
    void paintEvent(QPaintEvent *e)
    {
	QPainter paint(this);
	QRect r = rect();
	r.adjust(10, 10, -10, -10);
	paint.fillRect(r, fill_color);
    }

private:
    QColor fill_color;
};
