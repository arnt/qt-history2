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

#ifndef QITEMEDITORFACTORY_H
#define QITEMEDITORFACTORY_H

#include <qbytearray.h>
#include <qhash.h>
#include <qvariant.h>

class QWidget;

class QItemEditorCreatorBase
{
public:
    virtual QWidget *createWidget(const QWidget *parent = 0) const = 0;
    virtual QByteArray valuePropertyName() const = 0;
};

template <class T>
class QItemEditorCreator : public QItemEditorCreatorBase
{
public:
    inline QItemEditorCreator(const QByteArray &valuePropertyName)
        : propertyName(valuePropertyName) {}
    inline QWidget *createWidget(QWidget *parent) const { return new T(parent); }
    inline QByteArray valuePropertyName() const { return propertyName; }

private:
    QByteArray propertyName;
};

class QItemEditorFactory
{
public:
    virtual QWidget *createEditor(QVariant::Type type, QWidget *parent) const;
    virtual QByteArray valuePropertyName(QVariant::Type type) const;
    virtual ~QItemEditorFactory();

    void registerEditor(QVariant::Type type, QItemEditorCreatorBase *creator);

    static const QItemEditorFactory *defaultFactory();
    static void setDefaultFactory(QItemEditorFactory *factory);

private:
    QHash<QVariant::Type, QItemEditorCreatorBase *> creatorMap;
};

#endif
