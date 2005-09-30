/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "bookwindow.h"
#include "bookdelegate.h"
#include "initdb.h"

#include <QtSql>

BookWindow::BookWindow()
{
    ui.setupUi(this);

    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        QMessageBox::critical(this, "Unable to load database", "This demo needs the SQLITE driver");

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
    model->setHeaderData(model->fieldIndex("rating"), Qt::Horizontal, tr("Rating"));

    // Populate the model
    if (!model->select()) {
        showError(model->lastError());
        return;
    }

    // Set the model and hide the ID column
    ui.bookTable->setModel(model);
    ui.bookTable->setItemDelegate(new BookDelegate(ui.bookTable));
    ui.bookTable->setColumnHidden(model->fieldIndex("id"), true);

    // Initialize the Author combo box
    ui.authorEdit->setModel(model->relationModel(authorIdx));
    ui.authorEdit->setModelColumn(model->relationModel(authorIdx)->fieldIndex("name"));

    ui.genreEdit->setModel(model->relationModel(genreIdx));
    ui.genreEdit->setModelColumn(model->relationModel(genreIdx)->fieldIndex("name"));

    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(dataChanged(QModelIndex)));
    connect(ui.bookTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
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
    ui.ratingEdit->setEnabled(enabled);

    QSqlRecord rec = model->record(index.row());
    ui.titleEdit->setText(rec.value("title").toString());
    ui.yearEdit->setValue(rec.value("year").toInt());
    ui.authorEdit->setCurrentIndex(ui.authorEdit->findText(rec.value(authorIdx).toString()));
    ui.genreEdit->setCurrentIndex(ui.genreEdit->findText(rec.value(genreIdx).toString()));
    ui.ratingEdit->setCurrentIndex(rec.value("rating").toInt());
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

void BookWindow::on_ratingEdit_activated(int value)
{
    QModelIndex currentIndex = model->index(ui.bookTable->currentIndex().row(),
            model->fieldIndex("rating"));
    if (model->data(currentIndex).toInt() != value)
        model->setData(currentIndex, value);
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

void BookWindow::dataChanged(const QModelIndex &index)
{
    if (index.row() == ui.bookTable->currentIndex().row())
        currentBookChanged(index);
}
