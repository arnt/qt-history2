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

#ifndef FINDICONDIALOG_H
#define FINDICONDIALOG_H

#include "shared_global_p.h"

#include <QtGui/QDialog>

class QDesignerFormWindowInterface;
class QListWidgetItem;
class QModelIndex;

namespace Ui
{
    class FindIconDialog;
} // namespace Ui

namespace qdesigner_internal {

class QT_SHARED_EXPORT FindIconDialog : public QDialog
{
    Q_OBJECT

public:
    FindIconDialog(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~FindIconDialog();

    void setPaths(const QString &qrcPath, const QString &filePath);
    QString qrcPath() const;
    QString filePath() const;

private slots:
    void updateBoxes();
    void imageFileSelected(QListWidgetItem*);
    void browseFileDir();
    void setActiveBox();
    void resourceSelected(const QModelIndex&);
    void updateButtons();

private:
    enum InputBox { FileBox, ResourceBox };

    void activateBox(InputBox box);
    InputBox activeBox() const;

    Ui::FindIconDialog *ui;
    QString m_icon_file_name;
    QDesignerFormWindowInterface *m_form;
};

} // namespace qdesigner_internal

#endif // FINDICONDIALOG_H
