/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MWERKS_MAC_H
#define MWERKS_MAC_H

#include "qglobal.h"
#ifdef Q_OS_MAC

#ifdef Q_OS_MAC9
 //these don't belong here, need a way to access this outside .h files
# define QT_NO_CODECS
# define MOC_MWERKS_PLUGIN
#endif
#define macintosh

/*make moc a plugin*/
enum moc_status {
    moc_success = 1,
    moc_parse_error = 2,
    moc_no_qobject = 3,
    moc_not_time = 4,
    moc_no_source = 5,
    moc_general_error = 6
};

//get the qt mac9 stuff
#ifdef Q_OS_MAC9
# include "qt_mac9.h"
#endif

#endif
#endif
