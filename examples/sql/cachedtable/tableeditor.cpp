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

    QTableView *view = new QTableView(this);
    view->setModel(model);

    submitButton = new QPushButton(tr("Submit"), this);
    submitButton->setDefault(true);
    revertButton = new QPushButton(tr("&Revert"), this);
    quitButton = new QPushButton(tr("Quit"), this);

    connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));
    connect(revertButton, SIGNAL(clicked()), model, SLOT(cancelChanges()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(submitButton);
    buttonLayout->addWidget(revertButton);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(view);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Cached Table"));
}

void TableEditor::submit()
{
    model->database().transaction();
    if (model->submitChanges()) {
        model->database().commit();
    } else {
        model->database().rollback();
        QMessageBox::warning(this, tr("Cached Table"),
                             tr("The database reported an error: %1")
                             .arg(model->lastError().text()));
    }
}
