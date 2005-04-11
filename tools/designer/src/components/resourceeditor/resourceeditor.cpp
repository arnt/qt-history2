#include <QtCore/QFileInfo>
#include <QtCore/qdebug.h>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeView>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QToolButton>
#include <QtGui/QItemDelegate>
#include <QtGui/QKeyEvent>
#include <QtGui/QDrag>

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindowmanager.h>

#include <resourcefile.h>
#include <iconloader.h>

#include "resourceeditor.h"

#define COMBO_EMPTY_DATA 0
#define COMBO_NEW_DATA 1
#define COMBO_OPEN_DATA 2

/******************************************************************************
** QrcItemDelegate
*/

class QrcItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    QrcItemDelegate(QObject *parent = 0);
    virtual void setModelData(QWidget *editor,
                                QAbstractItemModel *model,
                                const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
};

QrcItemDelegate::QrcItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void QrcItemDelegate::setModelData(QWidget *_editor,
                            QAbstractItemModel *_model,
                            const QModelIndex &index) const
{
    QLineEdit *editor = qobject_cast<QLineEdit*>(_editor);
    if (editor == 0)
        return;

    ResourceModel *model = qobject_cast<ResourceModel*>(_model);
    if (model == 0)
        return;
                
    QString new_prefix = ResourceFile::fixPrefix(editor->text());
    QString old_prefix, file;
    model->getItem(index, old_prefix, file);

    if (old_prefix.isEmpty())
        return;

    if (old_prefix == new_prefix)
        return;
        
    model->changePrefix(index, new_prefix);
    model->save();
}

void QrcItemDelegate::setEditorData(QWidget *_editor, const QModelIndex &index) const
{
    const ResourceModel *model = qobject_cast<const ResourceModel*>(index.model());
    if (model == 0)
        return;

    QLineEdit *editor = qobject_cast<QLineEdit*>(_editor);
    if (editor == 0)
        return;
        
    QString prefix, file;
    model->getItem(index, prefix, file);
    editor->setText(prefix);
}

/******************************************************************************
** QrcView
*/

class QrcView : public QTreeView
{
    Q_OBJECT
public:
    QrcView(QWidget *parent = 0);
};

QrcView::QrcView(QWidget *parent)
    : QTreeView(parent)
{
    setItemDelegate(new QrcItemDelegate(this));
    setEditTriggers(QTreeView::DoubleClicked
                            | QTreeView::AnyKeyPressed);
    header()->hide();
    setDragEnabled(true);
}

/******************************************************************************
** EditableResourceModel
*/

class EditableResourceModel : public QAbstractFormBuilderModel
{
public:
    EditableResourceModel(const ResourceFile &resource_file, QObject *parent = 0);
    virtual ItemFlags flags(const QModelIndex &index) const;
    virtual QStringList mimeTypes() const;
};

EditableResourceModel::EditableResourceModel(const ResourceFile &resource_file,
                                                QObject *parent)
    : ResourceModel(resource_file, parent)
{
}

QAbstractItemModel::ItemFlags EditableResourceModel::flags(const QModelIndex &index) const
{
    QAbstractItemModel::ItemFlags result
        = ItemIsSelectable
            | ItemIsEnabled
            | ItemIsDropEnabled;
    
    QString prefix, file;
    getItem(index, prefix, file);
    
    if (file.isEmpty())
        result |= ItemIsEditable;

    return result;
}

QStringList EditableResourceModel::mimeTypes() const
{
    QStringList result = QAbstractItemModel::mimeTypes();
    result << QLatin1String("text/uri-list");
    return result;
}

/******************************************************************************
** ModelCache
*/

class ModelCache
{
public:
    ResourceModel *model(const QString &file);

private:
    QList<ResourceModel*> m_model_list;
};

Q_GLOBAL_STATIC(ModelCache, g_model_cache)

