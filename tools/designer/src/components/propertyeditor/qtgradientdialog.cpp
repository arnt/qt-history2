#include "qtgradientdialog.h"
#include "ui_qtgradientdialog.h"

class QtGradientDialogPrivate
{
    QtGradientDialog *q_ptr;
    Q_DECLARE_PUBLIC(QtGradientDialog)
public:
    Ui::QtGradientDialog m_ui;
};

QtGradientDialog::QtGradientDialog(QWidget *parent)
    : QDialog(parent)
{
    d_ptr = new QtGradientDialogPrivate();
    d_ptr->q_ptr = this;
    d_ptr->m_ui.setupUi(this);
}

QtGradientDialog::~QtGradientDialog()
{
    delete d_ptr;
}

void QtGradientDialog::setGradient(const QGradient &gradient)
{
    d_ptr->m_ui.gradientEditor->setGradient(gradient);
}

QGradient QtGradientDialog::gradient() const
{
    return d_ptr->m_ui.gradientEditor->gradient();
}
