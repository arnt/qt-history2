#ifndef QTABLEVIEW_H
#define QTABLEVIEW_H

#ifndef QT_H
#include <qgenerictableview.h>
#include <qtablemodel.h>
#endif

class Q_GUI_EXPORT QTableView : public QGenericTableView
{
    Q_OBJECT

public:
    QTableView(QWidget *parent = 0);
    QTableView(QTableModel *model, QWidget *parent = 0);
    ~QTableView();

    virtual void setRowCount(int rows);
    virtual void setColumnCount(int columns);
    int rowCount() const;
    int columnCount() const;

    virtual void setText(int row, int column, const QString &text);
    virtual void setIconSet(int row, int column, const QIconSet &iconSet);
    QString text(int row, int column) const;
    QIconSet iconSet(int row, int column) const;

    virtual void setRowText(int row, const QString &text);
    virtual void setRowIconSet(int row, const QIconSet &iconSet);
    QString rowText(int row) const;
    QIconSet rowIconSet(int row) const;

    virtual void setColumnText(int column, const QString &text);
    virtual void setColumnIconSet(int column, const QIconSet &iconSet);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;

    inline QTableModel *model() const { return ::qt_cast<QTableModel*>(QAbstractItemView::model()); }
};

#endif
