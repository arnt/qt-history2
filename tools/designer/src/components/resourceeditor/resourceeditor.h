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

#ifndef RESOURCEEDITOR_H
#define RESOURCEEDITOR_H

#include <QtGui/QWidget>

#include "resourceeditor_global.h"

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class ResourceModel;
class QPushButton;
class QToolButton;
class QLineEdit;
class QComboBox;
class QStackedWidget;
class QString;
class QTreeView;

namespace qdesigner_internal {

class QT_RESOURCEEDITOR_EXPORT ResourceEditor : public QWidget
{
    Q_OBJECT

public:
    ResourceEditor(QDesignerFormEditorInterface *core, QWidget *parent = 0);

    QDesignerFormWindowInterface *form() const { return m_form; }
    int qrcCount() const;

public slots:
    void saveCurrentView();
    void removeCurrentView();
    void reloadCurrentView();
    void newView();
    void openView();

    void setActiveForm(QDesignerFormWindowInterface *form);

private slots:
    void updateQrcPaths();
    void updateQrcStack();
    void updateUi();
    void addPrefix();
    void addFiles();
    void deleteItem();
    void setCurrentIndex(int i);
    void addView(const QString &file_name);

private:
    QDesignerFormWindowInterface *m_form;

    void getCurrentItem(QString &prefix, QString &file);
    QTreeView *currentView() const;
    ResourceModel *currentModel() const;
    QTreeView *view(int i) const;
    ResourceModel *model(int i) const;
    int indexOfView(QTreeView *view);
    QString qrcName(const QString &path) const;
    int currentIndex() const;

    void insertEmptyComboItem();
    void removeEmptyComboItem();

private:
    QComboBox *m_qrc_combo;
    QStackedWidget *m_qrc_stack;
    QToolButton *m_add_button;
    QToolButton *m_remove_button;
    QPushButton *m_add_files_button;
    QToolButton *m_remove_qrc_button;
};

} // namespace qdesigner_internal

#endif // RESOURCEEDITOR_H
