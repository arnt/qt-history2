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

#ifndef ABSTRACTDIALOGGUI_H
#define ABSTRACTDIALOGGUI_H

#include <QtDesigner/sdk_global.h>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

QT_BEGIN_HEADER

class QWidget;

class QDESIGNER_SDK_EXPORT QDesignerDialogGuiInterface
{
public:
    QDesignerDialogGuiInterface();
    virtual ~QDesignerDialogGuiInterface();

    enum Message { FormLoadFailureMessage, UiVersionMismatchMessage, ResourceLoadFailureMessage,
                   TopLevelSpacerMessage, PropertyEditorMessage, SignalSlotEditorMessage, FormEditorMessage,
                   PreviewFailureMessage, PromotionErrorMessage, ResourceEditorMessage,
                   ScriptDialogMessage, SignalSlotDialogMessage, OtherMessage };

    virtual QMessageBox::StandardButton
        message(QWidget *parent, Message context, QMessageBox::Icon icon,
                const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) = 0;

    virtual QString getExistingDirectory(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), QFileDialog::Options options = QFileDialog::ShowDirsOnly)= 0;
    virtual QString getOpenFileName(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = 0, QFileDialog::Options options = 0)= 0;
    virtual QStringList getOpenFileNames(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = 0, QFileDialog::Options options = 0)= 0;
    virtual QString getSaveFileName(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = 0, QFileDialog::Options options = 0)= 0;

private:
    QDesignerDialogGuiInterface(const QDesignerDialogGuiInterface &other);
    QDesignerDialogGuiInterface &operator=(const QDesignerDialogGuiInterface &other);
};

QT_END_HEADER

#endif // ABSTRACTDIALOGGUI_H
