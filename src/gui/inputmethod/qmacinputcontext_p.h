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

#include <private/qt_mac_p.h>
#include <QtGui/qinputcontext.h>

class Q_GUI_EXPORT QMacInputContext : public QInputContext
{
    Q_OBJECT
    //Q_DECLARE_PRIVATE(QMacInputContext)
public:
    explicit QMacInputContext(QObject* parent = 0);
    virtual ~QMacInputContext();

    virtual void setFocusWidget(QWidget *w);
    virtual QString identifierName() { return QLatin1String("mac"); }
    virtual QString language();

    virtual void reset();

    virtual bool isComposing() const;

    static OSStatus globalEventProcessor(EventHandlerCallRef, EventRef, void *);
    static void initialize();
    static void cleanup();
private:
    bool composing;
    TSMDocumentID id;
};

#endif // QMACINPUTCONTEXT_P_H
