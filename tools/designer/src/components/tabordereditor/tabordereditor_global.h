/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TABORDEREDITOR_GLOBAL_H
#define TABORDEREDITOR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_TABORDEREDITOR_LIBRARY
# define QT_TABORDEREDITOR_EXPORT
#else
# define QT_TABORDEREDITOR_EXPORT
#endif
#else
#define QT_TABORDEREDITOR_EXPORT
#endif

#endif // TABORDEREDITOR_GLOBAL_H
