#ifndef PREVIEWFORM_H
#define PREVIEWFORM_H

#include <QDialog>
#include <QList>

class QComboBox;
class QLabel;
class QPushButton;
class QTextCodec;
class QTextEdit;

class PreviewForm : public QDialog
{
    Q_OBJECT

public:
    PreviewForm(QWidget *parent = 0);

    void setCodecList(const QList<QTextCodec *> &list);
    void setEncodedData(const QByteArray &data);
    QString decodedString() const { return decodedStr; }

private slots:
    void updateTextEdit();

private:
    QByteArray encodedData;
    QString decodedStr;

    QComboBox *encodingComboBox;
    QLabel *encodingLabel;
    QTextEdit *textEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif
