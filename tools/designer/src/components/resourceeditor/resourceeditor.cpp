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

#include <abstractformeditor.h>
#include <abstractformwindowmanager.h>

#include <resourcefile.h>
#include <iconloader.h>

#include "resourceeditor.h"

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
    
public slots:
    void updateQrcList();
    void updateUi();
    void addPrefix();
    void addFiles();
    void deleteItem();
    void setCurrentPrefix(const QString &prefix);
    void setCurrentIndex(int i);
    int currentIndex() const;
    
    void addView(const QString &file_name);
    
    void saveCurrentView();
    void removeCurrentView();
    void reloadCurrentView();
    void newView();
    void openView();
    
private:
    QToolButton *m_new_button;
    QToolButton *m_open_button;
    QToolButton *m_save_button;
    QToolButton *m_remove_button;
    QToolButton *m_reload_button;

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

static QToolButton *createToolButton(QWidget *parent, const QString &text,
                                        const QString &icon_path, const char *slot)
{
    QToolButton *result = new QToolButton(parent);
    result->setText(text);
    result->setIcon(createIconSet(icon_path));
    QObject::connect(result, SIGNAL(clicked()), parent, slot);
    return result;
}

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
    
    m_new_button = createToolButton(this, tr("New"), QLatin1String("filenew.png"),
                                    SLOT(newView()));
    m_open_button = createToolButton(this, tr("Open"), QLatin1String("fileopen.png"),
                                    SLOT(openView()));
    m_save_button = createToolButton(this, tr("Save"), QLatin1String("filesave.png"),
                                    SLOT(saveCurrentView()));
    m_remove_button = createToolButton(this, tr("Remove"), QLatin1String("editdelete.png"),
                                    SLOT(removeCurrentView()));
    m_reload_button = /*createToolButton(this, tr("Reload"), QLatin1String("filereload.png"),
                                    SLOT(reloadCurrentView()));*/ 0;
    layout2->addWidget(m_new_button);
    layout2->addWidget(m_open_button);
    layout2->addWidget(m_save_button);
    layout2->addWidget(m_remove_button);
//    layout2->addWidget(m_reload_button);

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
    m_prefix_edit->setFocus();
    updateUi();
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
    updateUi();
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
    updateUi();
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
    
    QModelIndex idx = model->deleteItem(cur_idx);        

    if (idx.isValid()) {
        QModelIndex pref_idx = model->prefixIndex(idx);
        if (pref_idx != idx)
            view->setExpanded(pref_idx, true);
        view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
    }
    updateUi();
}

void FormTab::updateUi()
{
    QString prefix, file;
    getCurrentItem(prefix, file);

    m_add_prefix_button->setEnabled(currentModel() != 0);
    m_add_files_button->setEnabled(!prefix.isEmpty());
    m_delete_button->setEnabled(!prefix.isEmpty());
    m_prefix_edit->setEnabled(!prefix.isEmpty());
    
    m_prefix_edit->blockSignals(true);
    m_prefix_edit->setText(prefix);
    m_prefix_edit->blockSignals(false);

    m_new_button->setEnabled(true);
    m_open_button->setEnabled(true);
    m_save_button->setEnabled(currentModel() != 0 && currentModel()->dirty());
    m_remove_button->setEnabled(currentModel() != 0);
//    m_reload_button->setEnabled(currentModel() != 0);
}

int FormTab::currentIndex() const
{
    return m_qrc_stack->currentIndex();
}

void FormTab::setCurrentIndex(int i)
{
    if (i > qrcCount())
        return;

    m_qrc_combo->blockSignals(true);
    m_qrc_combo->setCurrentIndex(i);
    m_qrc_combo->blockSignals(false);
    m_qrc_stack->setCurrentIndex(i);

    updateUi();
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

    QTreeView *view = new QTreeView;
    ResourceModel *model = m_editor->model(qrc_file);
    if (model == 0)
        return;
    view->setModel(model);
    view->header()->hide();
    QString name;
    if (qrc_file.isEmpty())
        name = tr("Unnamed");
    else
        name = m_form->relativePath(qrc_file);
    m_qrc_combo->addItem(name);
    m_qrc_stack->addWidget(view);
    connect(view->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(updateUi()));
    connect(model, SIGNAL(dirtyChanged(bool)), this, SLOT(updateUi()));
            
    setCurrentIndex(idx);

    if (!qrc_file.isEmpty())
        m_form->addResourceFile(qrc_file);

    updateUi();
}

void FormTab::saveCurrentView()
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
        m_qrc_combo->blockSignals(true);
        m_qrc_combo->setItemText(currentIndex(), s);
        m_qrc_combo->setCurrentIndex(-1);
        m_qrc_combo->setCurrentIndex(currentIndex());
        m_qrc_combo->blockSignals(false);
    }
    
    model->save();
    updateUi();
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

    disconnect(model, SIGNAL(dirtyChanged(bool)), this, SLOT(updateUi()));
    
    if (!file_name.isEmpty())
        m_form->removeResourceFile(file_name);

    if (idx < qrcCount())
        setCurrentIndex(idx);
    else if (idx > 0)
        setCurrentIndex(idx - 1);
    updateUi();
}

void FormTab::reloadCurrentView()
{
    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    model->reload();
    updateUi();
}

void FormTab::newView()
{
    addView(QString());
}

void FormTab::openView()
{
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open resource file"),
                                                        m_form->absolutePath(QString()),
                                                        tr("Resource files (*.qrc)"));
    if (file_name.isEmpty())
        return;

    addView(file_name);
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
    if (!rf.load()) {
        QMessageBox::warning(this, tr("Error opening resource file"),
                                tr("Failed to open \"%1\":\n%2")
                                    .arg(file).arg(rf.errorMessage()),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return 0;
    }
                
    ResourceModel *model = new ResourceModel(rf);
    m_model_list.append(model);
    return model;
}

FormTab *ResourceEditor::currentFormTab() const
{
    return qobject_cast<FormTab*>(m_tabs->currentWidget());
}

#include "resourceeditor.moc"
