#ifndef QTBRUSHPATTERNDIALOG_H
#define QTBRUSHPATTERNDIALOG_H

#include <QDialog>

class QtBrushPatternDialog : public QDialog
{
    Q_OBJECT
public:
    QtBrushPatternDialog(QWidget *parent = 0);
    ~QtBrushPatternDialog();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

private:
    class QtBrushPatternDialogPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushPatternDialog)
    Q_DISABLE_COPY(QtBrushPatternDialog)
};

#endif
