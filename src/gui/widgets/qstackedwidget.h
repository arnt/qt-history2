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

#ifndef QSTACKEDWIDGET_H
#define QSTACKEDWIDGET_H

#include <qframe.h>

class QStackedWidgetPrivate;

class Q_GUI_EXPORT QStackedWidget : public QFrame
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QStackedWidget)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    QDOC_PROPERTY(int count READ count)
public:
    QStackedWidget(QWidget *parent=0);
    ~QStackedWidget();

    int addWidget(QWidget *w);
    int insertWidget(int index, QWidget *w);
    void removeWidget(QWidget *w);

    QWidget *currentWidget() const;
    int currentIndex() const;

    int indexOf(QWidget *) const;
    QWidget *widget(int) const;
    int count() const;

public slots:
    void setCurrentIndex(int);

signals:
    void currentChanged(int);
    void widgetRemoved(int index);

protected:
    void childEvent(QChildEvent *e);

private:
    Q_DISABLE_COPY(QStackedWidget)
};


#endif
