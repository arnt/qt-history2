#ifndef QLISTVIEW_H
#define QLISTVIEW_H

#ifndef QT_H
#include <qgenericlistview.h>
#include <qlistmodel.h>
#endif

class Q_GUI_EXPORT QListView_ : public QGenericListView
{
    Q_OBJECT

public:
    QListView_(QWidget *parent = 0, const char *name = 0);
    QListView_(QListModel *model, QWidget *parent = 0, const char *name = 0);
    ~QListView_();

    virtual void setText(int row, const QString &text);
    virtual void setIconSet(int row, const QIconSet &iconSet);
    QString text(int row) const;
    QIconSet iconSet(int row) const;

    inline QListModel *list() const { return ::qt_cast<QListModel*>(model()); }
};

#endif // QLISTVIEW_H
