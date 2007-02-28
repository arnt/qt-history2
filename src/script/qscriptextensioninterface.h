/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCRIPTEXTENSIONINTERFACE_H
#define QSCRIPTEXTENSIONINTERFACE_H

#include <QtCore/qfactoryinterface.h>
#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Script)

class QScriptEngine;

struct Q_SCRIPT_EXPORT QScriptExtensionInterface
    : public QFactoryInterface
{
    virtual void initialize(const QString &key, QScriptEngine *engine) = 0;
};

Q_DECLARE_INTERFACE(QScriptExtensionInterface, "com.trolltech.Qt.QScriptExtensionInterface/1.0")

QT_END_HEADER

#endif // QSCRIPTEXTENSIONINTERFACE_H
