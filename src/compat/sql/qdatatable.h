/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDATATABLE_H
#define QDATATABLE_H

#include "qstring.h"
#include "qvariant.h"
#include "qtable.h"
#include "qsql.h"
#include "qsqlcursor.h"
#include "qsqlindex.h"
#include "qsqleditorfactory.h"
#include "qiconset.h"

#ifndef QT_NO_SQL_VIEW_WIDGETS

class QPainter;
class QSqlField;
class QSqlPropertyMap;
class QDataTablePrivate;

class Q_COMPAT_EXPORT QDataTable : public QTable
{
    Q_OBJECT

    Q_PROPERTY(QString nullText READ nullText WRITE setNullText)
    Q_PROPERTY(QString trueText READ trueText WRITE setTrueText)
    Q_PROPERTY(QString falseText READ falseText WRITE setFalseText)
    Q_PROPERTY(Qt::DateFormat dateFormat READ dateFormat WRITE setDateFormat)
    Q_PROPERTY(bool confirmEdits READ confirmEdits WRITE setConfirmEdits)
    Q_PROPERTY(bool confirmInsert READ confirmInsert WRITE setConfirmInsert)
    Q_PROPERTY(bool confirmUpdate READ confirmUpdate WRITE setConfirmUpdate)
    Q_PROPERTY(bool confirmDelete READ confirmDelete WRITE setConfirmDelete)
    Q_PROPERTY(bool confirmCancels READ confirmCancels WRITE setConfirmCancels)
    Q_PROPERTY(bool autoEdit READ autoEdit WRITE setAutoEdit)
    Q_PROPERTY(QString filter READ filter WRITE setFilter)
    Q_PROPERTY(QStringList sort READ sort WRITE setSort)
    Q_PROPERTY(int numCols READ numCols)
    Q_PROPERTY(int numRows READ numRows)

public:
    QDataTable (QWidget* parent=0, const char* name=0);
    QDataTable (QSqlCursor* cursor, bool autoPopulate = false, QWidget* parent=0, const char* name=0);
    ~QDataTable();

    virtual void addColumn(const QString& fieldName,
                            const QString& label = QString(),
                            int width = -1,
                            const QIconSet& iconset = QIconSet());
    virtual void removeColumn(int col);
    virtual void setColumn(int col, const QString& fieldName,
                            const QString& label = QString(),
                            int width = -1,
                            const QIconSet& iconset = QIconSet());

    QString      nullText() const;
    QString      trueText() const;
    QString      falseText() const;
    Qt::DateFormat   dateFormat() const;
    bool         confirmEdits() const;
    bool         confirmInsert() const;
    bool         confirmUpdate() const;
    bool         confirmDelete() const;
    bool         confirmCancels() const;
    bool         autoDelete() const;
    bool         autoEdit() const;
    QString      filter() const;
    QStringList  sort() const;

    virtual void setSqlCursor(QSqlCursor* cursor = 0,
                            bool autoPopulate = false, bool autoDelete = false);
    QSqlCursor* sqlCursor() const;

    virtual void setNullText(const QString& nullText);
    virtual void setTrueText(const QString& trueText);
    virtual void setFalseText(const QString& falseText);
    virtual void setDateFormat(const Qt::DateFormat f);
    virtual void setConfirmEdits(bool confirm);
    virtual void setConfirmInsert(bool confirm);
    virtual void setConfirmUpdate(bool confirm);
    virtual void setConfirmDelete(bool confirm);
    virtual void setConfirmCancels(bool confirm);
    virtual void setAutoDelete(bool enable);
    virtual void setAutoEdit(bool autoEdit);
    virtual void setFilter(const QString& filter);
    virtual void setSort(const QStringList& sort);
    virtual void setSort(const QSqlIndex& sort);

    enum Refresh {
        RefreshData = 1,
        RefreshColumns = 2,
        RefreshAll = 3
    };
    void         refresh(Refresh mode);
    void         sortColumn (int col, bool ascending = true,
                              bool wholeRows = false);
    QString      text (int row, int col) const;
    QVariant     value (int row, int col) const;
    QSqlRecord*  currentRecord() const;

    void         installEditorFactory(QSqlEditorFactory * f);
    void         installPropertyMap(QSqlPropertyMap* m);

    int          numCols() const;
    int          numRows() const;
    void         setNumCols(int c);
    void         setNumRows (int r);
    bool         findBuffer(const QSqlIndex& idx, int atHint = 0);

    void          hideColumn(int col);
    void          showColumn(int col);
signals:
    void         currentChanged(QSqlRecord* record);
    void         primeInsert(QSqlRecord* buf);
    void         primeUpdate(QSqlRecord* buf);
    void         primeDelete(QSqlRecord* buf);
    void         beforeInsert(QSqlRecord* buf);
    void         beforeUpdate(QSqlRecord* buf);
    void         beforeDelete(QSqlRecord* buf);
    void         cursorChanged(QSql::Op mode);

public slots:
    virtual void find(const QString & str, bool caseSensitive,
                             bool backwards);
    virtual void sortAscending(int col);
    virtual void sortDescending(int col);
    virtual void refresh();
    void setColumnWidth(int col, int w);
    void adjustColumn(int col);
    void setColumnStretchable(int col, bool stretch);
    void swapColumns(int col1, int col2, bool swapHeaders = false);

protected:
    virtual bool insertCurrent();
    virtual bool updateCurrent();
    virtual bool deleteCurrent();

    virtual QSql::Confirm confirmEdit(QSql::Op m);
    virtual QSql::Confirm confirmCancel(QSql::Op m);

    virtual void handleError(const QSqlError& e);

    virtual bool beginInsert();
    virtual QWidget* beginUpdate (int row, int col, bool replace);

    bool         eventFilter(QObject *o, QEvent *e);
    void         keyPressEvent(QKeyEvent*);
    void         resizeEvent (QResizeEvent *);
    void         contentsMousePressEvent(QMouseEvent* e);
    void         contentsContextMenuEvent(QContextMenuEvent* e);
    void         endEdit(int row, int col, bool accept, bool replace);
    QWidget *    createEditor(int row, int col, bool initFromCell) const;
    void         activateNextCell();
    int          indexOf(int i) const;
    void         reset();
    void         setSize(QSqlCursor* sql);
    void         repaintCell(int row, int col);
    void         paintCell (QPainter * p, int row, int col, const QRect & cr,
                             bool selected, const QPalette &pal);
    virtual void paintField(QPainter * p, const QSqlField &field, const QRect & cr,
                             bool selected);
    void         drawContents(QPainter * p, int cx, int cy, int cw, int ch);
    virtual int  fieldAlignment(const QSqlField* field);
    void         columnClicked (int col);
    void         resizeData (int len);

    QTableItem * item (int row, int col) const;
    void         setItem (int row, int col, QTableItem * item);
    void         clearCell (int row, int col) ;
    void         setPixmap (int row, int col, const QPixmap & pix);
    void         takeItem (QTableItem * i);

private slots:
    void         loadNextPage();
    void         setCurrentSelection(int row, int col);
    void         updateCurrentSelection();
    void          sliderPressed();
    void          sliderReleased();
    void         doInsertCurrent();
    void         doUpdateCurrent();

private:
    QString          fieldToString(const QSqlField &field);
    void         init();
    QWidget*     beginEdit (int row, int col, bool replace);
    void         updateRow(int row);
    void         endInsert();
    void         endUpdate();
    QDataTablePrivate* d;

    Q_DISABLE_COPY(QDataTable)
};

#endif

#endif
