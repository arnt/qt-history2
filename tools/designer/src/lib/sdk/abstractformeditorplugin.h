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

#ifndef ABSTRACTFORMEDITORPLUGIN_H
#define ABSTRACTFORMEDITORPLUGIN_H

#include "sdk_global.h"

#include <QtCore/QObject>

class AbstractFormEditor;
class QAction;

struct QT_SDK_EXPORT AbstractFormEditorPlugin
{
    virtual ~AbstractFormEditorPlugin() {}

    virtual bool isInitialized() const = 0;
    virtual void initialize(AbstractFormEditor *core) = 0;
    virtual QAction *action() const = 0;

    virtual AbstractFormEditor *core() const = 0;
};
Q_DECLARE_INTERFACE(AbstractFormEditorPlugin, "http://trolltech.com/Qt/IDE/AbstractFormEditorPlugin")

#endif // ABSTRACTFORMEDITORPLUGIN_H
