#ifndef ABSTRACTUNDOMANAGER_H
#define ABSTRACTUNDOMANAGER_H

#include "sdk_global.h"

#include <QObject>

class AbstractFormEditor;

class QT_SDK_EXPORT AbstractUndoManager: public QObject
{
    Q_OBJECT
public:
    AbstractUndoManager(QObject *parent);
    virtual ~AbstractUndoManager();

    virtual AbstractFormEditor *core() const = 0;
};

#endif // ABSTRACTUNDOMANAGER_H
