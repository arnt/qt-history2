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

#include "qcustomfontengine_qws.h"

#include <private/qtextengine_p.h>

QCustomFontInfo::QCustomFontInfo()
{
}

QCustomFontInfo::QCustomFontInfo(const QString &family)
{
    setFamily(family);
}

QCustomFontInfo::QCustomFontInfo(const QCustomFontInfo &other)
    : QHash<int, QVariant>(other)
{
}

QCustomFontInfo &QCustomFontInfo::operator=(const QCustomFontInfo &other)
{
    QHash<int, QVariant>::operator=(other);
    return *this;
}

void QCustomFontInfo::setPixelSize(qreal size)
{
    insert(PixelSize, QFixed::fromReal(size).value());
}

qreal QCustomFontInfo::pixelSize() const
{
    return QFixed::fromFixed(value(PixelSize).toInt()).toReal();
}

class QCustomFontEnginePluginPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QCustomFontEnginePlugin)

    QString foundry;
};

QCustomFontEnginePlugin::QCustomFontEnginePlugin(const QString &foundry, QObject *parent)
    : QObject(*new QCustomFontEnginePluginPrivate, parent)
{
    Q_D(QCustomFontEnginePlugin);
    d->foundry = foundry;
}

QCustomFontEnginePlugin::~QCustomFontEnginePlugin()
{
}

QStringList QCustomFontEnginePlugin::keys() const
{
    Q_D(const QCustomFontEnginePlugin);
    return QStringList(d->foundry);
}

