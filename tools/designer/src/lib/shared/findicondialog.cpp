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

#include "resourcefile_p.h"
#include "findicondialog_p.h"
#include "ui_findicondialog.h"

#include <QtDesigner/abstractformwindow.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>


FindIconDialog::FindIconDialog(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::FindIconDialog;
    ui->setupUi(this);

    connect(ui->m_file_dir_input, SIGNAL(textChanged(QString)), this, SLOT(updateBoxes()));
    connect(ui->m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->m_file_image_list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                this, SLOT(imageFileSelected(QListWidgetItem*)));
    connect(ui->m_file_dir_browse, SIGNAL(clicked()), this, SLOT(browseFileDir()));
    connect(ui->m_specify_file_input, SIGNAL(clicked()), this, SLOT(setActiveBox()));
    connect(ui->m_specify_resource_input, SIGNAL(clicked()), this, SLOT(setActiveBox()));
    connect(ui->m_resource_combo, SIGNAL(activated(int)), this, SLOT(updateBoxes()));

    m_form = form;
    QStringList res_list = m_form->resourceFiles();
    QStringList rel_res_list;
    foreach (QString res, res_list)
        rel_res_list.append(m_form->absoluteDir().relativeFilePath(res));
    ui->m_resource_combo->addItems(rel_res_list);
    ui->m_resource_tree->header()->hide();

    updateButtons();
}

FindIconDialog::~FindIconDialog()
{
    delete ui;
    ui = 0;
}

void FindIconDialog::setPaths(const QString &qrcPath, const QString &filePath)
{
    if (qrcPath.isEmpty()) {
        activateBox(FileBox);
        QString fileDir;
        QString fileName;
        if (filePath.endsWith(QString(QDir::separator()))) {
            fileDir = filePath;
        } else {
            QFileInfo fi(filePath);
            fileDir = fi.absolutePath();
            fileName = fi.fileName();
        }
        ui->m_file_dir_input->setText(fileDir);
        m_icon_file_name = fileName;
    } else {
        activateBox(ResourceBox);
        int idx = ui->m_resource_combo->findText(m_form->absoluteDir().relativeFilePath(qrcPath));
        if (idx != -1) {
            ui->m_resource_combo->setCurrentIndex(idx);
            m_icon_file_name = filePath;
        }
    }

    updateBoxes();
}

void FindIconDialog::resourceSelected(const QModelIndex &index)
{
    m_icon_file_name.clear();

    ResourceModel *model = qobject_cast<ResourceModel*>(ui->m_resource_tree->model());
    if (model != 0) {
        QString prefix, file;
        model->getItem(index, prefix, file);
        if (!file.isEmpty()) {
            if (model->iconFileExtension(file))
                m_icon_file_name = QLatin1String(":") + prefix + QLatin1String("/") + file;
            else
                m_icon_file_name.clear();
        }
    }
    updateButtons();
}

void FindIconDialog::updateBoxes()
{
    if (activeBox() == FileBox) {
        ui->m_file_image_list->clear();
        QString dir_path = ui->m_file_dir_input->text();
        if (dir_path.isEmpty()) {
            dir_path = m_form->absoluteDir().absolutePath();
            bool blocked = ui->m_file_dir_input->blockSignals(true);
            ui->m_file_dir_input->setText(dir_path);
            ui->m_file_dir_input->blockSignals(blocked);
        }
        QDir dir(dir_path);
        if (dir.exists()) {
            QStringList file_list = dir.entryList(QStringList()
                    << QString::fromUtf8("*.jpg")
                    << QString::fromUtf8("*.gif")
                    << QString::fromUtf8("*.png"));

            foreach (QString file, file_list) {
                QListWidgetItem *item = new QListWidgetItem(ui->m_file_image_list);
                item->setText(file);
                item->setIcon(QIcon(dir.filePath(file)));
                if (item->text() == m_icon_file_name) {
                    ui->m_file_image_list->setItemSelected(item, true);
                    ui->m_file_image_list->setCurrentItem(item);
                }
            }
        }
        if (ui->m_file_image_list->currentItem() == 0)
            m_icon_file_name.clear();
    } else {
        int idx = ui->m_resource_combo->currentIndex();
        if (idx != -1) {
            QString qrc_file = m_form->absoluteDir().absoluteFilePath(ui->m_resource_combo->itemText(idx));
            ResourceFile rf(qrc_file);
            rf.load();
            QAbstractItemModel *old_model = ui->m_resource_tree->model();
            ui->m_resource_tree->setModel(0);
            delete old_model;
            ResourceModel *new_model = new ResourceModel(rf);
            ui->m_resource_tree->setModel(new_model);

            if (m_icon_file_name.startsWith(QLatin1String(":"))) {
                QString prefix, file;
                rf.split(m_icon_file_name, &prefix, &file);
                QModelIndex file_index = new_model->getIndex(prefix, file);
                QModelIndex prefix_index = new_model->prefixIndex(file_index);
                ui->m_resource_tree->setExpanded(prefix_index, true);
                ui->m_resource_tree->setCurrentIndex(file_index);
            } else {
                m_icon_file_name.clear();
            }

            connect(ui->m_resource_tree->selectionModel(),
                        SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                        this, SLOT(resourceSelected(QModelIndex)));
        }
    }

    updateButtons();
}

void FindIconDialog::imageFileSelected(QListWidgetItem *item)
{
    m_icon_file_name.clear();
    if (item != 0)
        m_icon_file_name = item->text();

    updateButtons();
}

void FindIconDialog::updateButtons()
{
    ui->m_ok_button->setEnabled(!m_icon_file_name.isEmpty());
}

void FindIconDialog::setActiveBox()
{
    activateBox(sender() == ui->m_specify_file_input ? FileBox : ResourceBox);
}

void FindIconDialog::activateBox(InputBox box)
{
    ui->m_specify_file_input->setChecked(box == FileBox);
    ui->m_specify_file_box->setEnabled(box == FileBox);

    ui->m_specify_resource_box->setEnabled(box == ResourceBox);
    ui->m_specify_resource_input->setChecked(box == ResourceBox);

    updateBoxes();
}

FindIconDialog::InputBox FindIconDialog::activeBox() const
{
    if (ui->m_specify_file_input->isChecked())
        return FileBox;
    return ResourceBox;
}

QString FindIconDialog::qrcPath() const
{
    if (activeBox() == FileBox)
        return QString();
    return m_form->absoluteDir().absoluteFilePath(ui->m_resource_combo->currentText());
}

QString FindIconDialog::filePath() const
{
    if (activeBox() == FileBox) {
        return QDir::cleanPath(ui->m_file_dir_input->text() + QDir::separator() + m_icon_file_name);
    } else {
        return QDir::cleanPath(m_icon_file_name);
    }
}

void FindIconDialog::browseFileDir()
{
    QString dir = QFileDialog::getExistingDirectory();
    if (!dir.isEmpty())
        ui->m_file_dir_input->setText(dir);
}

