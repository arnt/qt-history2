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

#ifndef QSCRIPTOBJECTDATA_P_H
#define QSCRIPTOBJECTDATA_P_H

#include <QtCore/qshareddata.h>

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

class QScriptObjectData: public QSharedData
{
protected:
    inline QScriptObjectData() {}

public:
    virtual ~QScriptObjectData() {}

private:
    Q_DISABLE_COPY(QScriptObjectData)
};

#endif
