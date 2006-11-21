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

#ifndef STYLESHEETEDITOR_H
#define STYLESHEETEDITOR_H

#include <QtGui/QTextEdit>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include "shared_global_p.h"

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT StyleSheetEditor : public QTextEdit
{
    Q_OBJECT
public:
    StyleSheetEditor(QWidget *parent = 0);
};

class QDESIGNER_SHARED_EXPORT StyleSheetEditorDialog : public QDialog
{
    Q_OBJECT
public:
    StyleSheetEditorDialog(QWidget *parent, QWidget *widget);
    StyleSheetEditor *editor() const;

    static bool isStyleSheetValid(const QString &styleSheet);

private slots:
    void applyStyleSheet();
    void validateStyleSheet();

private:
    StyleSheetEditor *m_editor;
    QDesignerFormWindowInterface *m_fw;
    QWidget *m_widget;
    QLabel *validityLabel;
};

} // namespace qdesigner_internal

#endif // STYLESHEETEDITOR_H
