// Prototype preference dialog

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include "preferencedialog.h"

#include "designerpreferences.h"  // Someday we'll need to load most of these dynamically.

PreferenceDialog::PreferenceDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Qt Designer Preferences"));
    PreferenceInterface *iface = new DesignerPreferences(this);
    m_preferences.append(iface);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *layout = new QHBoxLayout();
    mainLayout->addLayout(layout);
    m_stack = new QStackedWidget(this);
    QTreeWidget *treeWidget = new QTreeWidget(this);
    treeWidget->setColumnCount(1);
    treeWidget->header()->setResizeMode(QHeaderView::Stretch);
    treeWidget->header()->hide();
    QTreeWidgetItem *root = new QTreeWidgetItem(treeWidget);
    root->setText(0, tr("Standard Preferences"));
    layout->addWidget(treeWidget);
    layout->addWidget(m_stack);
    connect(treeWidget, SIGNAL(currentChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(changePane(QTreeWidgetItem *)));
    QTreeWidgetItem *item;
    for (int listIndex = 0; listIndex < m_preferences.size(); ++listIndex) {
        iface = m_preferences.at(listIndex);
        int index = m_stack->addWidget(iface->createPreferenceWidget(0));
        item = new QTreeWidgetItem(root);
        item->setText(0, iface->preferenceName());
        QIcon icon = iface->preferenceIcon();
        if (!icon.isNull())
            item->setIcon(0, icon);
        item->setText(1, QString::number(index));
        item->setText(2, QString::number(listIndex));
    }
    // add in all the dynamic things into the list here
    // ...

    // ### Read in the preferences of the last item that was selected.
    item = root->child(0);
    if (item) {
        treeWidget->setSelected(item, true);
        treeWidget->setCurrentItem(item);
        treeWidget->openItem(root);
    }
    layout = new QHBoxLayout();
    mainLayout->addLayout(layout);
    layout->addStretch();
    QPushButton *btn = new QPushButton(tr("OK"), this);
    btn->setDefault(true);
    connect(btn, SIGNAL(clicked()), this, SLOT(accept()));
    layout->addWidget(btn);
    btn = new QPushButton(tr("Cancel"), this);
    connect(btn, SIGNAL(clicked()), this, SLOT(reject()));
    layout->addWidget(btn);
}

PreferenceDialog::~PreferenceDialog()
{
    //qDeleteAll(m_preferences);

    // ### Save Preferences of last item viewed.
}

void PreferenceDialog::changePane(QTreeWidgetItem *item)
{
    if (!item || !item->parent())
        return;

    m_stack->setCurrentIndex(item->text(1).toInt());
    PreferenceInterface *iface = m_preferences.at(item->text(2).toInt());
    if (!iface->settingsChanged())
        iface->readSettings();
}

void PreferenceDialog::reject()
{
    QDialog::reject();
}

void PreferenceDialog::accept()
{
    foreach (PreferenceInterface *iface, m_preferences) {
        if (iface->settingsChanged())
            iface->saveSettings();
    }
    QDialog::accept();
}
