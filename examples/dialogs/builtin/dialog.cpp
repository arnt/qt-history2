#include "dialog.h"

const QString MESSAGE = QObject::tr(
        "<p align=\"center\">Message boxes have a caption, a text, "
        "and one, two, or three buttons, each with standard or custom texts."
        "<p align=\"center\">Click a button or press Esc.");


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    errorMessageDialog = new QErrorMessage(this);

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
            new QPushButton(tr("QFileDialog::getE&xistingDirectory()"), this);

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

    criticalLabel = new QLabel(this);
    criticalLabel->setFrameStyle(frameStyle);
    QPushButton *criticalButton =
            new QPushButton(tr("QMessageBox::critica&l()"), this);

    informationLabel = new QLabel(this);
    informationLabel->setFrameStyle(frameStyle);
    QPushButton *informationButton =
            new QPushButton(tr("QMessageBox::i&nformation()"), this);

    questionLabel = new QLabel(this);
    questionLabel->setFrameStyle(frameStyle);
    QPushButton *questionButton =
            new QPushButton(tr("QMessageBox::&question()"), this);

    warningLabel = new QLabel(this);
    warningLabel->setFrameStyle(frameStyle);
    QPushButton *warningButton =
            new QPushButton(tr("QMessageBox::&warning()"), this);

    errorLabel = new QLabel(this);
    errorLabel->setFrameStyle(frameStyle);
    QPushButton *errorButton =
            new QPushButton(tr("QErrorMessage::&error()"), this);

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
    grid->addWidget(criticalButton, 10, 0);
    grid->addWidget(criticalLabel, 10, 1);
    grid->addWidget(informationButton, 11, 0);
    grid->addWidget(informationLabel, 11, 1);
    grid->addWidget(questionButton, 12, 0);
    grid->addWidget(questionLabel, 12, 1);
    grid->addWidget(warningButton, 13, 0);
    grid->addWidget(warningLabel, 13, 1);
    grid->addWidget(errorButton, 14, 0);
    grid->addWidget(errorLabel, 14, 1);

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
    connect(criticalButton, SIGNAL(clicked()),
            this, SLOT(criticalMessage()));
    connect(informationButton, SIGNAL(clicked()),
            this, SLOT(informationMessage()));
    connect(questionButton, SIGNAL(clicked()),
            this, SLOT(questionMessage()));
    connect(warningButton, SIGNAL(clicked()),
            this, SLOT(warningMessage()));
    connect(errorButton, SIGNAL(clicked()),
            this, SLOT(errorMessage()));

    setWindowTitle(tr("Built-in Dialogs"));
}


void Dialog::setInteger()
{
    bool ok;
    int i =
        QInputDialog::getInteger(tr("QInputDialog::getInteger()"),
                                 tr("Percentage:"), 25, 0, 100, 1, &ok, this);
    if (ok)
        integerLabel->setText(QString("%1%%").arg(i));
}


void Dialog::setDouble()
{
    bool ok;
    double d =
        QInputDialog::getDouble(tr("QInputDialog::getDouble()"),
                                tr("Amount:"), 37.56, -10000, 10000, 2,
                                &ok, this);
    if (ok)
        doubleLabel->setText(QString("$%1").arg(d));
}


void Dialog::setItem()
{
    QStringList items;
    items << tr("Spring") << tr("Summer") << tr("Fall") << tr("Winter");
    bool ok;
    QString season = QInputDialog::getItem(tr("QInputDialog::getItem()"),
                                           tr("Season:"), items, 0,
                                           false, &ok, this);
    if (ok && !season.isEmpty())
        itemLabel->setText(season);
}


void Dialog::setText()
{
    bool ok;
    QString name = QInputDialog::getText(tr("QInputDialog::getText()"),
                                         tr("Username:"), QLineEdit::Normal,
                                         QString(), &ok, this);
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
    QString directory = QFileDialog::getExistingDirectory(
                                this, tr("QFileDialog::getExistingDirectory()"),
                                directoryLabel->text()
                                );
                                /*
                                ,
                                QFileDialog::DontResolveSymlinks|
                                QFileDialog::ShowDirsOnly);
                                */
    if (!directory.isEmpty())
        directoryLabel->setText(directory);
}


void Dialog::setOpenFileName()
{
    QString filename = QFileDialog::getOpenFileName(
                                this, tr("QFileDialog::getOpenFileName()"),
                                openFileNameLabel->text(),
                                "All (*);;Text files (*.txt)");
    if (!filename.isEmpty())
        openFileNameLabel->setText(filename);
}


void Dialog::setOpenFileNames()
{
    QStringList files = QFileDialog::getOpenFileNames(
                                this, tr("QFileDialog::getOpenFileNames()"),
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
                                this, tr("QFileDialog::getSaveFileName()"),
                                saveFileNameLabel->text(),
                                "All (*);;Text files (*.txt)");
    if (!filename.isEmpty())
        saveFileNameLabel->setText(filename);
}


void Dialog::criticalMessage()
{
    int reply = QMessageBox::critical(this, tr("QMessageBox::critical()"),
                                      MESSAGE,
                                      QMessageBox::Abort,
                                      QMessageBox::Retry,
                                      QMessageBox::Ignore);
    QString answer;
    if (reply == QMessageBox::Abort)
        answer = tr("Abort");
    else if (reply == QMessageBox::Retry)
        answer = tr("Retry");
    else
        answer = tr("Ignore");
    criticalLabel->setText(answer);
}


void Dialog::informationMessage()
{
    QMessageBox::information(this, tr("QMessageBox::information()"), MESSAGE);
    informationLabel->setText(tr("Closed with OK or Esc"));
}


void Dialog::questionMessage()
{
    int reply = QMessageBox::question(this, tr("QMessageBox::question()"),
                                      MESSAGE,
                                      QMessageBox::Yes,
                                      QMessageBox::No,
                                      QMessageBox::Cancel);
    QString answer;
    if (reply == QMessageBox::Yes)
        answer = tr("Yes");
    else if (reply == QMessageBox::No)
        answer = tr("No");
    else
        answer = tr("Cancel");
    questionLabel->setText(answer);
}


void Dialog::warningMessage()
{
    int reply = QMessageBox::warning(this, tr("QMessageBox::warning()"),
                                     MESSAGE,
                                     tr("Save &Again"),
                                     tr("&Continue"));
    QString answer;
    if (reply == 0)
        answer = tr("Save Again");
    else
        answer = tr("Continue");
    warningLabel->setText(answer);
}


void Dialog::errorMessage()
{
    errorMessageDialog->message(
            tr("This dialog shows and remembers error messages. "
               "If the checkbox is checked (as it is by default), "
               "the shown message will be shown again, "
               "but if the user unchecks the box the message "
               "will not appear again if QMessageBox::message() "
               "is called with the same message."));
    errorLabel->setText(tr("If the box is unchecked the message "
                           "won't appear again."));
}

