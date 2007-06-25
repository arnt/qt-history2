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

#include "qdesigner_membersheet_p.h"
#include <private/qobject_p.h>

#include <QtGui/QWidget>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>

// ------------ QDesignerMemberSheetPrivate
class QDesignerMemberSheetPrivate : public  QObjectPrivate {
public:
    explicit QDesignerMemberSheetPrivate(QObject *object);

    const QMetaObject *m_meta;

    class Info {
    public:
        inline Info() : visible(true) {}

        QString group;
        bool visible;
    };

    typedef QHash<int, Info> InfoHash;

    Info &ensureInfo(int index);

    InfoHash m_info;
};

QDesignerMemberSheetPrivate::QDesignerMemberSheetPrivate(QObject *object) :
    m_meta(object->metaObject())
{
}

QDesignerMemberSheetPrivate::Info &QDesignerMemberSheetPrivate::ensureInfo(int index)
{
    InfoHash::iterator it = m_info.find(index);
    if (it == m_info.end()) {
        it = m_info.insert(index, Info());
    }
    return it.value();
}

// --------- QDesignerMemberSheet

QDesignerMemberSheet::QDesignerMemberSheet(QObject *object, QObject *parent) :
    QObject(*(new QDesignerMemberSheetPrivate(object)), parent)
{
}

QDesignerMemberSheet::~QDesignerMemberSheet()
{
}

int QDesignerMemberSheet::count() const
{
    Q_D(const QDesignerMemberSheet);
    return d->m_meta->methodCount();
}

int QDesignerMemberSheet::indexOf(const QString &name) const
{
    Q_D(const QDesignerMemberSheet);
    return d->m_meta->indexOfMethod(name.toUtf8());
}

QString QDesignerMemberSheet::memberName(int index) const
{
    Q_D(const QDesignerMemberSheet);
    return QString::fromUtf8(d->m_meta->method(index).tag());
}

QString QDesignerMemberSheet::declaredInClass(int index) const
{
    Q_D(const QDesignerMemberSheet);
    const char *member = d->m_meta->method(index).signature();

    // Find class whose superclass does not contain the method.
    const QMetaObject *meta_obj = d->m_meta;

    for (;;) {
        const QMetaObject *tmp = meta_obj->superClass();
        if (tmp == 0)
            break;
        if (tmp->indexOfMethod(member) == -1)
            break;
        meta_obj = tmp;
    }

    return QLatin1String(meta_obj->className());
}

QString QDesignerMemberSheet::memberGroup(int index) const
{
    Q_D(const QDesignerMemberSheet);
    return d->m_info.value(index).group;
}

void QDesignerMemberSheet::setMemberGroup(int index, const QString &group)
{
    Q_D(QDesignerMemberSheet);
    d->ensureInfo(index).group = group;
}

QString QDesignerMemberSheet::signature(int index) const
{
    Q_D(const QDesignerMemberSheet);
    return QString::fromUtf8(QMetaObject::normalizedSignature(d->m_meta->method(index).signature()));
}

bool QDesignerMemberSheet::isVisible(int index) const
{
    typedef QDesignerMemberSheetPrivate::InfoHash InfoHash;
    Q_D(const QDesignerMemberSheet);
    const InfoHash::const_iterator it = d->m_info.constFind(index);
    if (it != d->m_info.constEnd())
        return it.value().visible;

   return d->m_meta->method(index).methodType() == QMetaMethod::Signal
           || d->m_meta->method(index).access() == QMetaMethod::Public;
}

void QDesignerMemberSheet::setVisible(int index, bool visible)
{
    Q_D(QDesignerMemberSheet);
    d->ensureInfo(index).visible = visible;
}

bool QDesignerMemberSheet::isSignal(int index) const
{
    Q_D(const QDesignerMemberSheet);
    return d->m_meta->method(index).methodType() == QMetaMethod::Signal;
}

bool QDesignerMemberSheet::isSlot(int index) const
{
    Q_D(const QDesignerMemberSheet);
    return d->m_meta->method(index).methodType() == QMetaMethod::Slot;
}

bool QDesignerMemberSheet::inheritedFromWidget(int index) const
{
    Q_D(const QDesignerMemberSheet);
    const char *name = d->m_meta->method(index).signature();
    return QWidget::staticMetaObject.indexOfMethod(name) != -1;
}


QList<QByteArray> QDesignerMemberSheet::parameterTypes(int index) const
{
    Q_D(const QDesignerMemberSheet);
    return d->m_meta->method(index).parameterTypes();
}

QList<QByteArray> QDesignerMemberSheet::parameterNames(int index) const
{
    Q_D(const QDesignerMemberSheet);
    return d->m_meta->method(index).parameterNames();
}

bool QDesignerMemberSheet::signalMatchesSlot(const QString &signal, const QString &slot)
{
    bool result = true;

    do {
        int signal_idx = signal.indexOf(QLatin1Char('('));
        int slot_idx = slot.indexOf(QLatin1Char('('));
        if (signal_idx == -1 || slot_idx == -1)
            break;

        ++signal_idx; ++slot_idx;

        if (slot.at(slot_idx) == QLatin1Char(')'))
            break;

        while (signal_idx < signal.size() && slot_idx < slot.size()) {
            const QChar signal_c = signal.at(signal_idx);
            const QChar slot_c = slot.at(slot_idx);

            if (signal_c == QLatin1Char(',') && slot_c == QLatin1Char(')'))
                break;

            if (signal_c == QLatin1Char(')') && slot_c == QLatin1Char(')'))
                break;

            if (signal_c != slot_c) {
                result = false;
                break;
            }

            ++signal_idx; ++slot_idx;
        }
    } while (false);

    return result;
}

// ------------ QDesignerMemberSheetFactory

QDesignerMemberSheetFactory::QDesignerMemberSheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerMemberSheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(QDesignerMemberSheetExtension)) {
        return new QDesignerMemberSheet(object, parent);
    }

    return 0;
}
