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

#include "qkeysequence.h"
#include <qvector.h>

// To enable dump output uncomment below
//#define Dump_QShortcutMap

class QKeyEvent;
struct QShortcutEntry;
class QShortcutMapPrivate;
class QWidget;

class QShortcutMap
{
    Q_DECLARE_PRIVATE(QShortcutMap)
public:
    QShortcutMap();
    ~QShortcutMap();

    int addShortcut(const QWidget *owner, const QObject *monitor, const QKeySequence &key, Qt::ShortcutContext context);
    int removeShortcut(int id, const QWidget *owner, const QObject *monitor, const QKeySequence &key = QKeySequence());
    int setShortcutEnabled(bool enable, int id, const QWidget *owner, const QObject *monitor, const QKeySequence &key = QKeySequence());
    int changeMonitor(const QObject *monitor, const QKeySequence &key, bool enabled);


    void resetState();
    QKeySequence::SequenceMatch nextState(QKeyEvent *e);
    QKeySequence::SequenceMatch state();
    void dispatchEvent();
    bool tryShortcutEvent(QWidget *w, QKeyEvent *e);

#ifdef Dump_QShortcutMap
    void dumpMap() const;
#endif

private:
    QShortcutMapPrivate *d_ptr;

    QKeySequence::SequenceMatch find(QKeyEvent *e);
    QVector<const QShortcutEntry *> matches() const;
    void createNewSequence(QKeyEvent *e, QKeySequence &seq);
    void clearSequence(QKeySequence &seq);
    bool correctContext(const QShortcutEntry &item);
    int translateModifiers(Qt::ButtonState state);
};

#endif // QSHORTCUTMAP_P_H
