#include <QtGui>

#include "dialog.h"

#define MESSAGE \
    Dialog::tr("<p>Message boxes have a caption, a text, " \
               "and up to three buttons, each with standard or custom texts." \
               "<p>Click a button or press Esc.")

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    errorMessageDialog = new QErrorMessage(this);

    int frameStyle = QFrame::Sunken | QFrame::Panel;

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
            new QPushButton(tr("QErrorMessage::show&M&essage()"), this);

    connect(integerButton, SIGNAL(clicked()), this, SLOT(setInteger()));
    connect(doubleButton, SIGNAL(clicked()), this, SLOT(setDouble()));
    connect(itemButton, SIGNAL(clicked()), this, SLOT(setItem()));
    connect(textButton, SIGNAL(clicked()), this, SLOT(setText()));
    connect(colorButton, SIGNAL(clicked()), this, SLOT(setColor()));
    connect(fontButton, SIGNAL(clicked()), this, SLOT(setFont()));
    connect(directoryButton, SIGNAL(clicked()),
            this, SLOT(setExistingDirectory()));
    connect(openFileNameButton, SIGNAL(clicked()),
            this, SLOT(setOpenFileName()));
    connect(openFileNamesButton, SIGNAL(clicked()),
            this, SLOT(setOpenFileNames()));
    connect(saveFileNameButton, SIGNAL(clicked()),
            this, SLOT(setSaveFileName()));
    connect(criticalButton, SIGNAL(clicked()), this, SLOT(criticalMessage()));
    connect(informationButton, SIGNAL(clicked()),
            this, SLOT(informationMessage()));
    connect(questionButton, SIGNAL(clicked()), this, SLOT(questionMessage()));
    connect(warningButton, SIGNAL(clicked()), this, SLOT(warningMessage()));
    connect(errorButton, SIGNAL(clicked()), this, SLOT(errorMessage()));

    QGridLayout *layout = new QGridLayout(this);
    layout->setColumnStretch(1, 1);
    layout->setColumnMinimumWidth(1, 250);
    layout->addWidget(integerButton, 0, 0);
    layout->addWidget(integerLabel, 0, 1);
    layout->addWidget(doubleButton, 1, 0);
    layout->addWidget(doubleLabel, 1, 1);
    layout->addWidget(itemButton, 2, 0);
    layout->addWidget(itemLabel, 2, 1);
    layout->addWidget(textButton, 3, 0);
    layout->addWidget(textLabel, 3, 1);
    layout->addWidget(colorButton, 4, 0);
    layout->addWidget(colorLabel, 4, 1);
    layout->addWidget(fontButton, 5, 0);
    layout->addWidget(fontLabel, 5, 1);
    layout->addWidget(directoryButton, 6, 0);
    layout->addWidget(directoryLabel, 6, 1);
    layout->addWidget(openFileNameButton, 7, 0);
    layout->addWidget(openFileNameLabel, 7, 1);
    layout->addWidget(openFileNamesButton, 8, 0);
    layout->addWidget(openFileNamesLabel, 8, 1);
    layout->addWidget(saveFileNameButton, 9, 0);
    layout->addWidget(saveFileNameLabel, 9, 1);
    layout->addWidget(criticalButton, 10, 0);
    layout->addWidget(criticalLabel, 10, 1);
    layout->addWidget(informationButton, 11, 0);
    layout->addWidget(informationLabel, 11, 1);
    layout->addWidget(questionButton, 12, 0);
    layout->addWidget(questionLabel, 12, 1);
    layout->addWidget(warningButton, 13, 0);
    layout->addWidget(warningLabel, 13, 1);
    layout->addWidget(errorButton, 14, 0);
    layout->addWidget(errorLabel, 14, 1);

    setWindowTitle(tr("Standard Dialogs"));
}

void Dialog::setInteger()
{
    bool ok;
    int i = QInputDialog::getInteger(this, tr("QInputDialog::getInteger()"),
                                     tr("Percentage:"), 25, 0, 100, 1, &ok);
    if (ok)
        integerLabel->setText(tr("%1%").arg(i));
}

