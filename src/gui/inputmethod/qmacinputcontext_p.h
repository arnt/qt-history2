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

/****************************************************************************
**
** Definition of QXIMInputContext class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef QMACINPUTCONTEXT_P_H
#define QMACINPUTCONTEXT_P_H

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

#include <QtGui/qinputcontext.h>

class Q_GUI_EXPORT QMacInputContext : public QInputContext
{
    Q_OBJECT
    //Q_DECLARE_PRIVATE(QMacInputContext)
public:
    explicit QMacInputContext(QObject* parent = 0);
    virtual ~QMacInputContext();

    virtual QString identifierName() { return QLatin1String("mac"); }
    virtual QString language();

    virtual void reset();

    virtual bool isComposing() const;
};

#endif // QMACINPUTCONTEXT_P_H
