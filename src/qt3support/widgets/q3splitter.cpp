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

#include "q3splitter.h"
#include "private/qsplitter_p.h"


class Q3SplitterPrivate : public QSplitterPrivate
{
    Q_DECLARE_PUBLIC(Q3Splitter)
public:
    QSplitterLayoutStruct *findWidget(QWidget *) const;
};


QSplitterLayoutStruct *Q3SplitterPrivate::findWidget(QWidget *w) const
{
    for (int i = 0; i < list.size(); ++i) {
        if (list.at(i)->widget == w)
            return list.at(i);
    }
    return 0;
}

/*! \ class Q3Splitter

    \compat
    \since 4.1
*/

/*!
    Create a new Q3Splitter with the given \a parent and \a name.
*/
Q3Splitter::Q3Splitter(QWidget *parent, const char *name)
    : QSplitter(parent, name)
{
}

/*!
    Create a new Q3Splitter with \a orientation, given \a parent and \a name.
*/
Q3Splitter::Q3Splitter(Qt::Orientation orientation, QWidget *parent, const char *name)
    : QSplitter(orientation, parent, name)
{
}

/*!
  \reimp
*/
void Q3Splitter::childEvent(QChildEvent *event)
{
    if (!event->child()->isWidgetType())
        return;
    QWidget *w = static_cast<QWidget*>(event->child());
    if (event->added() && !d_func()->blockChildAdd && !w->isWindow() && !d_func()->findWidget(w)) {
        addWidget(w);
    } else {
        QSplitter::childEvent(event); // Handle all the goodness done by QSplitter
    }
}
