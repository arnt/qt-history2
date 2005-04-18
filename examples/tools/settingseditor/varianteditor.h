#ifndef VARIANTEDITOR_H
#define VARIANTEDITOR_H

class VariantEditor : public QDialog
{
    Q_OBJECT

public:
    VariantEditor(QWidget *parent = 0);

    void setVariant(const QVariant &value);
    QVariant variant() const { return value; }

private:
    QVariant value;
};

#endif