void Dialog::setDouble()
{
    bool ok;
    double d = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Amount:"), 37.56, -10000, 10000, 2, &ok);
    if (ok)
        doubleLabel->setText(QString("$%1").arg(d));
}

void Dialog::setItem()
{
    QStringList items;
    items << tr("Spring") << tr("Summer") << tr("Fall") << tr("Winter");

    bool ok;
    QString item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
                                         tr("Season:"), items, 0, false, &ok);
    if (ok && !item.isEmpty())
        itemLabel->setText(item);
}

void Dialog::setText()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("User name:"), QLineEdit::Normal,
                                         QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty())
        textLabel->setText(text);
}

void Dialog::setColor()
{
    QColor color = QColorDialog::getColor(Qt::green, this);
    if (color.isValid()) {
        colorLabel->setText(color.name());
        colorLabel->setPalette(QPalette(color));
    }
}

void Dialog::setFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QFont(fontLabel->text()), this);
    if (ok)
        fontLabel->setText(font.key());
}

void Dialog::setExistingDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(this,
                                tr("QFileDialog::getExistingDirectory()"),
                                directoryLabel->text(),
                                QFileDialog::DontResolveSymlinks
                                | QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        directoryLabel->setText(directory);
}

void Dialog::setOpenFileName()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("QFileDialog::getOpenFileName()"),
                                openFileNameLabel->text(),
                                tr("All Files (*);;Text Files (*.txt)"));
    if (!fileName.isEmpty())
        openFileNameLabel->setText(fileName);
}

void Dialog::setOpenFileNames()
{
    QStringList files = QFileDialog::getOpenFileNames(
                                this, tr("QFileDialog::getOpenFileNames()"),
                                openFilesPath,
                                tr("All Files (*);;Text Files (*.txt)"));
    if (files.count()) {
        openFilesPath = files[0];
        openFileNamesLabel->setText(QString("[%1]").arg(files.join(", ")));
    }
}

void Dialog::setSaveFileName()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                tr("QFileDialog::getSaveFileName()"),
                                saveFileNameLabel->text(),
                                tr("All Files (*);;Text Files (*.txt)"));
    if (!fileName.isEmpty())
        saveFileNameLabel->setText(fileName);
}

void Dialog::criticalMessage()
{
    int reply = QMessageBox::critical(this, tr("QMessageBox::showCritical()"),
                                      MESSAGE,
                                      QMessageBox::Abort,
                                      QMessageBox::Retry,
                                      QMessageBox::Ignore);
    if (reply == QMessageBox::Abort)
        criticalLabel->setText(tr("Abort"));
    else if (reply == QMessageBox::Retry)
        criticalLabel->setText(tr("Retry"));
    else
        criticalLabel->setText(tr("Ignore"));
}

void Dialog::informationMessage()
{
    QMessageBox::information(this, tr("QMessageBox::showInformation()"), MESSAGE);
    informationLabel->setText(tr("Closed with OK or Esc"));
}

void Dialog::questionMessage()
{
    int reply = QMessageBox::question(this, tr("QMessageBox::showQuestion()"),
                                      MESSAGE,
                                      QMessageBox::Yes,
                                      QMessageBox::No,
                                      QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
        questionLabel->setText(tr("Yes"));
    else if (reply == QMessageBox::No)
        questionLabel->setText(tr("No"));
    else
        questionLabel->setText(tr("Cancel"));
}

void Dialog::warningMessage()
{
    int reply = QMessageBox::warning(this, tr("QMessageBox::showWarning()"),
                                     MESSAGE,
                                     tr("Save &Again"),
                                     tr("&Continue"));
    if (reply == 0)
        warningLabel->setText(tr("Save Again"));
    else
        warningLabel->setText(tr("Continue"));

}

void Dialog::errorMessage()
{
    errorMessageDialog->showMessage(
            tr("This dialog shows and remembers error messages. "
               "If the checkbox is checked (as it is by default), "
               "the shown message will be shown again, "
               "but if the user unchecks the box the message "
               "will not appear again if QErrorMessage::showMessage() "
               "is called with the same message."));
    errorLabel->setText(tr("If the box is unchecked, the message "
                           "won't appear again."));
}
