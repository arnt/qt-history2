#ifndef PREVIEWFORM_H
#define PREVIEWFORM_H

#include <QDialog>

class QComboBox;
class QLabel;
class QPushButton;
class QTextEdit;

class PreviewForm : public QDialog
{
    Q_OBJECT

public:
    PreviewForm(QWidget *parent = 0);

    void setEncodedData(const QByteArray &data);
    QString decodedString() const { return decodedStr; }

private slots:
    void updateTextEdit();

private:
    void populateEncodingComboBox();

    QByteArray encodedData;
    QString decodedStr;

    QComboBox *encodingComboBox;
    QLabel *encodingLabel;
    QTextEdit *textEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif
