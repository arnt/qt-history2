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

#include "resourcefile_p.h"
#include "findicondialog_p.h"
#include "ui_findicondialog.h"
#include "resourceeditor_p.h"

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractresourcebrowser.h>
#include <QtDesigner/abstractlanguage.h>
#include <QtDesigner/qextensionmanager.h>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

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
#include <QtGui/QPushButton>

namespace qdesigner_internal {

QStringList extensionList()
{
    static QStringList extension_list;

    if (extension_list.isEmpty()) {
        QList<QByteArray> _extension_list = QImageReader::supportedImageFormats();
        foreach (const QByteArray &ext, _extension_list)
            extension_list.append(QLatin1String("*.") + QString::fromAscii(ext));
    }

    return extension_list;
}

bool isIconValid(const QString &file)
{
    bool enabled = !file.isEmpty();
    if (enabled) {
        QStringList ext_list = extensionList();
        foreach (QString ext, ext_list) {
            if (file.endsWith(ext.remove(0, 2), Qt::CaseInsensitive)) {
                return true;
            }
        }
    }
    return false;
}

    enum {  g_file_item_id, g_dir_item_id };
    static const char* FindDialogDirSettingsKey="FindIconDialog/RecentDirectories";

QListWidgetItem *createListWidgetItem(const QIcon &icon, const QString &text, int item_id, QListWidget *parent)
{
    QListWidgetItem *result = new QListWidgetItem(icon, text, parent);
    QSize s = parent->iconSize();
    result->setSizeHint(QSize(s.width()*3, s.height()*2));
    result->setData(Qt::UserRole, item_id);
    return result;
}

bool dirItem(QListWidgetItem *item)
{
    QVariant v = item->data(Qt::UserRole);
    if (!v.canConvert(QVariant::Int))
        return false;
    return v.toInt() == g_dir_item_id;
}
}

namespace qdesigner_internal {

FindIconDialog::FindIconDialog(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::FindIconDialog),
      m_form (form),
      m_view_dir(QDir::temp()),
      m_resource_editor(0),
      m_language_editor(0)
{
    ui->setupUi(this);
    const QSize icon_size(24, 24);
    ui->m_icon_view->setViewMode(QListWidget::IconMode);
    ui->m_icon_view->setMovement(QListWidget::Static);
    ui->m_icon_view->setResizeMode(QListWidget::Adjust);
    ui->m_icon_view->setIconSize(icon_size);
    ui->m_icon_view->setTextElideMode(Qt::ElideRight);

    ui->m_file_input->setMinimumContentsLength(40);
    ui->m_file_input->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    QSettings settings;
    QStringList recent_dir_list = settings.value(QLatin1String(FindDialogDirSettingsKey)).toStringList();
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


    QDesignerFormEditorInterface *core = form->core();
    if (QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core)) {
        m_language_editor = lang->createResourceBrowser(ui->m_widget_stack);
	connect(m_language_editor, SIGNAL( currentPathChanged(const QString&)),
		 this,SLOT(itemChanged(const QString&)));
	connect(m_language_editor, SIGNAL( pathActivated(const QString&)),
		 this,SLOT(itemActivated(const QString&)));
        ui->m_widget_stack->addWidget(m_language_editor);
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

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

#ifdef Q_OS_WIN
    isRoot = false;
    
    QSettings myComputer("HKEY_CLASSES_ROOT\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}", QSettings::NativeFormat);
    rootDir = myComputer.value(".").toString();
#endif

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
        settings.setValue(QLatin1String(FindDialogDirSettingsKey), recent_dir_list);
    }
    if (activeBox() == ResourceBox) {
        setDefaultQrcPath(qrcPath());
    } else if (activeBox() == LanguageBox) {
        setDefaultLanguagePath(filePath());
    } else {
        setDefaultFilePath(QFileInfo(filePath()).absolutePath());
    }
    setPreviousInputBox(activeBox());
    QDialog::accept();
}

void FindIconDialog::cdUp()
{
    QDir dir = m_view_dir;

#ifdef Q_OS_WIN
    if (dir.cdUp() && !isRoot) {
        setFile(dir.canonicalPath());
    } else if (!isRoot)
        setFile(rootDir);
#else
    if (dir.cdUp())
        setFile(dir.path());
#endif
  
    updateButtons();
}

