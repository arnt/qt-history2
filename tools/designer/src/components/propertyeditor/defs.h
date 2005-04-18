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

#include <QtGui/QSizePolicy>
#include <QtCore/QString>

namespace qdesigner { namespace components { namespace propertyeditor {

int size_type_to_int(QSizePolicy::Policy t);
QString size_type_to_string(QSizePolicy::Policy t);
QSizePolicy::Policy int_to_size_type(int i);

} } } // namespace qdesigner::components::propertyeditor

#endif // DEFS_H
