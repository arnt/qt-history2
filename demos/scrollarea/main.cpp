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

#include <QtGui>


class Grid : public QWidget
{
public:
    Grid(QWidget *parent = 0):QWidget(parent){}

    QSize sizeHint() const { return QSize(128000, 128000); }

    void paintEvent(QPaintEvent *e) {
        QPainter p(this);
        for (int y = e->rect().top() - e->rect().top() % 128; y <= e->rect().bottom(); y += 128) {
            p.drawLine(e->rect().left(), y, e->rect().right(), y);
            for (int x = e->rect().left() - e->rect().left() % 128; x <= e->rect().right(); x += 128) {
                p.drawLine(x, y, x, y + 128);
                p.drawText(x, y, 128, 128, Qt::AlignCenter, QString("%1/%2").arg(x).arg(y));
            }
        }
    }

    void mousePressEvent(QMouseEvent *e) {
        QPushButton *button = new QPushButton(this);
        button->setText(QString("%1/%2").arg(e->x()).arg(e->y()));
        button->adjustSize();
        button->move(e->pos() - button->rect().center());
        button->show();
    }

};


int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    QScrollArea *scroller = new QScrollArea;
    scroller->setWindowTitle( "Qt Demo - ScrollArea" );

    Grid *grid = new Grid;
    scroller->setWidget(grid);
    grid->setPalette(Qt::green);

    Grid *iframe = new Grid(scroller->widget());
    iframe->setPalette(Qt::magenta);
    iframe->setGeometry(256, 256, 512, 66560);

    QLabel * label = new QLabel(scroller->widget());
    label->setPalette(Qt::yellow);
    label->setWordWrap(true);
    label->setText(
        "This is an example for QScrollArea. "
        "It shows that you can have Qt widgets that exceed the "
        "built-in 16-bit limit of MS-Windows, X11 and Mac OS X. "
        "The green grid is one big widget that is 128000 x 128000 pixels large. It contains "
        "a magenta child widget that itself has a height of 66560 pixels. Click anywhere "
        "to add small child widgets.");
    label->setFrameStyle(QFrame::Box);
    label->setMargin(8);

    scroller->show();

    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
