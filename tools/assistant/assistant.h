#ifndef ASSISTANT_H
#define ASSISTANT_H

#include <qwidget.h>

class QSizeGrip;
class QTextBrowser;
class QToolButton;

class Assistant : public QWidget
{
    Q_OBJECT

public:
    Assistant( QWidget *parent );

protected:
    void paintEvent( QPaintEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void resizeEvent( QResizeEvent *e );

private:
    QPoint offset;
    QSizeGrip *sizeGrip;
    QTextBrowser *browser;
    QToolButton *closeButton;
};


#endif