void FindIconDialog::itemActivated(const QString&, const QString &file_name)
{
    if (activeBox() != ResourceBox)
        return;
    if (isIconValid(file_name))
        accept();

    updateButtons();
}

void FindIconDialog::itemActivated(QListWidgetItem *item)
{
    if (!item || activeBox() != FileBox)
        return;
    QString file = item->text();
    QString path = m_view_dir.filePath(file);

    if (dirItem(item)) {
#ifdef Q_OS_WIN
        isRoot = false;
#endif
        QMetaObject::invokeMethod(this, "setFile", Qt::QueuedConnection, Q_ARG(QString, path));
    } else
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

void FindIconDialog::itemChanged( const QString &file_name)
{
    if (activeBox() != LanguageBox)
        return;

    m_language_data.file = file_name;

    updateButtons();
}

void FindIconDialog::itemActivated(const QString &file_name)
{
    if (activeBox() != LanguageBox)
        return;

    itemChanged(file_name);

    if (isIconValid(file_name))
        accept();

    updateButtons();
}

void FindIconDialog::setViewDir(const QString &path)
{
    static const QIcon dir_icon(style()->standardPixmap(QStyle::SP_DirClosedIcon));
#ifdef Q_OS_WIN
    static const QIcon drive_icon(style()->standardPixmap(QStyle::SP_DriveHDIcon));
    if(!isRoot)
#endif
    {
        if (path == m_view_dir.path() || !QFile::exists(path))
            return;
    }

    m_view_dir.setPath(path);
    ui->m_icon_view->clear();

    QStringList subdir_list;
#ifdef Q_OS_WIN
    if (isRoot) {
        QFileInfoList qFIL = QDir::drives();
        foreach(const QFileInfo &info, qFIL) 
            subdir_list.append(info.path());
    } else
        subdir_list = m_view_dir.entryList(QStringList() << "*", QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QString &subdir, subdir_list)
        createListWidgetItem((isRoot ? drive_icon : dir_icon), subdir, g_dir_item_id, ui->m_icon_view);
#else
    subdir_list = m_view_dir.entryList(QStringList() << "*", QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &subdir, subdir_list)
        createListWidgetItem(dir_icon, subdir, g_dir_item_id, ui->m_icon_view);
#endif

    QStringList icon_file_list = m_view_dir.entryList(extensionList(), QDir::Files);
    foreach (const QString &icon_file, icon_file_list) {
        QIcon icon(m_view_dir.filePath(icon_file));
        if (!icon.isNull())
            createListWidgetItem(icon, icon_file, g_file_item_id, ui->m_icon_view);
    }
}

void FindIconDialog::setFile(const QString &path)
{
    QString file;
    QString dir = path;
#ifdef Q_OS_WIN
    isRoot = false;
    if (dir.contains(rootDir, Qt::CaseInsensitive))
        isRoot = true;

    if (!isRoot)
#endif
    {
        QFileInfo info(path);

        if (info.isFile()) {
            dir = info.path();
            file = info.fileName();
        }
    }

    setViewDir(dir);

    int cursorPos = ui->m_file_input->lineEdit()->cursorPosition();
    ui->m_file_input->lineEdit()->setText(path);
    ui->m_file_input->lineEdit()->setCursorPosition(cursorPos);

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
    if (!m_resource_editor)
        return;
    m_resource_editor->setCurrentFile(qrc_path, file_name);
    m_resource_data.file = file_name;
    m_resource_data.qrc = qrc_path;
    updateButtons();
}

void FindIconDialog::setLanguagePath(const QString &file_name)
{
    if (!m_language_editor)
        return;
    m_language_editor->setCurrentPath(file_name);
    m_language_data.file = file_name;
    updateButtons();
}

void FindIconDialog::setPaths(const QString &qrcPath, const QString &filePath)
{
    if (!qrcPath.isEmpty()) {
        setFile(defaultFilePath(m_form));
        setActiveBox(ResourceBox);
        setQrc(qrcPath, filePath);
    } else if (!filePath.isEmpty()) {
        QDesignerFormEditorInterface *core = m_form->core();
        QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core);
        if (lang && lang->isLanguageResource(filePath)) {
            setActiveBox(LanguageBox);
            m_language_editor->setCurrentPath(filePath);
        } else {
            setActiveBox(FileBox);
            setFile(filePath);
        }
    } else {
        if (previousInputBox() == ResourceBox && !defaultQrcPath().isEmpty()) {
            setFile(defaultFilePath(m_form));
            setActiveBox(ResourceBox);
            setQrc(defaultQrcPath(), "");
        } else if (previousInputBox() == LanguageBox) {
            setLanguagePath(defaultLanguagePath());
            setActiveBox(LanguageBox);
        } else {
            setActiveBox(FileBox);
            setFile(defaultFilePath(m_form));
        }
    }
}

