#include <QtCore/QFileInfo>
#include <QtCore/qdebug.h>
#include <QtCore/QAbstractItemModel>
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

#include <abstractformeditor.h>
#include <abstractformwindowmanager.h>

#include <resourcefile.h>
#include "resourceeditor.h"

/******************************************************************************
** ResourceModel
*/

class ResourceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ResourceModel(const ResourceFile &resource_file, QObject *parent = 0);
    
    QModelIndex index(int row, int column,
                        const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = DisplayRole) const;

    QString fileName() const { return m_resource_file.fileName(); }
    void getItem(const QModelIndex &index, QString &prefix, QString &file) const;

    QModelIndex addNewPrefix();
    QModelIndex addFiles(const QModelIndex &idx, const QStringList &file_list);
    void changePrefix(const QModelIndex &idx, const QString &prefix);
    QModelIndex prefixIndex(const QModelIndex &sel_idx) const;
    void deleteItem(const QModelIndex &idx);

    QString absolutePath(const QString &path) const { return m_resource_file.absolutePath(path); }

    void reload();
    void save();

    bool dirty() const { return m_dirty; }
    void setDirty(bool b);

signals:
    void dirtyChanged(bool b);
    
private:
    ResourceFile m_resource_file;
    bool m_dirty;
};

ResourceModel::ResourceModel(const ResourceFile &resource_file, QObject *parent)
    : QAbstractItemModel(parent), m_resource_file(resource_file)
{
    m_dirty = false;
}

void ResourceModel::setDirty(bool b)
{
    if (b == m_dirty)
        return;
    m_dirty = b;
    emit dirtyChanged(b);
}

QModelIndex ResourceModel::index(int row, int column,
                                    const QModelIndex &parent) const
{
    QModelIndex result;

    qint64 d = reinterpret_cast<qint64>(parent.data());
    
    if (!parent.isValid()) {
        if (row < m_resource_file.prefixCount())
            result = createIndex(row, 0, reinterpret_cast<void*>(-1));
    } else if (column == 0
                && d == -1
                && parent.row() < m_resource_file.prefixCount()
                && row < m_resource_file.fileCount(parent.row())) {
        result = createIndex(row, 0, reinterpret_cast<void*>(parent.row()));
    }

//    qDebug() << "ResourceModel::index(" << row << column << parent << "):" << result;
    
    return result;
}

QModelIndex ResourceModel::parent(const QModelIndex &index) const
{
    QModelIndex result;

    qint64 d = reinterpret_cast<qint64>(index.data());
    
    if (index.isValid() && d != -1)
        result = createIndex(d, 0, reinterpret_cast<void*>(-1));

//    qDebug() << "ResourceModel::parent(" << index << "):" << result;
    
    return result;
}

int ResourceModel::rowCount(const QModelIndex &parent) const
{
    int result = 0;
    
    qint64 d = reinterpret_cast<qint64>(parent.data());
    
    if (!parent.isValid())
        result = m_resource_file.prefixCount();
    else if (d == -1)
        result = m_resource_file.fileCount(parent.row());

//    qDebug() << "ResourceModel::rowCount(" << parent << "):" << result;
    
    return result;
}

int ResourceModel::columnCount(const QModelIndex &parent) const
{
    int result = 0;

    qint64 d = reinterpret_cast<qint64>(parent.data());
    
    if (!parent.isValid())
        result = m_resource_file.prefixCount() == 0 ? 0 : 1;
    else if (d == -1)
        result = m_resource_file.fileCount(parent.row()) == 0 ? 0 : 1;
    
//    qDebug() << "ResourceModel::columnCount(" << parent << "):" << result;

    return result;
}

