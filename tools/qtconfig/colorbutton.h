/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Configuration.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <qbutton.h>


class ColorButton : public QButton
{
    Q_OBJECT

public:
    ColorButton(QWidget *, const char * = 0);
    ColorButton(const QColor &, QWidget *, const char * = 0);

    const QColor &color() const { return col; }

    void setColor(const QColor &);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);


signals:
    void colorChanged(const QColor &);


protected:
    void drawButton(QPainter *);
    void drawButtonLabel(QPainter *);


private slots:
    void changeColor();


private:
    QColor col;
    QPoint presspos;
    bool mousepressed;
};


#endif // COLORBUTTON_H
