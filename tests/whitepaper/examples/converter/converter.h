/*
  converter.h
*/

#ifndef CONVERTER_H
#define CONVERTER_H

#include <qcombobox.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qsocket.h>

class Converter : public QDialog
{
    Q_OBJECT
public:
    Converter( QWidget *parent = 0, const char *name = 0 );

private slots:
    void convert();
    void updateTargetAmount();

private:
    QComboBox *create_currency_combobox( const char *initial );

    QLineEdit *sourceAmount;
    QComboBox *sourceCurrency;
    QLabel *targetAmount;
    QComboBox *targetCurrency;
    QSocket *socket;
};

#endif
