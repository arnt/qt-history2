#ifndef MEMBERSHEET_H
#define MEMBERSHEET_H

#include <extension.h>
#include <QList>
#include <QByteArray>

struct IMemberSheet
{
    virtual ~IMemberSheet() {}

    virtual int count() const = 0;
    
    virtual int indexOf(const QString &name) const = 0;
        
    virtual QString memberName(int index) const = 0;
    virtual QString memberGroup(int index) const = 0;
    virtual void setMemberGroup(int index, const QString &group) = 0;
    
    virtual bool isVisible(int index) const = 0;
    virtual void setVisible(int index, bool b) = 0;

    virtual bool isSignal(int index) const = 0;
    virtual bool isSlot(int index) const = 0;
    
    virtual QString signature(int index) const = 0;
    virtual QList<QByteArray> parameterTypes(int index) const = 0;
    virtual QList<QByteArray> parameterNames(int index) const = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(IMemberSheet, "http://trolltech.com/Qt/IDE/MemberSheet")

#endif // MEMBERSHEET_H
