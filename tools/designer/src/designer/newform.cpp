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
#include "qdesigner.h"
#include "qdesigner_workbench.h"
#include "qdesigner_actions.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_settings.h"

#include <qdesigner_formbuilder.h>
#include <sheet_delegate.h>

#include <QtDesigner/abstractformwindow.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QHeaderView>

#include <QtCore/qdebug.h>

enum NewForm_CustomRole
{
    TemplateNameRole = Qt::UserRole + 100
};

NewForm::NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget)
    : QDialog(parentWidget),
      m_workbench(workbench)
{
    ui.setupUi(this);
    ui.treeWidget->setItemDelegate(new SheetDelegate(ui.treeWidget, this));
    ui.treeWidget->header()->hide();
    ui.treeWidget->header()->setStretchLastSection(true);
    ui.lblPreview->setBackgroundRole(QPalette::Base);
    ui.chkShowOnStartup->setChecked(QDesignerSettings().showNewFormOnStartup());

    loadFrom(QLatin1String(":/trolltech/designer/templates/forms"));

    QDesignerSettings settings;
    foreach(QString path, settings.formTemplatePaths())
        loadFrom(path);
}

NewForm::~NewForm()
{
    QDesignerSettings().setShowNewFormOnStartup(ui.chkShowOnStartup->isChecked());
}

void NewForm::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    if (current && current->parent()) {
        ui.createButton->setEnabled(true);
        ui.createButton->setDefault(true);
        ui.lblPreview->setPixmap(formPreviewIcon(current->data(0, TemplateNameRole).toString()).pixmap(QSize(256, 256)));
    } else {
        ui.createButton->setEnabled(false);
        ui.lblPreview->setText(tr("Choose a template for a preview"));
    }
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

        int maxUntitled = 0;
        int totalWindows = m_workbench->formWindowCount();
        // This will cause some problems with i18n, but for now I need the string to be "static"
        QRegExp rx(QLatin1String("Untitled( (\\d+))*"));
        for (int i = 0; i < totalWindows; ++i) {
            if (rx.exactMatch(m_workbench->formWindow(i)->windowTitle())) {
                if (maxUntitled == 0)
                    ++maxUntitled;
                if (rx.numCaptures() > 1)
                    maxUntitled = qMax(rx.cap(2).toInt(), maxUntitled);
            }
        }

        QDesignerFormWindow *formWindow = workbench()->createFormWindow();
        if (QDesignerFormWindowInterface *editor = formWindow->editor()) {
            QString formTemplateName = item->data(0, TemplateNameRole).toString();
            QFile f(formTemplateName);
            if (f.open(QFile::ReadOnly)) {
                editor->setContents(&f);
                f.close();
            } else {
                editor->setContents(QString());
            }

            if (QWidget *container = editor->mainContainer())
                formWindow->resize(container->size());
        }
        QString newTitle = QString::fromUtf8("Untitled");
        if (maxUntitled)
            newTitle += QString::fromUtf8(" ") + QString::number(maxUntitled + 1);
        formWindow->setWindowTitle(newTitle);
        formWindow->show();
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

        qDesigner->processEvents();

        widget->ensurePolished();

        QPixmap pix = QPixmap::grabWidget(widget);
        QImage image = pix.toImage();
        image = image.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pix = QPixmap::fromImage(image);

        fake->deleteLater();

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
    // Try to get something that is easy to read.
    QString visiblePath = path;
    int index = visiblePath.lastIndexOf(QDir::separator());
    if (index != -1) {
        // try to find a second slash, just to be a bit better.
        int index2 = visiblePath.lastIndexOf(QDir::separator(), index - 1);
        if (index2 != -1)
            index = index2;
        visiblePath = visiblePath.mid(index + 1);
    }

    root->setText(0, visiblePath);
    root->setToolTip(0, path);

    foreach(QFileInfo fi, list) {
        if (!fi.isFile())
            continue;

        QTreeWidgetItem *item = new QTreeWidgetItem(root);
        item->setText(0, fi.baseName());
        item->setData(0, TemplateNameRole, fi.absoluteFilePath());

        QTreeWidgetItem *i = ui.treeWidget->currentItem();
        if (i == 0) {
            ui.treeWidget->setCurrentItem(item);
            ui.treeWidget->setItemSelected(item, true);
        }
    }
    ui.treeWidget->setItemExpanded(root, true);
}

void NewForm::on_openButton_clicked()
{
    hide();
    if (m_workbench->actionManager()->openForm())
        close();
    else
        show();
}
