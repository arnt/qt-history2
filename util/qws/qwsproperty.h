/****************************************************************************
**
** Implementation of Qt/FB central server.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
