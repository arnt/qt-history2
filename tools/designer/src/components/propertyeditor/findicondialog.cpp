#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QFileDialog>
#include <QtCore/qdebug.h>

#include <abstractformwindow.h>
#include <resourcefile.h>
#include "findicondialog.h"

FindIconDialog::FindIconDialog(AbstractFormWindow *form, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(m_file_dir_input, SIGNAL(textChanged(const QString&)), this, SLOT(updateBoxes()));
    connect(m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_file_image_list, SIGNAL( currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
                this, SLOT(imageFileSelected(QListWidgetItem*)));
    connect(m_file_dir_browse, SIGNAL(clicked()), this, SLOT(browseFileDir()));
    connect(m_specify_file_input, SIGNAL(clicked()), this, SLOT(setActiveBox()));
    connect(m_specify_resource_input, SIGNAL(clicked()), this, SLOT(setActiveBox()));
    connect(m_resource_combo, SIGNAL(activated()), this, SLOT(updateBoxes()));

    m_form = form;
    QStringList res_list = m_form->resourceFiles();
    QStringList rel_res_list;
    foreach (QString res, res_list)
        rel_res_list.append(m_form->relativePath(res));
    m_resource_combo->addItems(rel_res_list);
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
        qDebug() << "FindIconDialog::setPaths():" << filePath << fileDir << fileName;
        m_file_dir_input->setText(fileDir);
        m_icon_file_name = fileName;
    } else {
        activateBox(ResourceBox);
        int idx = m_resource_combo->findText(m_form->relativePath(qrcPath));
        if (idx != -1) {
            m_resource_combo->setCurrentIndex(idx);
            m_icon_file_name = filePath;
        }
    }

    updateBoxes();
}

void FindIconDialog::updateBoxes()
{
    if (activeBox() == FileBox) {
        m_file_image_list->clear();
        QString dir_path = m_file_dir_input->text();
        QDir dir(dir_path);
        if (!dir.exists()) {
            m_ok_button->setEnabled(false);
            return;
        }
    
        QStringList file_list = dir.entryList(QStringList() << "*.jpg" << "*.gif" << "*.png");
        foreach (QString file, file_list) {
            QListWidgetItem *item = new QListWidgetItem(m_file_image_list);
            item->setText(file);
            item->setIcon(QIcon(dir.filePath(file)));
            if (item->text() == m_icon_file_name) {
                m_file_image_list->setItemSelected(item, true);
                m_file_image_list->setCurrentItem(item);
            }
        }
    } else {
        int idx = m_resource_combo->currentIndex();
        if (idx == -1)
            return;
        QString qrc_file = m_form->absolutePath(m_resource_combo->itemText(idx));
        qDebug() << "FindIconDialog::updateBoxes(): reading " << qrc_file;
        ResourceFile rf(qrc_file);
        if (!rf.load())
            return;
        QAbstractItemModel *model = m_resource_tree->model();
        m_resource_tree->setModel(0);
        m_resource_tree->setModel(new ResourceModel(rf));
        delete model;
    }
}

void FindIconDialog::imageFileSelected(QListWidgetItem *item)
{
    if (item == 0) {
        m_ok_button->setEnabled(false);
    } else {
        m_icon_file_name = item->text();
        m_ok_button->setEnabled(true);
    }
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
    return m_form->absolutePath(m_resource_combo->currentText());
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

