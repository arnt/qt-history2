#ifndef QUCOM_H
#define QUCOM_H

#include <qcom.h>
#include <qstring.h>
#include <qvariant.h>



// {44C2A547-01E7-4e56-8559-35AF9D2F42B7}
extern const UUid TID_UType_QString;

struct Q_EXPORT UType_QString : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const QString & );
    QString &get( UObject * o ) { return *(QString*)o->payload.ptr; }

    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};
extern Q_EXPORT UType_QString * pUType_QString;


// 6dc75d58-a1d9-4417-b591-d45c63a3a4ea
extern const UUid TID_UType_QString;

struct Q_EXPORT UType_QVariant : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const QVariant & );
    QVariant &get( UObject * o ) { return *(QVariant*)o->payload.ptr; }

    bool canConvertFrom( UObject *, UType * );
    bool canConvertTo( UObject *, UType * );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};
extern Q_EXPORT UType_QVariant * pUType_QVariant;


#endif // QUCOM_H