ResourceModel *ModelCache::model(const QString &file)
{
    if (file.isEmpty()) {
        ResourceModel *model = new EditableResourceModel(ResourceFile());
        m_model_list.append(model);
        return model;
    }

    for (int i = 0; i < m_model_list.size(); ++i) {
        ResourceModel *model = m_model_list.at(i);
        if (model->fileName() == file)
            return model;
    }

    ResourceFile rf(file);
    if (!rf.load()) {
        QMessageBox::warning(0, QObject::tr("Error opening resource file"),
                                QObject::tr("Failed to open \"%1\":\n%2")
                                    .arg(file).arg(rf.errorMessage()),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return 0;
    }

    ResourceModel *model = new EditableResourceModel(rf);
    m_model_list.append(model);
    return model;
}

/******************************************************************************
** ResourceEditor
*/

ResourceEditor::ResourceEditor(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    m_form = form;

    connect(form, SIGNAL(mainContainerChanged(QWidget*)),
            this, SLOT(updateQrcStack()));
    connect(form, SIGNAL(fileNameChanged(const QString &)),
            this, SLOT(updateQrcPaths()));
    connect(m_qrc_combo, SIGNAL(activated(int)),
            this, SLOT(setCurrentIndex(int)));

    m_remove_qrc_button->setIcon(createIconSet("editdelete.png"));
    connect(m_remove_qrc_button, SIGNAL(clicked()), this, SLOT(removeCurrentView()));
    
    connect(m_add_button, SIGNAL(clicked()), this, SLOT(addPrefix()));
    connect(m_remove_button, SIGNAL(clicked()), this, SLOT(deleteItem()));
    connect(m_add_files_button, SIGNAL(clicked()), this, SLOT(addFiles()));

    updateUi();
}

void ResourceEditor::insertEmptyComboItem()
{
    if (m_qrc_combo->count() == 0)
        return;
    QVariant v = m_qrc_combo->itemData(0);
    if (v.type() == QVariant::Int && v.toInt() == COMBO_EMPTY_DATA)
        return;
    m_qrc_combo->insertItem(0, QIcon(), tr("<no resource fiels>"), QVariant(COMBO_EMPTY_DATA));
    m_qrc_combo->setCurrentIndex(0);
}

void ResourceEditor::removeEmptyComboItem()
{
    if (m_qrc_combo->count() == 0)
        return;
    QVariant v = m_qrc_combo->itemData(0);
    if (v.type() != QVariant::Int || v.toInt() != COMBO_EMPTY_DATA)
        return;
    m_qrc_combo->removeItem(0);
}

int ResourceEditor::qrcCount() const
{
    return m_qrc_stack->count();
}

QTreeView *ResourceEditor::view(int i) const
{
    if (i >= qrcCount() || i < 0)
        return 0;
    return qobject_cast<QTreeView*>(m_qrc_stack->widget(i));
}

ResourceModel *ResourceEditor::model(int i) const
{
    if (i >= qrcCount() || i < 0)
        return 0;
    return qobject_cast<ResourceModel*>(view(i)->model());
}

QTreeView *ResourceEditor::currentView() const
{
    int idx = currentIndex();
    if (idx == -1)
        return 0;
    return view(idx);
}

ResourceModel *ResourceEditor::currentModel() const
{
    int idx = currentIndex();
    if (idx == -1)
        return 0;
    return model(idx);
}

void ResourceEditor::getCurrentItem(QString &prefix, QString &file)
{
    prefix.clear();
    file.clear();

    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    model->getItem(view->currentIndex(), prefix, file);
}

void ResourceEditor::addPrefix()
{
    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    QModelIndex idx = model->addNewPrefix();
    view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
    model->save();
    updateUi();
}

void ResourceEditor::addFiles()
{
    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    QStringList file_list = QFileDialog::getOpenFileNames(this, tr("Open file"),
                                                            model->absolutePath(QString()),
                                                            tr("All files (*)"));
    if (file_list.isEmpty())
        return;

    QModelIndex idx = model->addFiles(view->currentIndex(), file_list);
    if (idx.isValid()) {
        view->setExpanded(model->prefixIndex(view->currentIndex()), true);
        view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
    }

    model->save();
    updateUi();
}

void ResourceEditor::deleteItem()
{
    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    QModelIndex cur_idx = view->currentIndex();
    if (!cur_idx.isValid())
        return;

    QModelIndex idx = model->deleteItem(cur_idx);

    if (idx.isValid()) {
        QModelIndex pref_idx = model->prefixIndex(idx);
        if (pref_idx != idx)
            view->setExpanded(pref_idx, true);
        view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
    }

    model->save();
    updateUi();
}

void ResourceEditor::updateUi()
{
    QString prefix, file;
    getCurrentItem(prefix, file);

    m_add_button->setEnabled(currentModel() != 0);
    m_remove_button->setEnabled(!prefix.isEmpty());
    m_add_files_button->setEnabled(!prefix.isEmpty());
    m_remove_qrc_button->setEnabled(currentModel() != 0);
}

int ResourceEditor::currentIndex() const
{
    return m_qrc_stack->currentIndex();
}

void ResourceEditor::setCurrentIndex(int i)
{
    QVariant v = m_qrc_combo->itemData(i);
    if (v.type() == QVariant::Int) {
        switch (v.toInt()) {
            case COMBO_EMPTY_DATA: {
                    bool blocked = m_qrc_combo->blockSignals(true);
                    m_qrc_combo->setCurrentIndex(i);
                    m_qrc_combo->blockSignals(blocked);
                }
                break;
            case COMBO_OPEN_DATA:
                openView();
                break;
            case COMBO_NEW_DATA:
                newView();
                break;
            default:
                break;
        }
    } else {
        bool blocked = m_qrc_combo->blockSignals(true);
        m_qrc_combo->setCurrentIndex(i);
        m_qrc_combo->blockSignals(blocked);
        m_qrc_stack->setCurrentIndex(i);
    }
        
    updateUi();
}

void ResourceEditor::updateQrcStack()
{
    m_qrc_combo->clear();
    while (m_qrc_stack->count() > 0) {
        QWidget *w = m_qrc_stack->widget(0);
        m_qrc_stack->removeWidget(w);
        delete w;
    }
    
    QStringList qrc_file_list = m_form->resourceFiles();
    foreach (QString qrc_file, qrc_file_list)
        addView(qrc_file);

    m_qrc_combo->addItem(QIcon(), tr("New..."), QVariant(COMBO_NEW_DATA));
    m_qrc_combo->addItem(QIcon(), tr("Open..."), QVariant(COMBO_OPEN_DATA));
    if (qrc_file_list.isEmpty())
        insertEmptyComboItem();
            
    updateUi();
}

QString ResourceEditor::qrcName(const QString &path) const
{
    if (path.isEmpty())
        return tr("Untitled");
    return m_form->relativePath(path);
}

void ResourceEditor::updateQrcPaths()
{
    for (int i = 0; i < m_qrc_stack->count(); ++i) {
        ResourceModel *model = this->model(i);
        m_qrc_combo->setItemText(i, qrcName(model->fileName()));
    }
}

void ResourceEditor::addView(const QString &qrc_file)
{
    int idx = qrcCount();

    QTreeView *view = new QrcView;
    ResourceModel *model = g_model_cache()->model(qrc_file);
    if (model == 0)
        return;
    removeEmptyComboItem();
        
    view->setModel(model);
    m_qrc_combo->insertItem(idx, qrcName(qrc_file));
    m_qrc_stack->addWidget(view);
    connect(view->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(updateUi()));
    connect(model, SIGNAL(dirtyChanged(bool)), this, SLOT(updateUi()));

    setCurrentIndex(idx);

    if (!qrc_file.isEmpty())
        m_form->addResourceFile(qrc_file);

    updateUi();
}

void ResourceEditor::saveCurrentView()
{
    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    if (model->fileName().isEmpty()) {
        QString file_name = QFileDialog::getSaveFileName(this, tr("Save resource file"),
                                                            m_form->absolutePath(QString()),
                                                            tr("Resource files (*.qrc)"));
        if (file_name.isEmpty())
            return;
        model->setFileName(file_name);
        m_form->addResourceFile(file_name);
        QString s = QFileInfo(file_name).fileName();
        bool blocked = m_qrc_combo->blockSignals(true);
        m_qrc_combo->setItemText(currentIndex(), s);
        m_qrc_combo->setCurrentIndex(-1);
        m_qrc_combo->setCurrentIndex(currentIndex());
        m_qrc_combo->blockSignals(blocked);
    }

    model->save();
    updateUi();
}

int ResourceEditor::indexOfView(QTreeView *view)
{
    for (int i = 0; i < m_qrc_stack->count(); ++i) {
        if (view == m_qrc_stack->widget(i))
            return i;
    }
    return -1;
}

void ResourceEditor::removeCurrentView()
{
    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    QString file_name = model->fileName();

    int idx = indexOfView(view);
    if (idx == -1)
        return;

    m_qrc_combo->removeItem(idx);
    m_qrc_stack->removeWidget(view);
    delete view;

    disconnect(model, SIGNAL(dirtyChanged(bool)), this, SLOT(updateUi()));

    if (!file_name.isEmpty())
        m_form->removeResourceFile(file_name);

    if (qrcCount() == 0) {
        insertEmptyComboItem();
    } else {
        if (idx < qrcCount())
            setCurrentIndex(idx);
        else if (idx > 0)
            setCurrentIndex(idx - 1);
    }
    updateUi();
}

void ResourceEditor::reloadCurrentView()
{
    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    model->reload();
    updateUi();
}

void ResourceEditor::newView()
{
    QString file_name = QFileDialog::getSaveFileName(this, tr("New resource file"),
                                                        m_form->absolutePath(QString()),
                                                        tr("Resource files (*.qrc)"));
    if (file_name.isEmpty()) {
        setCurrentIndex(m_qrc_stack->count() == 0 ? 0 : m_qrc_stack->currentIndex());
        return;
    }

    ResourceFile rf(file_name);
    if (!rf.load())
        rf.save();
                
    addView(file_name);
}

void ResourceEditor::openView()
{
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open resource file"),
                                                        m_form->absolutePath(QString()),
                                                        tr("Resource files (*.qrc)"));
    if (file_name.isEmpty()) {
        setCurrentIndex(m_qrc_stack->count() == 0 ? 0 : m_qrc_stack->currentIndex());
        return;
    }

    addView(file_name);
}

#include "resourceeditor.moc"
