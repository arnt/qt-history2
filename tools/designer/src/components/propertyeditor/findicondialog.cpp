#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QFileDialog>
#include <QtCore/qdebug.h>

#include "findicondialog.h"

FindIconDialog::FindIconDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(m_file_dir_input, SIGNAL(textChanged(const QString&)), this, SLOT(updateBoxes()));
    connect(m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_file_image_list, SIGNAL( currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
                this, SLOT(imageFileSelected(QListWidgetItem*)));
    connect(m_file_dir_browse, SIGNAL(clicked()), this, SLOT(browseFileDir()));
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
        m_resource_file_input->setText(qrcPath);
        m_icon_file_name = filePath;
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

void FindIconDialog::activateBox(InputBox box)
{
    m_specify_file_input->setChecked(box == FileBox);
    m_specify_file_box->setEnabled(box == FileBox);
    
    m_specify_resource_box->setEnabled(box == ResourceBox);
    m_specify_resource_input->setChecked(box == ResourceBox);
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
    return QDir::cleanPath(m_resource_file_input->text());
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