bool ResourceModel::hasChildren(const QModelIndex &parent) const
{
    bool result = false;
    
    qint64 d = reinterpret_cast<qint64>(parent.data());
    
    if (!parent.isValid())
        result = m_resource_file.prefixCount() > 0;
    else if (d == -1)
        result = m_resource_file.fileCount(parent.row()) > 0;

//    qDebug() << "ResourceModel::hasChildren(" << parent << "):" << result;
    
    return result;
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    qint64 d = reinterpret_cast<qint64>(index.data());

    QVariant result;

    switch (role) {
        case DisplayRole:
            if (d == -1)
                result = m_resource_file.prefix(index.row());
            else
                result = m_resource_file.file(d, index.row());
            break;
        default:
            break;
    };

    return result;
}

void ResourceModel::getItem(const QModelIndex &index, QString &prefix, QString &file) const
{
    prefix.clear();
    file.clear();

    if (!index.isValid())
        return;

    qint64 d = reinterpret_cast<qint64>(index.data());

    if (d == -1) {
        prefix = m_resource_file.prefix(index.row());
    } else {
        prefix = m_resource_file.prefix(d);
        file = m_resource_file.file(d, index.row());
    }
}

QModelIndex ResourceModel::prefixIndex(const QModelIndex &sel_idx) const
{
    if (!sel_idx.isValid())
        return QModelIndex();
    QModelIndex parent = this->parent(sel_idx);
    return parent.isValid() ? parent : sel_idx;
}

QModelIndex ResourceModel::addNewPrefix()
{
    int i = 1;
    QString prefix;
    do prefix = tr("/new/prefix%1").arg(i++);
    while (m_resource_file.contains(prefix));

    m_resource_file.addPrefix(prefix);
    i = m_resource_file.indexOfPrefix(prefix);
    emit rowsInserted(QModelIndex(), i, i);

    setDirty(true);
    
    return index(i, 0, QModelIndex());
}

QModelIndex ResourceModel::addFiles(const QModelIndex &idx, const QStringList &file_list)
{
    if (!idx.isValid())
        return QModelIndex();
    QModelIndex prefix_idx = prefixIndex(idx);
    QString prefix = m_resource_file.prefix(prefix_idx.row());
    Q_ASSERT(!prefix.isEmpty());

    int added = 0;
    foreach (QString file, file_list) {
        if (m_resource_file.contains(prefix, file))
            continue;
        m_resource_file.addFile(prefix, file);
        ++added;
    }

    int cnt = m_resource_file.fileCount(prefix_idx.row());
    if (added > 0) {
        emit rowsInserted(prefix_idx, cnt - added, cnt - 1);
        setDirty(true);
    }
    
    return index(cnt - 1, 0, prefix_idx);
}

void ResourceModel::changePrefix(const QModelIndex &idx, const QString &prefix)
{
    if (!idx.isValid())
        return;

    QString fixed_prefix = ResourceFile::fixPrefix(prefix);
    if (m_resource_file.contains(fixed_prefix))
        return;

    QModelIndex prefix_idx = prefixIndex(idx);

    QString old_prefix = m_resource_file.prefix(prefix_idx.row());
    Q_ASSERT(!old_prefix.isEmpty());

    m_resource_file.changePrefix(old_prefix, fixed_prefix);
    emit dataChanged(prefix_idx, prefix_idx);
    setDirty(true);
}

void ResourceModel::deleteItem(const QModelIndex &idx)
{
    if (!idx.isValid())
        return;

    QString prefix, file;
    getItem(idx, prefix, file);
    Q_ASSERT(!prefix.isEmpty());

    emit rowsAboutToBeRemoved(parent(idx), idx.row(), idx.row());
        
    if (file.isEmpty())
        m_resource_file.removePrefix(prefix);
    else
        m_resource_file.removeFile(prefix, file);

    setDirty(true);
}

void ResourceModel::reload()
{
    m_resource_file.load();
    emit reset();
    setDirty(false);
}

void ResourceModel::save()
{
    m_resource_file.save();
    setDirty(false);
}

/******************************************************************************
** FormTab
*/

class FormTab : public QWidget
{
    Q_OBJECT

public:
    FormTab(AbstractFormWindow *form, ResourceEditor *editor);

