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

#include "preferencesdialog.h"
#include "preferences.h"
#include "fontpanel.h"
#include <iconloader_p.h>
#include <gridpanel_p.h>

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QSpacerItem>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QPushButton>
#include <QtGui/QFileDialog>

#include <qdebug.h>

PreferencesDialog::PreferencesDialog(QWidget *parentWidget) :
    QDialog(parentWidget),
    m_uiModeCombo(new QComboBox),
    m_fontPanel(new FontPanel),
    m_gridPanel(new qdesigner_internal::GridPanel),
    m_templatePathListWidget(new QListWidget),
    m_removeTemplatePathButton(new QPushButton(qdesigner_internal::createIconSet(QString::fromUtf8("minus.png")), QString())),
    m_dialogButtonBox(new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Close))
{
    setModal(true);
    setWindowTitle(tr("Preferences"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *mainVLayout = new QVBoxLayout(this);
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    mainVLayout->addLayout(hboxLayout);
    QVBoxLayout *leftColumnLayout = new QVBoxLayout;
    hboxLayout->addLayout(leftColumnLayout);

    leftColumnLayout->addWidget(initUIModeControls());

    m_fontPanel->setCheckable(true);
    m_fontPanel->setTitle(tr("Toolwindow Font"));
    leftColumnLayout->addWidget(m_fontPanel);
    m_gridPanel->setTitle(tr("Default Grid"));
    leftColumnLayout->addWidget(m_gridPanel);
    leftColumnLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));

    QVBoxLayout *rightColumnLayout = new QVBoxLayout;
    hboxLayout->addLayout(rightColumnLayout);
    rightColumnLayout->addWidget(initTemplatePathControls());

    connect(m_dialogButtonBox , SIGNAL(rejected()), this, SLOT(reject()));
    connect(m_dialogButtonBox , SIGNAL(accepted()), this, SLOT(accept()));
    mainVLayout->addWidget(m_dialogButtonBox);
}

QWidget *PreferencesDialog::initUIModeControls()
{
    // UI mode
    QGroupBox *uiModeGroup = new QGroupBox();
    QHBoxLayout *hboxLayout = new QHBoxLayout(uiModeGroup);
    uiModeGroup->setTitle(tr("User Interface Mode"));
    m_uiModeCombo->addItem(tr("Docked Window"), QVariant(DockedMode));
    m_uiModeCombo->addItem(tr("Multiple Top-Level Windows"), QVariant(TopLevelMode));
    m_uiModeCombo->setEditable(false);
    hboxLayout->addWidget(m_uiModeCombo);
    hboxLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
    return uiModeGroup;
}

QWidget *PreferencesDialog::initTemplatePathControls()
{
    // Template paths
    QGroupBox *templatePathModeGroup = new QGroupBox();
    templatePathModeGroup->setTitle(tr("Additional Template Paths"));
    QVBoxLayout *templatePathVLayout = new QVBoxLayout(templatePathModeGroup);
    m_templatePathListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_templatePathListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(templatePathSelectionChanged()));
    templatePathVLayout->addWidget(m_templatePathListWidget);
    // Template paths buttons
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    QPushButton *addTemplatePathButton = new QPushButton( qdesigner_internal::createIconSet(QString::fromUtf8("plus.png")), QString());
    connect(addTemplatePathButton, SIGNAL(clicked()), this, SLOT(addTemplatePath()));
    hboxLayout->addWidget(addTemplatePathButton);
    connect(m_removeTemplatePathButton, SIGNAL(clicked()), this, SLOT(removeTemplatePath()));
    hboxLayout->addWidget(m_removeTemplatePathButton);
    hboxLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding,  QSizePolicy::Ignored));
    templatePathVLayout->addLayout(hboxLayout);
    return templatePathModeGroup;
}

bool PreferencesDialog::showDialog(Preferences &p)
{
    setPreferences(p);
    if (exec() != Accepted)
        return false;

    getPreferences(p);
    return true;
}

void PreferencesDialog::setPreferences(const Preferences &p)
{
    m_uiModeCombo->setCurrentIndex(m_uiModeCombo->findData(QVariant(p.m_uiMode)));
    m_fontPanel-> setWritingSystem(p.m_writingSystem);
    m_fontPanel->setSelectedFont(p.m_font);
    m_fontPanel->setChecked(p.m_useFont);
    // add paths and select 0
    m_templatePathListWidget->clear();
    if (p.m_additionalTemplatePaths.empty()) {
        // disable button
        templatePathSelectionChanged();
    } else {
        foreach (QString templatePath, p.m_additionalTemplatePaths)
            m_templatePathListWidget->addItem(templatePath);
        m_templatePathListWidget->setCurrentItem(m_templatePathListWidget->item(0));
    }
    m_gridPanel->setGrid(p.m_defaultGrid);
}

void PreferencesDialog::getPreferences(Preferences &p) const
{
    p.m_uiMode = static_cast<UIMode>(m_uiModeCombo->itemData(m_uiModeCombo->currentIndex()).toInt());
    p.m_font = m_fontPanel->selectedFont();
    p.m_useFont = m_fontPanel->isChecked();
    p.m_writingSystem =  m_fontPanel->writingSystem();

    p.m_additionalTemplatePaths.clear();
    const int count = m_templatePathListWidget->count();
    for (int i = 0; i <  count; i++) {
        p.m_additionalTemplatePaths += m_templatePathListWidget->item(i)->text();
    }
    p.m_defaultGrid = m_gridPanel->grid();
}

void PreferencesDialog::addTemplatePath()
{
    const QString templatePath = chooseTemplatePath(this);
    if (templatePath.isEmpty())
        return;

    const QList<QListWidgetItem *> existing = m_templatePathListWidget->findItems(templatePath, Qt::MatchExactly);
    if (!existing.empty())
        return;

    QListWidgetItem *newItem = new QListWidgetItem(templatePath);
    m_templatePathListWidget->addItem(newItem);
    m_templatePathListWidget->setCurrentItem(newItem);
}


void PreferencesDialog::removeTemplatePath()
{
    const QList<QListWidgetItem *> selectedPaths = m_templatePathListWidget->selectedItems();
    if (selectedPaths.empty())
        return;
    delete selectedPaths.front();
}

void PreferencesDialog::templatePathSelectionChanged()
{
    const QList<QListWidgetItem *> selectedPaths = m_templatePathListWidget->selectedItems();
    m_removeTemplatePathButton->setEnabled(!selectedPaths.empty());
}

QString PreferencesDialog::chooseTemplatePath(QWidget *parent)
{
    QString rc = QFileDialog::getExistingDirectory(parent, tr("Pick a directory to save templates in"));
    if (rc.isEmpty())
        return rc;

    if (rc.endsWith(QDir::separator()))
        rc.remove(rc.size() - 1, 1);
    return rc;
}
