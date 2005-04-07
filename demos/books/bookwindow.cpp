#include <QtSql>

#include "bookwindow.h"
#include "initdb.h"

BookWindow::BookWindow()
{
    ui.setupUi(this);

    // initialize the database
    QSqlError err = initDb();
    if (err.type() != QSqlError::NoError) {
        showError(err);
        return;
    }

    // Create the data model
    model = new QSqlRelationalTableModel(ui.bookTable);
    model->setTable("books");

    // Remeber the indexes of the columns
    authorIdx = model->fieldIndex("author");
    genreIdx = model->fieldIndex("genre");

    // Set the relations to the other database tables
    model->setRelation(authorIdx, QSqlRelation("authors", "id", "name"));
    model->setRelation(genreIdx, QSqlRelation("genres", "id", "name"));

    // Set the localized header captions
    model->setHeaderData(authorIdx, Qt::Horizontal, tr("Author Name"));
    model->setHeaderData(genreIdx, Qt::Horizontal, tr("Genre"));
    model->setHeaderData(model->fieldIndex("title"), Qt::Horizontal, tr("Title"));
    model->setHeaderData(model->fieldIndex("year"), Qt::Horizontal, tr("Year"));

    // Populate the model
    if (!model->select()) {
        showError(model->lastError());
        return;
    }

    // Set the model and hide the ID column
    ui.bookTable->setModel(model);
    ui.bookTable->setItemDelegate(new QSqlRelationalDelegate(ui.bookTable));
    ui.bookTable->setColumnHidden(model->fieldIndex("id"), true);

    // Initialize the Author combo box
    ui.authorEdit->setModel(model->relationModel(authorIdx));
    ui.authorEdit->setModelColumn(model->relationModel(authorIdx)->fieldIndex("name"));

    ui.genreEdit->setModel(model->relationModel(genreIdx));
    ui.genreEdit->setModelColumn(model->relationModel(genreIdx)->fieldIndex("name"));

    connect(ui.bookTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(currentBookChanged(QModelIndex)));
}

void BookWindow::showError(const QSqlError &err)
{
    QMessageBox::critical(this, "Unable to initialize Database",
                "Error initializing database: " + err.text());
}

void BookWindow::currentBookChanged(const QModelIndex &index)
{
    bool enabled = index.isValid();
    ui.titleEdit->setEnabled(enabled);
    ui.yearEdit->setEnabled(enabled);
    ui.authorEdit->setEnabled(enabled);
    ui.genreEdit->setEnabled(enabled);

    QSqlRecord rec = model->record(index.row());
    ui.titleEdit->setText(rec.value("title").toString());
    ui.yearEdit->setValue(rec.value("year").toInt());
    ui.authorEdit->setCurrentIndex(ui.authorEdit->findText(rec.value(authorIdx).toString()));
    ui.genreEdit->setCurrentIndex(ui.genreEdit->findText(rec.value(genreIdx).toString()));
}

void BookWindow::on_authorEdit_activated(const QString &text)
{
    QModelIndex currentIndex = model->index(ui.bookTable->currentIndex().row(), authorIdx);
    if (model->data(currentIndex).toString() != text)
        ui.bookTable->itemDelegate()->setModelData(ui.authorEdit, model, currentIndex);
}

void BookWindow::on_genreEdit_activated(const QString &text)
{
    QModelIndex currentIndex = model->index(ui.bookTable->currentIndex().row(), genreIdx);
    if (model->data(currentIndex).toString() != text)
        ui.bookTable->itemDelegate()->setModelData(ui.genreEdit, model, currentIndex);
}

void BookWindow::on_titleEdit_textChanged(const QString &text)
{
    QModelIndex currentIndex = model->index(ui.bookTable->currentIndex().row(),
            model->fieldIndex("title"));
    if (model->data(currentIndex).toString() != text)
        model->setData(currentIndex, text);
}

void BookWindow::on_yearEdit_valueChanged(int value)
{
    QModelIndex currentIndex = model->index(ui.bookTable->currentIndex().row(),
            model->fieldIndex("year"));
    if (model->data(currentIndex).toInt() != value)
        model->setData(currentIndex, value);
}

