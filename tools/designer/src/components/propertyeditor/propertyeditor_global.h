/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROPERTYEDITOR_GLOBAL_H
#define PROPERTYEDITOR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_PROPERTYEDITOR_LIBRARY
# define QT_PROPERTYEDITOR_EXPORT
#else
# define QT_PROPERTYEDITOR_EXPORT
#endif
#else
#define QT_PROPERTYEDITOR_EXPORT
#endif

#endif // PROPERTYEDITOR_GLOBAL_H
