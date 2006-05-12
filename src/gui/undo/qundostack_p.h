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

#ifndef QUNDOSTACK_P_H
#define QUNDOSTACK_P_H

#include <private/qobject_p.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtGui/qaction.h>

#include "qundostack.h"
class QCommand;
class QUndoGroup;

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

struct QCommandPrivate
{
    QCommandPrivate() : id(-1) {}
    QList<QCommand*> child_list;
    QString text;
    int id;
};

class QUndoStackPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QUndoStack)
public:
    QUndoStackPrivate() : index(0), clean_index(0), group(0) {}

    QList<QCommand*> command_list;
    QList<QCommand*> macro_stack;
    int index;
    int clean_index;
    QUndoGroup *group;

    void setIndex(int idx, bool clean);
};

class QUndoAction : public QAction
{
    Q_OBJECT
public:
    QUndoAction(const QString &prefix, QObject *parent = 0);
public slots:
    void setPrefixedText(const QString &text);
private:
    QString m_prefix;
};

#endif // QUNDOSTACK_P_H
