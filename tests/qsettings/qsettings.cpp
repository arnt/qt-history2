/****************************************************************************
**
**
** Implementation of QSettings class
**
** Created : 2000.06.26
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
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


    /*
      QSettings::QSettings(bool writable)
      : override_p(0), writable_p(writable)
      {

      }


      QSettings::QSettings(int system, const QString &path, bool writable)
      : override_p(0), writable_p(writable)
      {
      setPath(system, path);
      }


      QSettings::QSettings(const QMap<int,QString> &pathMap, bool writable)
      : override_p(0), writable_p(writable)
      {
      pathMap_p = pathMap;
      }


      QSettings::QSettings(QSettings *override, bool writable)
      : override_p(override), writable_p(writable)
      {

      }
    */


QSettings::QSettings(bool writable, QSettings *override, QObject *parent, const char *name)
    :  QObject(parent, name), override_p(override), writable_p(writable), node(0), tree(0)
{

}

QSettings::~QSettings() {
    cleanup();
}


bool QSettings::writable() const {
    return writable_p;
}


void QSettings::setWritable(bool writable) {
    writable_p = writable;
}


const QSettings *QSettings::override() const {
    return override_p;
}


void QSettings::setOverride(QSettings *override) {
    override_p = override;
}


const QString &QSettings::path(int system) const {
    return pathMap_p[system];
}


void QSettings::setPath(int system, const QString &path) {
    pathMap_p[system] = path;
}


/*
  --
  TODO This should be implemented in the platform specific files, i.e.

  qsettings_unix.cpp
  qsettings_win.cpp

  --

  void QSettings::write() {

  }


  const QVariant &QSettings::readEntry(const QString &key) const {

  }


  void QSettings::writeEntry(const QString &key, const QVariant &value) {

  }

*/
