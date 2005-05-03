#ifndef ITEMDIALOG_H
#define ITEMDIALOG_H

#include <QtGui/QDialog>
class Item;

class ItemDialog : public QDialog
{
    Q_OBJECT
public:
    ItemDialog(QWidget *parent, const Item *item);

};

#endif
