#ifndef QTBRUSHDIALOG_H
#define QTBRUSHDIALOG_H

#include <QDialog>

class QtBrushManager;

class QtBrushDialog : public QDialog
{
    Q_OBJECT
public:
    QtBrushDialog(QWidget *parent = 0);
    ~QtBrushDialog();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setBrushManager(QtBrushManager *manager);

signals:
    void textureChooserActivated(QWidget *parent, const QBrush &initialBrush);
private:
    class QtBrushDialogPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushDialog)
    Q_DISABLE_COPY(QtBrushDialog)
};

#endif
