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

#ifndef DEFS_H
#define DEFS_H

#include <QSizePolicy>
#include <QString>

int size_type_to_int(QSizePolicy::SizeType t);
QString size_type_to_string(QSizePolicy::SizeType t);
QSizePolicy::SizeType int_to_size_type(int i);

#endif // DEFS_H
