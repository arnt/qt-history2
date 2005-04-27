#include <QtGui>

#include "imagemodel.h"
#include "mainwindow.h"
#include "pixeldelegate.h"

MainWindow::MainWindow()
{
    model = 0;

    view = new QTableView;
    view->setItemDelegate(new PixelDelegate(this));
    view->setShowGrid(false);
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();
    setCentralWidget(view);

    QMenu *fileMenu = new QMenu(tr("&File"), this);
    QAction *openAction = fileMenu->addAction(tr("&Open"));
    openAction->setShortcut(QKeySequence(tr("Ctrl+O")));

    printAction = fileMenu->addAction(tr("&Print"));
    printAction->setEnabled(false);
    printAction->setShortcut(QKeySequence(tr("Ctrl+P")));

    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));

    QMenu *helpMenu = new QMenu(tr("&Help"), this);
    QAction *aboutAction = helpMenu->addAction(tr("&About"));

    menuBar()->addMenu(fileMenu);
    menuBar()->addSeparator();
    menuBar()->addMenu(helpMenu);

    connect(openAction, SIGNAL(triggered()), this, SLOT(chooseImage()));
    connect(printAction, SIGNAL(triggered()), this, SLOT(printImage()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutBox()));

    setWindowTitle(tr("Pixelator"));
    resize(640, 480);
}

void MainWindow::chooseImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Choose an image"), "", "*");

    if (!fileName.isEmpty())
        openImage(fileName);
}

void MainWindow::openImage(const QString &fileName)
{
    QImage image;

    if (image.load(fileName)) {
        ImageModel *newModel = new ImageModel(image);
        view->setModel(newModel);
        delete model;
        model = newModel;
        setWindowTitle(tr("%1 - %2").arg(fileName).arg(tr("Pixelator")));

        printAction->setEnabled(true);

        int rows = model->rowCount(QModelIndex());
        int columns = model->columnCount(QModelIndex());
        for (int row = 0; row < rows; ++row)
            view->resizeRowToContents(row);
        for (int column = 0; column < columns; ++column)
            view->resizeColumnToContents(column);
    }
}

void MainWindow::printImage()
{
    if (model->rowCount(QModelIndex())*model->columnCount(QModelIndex())
        > 90000) {
        int answer = QMessageBox::question(this, tr("Large Image Size"),
            tr("The printed image may be very large. Are you sure that "
               "you want to print it?"),
            QMessageBox::Yes, QMessageBox::No);
        if (answer == QMessageBox::No)
            return;
    }

    QPrinter printer(QPrinter::PrinterResolution);

    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    dlg->setWindowTitle(tr("Print Image"));

    if (dlg->exec() != QDialog::Accepted)
        return;

    QPainter painter;
    painter.begin(&printer);

    int rows = model->rowCount(QModelIndex());
    int columns = model->columnCount(QModelIndex());
    int sourceWidth = (columns+1) * ItemSize;
    int sourceHeight = (rows+1) * ItemSize;

    painter.save();

    double xscale = printer.pageRect().width()/double(sourceWidth);
    double yscale = printer.pageRect().height()/double(sourceHeight);
    double scale = qMin(xscale, yscale);

    painter.translate(printer.paperRect().x() + printer.pageRect().width()/2,
                      printer.paperRect().y() + printer.pageRect().height()/2);
    painter.scale(scale, scale);
    painter.translate(-sourceWidth/2, -sourceHeight/2);

    QStyleOptionViewItem option;
    QModelIndex parent = QModelIndex();

    QProgressDialog progress(tr("Printing..."), tr("Cancel"), 0, rows, this);
    float y = ItemSize/2;

    for (int row = 0; row < rows; ++row) {
        progress.setValue(row);
        qApp->processEvents();
        if (progress.wasCanceled())
            break;

        float x = ItemSize/2;

        for (int column = 0; column < columns; ++column) {
            option.rect = QRect(int(x), int(y), ItemSize, ItemSize);
            view->itemDelegate()->paint(&painter, option,
                                        model->index(row, column, parent));
            x = x + ItemSize;
        }
        y = y + ItemSize;
    }
    progress.setValue(rows);

    painter.restore();
    painter.end();

    if (progress.wasCanceled()) {
        QMessageBox::information(this, tr("Printing canceled"),
            tr("The printing process was canceled."), QMessageBox::Cancel);
    }
}

void MainWindow::showAboutBox()
{
    QMessageBox::about(this, tr("About the Pixelator example"),
        tr("This example demonstrates how a standard view and a custom\n"
           "delegate can be used to produce a specialized representation\n "
           "of data in a simple custom model."));
}
