#ifndef RESOURCEEDITOR_H
#define RESOURCEEDITOR_H

#include <QtGui/QWidget>

#include "resourceeditor_global.h"

class AbstractFormWindow;
class ResourceModel;
class QPushButton;
class QToolButton;
class QLineEdit;
class QComboBox;
class QStackedWidget;
class QString;
class QTreeView;

class ResourceEditor : public QWidget
{
    Q_OBJECT

public:
    ResourceEditor(AbstractFormWindow *form, QWidget *parent = 0);

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

#endif // RESOURCEEDITOR_H
