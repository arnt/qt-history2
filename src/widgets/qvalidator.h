/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvalidator.h#3 $
**
** Definition of 
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QVALIDATOR_H
#define QVALIDATOR_H

#include "qobject.h"
#include "qstring.h"


struct QValidatorPrivate;


class QValidator: public QObject
{
    Q_OBJECT
public:
    QValidator( QObject * parent = 0, const char * name = 0 );
    ~QValidator();

    enum Result{ Good, Uncertain, Bad };

    virtual Result validate( const char * );

    virtual void setInteger( int bottom, int top );
    virtual void setDouble( double bottom, double top, int decimals=0 );

    virtual void setPrefix( const char * );
    const char * prefix() const;
    virtual void setPostfix( const char * );
    const char * postfix() const;

    virtual void setCaseSensitive( bool );
    bool caseSensitive() const;

protected:
    virtual bool validateContent( const QString );
    virtual void fixup( QString & );

    virtual QString validateAffixes( const QString );

private:
    QValidatorPrivate * d;
};

#endif
