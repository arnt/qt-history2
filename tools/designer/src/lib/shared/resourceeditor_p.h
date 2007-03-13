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

#ifndef RESOURCEEDITOR_H
#define RESOURCEEDITOR_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "shared_global_p.h"

#include <QtGui/QWidget>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QPushButton;
class QToolButton;
class QComboBox;
class QStackedWidget;
class QTreeView;
class QModelIndex;

namespace qdesigner_internal {

class ResourceModel;

class QDESIGNER_SHARED_EXPORT ResourceEditor : public QWidget
{
    Q_OBJECT

public:
    ResourceEditor(QDesignerFormEditorInterface *core,
                   bool dragEnabled,
                   QWidget *parent = 0);
    
    QDesignerFormWindowInterface *form() const { return m_form; }
    int qrcCount() const;
    void setCurrentFile(const QString &qrc_path, const QString &file_path);
    bool isIcon(const QString &qrc_path, const QString &file_path) const;

signals:
    void fileActivated(const QString &qrc_path, const QString &file_path);
    void currentFileChanged(const QString &qrc_path, const QString &file_path);

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
    void itemActivated(const QModelIndex &index);
    void itemChanged(const QModelIndex &index);

private:
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

    QDesignerFormWindowInterface *m_form;
    QComboBox *m_qrc_combo;
    QStackedWidget *m_qrc_stack;
    QToolButton *m_add_button;
    QToolButton *m_remove_button;
    QPushButton *m_add_files_button;
    QToolButton *m_remove_qrc_button;
    bool m_ignore_update;
    const bool m_dragEnabled;
};

} // namespace qdesigner_internal

#endif // RESOURCEEDITOR_H
