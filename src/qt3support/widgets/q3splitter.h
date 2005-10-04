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

#ifndef Q3SPLITTER_H
#define Q3SPLITTER_H

#include <QtGui/qsplitter.h>

QT_MODULE(Qt3SupportLight)

class QChildEvent;
class Q3SplitterPrivate;
class Q_COMPAT_EXPORT Q3Splitter : public QSplitter
{
    Q_OBJECT
public:
    Q3Splitter(QWidget *parent = 0, const char *name = 0);
    Q3Splitter(Qt::Orientation, QWidget *parent = 0, const char *name = 0);

protected:
    void childEvent(QChildEvent *event);
private:
    Q_DISABLE_COPY(Q3Splitter)
    Q_DECLARE_PRIVATE(Q3Splitter)
};

#endif
