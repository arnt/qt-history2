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

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#include "QtGui/qwidget.h"

#ifndef QT_NO_STATUSBAR

class QStatusBarPrivate;

class Q_GUI_EXPORT QStatusBar: public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool sizeGripEnabled READ isSizeGripEnabled WRITE setSizeGripEnabled)

public:
    QStatusBar(QWidget* parent, const char* name);
    QStatusBar(QWidget* parent=0);
    virtual ~QStatusBar();

    virtual void addWidget(QWidget *, int stretch = 0, bool = false);
    virtual void removeWidget(QWidget *);

    void setSizeGripEnabled(bool);
    bool isSizeGripEnabled() const;

public slots:
    void message(const QString &);
    void message(const QString &, int);
    void clear();

signals:
    void messageChanged(const QString &text);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

    void reformat();
    void hideOrShow();
    bool event(QEvent *);

private:
    Q_DISABLE_COPY(QStatusBar)
    Q_DECLARE_PRIVATE(QStatusBar)
};

#endif // QT_NO_STATUSBAR

#endif // QSTATUSBAR_H
