//depot/qt/main/src/kernel/qdesktopwidget.h#4 - edit change 33507 (text)
#ifndef QDESKTOPWIDGET_H
#define QDESKTOPWIDGET_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QDesktopWidgetPrivate; /* Don't touch! */

class Q_EXPORT QDesktopWidget : public QWidget
{
    Q_OBJECT
public:
    QDesktopWidget();
    ~QDesktopWidget();

    bool isVirtualDesktop() const;

    int numScreens() const;
    int primaryScreen() const;

    int screenNumber( QWidget *widget = 0 ) const;
    int screenNumber( const QPoint & ) const;

    QWidget *screen( int screen = -1 );
    
    const QRect& screenGeometry( int screen = -1 ) const;

private:
    QDesktopWidgetPrivate *d;
};

#endif //QDESKTOPWIDGET_H
