/****************************************************************************
**
** Definition of QSplashScreen class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSPLASHSCREEN_H
#define QSPLASHSCREEN_H

#ifndef QT_H
#include "qpixmap.h"
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_SPLASHSCREEN
class QSplashScreenPrivate;

class Q_GUI_EXPORT QSplashScreen : public QWidget
{
    Q_OBJECT
public:
    QSplashScreen( const QPixmap &pixmap = QPixmap(), WFlags f = 0 );
    virtual ~QSplashScreen();

    void setPixmap( const QPixmap &pixmap );
    QPixmap* pixmap() const;
    void finish( QWidget *w );
    void repaint();

public slots:
    void message( const QString &str, int flags = AlignLeft,
		  const QColor &color = black );
    void clear();

signals:
    void messageChanged( const QString &str );

protected:
    virtual void drawContents( QPainter *painter );
    void mousePressEvent( QMouseEvent * );

private:
    void drawContents();

    QSplashScreenPrivate *d;
};
#endif //QT_NO_SPLASHSCREEN
#endif
