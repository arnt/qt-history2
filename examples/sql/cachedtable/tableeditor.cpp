#include <QtGui>
#include <QtSql>

#include "tableeditor.h"

TableEditor::TableEditor(const QString &tableName, QWidget *parent)
    : QDialog(parent)
{
    model = new QSqlTableModel(this);
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("First name"));
    model->setHeaderData(2, Qt::Horizontal, tr("Last name"));

    QTableView *view = new QTableView(this);
    view->setModel(model);

    submitButton = new QPushButton(tr("Submit"), this);
    submitButton->setDefault(true);
    revertButton = new QPushButton(tr("&Revert"), this);
    quitButton = new QPushButton(tr("Quit"), this);

    connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));
    connect(revertButton, SIGNAL(clicked()), model, SLOT(revertAll()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(submitButton);
    buttonLayout->addWidget(revertButton);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(view);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Cached Table"));
}

void TableEditor::submit()
{
    model->database().transaction();
    if (model->submitAll()) {
        model->database().commit();
    } else {
        model->database().rollback();
        QMessageBox::showWarning(this, tr("Cached Table"),
                             tr("The database reported an error: %1")
                             .arg(model->lastError().text()));
    }
}
