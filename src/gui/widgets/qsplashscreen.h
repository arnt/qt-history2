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

#ifndef QSPLASHSCREEN_H
#define QSPLASHSCREEN_H

#include "qpixmap.h"
#include "qwidget.h"

#ifndef QT_NO_SPLASHSCREEN
class QSplashScreenPrivate;

class Q_GUI_EXPORT QSplashScreen : public QWidget
{
    Q_OBJECT
public:
    QSplashScreen(const QPixmap &pixmap = QPixmap(), Qt::WFlags f = 0);
    virtual ~QSplashScreen();

    void setPixmap(const QPixmap &pixmap);
    QPixmap pixmap() const;
    void finish(QWidget *w);
    void repaint();

public slots:
    void message(const QString &str, int flags = Qt::AlignLeft,
                  const QColor &color = Qt::black);
    void clear();

signals:
    void messageChanged(const QString &str);

protected:
    virtual void drawContents(QPainter *painter);
    void mousePressEvent(QMouseEvent *);

private:
    Q_DISABLE_COPY(QSplashScreen)
    Q_DECLARE_PRIVATE(QSplashScreen)
};
#endif //QT_NO_SPLASHSCREEN
#endif
