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

#ifndef FINDICONDIALOG_H
#define FINDICONDIALOG_H

#include "shared_global_p.h"

#include <QtCore/QDir>
#include <QtGui/QDialog>

class QDesignerFormWindowInterface;
class QListWidgetItem;
class QModelIndex;

class QDesignerResourceBrowserInterface;

namespace qdesigner_internal {

namespace Ui
{
    class FindIconDialog;
} // namespace Ui

class ResourceEditor;

class QDESIGNER_SHARED_EXPORT FindIconDialog : public QDialog
{
    Q_OBJECT

public:
    FindIconDialog(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~FindIconDialog();

    void setPaths(const QString &qrcPath, const QString &filePath);
    QString qrcPath() const;
    QString filePath() const;

    virtual void accept();

private slots:
    void setActiveBox();
    void updateButtons();

    void setFile(const QString &path);
    void setQrc(const QString &qrc, const QString &file);
    void setLanguagePath(const QString &path);
    void cdUp();

    void itemActivated(QListWidgetItem *item); // File page
    void currentItemChanged(QListWidgetItem *item);
    
    void itemActivated(const QString &qrc_path, const QString &file_name); // QRC page
    void itemChanged(const QString &qrc_path, const QString &file_name);
    
    void itemActivated(const QString &file_name); // Language plugin page
    void itemChanged(const QString &file_name);

private:
    enum InputBox { FileBox, ResourceBox, LanguageBox };

    void setActiveBox(InputBox box);
    InputBox activeBox() const;

    qdesigner_internal::Ui::FindIconDialog *ui;
    QDesignerFormWindowInterface *m_form;

    void setViewDir(const QString &path);
    QDir m_view_dir;
    struct FileData {
        QString file;
    } m_file_data;
    struct LanguageData {
        QString file;
    } m_language_data;
    struct ResourceData {
        QString file;
        QString qrc;
    } m_resource_data;
    ResourceEditor *m_resource_editor;
    QDesignerResourceBrowserInterface *m_language_editor;

    static QString defaultQrcPath();
    static QString defaultFilePath(QDesignerFormWindowInterface *form);
    static QString defaultLanguagePath();
    static void setDefaultQrcPath(const QString &path);
    static void setDefaultFilePath(const QString &path);
    static void setDefaultLanguagePath(const QString &path);
    static InputBox previousInputBox();
    static void setPreviousInputBox(InputBox box);

#ifdef Q_OS_WIN
    bool isRoot;
    QString rootDir;
#endif
};

} // namespace qdesigner_internal

#endif // FINDICONDIALOG_H
