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

#include "newform.h"
#include "qdesigner_workbench.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_settings.h"

#include <qdesigner_formbuilder.h>
#include <sheet_delegate.h>

#include <abstractformwindow.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QHeaderView>

#include <QtCore/qdebug.h>

enum NewForm_CustomRole
{
    TemplateNameRole = QAbstractItemModel::UserRole + 100
};

NewForm::NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget)
    : QDialog(parentWidget),
      m_workbench(workbench)
{
    ui.setupUi(this);
    ui.treeWidget->setItemDelegate(new SheetDelegate(ui.treeWidget, this));
    ui.treeWidget->header()->hide();
    ui.treeWidget->header()->setStretchLastSection(true);

    loadFrom(":/trolltech/designer/templates/forms");

    QDesignerSettings settings;
    foreach(QString path, settings.formTemplatePaths())
        loadFrom(path);

    ui.closeButton->setDefault(false);
    ui.createButton->setDefault(true);
}

NewForm::~NewForm()
{
}

void NewForm::on_treeWidget_itemActivated(QTreeWidgetItem *item)
{
    if (item->data(0, TemplateNameRole).isValid())
        ui.createButton->animateClick(0);
}

void NewForm::on_createButton_clicked()
{
    if (QTreeWidgetItem *item = ui.treeWidget->currentItem()) {
        close();

        QDesignerFormWindow *formWindow = workbench()->createFormWindow();
        if (AbstractFormWindow *editor = formWindow->editor()) {
            QString formTemplateName = item->data(0, TemplateNameRole).toString();
            QFile f(formTemplateName);
            if (f.open(QFile::ReadOnly)) {
                editor->setContents(&f);
                f.close();
            } else {
                qWarning() << "template not found:" << formTemplateName;
                editor->setContents(QString());
            }
        }
    }
}

void NewForm::on_closeButton_clicked()
{
    close();
}

QDesignerWorkbench *NewForm::workbench() const
{
    return m_workbench;
}

QIcon NewForm::formPreviewIcon(const QString &fileName)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QDesignerFormBuilder formBuilder(workbench()->core());

        QWidget *fake = new QWidget(0);
        QWidget *widget = formBuilder.load(&f, fake);
        QSize size = widget->size();
        widget->setParent(fake, 0);
        widget->resize(size);
        widget->show();
        f.close();

        widget->ensurePolished();

        QPixmap pix = QPixmap::grabWidget(widget);
        QImage image = pix.toImage();
        image.scale(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pix = image;

        delete fake;

        return pix;
    }
    return QIcon();
}

void NewForm::loadFrom(const QString &path)
{
    QDir dir(path);

    if (!dir.exists())
        return;

    // Iterate through the directory and add the templates
    QFileInfoList list = dir.entryInfoList(QDir::Files);

    if (list.isEmpty())
        return;

    QTreeWidgetItem *root = new QTreeWidgetItem(ui.treeWidget);
    root->setText(0, path);

    foreach(QFileInfo fi, list) {
        if (!fi.isFile())
            continue;

        qApp->processEvents();

        QTreeWidgetItem *item = new QTreeWidgetItem(root);
        item->setText(0, fi.baseName());
        item->setData(0, TemplateNameRole, fi.absoluteFilePath());
        item->setIcon(0, formPreviewIcon(fi.absoluteFilePath()));
    }
    ui.treeWidget->setItemExpanded(root, true);
}

