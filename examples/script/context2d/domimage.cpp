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

#include "domimage.h"

#include <QVariant>

#include <qscriptcontext.h>

QScriptValue DomImage::s_self;

DomImage::DomImage()
{
}


int DomImage::width() const
{
    return m_image.width();
}


int DomImage::height() const
{
    return m_image.height();
}


QString DomImage::src() const
{
    return m_src;
}

void DomImage::setSrc(const QString &src)
{
    m_src = src;
    m_image = QPixmap(m_src);
}


QString DomImage::name() const
{
    return m_src;
}

static QScriptValue Image(QScriptContext *context, QScriptEngine *env)
{
    QScriptValue val = context->thisObject();
    DomImage *image = new DomImage();
    QScriptValue klass = env->newVariant(qVariantFromValue(image));
    klass.setPrototype(DomImage::s_self);
    return klass;
}


static QScriptValue width(QScriptContext *context, QScriptEngine *env)
{
    QScriptValue val = context->thisObject();

    DomImage *image = qvariant_cast<DomImage*> (val.toVariant());
    if (image)
        return QScriptValue(env, image->width());

    return QScriptValue(env, 0);
}


static QScriptValue height(QScriptContext *context, QScriptEngine *env)
{
    QScriptValue val = context->thisObject();

    DomImage *image = qvariant_cast<DomImage*> (val.toVariant());
    if (image)
        return QScriptValue(env, image->height());

    return QScriptValue(env, 0);
}


static QScriptValue setSrc(QScriptContext *context, QScriptEngine *env)
{
    QScriptValue val = context->thisObject();
    QString src  = context->argument(0).toString();

    DomImage *image = qvariant_cast<DomImage*> (val.toVariant());
    if (image)
        image->setSrc(src);

    return env->undefinedValue();
}


static QScriptValue name(QScriptContext *context, QScriptEngine *env)
{
    QScriptValue val = context->thisObject();

    DomImage *image = qvariant_cast<DomImage*> (val.toVariant());
    if (image)
        return QScriptValue(env, image->name());

    return QScriptValue(env, QString());
}


void DomImage::setup(QScriptEngine *e)
{
    qRegisterMetaType<DomImage>();

    e->globalObject().setProperty("Image",
                                  e->newFunction(::Image, 0));

    s_self = e->newObject();
    s_self.setProperty("setSrc", e->newFunction(&::setSrc, 1));
    s_self.setProperty("width", e->newFunction(&::width));
    s_self.setProperty("height", e->newFunction(&::height));
    s_self.setProperty("name", e->newFunction(&::name));

    e->setDefaultPrototype(qMetaTypeId<DomImage>(), s_self);
}
