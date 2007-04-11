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

/*
TRANSLATOR qdesigner_internal::QDesignerPromotionDialog
*/

#include "qdesigner_promotiondialog_p.h"
#include "promotionmodel_p.h"
#include "iconloader_p.h"
#include "widgetdatabase_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerPromotionInterface>
#include <QtDesigner/QDesignerWidgetDataBaseItemInterface>

#include <QtCore/QTimer>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QTreeView>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QItemSelection>
#include <QtGui/QMessageBox>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QCheckBox>
#include <QtGui/QRegExpValidator>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>


// Add a row consisting of widget and a description label to a grid.
static void addGridRow(const QString &description, QGridLayout *gridLayout, QWidget *w, int &row) {
    QLabel *label = new QLabel(description);
    gridLayout->addWidget(label, row, 0);
    gridLayout->addWidget(w, row, 1);
    ++row;
}

namespace qdesigner_internal {
    // PromotionParameters
    struct PromotionParameters {
        QString m_baseClass;
        QString m_className;
        QString m_includeFile;
    };

    //  NewPromotedClassPanel
    NewPromotedClassPanel::NewPromotedClassPanel(const QStringList &baseClasses,
                                                   int selectedBaseClass,
                                                   QWidget *parent) :
        QGroupBox(parent),
        m_baseClassCombo(new  QComboBox),
        m_classNameEdit(new QLineEdit),
        m_includeFileEdit(new QLineEdit),
        m_globalIncludeCheckBox(new QCheckBox),
        m_addButton(new QPushButton(tr("Add")))
    {
        setTitle(tr("New Promoted Class"));
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
        QHBoxLayout *hboxLayout = new QHBoxLayout(this);

        m_classNameEdit->setValidator(new QRegExpValidator(QRegExp(QLatin1String("[_a-zA-Z:][:_a-zA-Z0-9]*")), m_classNameEdit));
        connect(m_classNameEdit,   SIGNAL(textChanged(QString)), this, SLOT(slotNameChanged(QString)));
        connect(m_includeFileEdit, SIGNAL(textChanged(QString)), this, SLOT(slotIncludeFileChanged(QString)));

        m_baseClassCombo->setEditable(false);
        m_baseClassCombo->addItems(baseClasses);
        if (selectedBaseClass != -1)
            m_baseClassCombo->setCurrentIndex(selectedBaseClass);

        // Grid
        QGridLayout *gridLayout = new QGridLayout();
        int row = 0;
        addGridRow(tr("Base class name:"),     gridLayout, m_baseClassCombo, row);
        addGridRow(tr("Promoted class name:"), gridLayout, m_classNameEdit, row);
        addGridRow(tr("Header file:"),         gridLayout, m_includeFileEdit, row);
        addGridRow(tr("Global include"),       gridLayout, m_globalIncludeCheckBox, row);
        hboxLayout->addLayout(gridLayout);
        hboxLayout->addItem(new QSpacerItem(15, 0, QSizePolicy::Fixed, QSizePolicy::Ignored));
        // Button box
        QVBoxLayout *buttonLayout = new QVBoxLayout();

        m_addButton->setAutoDefault(false);
        connect(m_addButton, SIGNAL(clicked()), this, SLOT(slotAdd()));
        m_addButton->setEnabled(false);
        buttonLayout->addWidget(m_addButton);

        QPushButton *resetButton = new QPushButton(tr("Reset"));
        resetButton->setAutoDefault(false);
        connect(resetButton, SIGNAL(clicked()), this, SLOT(slotReset()));

        buttonLayout->addWidget(resetButton);
        buttonLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));
        hboxLayout->addLayout(buttonLayout);

        enableButtons();
    }

    void NewPromotedClassPanel::slotAdd() {
        bool ok = false;
        emit newPromotedClass(promotionParameters(), &ok);
        if (ok)
            slotReset();
    }

    void NewPromotedClassPanel::slotReset() {
        const QString empty;
        m_classNameEdit->setText(empty);
        m_includeFileEdit->setText(empty);
        m_globalIncludeCheckBox->setCheckState(Qt::Unchecked);
    }

    void NewPromotedClassPanel::grabFocus() {
        m_classNameEdit->setFocus(Qt::OtherFocusReason);
    }

    void NewPromotedClassPanel::slotNameChanged(const QString &className) {
        // Suggest a name
        if (!className.isEmpty()) {
            QString suggestedHeader = className.toLower().replace(QLatin1String("::"), QString(QLatin1Char('_')));
            suggestedHeader += QLatin1String(".h");

            const bool blocked = m_includeFileEdit->blockSignals(true);
            m_includeFileEdit->setText(suggestedHeader);
            m_includeFileEdit->blockSignals(blocked);
        }
        enableButtons();
    }

    void NewPromotedClassPanel::slotIncludeFileChanged(const QString &){
        enableButtons();
    }

    void NewPromotedClassPanel::enableButtons() {
        const bool enabled = !m_classNameEdit->text().isEmpty() && !m_includeFileEdit->text().isEmpty();
        m_addButton->setEnabled(enabled);
        m_addButton->setDefault(enabled);
    }

    PromotionParameters NewPromotedClassPanel::promotionParameters() const {
         PromotionParameters rc;
         rc.m_baseClass = m_baseClassCombo->currentText();
         rc.m_className = m_classNameEdit->text();
         rc.m_includeFile = buildIncludeFile(m_includeFileEdit->text(),
                                             m_globalIncludeCheckBox->checkState() == Qt::Checked ? IncludeGlobal : IncludeLocal);
         return rc;
     }

    void NewPromotedClassPanel::chooseBaseClass(const QString &baseClass) {
        const int index = m_baseClassCombo->findText (baseClass);
        if (index != -1)
            m_baseClassCombo->setCurrentIndex (index);
    }

    // QDesignerPromotionDialog
    QDesignerPromotionDialog::QDesignerPromotionDialog(QDesignerFormEditorInterface *core,
                                                       QWidget *parent,
                                                       const QString &promotableWidgetClassName,
                                                       QString *promoteTo) :
        QDialog(parent),
        m_mode(promotableWidgetClassName.isEmpty() || promoteTo == 0 ? ModeEdit : ModeEditChooseClass),
        m_promotableWidgetClassName(promotableWidgetClassName),
        m_promoteTo(promoteTo),
        m_promotion(core->promotion()),
        m_model(new PromotionModel(core)),
        m_treeView(new QTreeView),
        m_buttonBox(0),
        m_removeButton(new QPushButton(createIconSet(QString::fromUtf8("minus.png")), QString()))
    {
        m_buttonBox = createButtonBox();
        setModal(true);
        setWindowTitle(tr("Promoted Widgets"));
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        QVBoxLayout *vboxLayout = new QVBoxLayout(this);

        // tree view group
        QGroupBox *treeViewGroup = new QGroupBox();
        treeViewGroup->setTitle(tr("Promoted Classes"));
        QVBoxLayout *treeViewVBoxLayout = new QVBoxLayout(treeViewGroup);
        // tree view
        m_treeView->setModel (m_model);
        m_treeView->setMinimumWidth(450);

        connect(m_treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                this, SLOT(slotSelectionChanged(QItemSelection, QItemSelection)));

        QHeaderView *headerView = m_treeView->header();
        headerView->setResizeMode(QHeaderView::ResizeToContents);
        treeViewVBoxLayout->addWidget(m_treeView);
        // remove button
        QHBoxLayout *hboxLayout = new QHBoxLayout();
        hboxLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));

        m_removeButton->setAutoDefault(false);
        connect(m_removeButton, SIGNAL(clicked()), this, SLOT(slotRemove()));
        m_removeButton->setEnabled(false);
        hboxLayout->addWidget(m_removeButton);
        treeViewVBoxLayout->addLayout(hboxLayout);
        vboxLayout->addWidget(treeViewGroup);
        // Create new panel: Try to be smart and preselect a base class. Default to QFrame
        const QStringList &baseClassNameList = baseClassNames(m_promotion);
        int preselectedBaseClass = -1;
        if (m_mode == ModeEditChooseClass) {
            preselectedBaseClass = baseClassNameList.indexOf(m_promotableWidgetClassName);
        }
        if (preselectedBaseClass == -1)
            preselectedBaseClass = baseClassNameList.indexOf(QLatin1String("QFrame"));

        NewPromotedClassPanel *newPromotedClassPanel = new NewPromotedClassPanel(baseClassNameList, preselectedBaseClass);
        connect(newPromotedClassPanel, SIGNAL(newPromotedClass(PromotionParameters, bool *)), this, SLOT(slotNewPromotedClass(PromotionParameters, bool *)));
        connect(this, SIGNAL(selectedBaseClassChanged(QString)),
                newPromotedClassPanel, SLOT(chooseBaseClass(QString)));
        vboxLayout->addWidget(newPromotedClassPanel);
        // button box
        vboxLayout->addWidget(m_buttonBox);
        // connect model
        connect(m_model, SIGNAL(includeFileChanged(QDesignerWidgetDataBaseItemInterface*, QString)),
                this, SLOT(slotIncludeFileChanged(QDesignerWidgetDataBaseItemInterface*, QString)));

        connect(m_model, SIGNAL(classNameChanged(QDesignerWidgetDataBaseItemInterface*, QString)),
                this, SLOT(slotClassNameChanged(QDesignerWidgetDataBaseItemInterface*, QString)));

        // focus
        if (m_mode == ModeEditChooseClass)
            newPromotedClassPanel->grabFocus();

        slotUpdateFromWidgetDatabase();
    }

    QDialogButtonBox *QDesignerPromotionDialog::createButtonBox() {
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Close);

        connect(buttonBox , SIGNAL(accepted()), this, SLOT(slotAcceptPromoteTo()));
        buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Promote"));
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

        connect(buttonBox , SIGNAL(rejected()), this, SLOT(reject()));
        return buttonBox;
    }

    void QDesignerPromotionDialog::slotUpdateFromWidgetDatabase() {
        m_model->updateFromWidgetDatabase();
        m_treeView->expandAll();
        m_removeButton->setEnabled(false);
    }

    void QDesignerPromotionDialog::delayedUpdateFromWidgetDatabase() {
        QTimer::singleShot(0, this, SLOT(slotUpdateFromWidgetDatabase()));
    }

    const QStringList &QDesignerPromotionDialog::baseClassNames(const QDesignerPromotionInterface *promotion) {
        typedef QList<QDesignerWidgetDataBaseItemInterface *> WidgetDataBaseItemList;
        static QStringList rc;
        if (rc.empty()) {
            // Convert the item list into a string list.
            const WidgetDataBaseItemList dbItems =  promotion->promotionBaseClasses();
            const WidgetDataBaseItemList::const_iterator cend =  dbItems.constEnd();
            for (WidgetDataBaseItemList::const_iterator it = dbItems.constBegin() ; it != cend; ++it) {
                rc.push_back( (*it)->name());
            }
        }
        return rc;
    }

    void  QDesignerPromotionDialog::slotAcceptPromoteTo() {
        Q_ASSERT(m_mode == ModeEditChooseClass);
        unsigned flags;
        // Ok pressed: Promote to selected class
        if (QDesignerWidgetDataBaseItemInterface *dbItem = databaseItemAt(m_treeView->selectionModel()->selection(), flags)) {
            if (flags & CanPromote) {
                *m_promoteTo = dbItem ->name();
                accept();
            }
        }
    }

    void QDesignerPromotionDialog::slotRemove() {
        unsigned flags;
        QDesignerWidgetDataBaseItemInterface *dbItem = databaseItemAt(m_treeView->selectionModel()->selection(), flags);
        if (!dbItem || (flags & Referenced))
            return;

        QString errorMessage;
        if (m_promotion->removePromotedClass(dbItem->name(), &errorMessage)) {
            slotUpdateFromWidgetDatabase();
        } else {
            displayError(errorMessage);
        }
    }

    void QDesignerPromotionDialog::slotSelectionChanged(const QItemSelection &selected, const QItemSelection &) {
        // Enable deleting non-referenced items
        unsigned flags;
        const QDesignerWidgetDataBaseItemInterface *dbItem = databaseItemAt(selected, flags);
        m_removeButton->setEnabled(dbItem && !(flags & Referenced));
        // In choose mode, can we promote to the class?
        if (m_mode == ModeEditChooseClass) {
            const bool enablePromoted = flags & CanPromote;
            m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enablePromoted);
            m_buttonBox->button(QDialogButtonBox::Ok)->setDefault(enablePromoted);
        }
        // different base?
        if (dbItem) {
            const QString baseClass = dbItem->extends();
            if (baseClass !=  m_lastSelectedBaseClass) {
                m_lastSelectedBaseClass = baseClass;
                emit selectedBaseClassChanged(m_lastSelectedBaseClass);
            }
        }
    }

    QDesignerWidgetDataBaseItemInterface *QDesignerPromotionDialog::databaseItemAt(const QItemSelection &selected, unsigned &flags) const {
        flags = 0;
        const QModelIndexList indexes = selected.indexes();
        if (indexes.empty())
            return 0;

        bool referenced;
        QDesignerWidgetDataBaseItemInterface *dbItem = m_model->databaseItemAt(indexes.front(), &referenced);

        if (dbItem) {
            if (referenced)
                flags |= Referenced;
            // In choose mode, can we promote to the class?
            if (m_mode == ModeEditChooseClass &&  dbItem && dbItem->isPromoted() && dbItem->extends() ==  m_promotableWidgetClassName)
                flags |= CanPromote;

        }
        return dbItem;
    }

    void QDesignerPromotionDialog::slotNewPromotedClass(const PromotionParameters &p, bool *ok) {
        QString  errorMessage;
        *ok = m_promotion->addPromotedClass(p.m_baseClass, p.m_className, p.m_includeFile, &errorMessage);
        if (*ok) {
            // update and select
            slotUpdateFromWidgetDatabase();
            const QModelIndex newClassIndex = m_model->indexOfClass(p.m_className);
            if (newClassIndex.isValid()) {
                m_treeView->selectionModel()->select(newClassIndex, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
            }
        } else {
            displayError(errorMessage);
        }
    }

    void QDesignerPromotionDialog::slotIncludeFileChanged(QDesignerWidgetDataBaseItemInterface *dbItem, const QString &includeFile) {
        if (includeFile.isEmpty()) {
            delayedUpdateFromWidgetDatabase();
            return;
        }

        if (dbItem->includeFile() == includeFile)
            return;

        QString errorMessage;
        if (!m_promotion->setPromotedClassIncludeFile(dbItem->name(), includeFile, &errorMessage)) {
            displayError(errorMessage);
            delayedUpdateFromWidgetDatabase();
        }
    }

    void  QDesignerPromotionDialog::slotClassNameChanged(QDesignerWidgetDataBaseItemInterface *dbItem, const QString &newName) {
        if (newName.isEmpty()) {
            delayedUpdateFromWidgetDatabase();
            return;
        }
        const QString oldName = dbItem->name();
        if (newName == oldName)
            return;

        QString errorMessage;
        if (!m_promotion->changePromotedClassName(oldName , newName, &errorMessage)) {
            displayError(errorMessage);
            delayedUpdateFromWidgetDatabase();
        }
    }

    void QDesignerPromotionDialog::displayError(const QString &message) {
        QMessageBox::critical(this, tr("%1 - Error").arg(windowTitle()), message,  QMessageBox::Close);
    }
} // namespace qdesigner_internal
