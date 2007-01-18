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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef PROMOTIONEDITORDIALOG_H
#define PROMOTIONEDITORDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QGroupBox>


class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QDesignerPromotionInterface;
class QDesignerWidgetDataBaseItemInterface;

class QTreeView;
class QPushButton;
class QItemSelection;
class QDialogButtonBox;
class QComboBox;
class QLineEdit;
class QCheckBox;

namespace qdesigner_internal {
    struct PromotionParameters;
    class PromotionModel;


    // Panel for adding a new promoted class. Separate class for code cleanliness.
    class NewPromotedClassPanel : public QGroupBox {
        Q_OBJECT
    public:
        NewPromotedClassPanel(const QStringList &baseClasses,
                              int selectedBaseClass = -1,
                               QWidget *parent = 0);

        signals:
        void newPromotedClass(const PromotionParameters &, bool *ok);

    public slots:
        void grabFocus();
        void chooseBaseClass(const QString &);
    private slots:
        void slotNameChanged(const QString &);
        void slotIncludeFileChanged(const QString &);
        void slotAdd();
        void slotReset();

    private:
        PromotionParameters promotionParameters() const;
        void enableButtons();

        QComboBox *m_baseClassCombo;
        QLineEdit *m_classNameEdit;
        QLineEdit *m_includeFileEdit;
        QCheckBox *m_globalIncludeCheckBox;
        QPushButton *m_addButton;
    };

    // Dialog for editing promoted classes.
    class QDesignerPromotionDialog : public QDialog {
        Q_OBJECT

    public:
        enum Mode { ModeEdit, ModeEditChooseClass };

        QDesignerPromotionDialog(QDesignerFormEditorInterface *core,
                                 QWidget *parent = 0,
                                 const QString &promotableWidgetClassName = QString(),
                                 QString *promoteTo = 0);
        // Return an alphabetically ordered list of base class names for adding new classes.
        static const QStringList &baseClassNames(const QDesignerPromotionInterface *promotion);

        signals:
        void selectedBaseClassChanged(const QString &);
    private slots:
        void slotRemove();
        void slotAcceptPromoteTo();
        void slotSelectionChanged(const QItemSelection &, const QItemSelection &);
        void slotNewPromotedClass(const PromotionParameters &, bool *ok);

        void slotIncludeFileChanged(QDesignerWidgetDataBaseItemInterface *, const QString &includeFile);
        void slotClassNameChanged(QDesignerWidgetDataBaseItemInterface *, const QString &newName);
        void slotUpdateFromWidgetDatabase();
    private:
        QDialogButtonBox *createButtonBox();
        void delayedUpdateFromWidgetDatabase();
        // Return item at model index and a combination of flags or 0.
        enum { Referenced = 1, CanPromote = 2 };
        QDesignerWidgetDataBaseItemInterface *databaseItemAt(const QItemSelection &, unsigned &flags) const;
        void displayError(const QString &message);

        const Mode m_mode;
        const QString m_promotableWidgetClassName;
        QString *m_promoteTo;
        QDesignerPromotionInterface *m_promotion;
        PromotionModel *m_model;
        QTreeView *m_treeView;
        QDialogButtonBox *m_buttonBox;
        QPushButton *m_removeButton;
        QString m_lastSelectedBaseClass;
    };
} // namespace qdesigner_internal

#endif // PROMOTIONEDITORDIALOG_H
