// Prototype Preference dialog
#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QtGui/QDialog>
#include <QtCore/QList>

class PreferenceInterface;
class QStackedWidget;
class QTreeWidgetItem;
class PreferenceDialog : public QDialog
{
    Q_OBJECT
public:
    PreferenceDialog(QWidget *parent);
    ~PreferenceDialog();

private slots:
    void accept();
    void reject();
    void changePane(QTreeWidgetItem *);

private:
    QList<PreferenceInterface *> m_preferences;
    QStackedWidget *m_stack;
};
#endif
