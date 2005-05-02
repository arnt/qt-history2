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

#include "QtGui/qpixmap.h"
#include "QtGui/qwidget.h"

#ifndef QT_NO_SPLASHSCREEN
class QSplashScreenPrivate;

class Q_GUI_EXPORT QSplashScreen : public QWidget
{
    Q_OBJECT
public:
    explicit QSplashScreen(const QPixmap &pixmap = QPixmap(), Qt::WFlags f = 0);
    QSplashScreen(QWidget *parent, const QPixmap &pixmap = QPixmap(), Qt::WFlags f = 0);
    virtual ~QSplashScreen();

    void setPixmap(const QPixmap &pixmap);
    const QPixmap pixmap() const;
    void finish(QWidget *w);
    void repaint();

public slots:
    void showMessage(const QString &message, int alignment = Qt::AlignLeft,
                  const QColor &color = Qt::black);
    void clearMessage();
#ifdef QT3_SUPPORT
    inline QT_MOC_COMPAT void message(const QString &str, int alignment = Qt::AlignLeft,
        const QColor &color = Qt::black) { showMessage(str, alignment, color); }
    inline QT_MOC_COMPAT void clear() { clearMessage(); }
#endif

signals:
    void messageChanged(const QString &message);

protected:
    virtual void drawContents(QPainter *painter);
    void mousePressEvent(QMouseEvent *);

private:
    Q_DISABLE_COPY(QSplashScreen)
    Q_DECLARE_PRIVATE(QSplashScreen)
};
#endif // QT_NO_SPLASHSCREEN

#endif // QSPLASHSCREEN_H
