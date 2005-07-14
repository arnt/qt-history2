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

#ifndef QDESKTOPWIDGET_H
#define QDESKTOPWIDGET_H

#include "QtGui/qwidget.h"

QT_MODULE(Gui)

class QApplication;
class QDesktopWidgetPrivate;

class Q_GUI_EXPORT QDesktopWidget : public QWidget
{
    Q_OBJECT
public:
    QDesktopWidget();
    ~QDesktopWidget();

    bool isVirtualDesktop() const;

    int numScreens() const;
    int primaryScreen() const;

    int screenNumber(const QWidget *widget = 0) const;
    int screenNumber(const QPoint &) const;

    QWidget *screen(int screen = -1);

    const QRect screenGeometry(int screen = -1) const;
    const QRect screenGeometry(const QWidget *widget) const
    { return screenGeometry(screenNumber(widget)); }
    const QRect screenGeometry(const QPoint &point) const
    { return screenGeometry(screenNumber(point)); }

    const QRect availableGeometry(int screen = -1) const;
    const QRect availableGeometry(const QWidget *widget) const
    { return availableGeometry(screenNumber(widget)); }
    const QRect availableGeometry(const QPoint &point) const
    { return availableGeometry(screenNumber(point)); }

signals:
    void resized(int);
    void workAreaResized(int);

protected:
    void resizeEvent(QResizeEvent *e);

private:
    Q_DISABLE_COPY(QDesktopWidget)
    Q_DECLARE_PRIVATE(QDesktopWidget)

    friend class QApplication;
#ifdef Q_WS_QWS
    friend class QWSDisplay;
#endif
};

#endif // QDESKTOPWIDGET_H
