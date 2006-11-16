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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef PROPERTYLINEEDIT_H
#define PROPERTYLINEEDIT_H

#include "shared_global_p.h"

#include <QtGui/QLineEdit>

namespace qdesigner_internal {

    // A line edit with a special context menu allowing for adding (escaped) new  lines
    class PropertyLineEdit : public QLineEdit {
        Q_OBJECT
    public:
        PropertyLineEdit(QWidget *parent, bool wantNewLine);
    protected:
        void contextMenuEvent (QContextMenuEvent *event );
    private slots:
        void insertNewLine();
    private:
        void insertText(const QString &);
        const bool m_wantNewLine;
    };
}

#endif // PROPERTYLINEEDIT_H
