#include "default_membersheet.h"

#include <QVariant>
#include <QMetaObject>
#include <QMetaMember>
#include <qdebug.h>

QDesignerMemberSheet::QDesignerMemberSheet(QObject *object, QObject *parent)
    : QObject(parent),
      m_object(object),
      meta(object->metaObject())
{
}

QDesignerMemberSheet::~QDesignerMemberSheet()
{
}

int QDesignerMemberSheet::count() const
{
    return meta->memberCount();
}

int QDesignerMemberSheet::indexOf(const QString &name) const
{
    return meta->indexOfMember(name.toLatin1());
}

QString QDesignerMemberSheet::memberName(int index) const
{
    return QString::fromLatin1(meta->member(index).tag());
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
    return QString::fromLatin1(meta->member(index).signature());
}

bool QDesignerMemberSheet::isVisible(int index) const
{
    if (m_info.contains(index))
        return m_info.value(index).visible;
        
   return meta->member(index).memberType() == QMetaMember::Signal
           || meta->member(index).access() == QMetaMember::Public;
}

void QDesignerMemberSheet::setVisible(int index, bool visible)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].visible = visible;
}

bool QDesignerMemberSheet::isSignal(int index) const
{
    return meta->member(index).memberType() == QMetaMember::Signal;
}

bool QDesignerMemberSheet::isSlot(int index) const
{
    return meta->member(index).memberType() == QMetaMember::Slot;
}

QList<QByteArray> QDesignerMemberSheet::parameterTypes(int index) const
{ 
    return meta->member(index).parameterTypes();
}

QList<QByteArray> QDesignerMemberSheet::parameterNames(int index) const
{
    return meta->member(index).parameterNames();
}

QDesignerMemberSheetFactory::QDesignerMemberSheetFactory(QExtensionManager *parent)
    : DefaultExtensionFactory(parent)
{
}

QObject *QDesignerMemberSheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(IMemberSheet))
        return new QDesignerMemberSheet(object, parent);

    return 0;
}

