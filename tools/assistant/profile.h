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
    friend class Config;

public:
    Profile( const QString &str );
    inline bool isValid() const;

    inline void addDocFile( const QString &docfile );
    inline void addDocFileIcon( const QString docfile, const QString &icon );
    inline void addProperty( const QString &name, const QString &value );

private:
    void load( const QString &str );

private:
    int valid:1;
    QMap<QString,QString> props;
    QMap<QString,QString> icons;
    QStringList docs;
};


inline bool Profile::isValid() const
{
    return valid;
}

inline void Profile::addDocFile( const QString &docfile )
{
    docs << docfile;
}

inline void Profile::addDocFileIcon( const QString docfile,
				     const QString &icon )
{
    icons[docfile] = icon;
}

inline void Profile::addProperty( const QString &name,
				  const QString &value )
{
    props[name] = value;
}

#endif
