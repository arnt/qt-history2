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
#include "resourceeditor_p.h"

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindowmanager.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QMetaObject>
#include <QtCore/QSettings>
#include <QtCore/qdebug.h>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QImageReader>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>

namespace qdesigner_internal {

static QStringList extensionList()
{
    static QStringList extension_list;

    if (extension_list.isEmpty()) {
        QList<QByteArray> _extension_list = QImageReader::supportedImageFormats();
        foreach (const QByteArray &ext, _extension_list)
            extension_list.append(QLatin1String("*.") + QString::fromAscii(ext));
    }

    return extension_list;
}

static const int g_file_item_id = 0;
static const int g_dir_item_id = 1;

static QListWidgetItem *createListWidgetItem(const QIcon &icon, const QString &text, int item_id, QListWidget *parent)
{
    QListWidgetItem *result = new QListWidgetItem(icon, text, parent);
    QSize s = parent->iconSize();
    result->setSizeHint(QSize(s.width()*3, s.height()*2));
    result->setData(Qt::UserRole, item_id);
    return result;
}

static bool dirItem(QListWidgetItem *item)
{
    QVariant v = item->data(Qt::UserRole);
    if (!v.canConvert(QVariant::Int))
        return false;
    return v.toInt() == g_dir_item_id;
}

FindIconDialog::FindIconDialog(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent)
{
    m_form = form;
    m_view_dir = QDir::temp();

    ui = new Ui::FindIconDialog;
    ui->setupUi(this);

    QSize icon_size(24, 24);

    ui->m_icon_view->setViewMode(QListWidget::IconMode);
    ui->m_icon_view->setMovement(QListWidget::Static);
    ui->m_icon_view->setResizeMode(QListWidget::Adjust);
    ui->m_icon_view->setIconSize(icon_size);
    ui->m_icon_view->setTextElideMode(Qt::ElideRight);

    ui->m_file_input->setMinimumContentsLength(40);
    ui->m_file_input->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    QSettings settings;
    QStringList recent_dir_list = settings.value(QLatin1String("FindIconDialog/RecentDirectories")).toStringList();
    foreach (const QString &dir, recent_dir_list)
        ui->m_file_input->addItem(dir);

    ui->m_widget_stack->widget(0)->layout()->setMargin(0);
    QWidget *page = ui->m_widget_stack->widget(1);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setMargin(0);
    m_resource_editor = new ResourceEditor(form->core(), page);

    disconnect(form->core()->formWindowManager(),
                SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
                m_resource_editor, SLOT(setActiveForm(QDesignerFormWindowInterface*)));
    m_resource_editor->setActiveForm(form);
    layout->addWidget(m_resource_editor);
    m_resource_editor->layout()->setMargin(0);

    connect(ui->m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->m_specify_file_input, SIGNAL(clicked()), this, SLOT(setActiveBox()));
    connect(ui->m_specify_resource_input, SIGNAL(clicked()), this, SLOT(setActiveBox()));
    connect(ui->m_icon_view, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(currentItemChanged(QListWidgetItem*)));
    connect(ui->m_icon_view, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(itemActivated(QListWidgetItem*)));
    connect(ui->m_cd_up_button, SIGNAL(clicked()), this, SLOT(cdUp()));
    connect(ui->m_file_input->lineEdit(), SIGNAL(textEdited(QString)), this, SLOT(setFile(QString)));
    connect(ui->m_file_input, SIGNAL(currentIndexChanged(QString)), this, SLOT(setFile(QString)));
    connect(m_resource_editor, SIGNAL(fileActivated(QString, QString)),
            this, SLOT(itemActivated(QString, QString)));
    connect(m_resource_editor, SIGNAL(currentFileChanged(QString, QString)),
            this, SLOT(itemChanged(QString, QString)));

    updateButtons();
}

void FindIconDialog::accept()
{
    if (activeBox() == FileBox && !filePath().isEmpty()) {
        QStringList recent_dir_list;
        QString new_path = m_view_dir.path();
        recent_dir_list.append(new_path);
        for (int i = 0; i < 15 && i < ui->m_file_input->count(); ++i) {
            QString path = ui->m_file_input->itemText(i);
            if (path != new_path)
                recent_dir_list.append(path);
        }
        QSettings settings;
        settings.setValue(QLatin1String("FindIconDialog/RecentDirectories"), recent_dir_list);
    }
    QDialog::accept();
}

void FindIconDialog::cdUp()
{
    QDir dir = m_view_dir;
    if (dir.cdUp())
        setFile(dir.path());

    updateButtons();
}

void FindIconDialog::itemActivated(const QString&, const QString &file_name)
{
    if (activeBox() != ResourceBox)
        return;
    if (!file_name.isEmpty())
        accept();

    updateButtons();
}

void FindIconDialog::itemActivated(QListWidgetItem *item)
{
    if (activeBox() != FileBox)
        return;
    QString file = item->text();
    QString path = m_view_dir.filePath(file);

    if (dirItem(item))
        QMetaObject::invokeMethod(this, "setFile", Qt::QueuedConnection, Q_ARG(QString, path));
    else
        accept();

    updateButtons();
}

void FindIconDialog::itemChanged(const QString &qrc_path, const QString &file_name)
{
    if (activeBox() != ResourceBox)
        return;

    m_resource_data.file = file_name;
    m_resource_data.qrc = qrc_path;

    updateButtons();
}

void FindIconDialog::currentItemChanged(QListWidgetItem *item)
{
    if (activeBox() != FileBox)
        return;

    if (item == 0)
        return;
    QString path = m_view_dir.filePath(item->text());
    ui->m_file_input->lineEdit()->setText(path);

    if (dirItem(item))
        m_file_data.file.clear();
    else
        m_file_data.file = path;

    updateButtons();
}

void FindIconDialog::setViewDir(const QString &path)
{

    static const QIcon dir_icon(style()->standardPixmap(QStyle::SP_DirClosedIcon));

    if (path == m_view_dir.path())
        return;
    m_view_dir.setPath(path);
    ui->m_icon_view->clear();
    QStringList subdir_list = m_view_dir.entryList(QStringList() << "*", QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList icon_file_list = m_view_dir.entryList(extensionList(), QDir::Files);

    foreach (const QString &subdir, subdir_list)
        createListWidgetItem(dir_icon, subdir, g_dir_item_id, ui->m_icon_view);
    foreach (const QString &icon_file, icon_file_list) {
        QIcon icon(m_view_dir.filePath(icon_file));
        if (!icon.isNull())
            createListWidgetItem(icon, icon_file, g_file_item_id, ui->m_icon_view);
    }
}

void FindIconDialog::setFile(const QString &path)
{
    QFileInfo info(path);

    QString file, dir;
    if (info.isDir()) {
        dir = path;
    } else {
        dir = info.path();
        file = info.fileName();
    }

    setViewDir(dir);

    ui->m_file_input->lineEdit()->setText(path);

    m_file_data.file.clear();
    ui->m_icon_view->clearSelection();
    if (!file.isEmpty()) {
        QList<QListWidgetItem*> item_list = ui->m_icon_view->findItems(file, Qt::MatchExactly);
        if (!item_list.isEmpty()) {
            ui->m_icon_view->setItemSelected(item_list.first(), true);
            m_file_data.file = path;
        }
    }

    updateButtons();
}

FindIconDialog::~FindIconDialog()
{
    delete ui;
    ui = 0;
}

void FindIconDialog::setQrc(const QString &qrc_path, const QString &file_name)
{
    m_resource_editor->setCurrentFile(qrc_path, file_name);
    m_resource_data.file = file_name;
    m_resource_data.qrc = qrc_path;
    updateButtons();
}

void FindIconDialog::setPaths(const QString &qrcPath, const QString &filePath)
{
    if (qrcPath.isEmpty()) {
        setActiveBox(FileBox);
        setFile(filePath);
    } else {
        setFile(m_form->absoluteDir().path());
        setActiveBox(ResourceBox);
        setQrc(qrcPath, filePath);
    }
}

void FindIconDialog::updateButtons()
{
    ui->m_ok_button->setEnabled(!filePath().isEmpty());
}

void FindIconDialog::setActiveBox()
{
    setActiveBox(sender() == ui->m_specify_file_input ? FileBox : ResourceBox);
}

void FindIconDialog::setActiveBox(InputBox box)
{
    if (box == FileBox) {
        ui->m_specify_file_input->setChecked(true);
        ui->m_widget_stack->setCurrentIndex(0);
    } else {
        ui->m_specify_resource_input->setChecked(true);
        ui->m_widget_stack->setCurrentIndex(1);
    }

    updateButtons();
}

FindIconDialog::InputBox FindIconDialog::activeBox() const
{
    return ui->m_widget_stack->currentIndex() == 0 ? FileBox : ResourceBox;
}

QString FindIconDialog::qrcPath() const
{
    return activeBox() == FileBox ? QString() : m_resource_data.qrc;
}

QString FindIconDialog::filePath() const
{
    return activeBox() == FileBox ? m_file_data.file : m_resource_data.file;
}

} // namespace qdesigner_internal
