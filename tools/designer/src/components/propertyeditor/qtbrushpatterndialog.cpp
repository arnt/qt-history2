#include "qtbrushpatterndialog.h"
#include "ui_qtbrushpatterndialog.h"

class QtBrushPatternDialogPrivate
{
    QtBrushPatternDialog *q_ptr;
    Q_DECLARE_PUBLIC(QtBrushPatternDialog)
public:
    Ui::QtBrushPatternDialog m_ui;
};

QtBrushPatternDialog::QtBrushPatternDialog(QWidget *parent)
    : QDialog(parent)
{
    d_ptr = new QtBrushPatternDialogPrivate();
    d_ptr->q_ptr = this;
    d_ptr->m_ui.setupUi(this);
}

QtBrushPatternDialog::~QtBrushPatternDialog()
{
    delete d_ptr;
}

void QtBrushPatternDialog::setBrush(const QBrush &brush)
{
    d_ptr->m_ui.brushPatternEditor->setBrush(brush);
}

QBrush QtBrushPatternDialog::brush() const
{
    return d_ptr->m_ui.brushPatternEditor->brush();
}
