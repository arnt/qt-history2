#ifndef QUTYPES_H
#define QUTYPES_H

#include <qstring.h>
#ifndef UCOM_EXPORT
#define UCOM_EXPORT Q_EXPORT
#endif

#include <ucom.h>


// {44C2A547-01E7-4e56-8559-35AF9D2F42B7}
extern const UUid TID_UType_QString;

struct Q_EXPORT UType_QString : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const QString & );
    QString &get( UObject * o ) { return *(QString*)o->payload.ptr; }

    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};
extern Q_EXPORT UType_QString * pUType_QString;



#endif //QUTYPES_H

