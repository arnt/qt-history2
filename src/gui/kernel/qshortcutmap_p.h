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

#ifndef QSHORTCUTMAP_P_H
#define QSHORTCUTMAP_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qkeysequence.h"
#include <qvector.h>

// To enable dump output uncomment below
//#define Dump_QShortcutMap

class QKeyEvent;
struct QShortcutEntry;
class QShortcutMapPrivate;
class QWidget;
class QAction;
class QObject;

class QShortcutMap
{
    Q_DECLARE_PRIVATE(QShortcutMap)
public:
    QShortcutMap();
    ~QShortcutMap();

    int addShortcut(QObject *owner, const QKeySequence &key, Qt::ShortcutContext context);
    int removeShortcut(int id, QObject *owner, const QKeySequence &key = QKeySequence());
    int setShortcutEnabled(bool enable, int id, QObject *owner, const QKeySequence &key = QKeySequence());

    void resetState();
    QKeySequence::SequenceMatch nextState(QKeyEvent *e);
    QKeySequence::SequenceMatch state();
    void dispatchEvent();
    bool tryShortcutEvent(QWidget *w, QKeyEvent *e);

#ifdef Dump_QShortcutMap
    void dumpMap() const;
#endif

private:
    bool correctContext(Qt::ShortcutContext context, QWidget *w, QWidget *active_window);
    bool correctContext(Qt::ShortcutContext context,QAction *a, QWidget *active_window);
    QShortcutMapPrivate *d_ptr;

    QKeySequence::SequenceMatch find(QKeyEvent *e);
    QVector<const QShortcutEntry *> matches() const;
    void createNewSequence(QKeyEvent *e, QKeySequence &seq);
    void clearSequence(QKeySequence &seq);
    bool correctContext(const QShortcutEntry &item);
    int translateModifiers(Qt::KeyboardModifiers modifiers);
};

#endif // QSHORTCUTMAP_P_H
