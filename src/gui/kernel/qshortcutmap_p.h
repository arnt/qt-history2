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
    Q_DECLARE_PRIVATE(QShortcutMap)
public:
    QShortcutMap();
    ~QShortcutMap();

    int addShortcut(const QWidget *owner, const QKeySequence &key);
    int removeShortcut(const QWidget *owner, int id, const QKeySequence &key = QKeySequence());
    int setShortcutEnabled(const QWidget *owner, int id, bool enable, const QKeySequence &key = QKeySequence());

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
