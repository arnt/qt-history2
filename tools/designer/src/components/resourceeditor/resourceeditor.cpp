#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>

#include <abstractformeditor.h>
#include <abstractformwindowmanager.h>

#include <resourcefile.h>

#include "resourceeditor.h"

ResourceEditor::ResourceEditor(AbstractFormEditor *core, QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    m_resource_tree->setHeaderLabels(QStringList() << tr("Resources"));
    m_resource_tree->header()->hide();
    
    m_core = core;

    connect(core->formWindowManager(), SIGNAL(activeFormWindowChanged(AbstractFormWindow*)),
                this, SLOT(updateTree(AbstractFormWindow*)));
    connect(m_create_new_button, SIGNAL(clicked()), this, SLOT(createResourceFile()));
    connect(m_add_existing_button, SIGNAL(clicked()), this, SLOT(openResourceFile()));
}

void ResourceEditor::updateTree(AbstractFormWindow *form)
{
    if (form == 0) {
        m_create_new_button->setEnabled(false);
        m_add_existing_button->setEnabled(false);
        return;
    }
    
    m_create_new_button->setEnabled(true);
    m_add_existing_button->setEnabled(true);
    
    m_resource_tree->clear();

    QStringList res_list = form->resourceFiles();
    foreach (QString res, res_list)
        addToTree(res);
}

void ResourceEditor::addToTree(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    ResourceFile resource_file;
    if (!resource_file.load(file))
        return;

    QTreeWidgetItem *ritem = new QTreeWidgetItem(m_resource_tree);
    ritem->setText(0, path);
        
    QStringList prefix_list = resource_file.prefixList();
    foreach (QString prefix, prefix_list) {
        QTreeWidgetItem *pitem = new QTreeWidgetItem(ritem);
        pitem->setText(0, prefix);
        QStringList file_list = resource_file.fileList(prefix);
        foreach (QString f, file_list) {
            QTreeWidgetItem *fitem = new QTreeWidgetItem(pitem);
            fitem->setText(0, f);
        }
    }
}

void ResourceEditor::openResourceFile()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;
    
    QString file_name = QFileDialog::getOpenFileName(0, tr("Open resource file"),
                                                        QString(), tr("Resource files (*.qrc)"));
    if (file_name.isEmpty())
        return;

    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(0, tr("Resource error"),
                                tr("Failed to open \"%1\"\n%2").arg(file_name).arg(file.errorString()),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }
    
    ResourceFile rf;
    if (!rf.load(file)) {
        QMessageBox::warning(0, tr("Resource error"),
                                tr("Failed to parse \"%1\"").arg(file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    form->addResourceFile(file_name);

    updateTree(form);
}

void ResourceEditor::createResourceFile()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;
    
    QString file_name = QFileDialog::getSaveFileName(0, tr("Create resource file"),
                                                        QString(), tr("Resource files (*.qrc)"));
    if (file_name.isEmpty())
        return;

    QFile file(file_name);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(0, tr("Resource error"),
                                tr("Failed to create \"%1\"\n%2").arg(file_name).arg(file.errorString()),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    ResourceFile rf;
    rf.save(file);
    
    form->addResourceFile(file_name);
    updateTree(form);
}
