/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwsproperty_qws.h#3 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QWSPROPERTY_H
#define QWSPROPERTY_H

#ifndef QT_H
#include "qwscommand_qws.h"
#include <qdict.h>
#endif // QT_H

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

#ifndef QT_NO_QWS_PROPERTIES

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

#endif //QT_NO_QWS_PROPERTIES
#endif //QWSPROPERTY_H
