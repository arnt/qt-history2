#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>

#include <QtDesigner/abstractformwindow.h>
#include <resourcefile.h>
#include "findicondialog.h"

FindIconDialog::FindIconDialog(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(m_file_dir_input, SIGNAL(textChanged(QString)), this, SLOT(updateBoxes()));
    connect(m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_file_image_list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                this, SLOT(imageFileSelected(QListWidgetItem*)));
    connect(m_file_dir_browse, SIGNAL(clicked()), this, SLOT(browseFileDir()));
    connect(m_specify_file_input, SIGNAL(clicked()), this, SLOT(setActiveBox()));
    connect(m_specify_resource_input, SIGNAL(clicked()), this, SLOT(setActiveBox()));
    connect(m_resource_combo, SIGNAL(activated(int)), this, SLOT(updateBoxes()));

    m_form = form;
    QStringList res_list = m_form->resourceFiles();
    QStringList rel_res_list;
    foreach (QString res, res_list)
        rel_res_list.append(m_form->absoluteDir().relativeFilePath(res));
    m_resource_combo->addItems(rel_res_list);
    m_resource_tree->header()->hide();

    updateButtons();
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
        m_file_dir_input->setText(fileDir);
        m_icon_file_name = fileName;
    } else {
        activateBox(ResourceBox);
        int idx = m_resource_combo->findText(m_form->absoluteDir().relativeFilePath(qrcPath));
        if (idx != -1) {
            m_resource_combo->setCurrentIndex(idx);
            m_icon_file_name = filePath;
        }
    }

    updateBoxes();
}

void FindIconDialog::resourceSelected(const QModelIndex &index)
{
    m_icon_file_name.clear();

    ResourceModel *model = qobject_cast<ResourceModel*>(m_resource_tree->model());
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
        m_file_image_list->clear();
        QString dir_path = m_file_dir_input->text();
        if (dir_path.isEmpty()) {
            dir_path = m_form->absoluteDir().absolutePath();
            bool blocked = m_file_dir_input->blockSignals(true);
            m_file_dir_input->setText(dir_path);
            m_file_dir_input->blockSignals(blocked);
        }
        QDir dir(dir_path);
        if (dir.exists()) {
            QStringList file_list = dir.entryList(QStringList()
                    << QString::fromUtf8("*.jpg")
                    << QString::fromUtf8("*.gif")
                    << QString::fromUtf8("*.png"));

            foreach (QString file, file_list) {
                QListWidgetItem *item = new QListWidgetItem(m_file_image_list);
                item->setText(file);
                item->setIcon(QIcon(dir.filePath(file)));
                if (item->text() == m_icon_file_name) {
                    m_file_image_list->setItemSelected(item, true);
                    m_file_image_list->setCurrentItem(item);
                }
            }
        }
        if (m_file_image_list->currentItem() == 0)
            m_icon_file_name.clear();
    } else {
        int idx = m_resource_combo->currentIndex();
        if (idx != -1) {
            QString qrc_file = m_form->absoluteDir().absoluteFilePath(m_resource_combo->itemText(idx));
            ResourceFile rf(qrc_file);
            rf.load();
            QAbstractItemModel *old_model = m_resource_tree->model();
            m_resource_tree->setModel(0);
            delete old_model;
            ResourceModel *new_model = new ResourceModel(rf);
            m_resource_tree->setModel(new_model);

            if (m_icon_file_name.startsWith(QLatin1String(":"))) {
                QString prefix, file;
                rf.split(m_icon_file_name, &prefix, &file);
                QModelIndex file_index = new_model->getIndex(prefix, file);
                QModelIndex prefix_index = new_model->prefixIndex(file_index);
                m_resource_tree->setExpanded(prefix_index, true);
                m_resource_tree->setCurrentIndex(file_index);
            } else {
                m_icon_file_name.clear();
            }

            connect(m_resource_tree->selectionModel(),
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
    m_ok_button->setEnabled(!m_icon_file_name.isEmpty());
}

void FindIconDialog::setActiveBox()
{
    activateBox(sender() == m_specify_file_input ? FileBox : ResourceBox);
}

void FindIconDialog::activateBox(InputBox box)
{
    m_specify_file_input->setChecked(box == FileBox);
    m_specify_file_box->setEnabled(box == FileBox);

    m_specify_resource_box->setEnabled(box == ResourceBox);
    m_specify_resource_input->setChecked(box == ResourceBox);

    updateBoxes();
}

FindIconDialog::InputBox FindIconDialog::activeBox() const
{
    if (m_specify_file_input->isChecked())
        return FileBox;
    return ResourceBox;
}

QString FindIconDialog::qrcPath() const
{
    if (activeBox() == FileBox)
        return QString();
    return m_form->absoluteDir().absoluteFilePath(m_resource_combo->currentText());
}

QString FindIconDialog::filePath() const
{
    if (activeBox() == FileBox) {
        return QDir::cleanPath(m_file_dir_input->text() + QDir::separator() + m_icon_file_name);
    } else {
        return QDir::cleanPath(m_icon_file_name);
    }
}

void FindIconDialog::browseFileDir()
{
    QString dir = QFileDialog::getExistingDirectory();
    if (!dir.isEmpty())
        m_file_dir_input->setText(dir);
}

