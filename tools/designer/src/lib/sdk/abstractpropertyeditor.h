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

#ifndef ABSTRACTPROPERTYEDITOR_H
#define ABSTRACTPROPERTYEDITOR_H

#include "sdk_global.h"

#include <QWidget>

class AbstractFormEditor;
class QString;
class QVariant;

class QT_SDK_EXPORT AbstractPropertyEditor: public QWidget
{
    Q_OBJECT
public:
    AbstractPropertyEditor(QWidget *parent, Qt::WindowFlags flags = 0);
    virtual ~AbstractPropertyEditor();

    virtual AbstractFormEditor *core() const;
    
    virtual bool isReadOnly() const = 0;
    virtual QObject *object() const = 0;

signals:
    void propertyChanged(const QString &name, const QVariant &value);

public slots:
    virtual void setObject(QObject *object) = 0;
    virtual void setPropertyValue(const QString &name, const QVariant &value, bool changed = true) = 0;
    virtual void setReadOnly(bool readOnly) = 0;
};

#endif // ABSTRACTPROPERTYEDITOR_H
