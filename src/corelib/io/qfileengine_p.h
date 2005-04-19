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

#ifndef QFILEENGINE_P_H
#define QFILEENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qplatformdefs.h>
#include <qiodevice.h>

class QFileEngine;
class QFileEnginePrivate
{
public:
    QFileEnginePrivate() { }
    virtual ~QFileEnginePrivate() { }
protected:
    QFileEngine *q_ptr;
    Q_DECLARE_PUBLIC(QFileEngine)
private:
    //just in case I need this later --Sam
};

#endif // QFILEENGINE_P_H
