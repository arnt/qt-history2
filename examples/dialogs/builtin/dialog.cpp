#include "dialog.h"

const QString APPNAME(QObject::tr("Built-in Dialogs"));


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    int frameStyle = QFrame::Sunken|QFrame::Panel;

    integerLabel = new QLabel(this);
    integerLabel->setFrameStyle(frameStyle);
    QPushButton *integerButton =
            new QPushButton(tr("QInputDialog::get&Integer()"), this);
    doubleLabel = new QLabel(this);

    doubleLabel->setFrameStyle(frameStyle);
    QPushButton *doubleButton =
            new QPushButton(tr("QInputDialog::get&Double()"), this);
    itemLabel = new QLabel(this);
    itemLabel->setFrameStyle(frameStyle);
    QPushButton *itemButton =
            new QPushButton(tr("QInputDialog::getIte&m()"), this);

    textLabel = new QLabel(this);
    textLabel->setFrameStyle(frameStyle);
    QPushButton *textButton =
            new QPushButton(tr("QInputDialog::get&Text()"), this);

    colorLabel = new QLabel(this);
    colorLabel->setFrameStyle(frameStyle);
    QPushButton *colorButton =
            new QPushButton(tr("QColorDialog::get&Color()"), this);

    fontLabel = new QLabel(this);
    fontLabel->setFrameStyle(frameStyle);
    QPushButton *fontButton =
            new QPushButton(tr("QFontDialog::get&Font()"), this);

    directoryLabel = new QLabel(this);
    directoryLabel->setFrameStyle(frameStyle);
    QPushButton *directoryButton =
            new QPushButton(tr("QFileDialog::get&ExistingDirectory()"), this);

    openFileNameLabel = new QLabel(this);
    openFileNameLabel->setFrameStyle(frameStyle);
    QPushButton *openFileNameButton =
            new QPushButton(tr("QFileDialog::get&OpenFileName()"), this);

    openFileNamesLabel = new QLabel(this);
    openFileNamesLabel->setFrameStyle(frameStyle);
    QPushButton *openFileNamesButton =
            new QPushButton(tr("QFileDialog::&getOpenFileNames()"), this);

    saveFileNameLabel = new QLabel(this);
    saveFileNameLabel->setFrameStyle(frameStyle);
    QPushButton *saveFileNameButton =
            new QPushButton(tr("QFileDialog::get&SaveFileName()"), this);

    QGridLayout *grid = new QGridLayout(this);
    grid->setColumnStretch(1, 1);
    grid->setColumnSpacing(1, 250);
    grid->addWidget(integerButton, 0, 0);
    grid->addWidget(integerLabel, 0, 1);
    grid->addWidget(doubleButton, 1, 0);
    grid->addWidget(doubleLabel, 1, 1);
    grid->addWidget(itemButton, 2, 0);
    grid->addWidget(itemLabel, 2, 1);
    grid->addWidget(textButton, 3, 0);
    grid->addWidget(textLabel, 3, 1);
    grid->addWidget(colorButton, 4, 0);
    grid->addWidget(colorLabel, 4, 1);
    grid->addWidget(fontButton, 5, 0);
    grid->addWidget(fontLabel, 5, 1);
    grid->addWidget(directoryButton, 6, 0);
    grid->addWidget(directoryLabel, 6, 1);
    grid->addWidget(openFileNameButton, 7, 0);
    grid->addWidget(openFileNameLabel, 7, 1);
    grid->addWidget(openFileNamesButton, 8, 0);
    grid->addWidget(openFileNamesLabel, 8, 1);
    grid->addWidget(saveFileNameButton, 9, 0);
    grid->addWidget(saveFileNameLabel, 9, 1);

    connect(integerButton, SIGNAL(clicked()),
            this, SLOT(setInteger()));
    connect(doubleButton, SIGNAL(clicked()),
            this, SLOT(setDouble()));
    connect(itemButton, SIGNAL(clicked()),
            this, SLOT(setItem()));
    connect(textButton, SIGNAL(clicked()),
            this, SLOT(setText()));
    connect(colorButton, SIGNAL(clicked()),
            this, SLOT(setColor()));
    connect(fontButton, SIGNAL(clicked()),
            this, SLOT(setFont()));
    connect(directoryButton, SIGNAL(clicked()),
            this, SLOT(setExistingDirectory()));
    connect(openFileNameButton, SIGNAL(clicked()),
            this, SLOT(setOpenFileName()));
    connect(openFileNamesButton, SIGNAL(clicked()),
            this, SLOT(setOpenFileNames()));
    connect(saveFileNameButton, SIGNAL(clicked()),
            this, SLOT(setSaveFileName()));

    setWindowTitle(APPNAME);
}


void Dialog::setInteger()
{
    bool ok;
    int i = QInputDialog::getInteger(APPNAME, tr("Percentage:"),
                                     25, 0, 100, 1, &ok, this);
    if (ok)
        integerLabel->setText(QString("%1%%").arg(i));
}


void Dialog::setDouble()
{
    bool ok;
    double d = QInputDialog::getDouble(APPNAME, tr("Amount:"),
                                       37.56, -10000, 10000, 2, &ok, this);
    if (ok)
        doubleLabel->setText(QString("$%1").arg(d));
}


void Dialog::setItem()
{
    QStringList items;
    items << tr("Spring") << tr("Summer") << tr("Fall") << tr("Winter");
    bool ok;
    QString season = QInputDialog::getItem(APPNAME, tr("Season:"),
                                           items, 0, false, &ok, this);
    if (ok && !season.isEmpty())
        itemLabel->setText(season);
}


void Dialog::setText()
{
    bool ok;
    QString name = QInputDialog::getText(APPNAME, tr("Username:"),
                                         QLineEdit::Normal, QString(), &ok, this);
    if (ok && !name.isEmpty())
        textLabel->setText(name);
}


void Dialog::setColor()
{
    QColor color = QColorDialog::getColor(Qt::green, this);
    if (color.isValid()) {
        colorLabel->setText(color.name());
        // TODO Set the label's background colour to color.
    }
}


void Dialog::setFont()
{
    /* TODO Correct once Qt 4 version works.
    bool ok;
    QFont font = QFontDialog(&ok, QFont(fontLabel->text()), this);
    if (ok)
        fontLabel->setText(font.key());
    */
}


void Dialog::setExistingDirectory()
{
    /* TODO Correct once Qt 4 version works.
    QString directory = QFileDialog::getExistingDirectory(
                                this, APPNAME, directoryLabel->text(),
                                QFileDialog::DontResolveSymlinks|
                                QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        directoryLabel->setText(directory);
    */
}


void Dialog::setOpenFileName()
{
    QString filename = QFileDialog::getOpenFileName(
                                this, APPNAME,
                                openFileNameLabel->text(),
                                "All (*);;Text files (*.txt)");
    if (!filename.isEmpty())
        openFileNameLabel->setText(filename);
}


void Dialog::setOpenFileNames()
{
    QStringList files = QFileDialog::getOpenFileNames(
                                this, APPNAME,
                                openFilesPath,
                                "All (*);;Text files (*.txt)");
    if (files.count()) {
        openFilesPath = files[0];
        openFileNamesLabel->setText(QString("[%1]").arg(files.join(", ")));
    }
}


void Dialog::setSaveFileName()
{
    QString filename = QFileDialog::getSaveFileName(
                                this, APPNAME,
                                saveFileNameLabel->text(),
                                "All (*);;Text files (*.txt)");
    if (!filename.isEmpty())
        saveFileNameLabel->setText(filename);
}

