#ifndef QDESKTOPWIDGET_H
#define QDESKTOPWIDGET_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class Q_EXPORT QDesktopWidget : public QWidget
{
    Q_OBJECT
public:
    QDesktopWidget();
    ~QDesktopWidget();

    int numScreens() const;
    int primaryScreen() const;

    int screenNumber( QWidget *widget = 0 ) const;
    int screenNumber( const QPoint & ) const;

    QWidget *screen( int screen = -1 );

    const QRect& screenGeometry( int screen = -1 ) const;

private:
    class Private;
    Private *d;
};

#endif //QDESKTOPWIDGET_H
