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

#include "qtextcodecfactory.h"
#include "qcoreapplication.h"
#include "qtextcodecplugin.h"
#include "private/qfactoryloader_p.h"

#ifndef QT_NO_COMPONENT
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QTextCodecFactoryInterface_iid, QCoreApplication::libraryPaths(), QLatin1String("/codecs")))
#endif

QTextCodec *QTextCodecFactory::createForName(const QString &name)
{
#ifndef QT_NO_COMPONENT
    if (QTextCodecFactoryInterface *factory =
        qt_cast<QTextCodecFactoryInterface*>(loader()->instance(name)))
        return factory->create(name);
#endif
    return 0;
}

QTextCodec *QTextCodecFactory::createForMib(int mib)
{
#ifndef QT_NO_COMPONENT
    QString name = QLatin1String("MIB-") + QString::number(mib);
    if (QTextCodecFactoryInterface *factory =
        qt_cast<QTextCodecFactoryInterface*>(loader()->instance(name)))
        return factory->create(name);
#endif
    return 0;
}
