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

#ifndef QSTACKEDLAYOUT_H
#define QSTACKEDLAYOUT_H

#include <qlayout.h>

class QStackedLayoutPrivate;

class Q_GUI_EXPORT QStackedLayout : public QLayout
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QStackedLayout)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    QDOC_PROPERTY(int count READ count)
public:
    QStackedLayout(QWidget *parent);
    QStackedLayout(QLayout *parentLayout);
    ~QStackedLayout();

    int addWidget(QWidget *w);
    int insertWidget(int index, QWidget *w);

    QWidget *currentWidget() const;
    int currentIndex() const;
    int indexOf(QWidget *) const;
    QWidget *widget(int) const;
    int count() const;

    void setCurrentIndex(int);

    // abstract virtual functions:
   void addItem(QLayoutItem *item);
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutItem *itemAt(int) const;
    QLayoutItem *takeAt(int);
    void setGeometry(const QRect &rect);

signals:
    void widgetRemoved(int index);
private:
#if defined(Q_DISABLE_COPY)
    QStackedLayout(const QStackedLayout &);
    QStackedLayout &operator=(const QStackedLayout &);
#endif

};




#endif
