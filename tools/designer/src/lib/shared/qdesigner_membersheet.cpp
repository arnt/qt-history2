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

#include <QtGui/QWidget>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>

QDesignerMemberSheet::QDesignerMemberSheet(QObject *object, QObject *parent)
    : QObject(parent),
      m_meta(object->metaObject())
{
}

QDesignerMemberSheet::~QDesignerMemberSheet()
{
}

int QDesignerMemberSheet::count() const
{
    return m_meta->methodCount();
}

int QDesignerMemberSheet::indexOf(const QString &name) const
{
    return m_meta->indexOfMethod(name.toUtf8());
}

QString QDesignerMemberSheet::memberName(int index) const
{
    return QString::fromUtf8(m_meta->method(index).tag());
}

QString QDesignerMemberSheet::declaredInClass(int index) const
{
    const char *member = m_meta->method(index).signature();

    // Find class whose superclass does not contain the method.
    const QMetaObject *meta_obj = m_meta;

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
    return m_info.value(index).group;
}

void QDesignerMemberSheet::setMemberGroup(int index, const QString &group)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].group = group;
}

QString QDesignerMemberSheet::signature(int index) const
{
    return QString::fromUtf8(QMetaObject::normalizedSignature(m_meta->method(index).signature()));
}

bool QDesignerMemberSheet::isVisible(int index) const
{
    if (m_info.contains(index))
        return m_info.value(index).visible;

   return m_meta->method(index).methodType() == QMetaMethod::Signal
           || m_meta->method(index).access() == QMetaMethod::Public;
}

void QDesignerMemberSheet::setVisible(int index, bool visible)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].visible = visible;
}

bool QDesignerMemberSheet::isSignal(int index) const
{
    return m_meta->method(index).methodType() == QMetaMethod::Signal;
}

bool QDesignerMemberSheet::isSlot(int index) const
{
    return m_meta->method(index).methodType() == QMetaMethod::Slot;
}

bool QDesignerMemberSheet::inheritedFromWidget(int index) const
{
    const char *name = m_meta->method(index).signature();
    return QWidget::staticMetaObject.indexOfMethod(name) != -1;
}


QList<QByteArray> QDesignerMemberSheet::parameterTypes(int index) const
{
    return m_meta->method(index).parameterTypes();
}

QList<QByteArray> QDesignerMemberSheet::parameterNames(int index) const
{
    return m_meta->method(index).parameterNames();
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
