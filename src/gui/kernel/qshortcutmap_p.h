#ifndef QSHORTCUTMAP_H
#define QSHORTCUTMAP_H

#include "qkeysequence.h"
#include <qvector.h>

// To enable dump output uncomment below
//#define Dump_QShortcutMap

class QKeyEvent;
class QShortcutMapPrivate;
struct QShortcutEntry;
class QWidget;

class QShortcutMap
{
    Q_DECLARE_PRIVATE(QShortcutMap);
public:
    QShortcutMap();
    ~QShortcutMap();

    int addShortcut(const QWidget *owner, const QKeySequence &key);
    int removeShortcut(const QWidget *owner, const QKeySequence &key, int id = 0);
    int setShortcutEnabled(bool enable, const QWidget *owner, const QKeySequence &key, int id = 0);

    void resetState();
    Qt::SequenceMatch nextState(QKeyEvent *e);
    Qt::SequenceMatch state();
    void dispatchEvent();
    bool tryShortcutEvent(QWidget *w, QKeyEvent *e);

#ifdef Dump_QShortcutMap
    void dumpMap() const;
#endif

private:
    QShortcutMapPrivate *d_ptr;

    Qt::SequenceMatch find(QKeyEvent *e);
    QVector<const QShortcutEntry *> matches() const;
    void createNewSequence(QKeyEvent *e, QKeySequence &seq);
    void clearSequence(QKeySequence &seq);
    bool correctSubWindow(const QWidget* w);
    int translateModifiers(Qt::ButtonState state);
};

#endif // QSHORTCUTMAP_H
