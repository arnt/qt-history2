#ifndef QOBJECTCLEANUPHANDLER_H
#define QOBJECTCLEANUPHANDLER_H

#ifndef QT_H
#include <qobject.h>
#endif // QT_H

class QObjectList;

class Q_EXPORT QObjectCleanupHandler : public QObject
{
    Q_OBJECT

public:
    QObjectCleanupHandler();
    ~QObjectCleanupHandler();

    QObject* add( QObject* object );
    void remove( QObject *object );
    bool isEmpty() const;
    void clear();

private:
    QObjectList *cleanupObjects;

private slots:
    void objectDestroyed();
};

#endif // QOBJECTCLEANUPHANDLER_H
