#ifndef PIMEDITOR_H
#define PIMEDITOR_H

#include <qwidget.h>

class PimModel;
class QModelIndex;
class QPushButton;
class QLineEdit;

class PimEditor : public QWidget
{
    Q_OBJECT
public:
    PimEditor(QWidget *parent = 0);
    ~PimEditor();

    void setModel(PimModel *model);
    PimModel *model() const;

public slots:
    void create();
    void edit(const QModelIndex &index);
    void accept();
    void cancel();
    void photo();

signals:
    void done();

private:
    PimModel *m;
    int entry;

    QPushButton *photoButton;
    QLineEdit *firstNameEdit;
    QLineEdit *lastNameEdit;
    QLineEdit *middleNameEdit;
    QLineEdit *companyEdit;
    QLineEdit *departmentEdit;
    QLineEdit *jobTitleEdit;
    QPushButton *acceptButton;
    QPushButton *cancelButton;
};

#endif // PIMEDITOR_H
