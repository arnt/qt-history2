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

#ifndef CODEPREVIEWDIALOG_H
#define CODEPREVIEWDIALOG_H

#include "shared_global_p.h"
#include <QtGui/QDialog>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
// Dialog for viewing code.
class QDESIGNER_SHARED_EXPORT CodeDialog : public QDialog
{
    Q_OBJECT
    CodeDialog(QWidget *parent = 0);
public:
    virtual ~CodeDialog();

    static bool generateCode(const QDesignerFormWindowInterface *fw, QString *code, QString *errorMessage);
    static bool showCodeDialog(const QDesignerFormWindowInterface *fw, QWidget *parent, QString *errorMessage);

private slots:
    void slotSaveAs();
    void copyAll();

private:
    void setCode(const QString &code);
    QString code() const;
    void setFormFileName(const QString &f);
    QString formFileName() const;

    void warning(const QString &msg);

    struct CodeDialogPrivate;
    CodeDialogPrivate *m_impl;
};
} // namespace qdesigner_internal

#endif // CODEPREVIEWDIALOG_H
