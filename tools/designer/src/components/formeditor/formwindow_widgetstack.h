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

#ifndef FORMWINDOW_WIDGETSTACK_H
#define FORMWINDOW_WIDGETSTACK_H

#include "formeditor_global.h"

#include <QtGui/QWidget>

class QT_FORMEDITOR_EXPORT FormWindowWidgetStack: public QWidget
{
    Q_OBJECT
public:
    FormWindowWidgetStack(QWidget *parent = 0);
    virtual ~FormWindowWidgetStack();

    int count() const;
    QWidget *widget(int index) const;
    int indexOf(QWidget *widget) const;

    int currentIndex() const;

signals:
    void currentIndexChanged(int index);

public slots:
    void setCurrentIndex(int index);
    void addWidget(QWidget *widget);
    void insertWidget(int index, QWidget *widget);

protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    QList<QWidget*> m_widgets;
    int m_currentIndex;
};

#endif // FORMWINDOW_WIDGETSTACK_H
