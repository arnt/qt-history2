/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Assistant.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef PROFILE_H
#define PROFILE_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

class ProfileHandler;

class Profile
{
    friend class ProfileHandler;

public:
    Profile( const QString &str );
    inline bool isValid() const;

private:
    void load( const QString &str );

private:
    int valid:1;
    QMap<QString,QString> props;
    QStringList docs;
};


inline bool Profile::isValid() const
{
    return valid;
}


#endif
