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

#ifndef QGFXDRIVERPLUGIN_QWS_H
#define QGFXDRIVERPLUGIN_QWS_H

#include "QtCore/qplugin.h"
#include "QtCore/qfactoryinterface.h"

#ifndef QT_NO_COMPONENT

class QScreen;

struct Q_GUI_EXPORT QGfxDriverFactoryInterface : public QFactoryInterface
{
    virtual QScreen* create(const QString& driver, int displayId) = 0;
};

Q_DECLARE_INTERFACE(QGfxDriverFactoryInterface, "com.trolltech.Qt.QGfxDriverFactoryInterface")

class Q_GUI_EXPORT QGfxDriverPlugin : public QObject, public QGfxDriverFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QGfxDriverFactoryInterface:QFactoryInterface)
public:
    explicit QGfxDriverPlugin(QObject *parent = 0);
    ~QGfxDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QScreen *create(const QString& driver, int displayId) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QGFXDRIVERPLUGIN_QWS_H
