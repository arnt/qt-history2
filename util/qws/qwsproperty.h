/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwsproperty.h#3 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QWSPROPERTY_H
#define QWSPROPERTY_H

#include "qwscommand.h"

#include <qcstring.h>
#include <qstring.h>

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

class QWSPropertyManager
{
public:
    enum Mode {
	PropReplace = 0,
	PropPrepend,
	PropAppend
    };

    QWSPropertyManager();
    
    int atom( const QString &name );
    bool setProperty( int winId, int property, int mode, const QByteArray &data );
    bool hasProperty( int winId, int property );
    bool removeProperty( int winId, int property );
    bool addProperty( int winId, int property );
    
private:
    struct PropertyKey 
    {
	PropertyKey() : winId( -1 ), property( -1 ) {}
	PropertyKey( int w, int p ) : winId( w ), property( p ) {}
	
	bool operator==( const PropertyKey &k ) const {
	    return ( winId == k.winId &&
		     property == k.property );
	}
	bool operator<( const PropertyKey &k ) const {
	    QString s1( "%1_%2" );
	    s1.arg( winId ).arg( property );
	    QString s2( "%1_%2" );
	    s2.arg( k.winId ).arg( k.property );
	    return s1 < s2;
	}
	PropertyKey &operator=( const PropertyKey &k ) {
	    winId = k.winId;
	    property = k.property;
	    return *this;
	}

    	int winId, property;
    };

    QMap<QString, int> atoms;
    QMap<PropertyKey, QByteArray> properties;
    
};

/*********************************************************************
 *
 * Class: QWSSetPropertyCommand
 *
 *********************************************************************/

class QWSSetPropertyCommand : public QWSCommand
{
public:
    QWSSetPropertyCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSSetPropertyCommand();

    virtual void readData();
    virtual void execute();

private:
    int winId, property, mode;
    QByteArray data;

};

#endif
