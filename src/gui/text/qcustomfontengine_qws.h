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

#ifndef QCUSTOMFONTENGINE_QWS_H
#define QCUSTOMFONTENGINE_QWS_H

#include <QtCore/qobject.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class Q_GUI_EXPORT QCustomFontInfo : public QHash<int, QVariant>
{
    enum Property {
        Family,
        PixelSize,
        Weight,
        Style
    };

    QCustomFontInfo();
    explicit QCustomFontInfo(const QString &family);
    QCustomFontInfo(const QCustomFontInfo &other);
    QCustomFontInfo &operator=(const QCustomFontInfo &other);

    inline void setFamily(const QString &name)
    { insert(Family, name); }
    inline QString family() const
    { return value(Family).toString(); }

    void setPixelSize(qreal size);
    qreal pixelSize() const;

    inline void setWeight(int weight)
    { insert(Weight, weight); }
    inline int weight() const
    { return value(Weight).toInt(); }

    inline void setStyle(int style)
    { insert(Style, style); }
    inline int style() const
    { return value(Style).toInt(); }
};

class QCustomFontEngine;

class Q_GUI_EXPORT QCustomFontEngineFactory : public QObject
{
    Q_OBJECT
public:
    QCustomFontEngineFactory(QObject *parent = 0);

    virtual QCustomFontEngine *create(const QCustomFontInfo &info) = 0;
};

struct Q_GUI_EXPORT QCustomFontEngineFactoryInterface : public QFactoryInterface
{
     virtual QCustomFontEngine *create(const QCustomFontInfo &info) = 0;
     virtual QList<QCustomFontInfo> availableFonts() const = 0;
};

#define QCustomFontEngineFactoryInterface_iid "com.trolltech.Qt.QCustomFontEngineFactoryInterface"
Q_DECLARE_INTERFACE(QCustomFontEngineFactoryInterface, QCustomFontEngineFactoryInterface_iid)

class QCustomFontEnginePluginPrivate;

class QCustomFontEnginePlugin : public QObject, public QCustomFontEngineFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QCustomFontEngineFactoryInterface:QFactoryInterface)
public:
    QCustomFontEnginePlugin(const QString &foundry, QObject *parent = 0);
    ~QCustomFontEnginePlugin();

    virtual QStringList keys() const;

    virtual QCustomFontEngine *create(const QCustomFontInfo &info) = 0;
    virtual QList<QCustomFontInfo> availableFonts() const = 0;

private:
    Q_DECLARE_PRIVATE(QCustomFontEnginePlugin)
    Q_DISABLE_COPY(QCustomFontEnginePlugin)
};

QT_END_HEADER

#endif
