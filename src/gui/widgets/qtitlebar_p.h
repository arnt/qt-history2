/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTITLEBAR_P_H
#define QTITLEBAR_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qworkspace.cpp and qdockwindow.cpp.  This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//
//


#ifndef QT_H
#include "qbutton.h"
#include "qlabel.h"
#endif // QT_H

#if !defined(QT_NO_TITLEBAR)

class QToolTip;
class QTitleBarPrivate;
class QPixmap;

class Q_GUI_EXPORT QTitleBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)

public:
    QTitleBar (QWidget* w, QWidget* parent, const char* name=0);
    ~QTitleBar();

    bool isActive() const;
    bool usesActiveColor() const;

    bool isMovable() const;
    void setMovable(bool);

    bool autoRaise() const;
    void setAutoRaise(bool);

    QWidget *window() const;

    QSize sizeHint() const;

public slots:
    void setActive(bool);

signals:
    void doActivate();
    void doNormal();
    void doClose();
    void doMaximize();
    void doMinimize();
    void doShade();
    void showOperationMenu();
    void popupOperationMenu(const QPoint&);
    void doubleClicked();

protected:
    bool event(QEvent *);
    void resizeEvent(QResizeEvent *);
    void contextMenuEvent(QContextMenuEvent *);
    void changeEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void paintEvent(QPaintEvent *p);

    virtual void cutText();

private:
    void readColors();

    QTitleBarPrivate *d;
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QTitleBar(const QTitleBar &);
    QTitleBar &operator=(const QTitleBar &);
#endif
};

#endif
#endif //QTITLEBAR_P_H
