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

#ifndef SCRIPTDIALOG_H
#define SCRIPTDIALOG_H

#include "shared_global_p.h"

#include <QtGui/QDialog>

class QDesignerDialogGuiInterface;

class QTextEdit;

namespace qdesigner_internal {

    // Dialog for showing script errors
    class  QDESIGNER_SHARED_EXPORT ScriptDialog : public QDialog {
        Q_OBJECT

    public:
        explicit ScriptDialog(QDesignerDialogGuiInterface *dialogGui, QWidget *parent);
        bool editScript(QString &script);

    private slots:
        void slotAccept();

    private:
        QString trimmedScript() const;
        bool checkScript();

        QDesignerDialogGuiInterface *m_dialogGui;
        QTextEdit *m_textEdit;
    };
} // namespace qdesigner_internal

#endif // SCRIPTDIALOG_H
