#ifndef FORMEDITOR_H
#define FORMEDITOR_H

#include "formeditor_global.h"

#include <abstractformeditor.h>

#include <QObject>

class QT_FORMEDITOR_EXPORT FormEditor: public AbstractFormEditor
{
    Q_OBJECT
public:
    FormEditor(QObject *parent = 0);
    virtual ~FormEditor();
};

#endif
