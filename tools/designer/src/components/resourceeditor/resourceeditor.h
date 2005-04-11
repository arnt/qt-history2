#ifndef RESOURCEEDITOR_H
#define RESOURCEEDITOR_H

#include <QtGui/QDialog>

#include "resourceeditor_global.h"
#include "ui_resourceeditor.h"

class QDesignerFormWindowInterface;
class ResourceModel;
class QPushButton;
class QToolButton;
class QLineEdit;
class QComboBox;
class QStackedWidget;
class QString;
class QTreeView;

class ResourceEditor : public QDialog, public Ui::ResourceEditor
{
    Q_OBJECT

public:
    ResourceEditor(QDesignerFormWindowInterface *form, QWidget *parent = 0);

    QDesignerFormWindowInterface *form() const { return m_form; }
    int qrcCount() const;
    
public slots:    
    void saveCurrentView();
    void removeCurrentView();
    void reloadCurrentView();
    void newView();
    void openView();
    
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
};

#endif // RESOURCEEDITOR_H
