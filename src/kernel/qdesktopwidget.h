#ifndef QDESKTOPWIDGET_H
#define QDESKTOPWIDGET_H

#include "qwidget.h"

class QDesktopWidgetPrivate;

class Q_EXPORT QDesktopWidget : public QWidget
{
    Q_OBJECT
public:
    QDesktopWidget();
    ~QDesktopWidget();

    int numScreens() const;
    int primaryScreen() const;

    QWidget *screen( int screen = -1 );
    QRect geometry( int screen = - 1 ) const;

private:
    QDesktopWidgetPrivate *d;
};

#endif //QDESKTOPWIDGET_H