    AbstractFormWindow *form() const { return m_form; }
    int qrcCount();
    bool dirty() const;
    
public slots:
    void updateQrcList();
    void updateUi();
    void addPrefix();
    void addFiles();
    void deleteItem();
    void setCurrentPrefix(const QString &prefix);
    void setCurrentIndex(int i);
    
    void addView(const QString &file_name);
    void saveCurrentView();
    void removeCurrentView();
    void reloadCurrentView();

signals:
    void dirtyChanged(bool);
    
private:
    AbstractFormWindow *m_form;
    ResourceEditor *m_editor;

    QComboBox *m_qrc_combo;
    QStackedWidget *m_qrc_stack;
    QPushButton *m_add_prefix_button;
    QPushButton *m_add_files_button;
    QPushButton *m_delete_button;
    QLineEdit *m_prefix_edit;

    void getCurrentItem(QString &prefix, QString &file);
    QTreeView *currentView() const;
    ResourceModel *currentModel() const;
    int indexOfView(QTreeView *view);
};

FormTab::FormTab(AbstractFormWindow *form, ResourceEditor *editor)
    : QWidget(0)
{
    m_form = form;
    m_editor = editor;

    connect(form, SIGNAL(mainContainerChanged(QWidget*)),
            this, SLOT(updateQrcList()));

    QVBoxLayout *layout1 = new QVBoxLayout(this);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout1->addLayout(layout2);
    layout2->addWidget(new QLabel(tr("Resource file:"), this));
    m_qrc_combo = new QComboBox(this);
    m_qrc_combo->setEditable(false);
    m_qrc_combo->setInsertPolicy(QComboBox::NoInsert);
    m_qrc_combo->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    connect(m_qrc_combo, SIGNAL(activated(int)), this, SLOT(setCurrentIndex(int)));
    layout2->addWidget(m_qrc_combo);

    m_qrc_stack = new QStackedWidget(this);
    layout1->addWidget(m_qrc_stack);

    QHBoxLayout *layout4 = new QHBoxLayout;
    layout1->addLayout(layout4);
    layout4->addWidget(new QLabel(tr("Prefix:"), this));
    m_prefix_edit = new QLineEdit(this);
    connect(m_prefix_edit, SIGNAL(textChanged(const QString&)),
            this, SLOT(setCurrentPrefix(const QString&)));
    layout4->addWidget(m_prefix_edit);
    
    QHBoxLayout *layout3 = new QHBoxLayout;
    layout1->addLayout(layout3);
    layout3->addStretch();
    m_add_prefix_button = new QPushButton(tr("Add prefix"), this);
    connect(m_add_prefix_button, SIGNAL(clicked()), this, SLOT(addPrefix()));
    m_add_files_button = new QPushButton(tr("Add files"), this);
    connect(m_add_files_button, SIGNAL(clicked()), this, SLOT(addFiles()));
    m_delete_button = new QPushButton(tr("Delete"), this);
    connect(m_delete_button, SIGNAL(clicked()), this, SLOT(deleteItem()));
    layout3->addWidget(m_add_prefix_button);
    layout3->addWidget(m_add_files_button);
    layout3->addWidget(m_delete_button);

    updateUi();
}

int FormTab::qrcCount()
{
    return m_qrc_stack->count();
}

QTreeView *FormTab::currentView() const
{
    return qobject_cast<QTreeView*>(m_qrc_stack->currentWidget());
}

ResourceModel *FormTab::currentModel() const
{
    QTreeView *view = currentView();
    if (view == 0)
        return 0;
    return qobject_cast<ResourceModel*>(view->model());
}

bool FormTab::dirty() const
{
    ResourceModel *model = currentModel();
    if (model == 0)
        return false;
    
    return model->dirty();
}

void FormTab::getCurrentItem(QString &prefix, QString &file)
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

void FormTab::addPrefix()
{
    QTreeView *view = currentView();
    if (view == 0)
        return;
    
    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    QModelIndex idx = model->addNewPrefix();
    view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
}

