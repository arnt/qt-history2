/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwsproperty.h#3 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QWSPROPERTY_H
#define QWSPROPERTY_H

#include "qwscommand.h"
#include <qdict.h>

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

    // pre-defined properties
    enum Atom {
	PropSelection = 0
    };
    
    QWSPropertyManager();

    bool setProperty( int winId, int property, int mode, const char *data, int len );
    bool hasProperty( int winId, int property );
    bool removeProperty( int winId, int property );
    bool addProperty( int winId, int property );
    bool getProperty( int winId, int property, char *&data, int &len );

private:
    char *createKey( int winId, int property ) const;

private:
    struct Property {
	int len;
	char *data;
    };

    QDict<Property> properties;

};


#endif
