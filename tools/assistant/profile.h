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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
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
    friend class ProfileDialog;

public:
    inline bool isValid() const;

    inline void addDocFile( const QString &docfile );
    inline void addDocFileIcon( const QString docfile, const QString &icon );
    inline void addDocFileTitle( const QString docfile, const QString &title );
    inline void addDocFileImageDir( const QString docfile, const QString &imgDir );
    inline void addProperty( const QString &name, const QString &value );
    void removeDocFileEntry( const QString &title );

    static Profile* createProfile( const QString &file );
    static Profile* createDefaultProfile();
    static QString makeRelativePath( const QString &base, const QString &path );

private:
    Profile();
    Profile( const Profile *p );
    bool load( const QString &name );
    void save( const QString &name );

private:
    int valid:1;
    bool changed;
    QMap<QString,QString> props;
    QMap<QString,QString> icons;
    QMap<QString,QString> titles;
    QMap<QString,QString> imageDirs;
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

inline void Profile::addDocFileTitle( const QString docfile,
				      const QString &title )
{
    titles[docfile] = title;
}

inline void Profile::addDocFileImageDir( const QString docfile,
				     const QString &imgDir )
{
    imageDirs[docfile] = imgDir;
}

inline void Profile::addProperty( const QString &name,
				  const QString &value )
{
    props[name] = value;
}

#endif