void FormTab::setCurrentPrefix(const QString &prefix)
{
    QTreeView *view = currentView();
    if (view == 0)
        return;
    
    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    model->changePrefix(view->currentIndex(), prefix);
}

void FormTab::addFiles()
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
}

void FormTab::deleteItem()
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
        
    QModelIndex idx = view->indexBelow(cur_idx);
    while (idx.isValid() && model->parent(idx) == cur_idx)
        idx = view->indexBelow(idx);
    if (!idx.isValid())
        idx = view->indexAbove(idx);
    
    model->deleteItem(cur_idx);        

    if (idx.isValid())
        view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
}

void FormTab::updateUi()
{
    QString prefix, file;
    getCurrentItem(prefix, file);

    m_add_prefix_button->setEnabled(true);
    m_add_files_button->setEnabled(!prefix.isEmpty());
    m_delete_button->setEnabled(!prefix.isEmpty());
    m_prefix_edit->setEnabled(!prefix.isEmpty());
    
    m_prefix_edit->blockSignals(true);
    m_prefix_edit->setText(prefix);
    m_prefix_edit->blockSignals(false);
}

void FormTab::setCurrentIndex(int i)
{
    if (i > qrcCount())
        return;

    ResourceModel *old_model = currentModel();
    if (old_model != 0)
        disconnect(old_model, SIGNAL(dirtyChanged(bool)), this, SIGNAL(dirtyChanged(bool)));
    bool old_dirty = dirty();

    m_qrc_combo->blockSignals(true);
    m_qrc_combo->setCurrentIndex(i);
    m_qrc_combo->blockSignals(false);
    m_qrc_stack->setCurrentIndex(i);

    ResourceModel *new_model = currentModel();
    if (new_model != 0)
        connect(new_model, SIGNAL(dirtyChanged(bool)), this, SIGNAL(dirtyChanged(bool)));
    bool new_dirty = dirty();

    if (old_dirty != new_dirty)
        emit dirtyChanged(new_dirty);
}

void FormTab::updateQrcList()
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

    updateUi();
}

void FormTab::addView(const QString &qrc_file)
{
    int idx = qrcCount();

    QString name;
    if (qrc_file.isEmpty())
        name = tr("Untitled");
    else
        name = m_form->relativePath(qrc_file);
    m_qrc_combo->addItem(name);
    QTreeView *view = new QTreeView;
    view->setModel(m_editor->model(qrc_file));
    view->header()->hide();
    m_qrc_stack->addWidget(view);
    connect(view->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(updateUi()));

    setCurrentIndex(idx);

    if (!qrc_file.isEmpty())
        m_form->addResourceFile(qrc_file);
}

void FormTab::saveCurrentView()
{
    ResourceModel *model = currentModel();
    if (model == 0)
        return;
    
    model->save();
}

int FormTab::indexOfView(QTreeView *view)
{
    for (int i = 0; i < m_qrc_stack->count(); ++i) {
        if (view == m_qrc_stack->widget(i))
            return i;
    }
    return -1;
}

void FormTab::removeCurrentView()
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

    if (!file_name.isEmpty())
        m_form->removeResourceFile(file_name);

    if (idx < qrcCount())
        setCurrentIndex(idx);
    else if (idx > 0)
        setCurrentIndex(idx - 1);
}

void FormTab::reloadCurrentView()
{
    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    model->reload();
}

/******************************************************************************
** ResourceEditor
*/