void FindIconDialog::updateButtons()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isIconValid(filePath()));
}

void FindIconDialog::setActiveBox()
{
    InputBox inputBox = FileBox;
    if (sender() != ui->m_specify_file_input) {
        if (m_language_editor)
            inputBox = LanguageBox;
        else
            inputBox = ResourceBox;
    }
    setActiveBox(inputBox);
}

void FindIconDialog::setActiveBox(InputBox box)
{
    if (box == FileBox) {
        ui->m_specify_file_input->setChecked(true);
        ui->m_widget_stack->setCurrentIndex(0);
    } else {
        ui->m_specify_resource_input->setChecked(true);
        if (box == ResourceBox || !m_language_editor)
            ui->m_widget_stack->setCurrentIndex(1);
        else
            ui->m_widget_stack->setCurrentIndex(2);
    }

    updateButtons();
}

FindIconDialog::InputBox FindIconDialog::activeBox() const
{
    const int page = ui->m_widget_stack->currentIndex();
    switch (page) {
        case 2:  return LanguageBox;
        case 1:  return ResourceBox;
        case 0:
        default: return FileBox;
    }
    return FileBox;
}

QString FindIconDialog::qrcPath() const
{
    return activeBox() == ResourceBox ? m_resource_data.qrc : QString();
}

QString FindIconDialog::filePath() const
{
    switch (activeBox()) {
        case FileBox:     return m_file_data.file;
        case ResourceBox: return m_resource_data.file;
        case LanguageBox: return m_language_data.file;
    }
    return QString();
}

QString FindIconDialog::defaultQrcPath()
{
    QSettings settings;
    return settings.value("FindIconDialog/defaultQrcPath").toString();
}

QString FindIconDialog::defaultFilePath(QDesignerFormWindowInterface *form)
{
    QSettings settings;
    QString path = settings.value("FindIconDialog/defaultFilePath").toString();
    if (path.isEmpty())
        path = form->absoluteDir().path();
    return path;
}

QString FindIconDialog::defaultLanguagePath()
{
    QSettings settings;
    return settings.value("FindIconDialog/defaultLanguagePath").toString();
}

void FindIconDialog::setDefaultQrcPath(const QString &path)
{
    QSettings settings;
    settings.setValue("FindIconDialog/defaultQrcPath", path);
}

void FindIconDialog::setDefaultFilePath(const QString &path)
{
    QSettings settings;
    settings.setValue("FindIconDialog/defaultFilePath", path);
}

void FindIconDialog::setDefaultLanguagePath(const QString &path)
{
    QSettings settings;
    settings.setValue("FindIconDialog/defaultLanguagePath", path);
}

FindIconDialog::InputBox FindIconDialog::previousInputBox()
{
    QSettings settings;
    QString box = settings.value("FindIconDialog/previousInputBox").toString();
    if (box == QLatin1String("language"))
        return LanguageBox;
    if (box == QLatin1String("resource"))
        return ResourceBox;
    return FileBox;
}

void FindIconDialog::setPreviousInputBox(InputBox box)
{
    QSettings settings;
    QString val;
    switch (box) {
        case FileBox:     val = QLatin1String("file"); break;
        case ResourceBox: val = QLatin1String("resource"); break;
        case LanguageBox: val = QLatin1String("language"); break;
    }
    settings.setValue("FindIconDialog/previousInputBox", val);
}

} // namespace qdesigner_internal
