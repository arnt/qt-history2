// Copyright (c) 2000 Brad Hughes <bhughes@trolltech.com>
//
// Use, modification and distribution is allowed without limitation,
// warranty, or liability of any kind.
//

#ifndef __colorbutton_h
#define __colorbutton_h

class ColorButton;

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


#endif // __colorbutton_h