ResourceEditor::ResourceEditor(AbstractFormEditor *core, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    layout->addWidget(m_tabs);

    m_new_qrc_action = new QAction(tr("New"), this);
    m_open_qrc_action = new QAction(tr("Open"), this);
    m_save_qrc_action = new QAction(tr("Save"), this);
    m_remove_qrc_action = new QAction(tr("Remove"), this);
    m_reload_qrc_action = new QAction(tr("Reload"), this);

    connect(m_new_qrc_action, SIGNAL(triggered()), this, SLOT(newQrc()));
    connect(m_open_qrc_action, SIGNAL(triggered()), this, SLOT(openQrc()));
    connect(m_remove_qrc_action, SIGNAL(triggered()), this, SLOT(removeQrc()));
    connect(m_save_qrc_action, SIGNAL(triggered()), this, SLOT(saveQrc()));
    connect(m_reload_qrc_action, SIGNAL(triggered()), this, SLOT(reloadQrc()));

    connect(core->formWindowManager(), SIGNAL(formWindowAdded(AbstractFormWindow*)),
            this, SLOT(addTab(AbstractFormWindow*)));
    connect(core->formWindowManager(), SIGNAL(formWindowRemoved(AbstractFormWindow*)),
            this, SLOT(removeTab(AbstractFormWindow*)));
}

static QString tabName(const QString &form_name)
{
    if (form_name.isEmpty())
        return QObject::tr("Unnamed");
    return QFileInfo(form_name).fileName();
}

void ResourceEditor::addTab(AbstractFormWindow *form)
{
    m_tabs->addTab(new FormTab(form, this), tabName(form->fileName()));
    connect(form, SIGNAL(fileNameChanged(const QString&)),
            this, SLOT(formNameChanged(const QString&)));
    m_tabs->setCurrentIndex(m_tabs->count() - 1);
}

int ResourceEditor::indexOfForm(AbstractFormWindow *form)
{
    for (int i = 0; i < m_tabs->count(); ++i) {
        FormTab *form_tab = qobject_cast<FormTab*>(m_tabs->widget(i));
        if (form_tab->form() == form)
            return i;
    }
    return -1;
}

void ResourceEditor::removeTab(AbstractFormWindow *form)
{
    int idx = indexOfForm(form);
    if (idx == -1)
        return;
    QWidget *form_tab = m_tabs->widget(idx);
    m_tabs->removeTab(idx);
    delete form_tab;
}

void ResourceEditor::formNameChanged(const QString &name)
{
    AbstractFormWindow *form = qobject_cast<AbstractFormWindow*>(sender());
    if (form == 0)
        return;

    int idx = indexOfForm(form);
    if (idx == -1)
        return;
        
    m_tabs->setTabText(idx, tabName(name));
}

ResourceModel *ResourceEditor::model(const QString &file)
{
    if (file.isEmpty()) {
        ResourceModel *model = new ResourceModel(ResourceFile());
        m_model_list.append(model);
        return model;
    }

    for (int i = 0; i < m_model_list.size(); ++i) {
        ResourceModel *model = m_model_list.at(i);
        if (model->fileName() == file)
            return model;
    }

    ResourceFile rf(file);
    if (!rf.load())
        return 0;
        
    ResourceModel *model = new ResourceModel(rf);
    m_model_list.append(model);
    return model;
}

FormTab *ResourceEditor::currentFormTab() const
{
    return qobject_cast<FormTab*>(m_tabs->currentWidget());
}

void ResourceEditor::newQrc()
{
    FormTab *tab = currentFormTab();
    if (tab == 0)
        return;

    tab->addView(QString());
}

void ResourceEditor::openQrc()
{
    FormTab *tab = currentFormTab();
    if (tab == 0)
        return;

    QString file_name = QFileDialog::getOpenFileName(this, tr("Open resource file"),
                                                        tab->form()->absolutePath(QString()),
                                                        tr("Resource files (*.qrc)"));
    if (file_name.isEmpty())
        return;

    tab->addView(file_name);
}

void ResourceEditor::saveQrc()
{
    FormTab *tab = currentFormTab();
    if (tab == 0)
        return;

    tab->saveCurrentView();
}

void ResourceEditor::removeQrc()
{
    FormTab *tab = currentFormTab();
    if (tab == 0)
        return;

    tab->removeCurrentView();
}

void ResourceEditor::reloadQrc()
{
    FormTab *tab = currentFormTab();
    if (tab == 0)
        return;

    tab->reloadCurrentView();
}

#include "resourceeditor.moc"
