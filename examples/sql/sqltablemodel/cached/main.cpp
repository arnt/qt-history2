/****************************************************************************
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is an example program for the Qt SQL module.
 ** EDITIONS: NOLIMITS
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

/* This examples demonstrates how to use QSqlTableModel to edit a database
   table by using cached updates and transactions.
   The QSQLITE driver is required for this example.
 */

#include <qapplication.h>
#include <qgenerictableview.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsqlerror.h>
#include <qsqltablemodel.h>
#include <qvbox.h>

#include "../../connection.h"

class TableEditor: public QWidget
{
    Q_OBJECT
public:
    TableEditor(const QString &tableName, QWidget *parent = 0);

public slots:
    void submit();

private:
    QSqlTableModel *model;
};

TableEditor::TableEditor(const QString &tableName, QWidget *parent)
    : QWidget(parent)
{
    model = new QSqlTableModel(this);
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    QVBoxLayout *layout = new QVBoxLayout(this);
    QGenericTableView *view = new QGenericTableView(this);
    view->setModel(model);

    QHBox *box = new QHBox(this);
    QPushButton *submitButton = new QPushButton(tr("Submit"), box);
    QPushButton *revertButton = new QPushButton(tr("Revert"), box);

    connect(submitButton, SIGNAL(clicked()), SLOT(submit()));
    connect(revertButton, SIGNAL(clicked()), model, SLOT(cancelChanges()));

    layout->addWidget(view);
    layout->addWidget(box);
}

void TableEditor::submit()
{
    model->database().transaction();
    if (model->submitChanges()) {
        model->database().commit();
    } else {
        model->database().rollback();
        QMessageBox::warning(this, tr("Error submitting values."),
                tr("The database reported an error: ") + model->lastError().text());
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    createConnection();

    TableEditor editor("persons");
    app.setMainWidget(&editor);
    editor.show();

    return app.exec();
}

#include "main.moc"
