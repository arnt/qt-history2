/****************************************************************************
**
**
** Implementation of QSettings class
**
** Created : 2000.06.26
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsettings.h"




/*!
  Construct a QSettings object.
  
  The object is modifiable if \a writable is TRUE.  Calls to readEntry() will
  be passed to \a override if \a override is non-zero.
  
  \sa setWritable(), setOverride()
 */
QSettings::QSettings( bool writable, QSettings *override )
    : override_p(override), writable_p(writable), node(0), tree(0)
{
}

/*!
  Destroys the QSettings object.
*/
QSettings::~QSettings() {
    // if (tree) delete tree;
    cleanup();
}


/*!
  Returns TRUE if modifications to the settings are allowed ( with
  removeEntry() and writeEntry() ), and returns FALSE if not.
  
  \sa setWritable()
*/
bool QSettings::writable() const {
    return writable_p;
}


/*!
  Allows the modifications to the settings if \a writable is TRUE,
  and disallows them if \a writable is FALSE.
  
  \sa writable()
*/
void QSettings::setWritable( bool writable ) {
    writable_p = writable;
}


/*!
  Returns the override settings for the object, and returns zero
  if there is no override set.
  
  \sa setOverride(), readEntry()
*/
const QSettings *QSettings::override() const {
    return (const QSettings *) override_p;
}


/*
  Sets the override settings object to \a override.  Pass \a override
  as zero to disable the use of override settings.
  
  See readEntry() for more information on how override settings work.
  
  \sa override(), readEntry()
*/
void QSettings::setOverride( QSettings *override ) {
    override_p = override;
}


/*!
  Returns the file name where the settings are stored for the specified
  \a system.
  
  \sa setPath()
*/
const QString &QSettings::path( System system ) const {
    return pathMap_p[system];
}


/*!
  Set the file name where the settings are stored for the specified
  \a system to \a path.  Since Qt supports multiple operating systems,
  this function must be called for each operating system you intend to
  support.
  
  For example:
  
  \code
  QSettings settings;
  
  settings.setPath(QSettings::Unix,    "/opt/mysoft/etc/myapp.rc");
  settings.setPath(QSettings::Windows, "C:\Program Files\MySoft\MyApp.rc");
  \endcode
  
  As Qt is ported to more operating systems, the System enum will be
  extended to include these systems.  Including support in your application
  will only require the addition of another line similar to the above.
  
  \sa path()
 */
void QSettings::setPath( System system, const QString &path ) {
    pathMap_p[system] = path;
}
