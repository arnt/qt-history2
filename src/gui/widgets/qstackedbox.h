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

#ifndef QSTACKEDBOX_H
#define QSTACKEDBOX_H

#include <qframe.h>

class QStackedBoxPrivate;

class Q_GUI_EXPORT QStackedBox : public QFrame
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QStackedBox)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    QDOC_PROPERTY(int count READ count)
public:
    QStackedBox(QWidget *parent=0);
    ~QStackedBox();

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
#if defined(Q_DISABLE_COPY)
    QStackedBox(const QStackedBox &);
    QStackedBox &operator=(const QStackedBox &);
#endif

};


#endif
