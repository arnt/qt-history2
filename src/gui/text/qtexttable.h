#ifndef QTEXTTABLE_H
#define QTEXTTABLE_H

#ifndef QT_H
#include <qglobal.h>
#include <qshareddata.h>
#include <qobject.h>
#include "qtextobject.h"
#endif // QT_H

class QTextCursor;
class QTextTablePrivate;

class Q_GUI_EXPORT QTextTableCell
{
public:
    QTextTableCell() : table(0) {}
    ~QTextTableCell() {}
    QTextTableCell(const QTextTableCell &o) : table(o.table), fragment(o.fragment) {}
    QTextTableCell &operator=(const QTextTableCell &o)
    { table = o.table; fragment = o.fragment; return *this; }

    QTextCharFormat format() const;

    int row() const;
    int column() const;

    int rowSpan() const;
    int columnSpan() const;

    inline bool isValid() const { return table != 0; }

    QTextCursor firstCursorPosition() const;
    QTextCursor lastCursorPosition() const;
    int firstPosition() const;
    int lastPosition() const;

    inline bool operator==(const QTextTableCell &other) const
    { return table == other.table && fragment == other.fragment; }
    inline bool operator!=(const QTextTableCell &other) const
    { return !operator==(other); }

    QTextFrame::iterator begin() const;
    QTextFrame::iterator end() const;

private:
    friend class QTextTable;
    QTextTableCell(const QTextTable *t, int f)
        : table(t), fragment(f) {}

    const QTextTable *table;
    int fragment;
};

class Q_GUI_EXPORT QTextTable : public QTextFrame
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextTable)
public:
    QTextTable(QTextDocument *doc);
    ~QTextTable();

    void resize(int rows, int cols);
    void insertRows(int pos, int num);
    void insertColumns(int pos, int num);
    void removeRows(int pos, int num);
    void removeColumns(int pos, int num);

    int rows() const;
    int columns() const;

#if 0
    void mergeCells(const QTextCursor &selection);
#endif

    QTextTableCell cellAt(int row, int col) const;
    QTextTableCell cellAt(int position) const;
    QTextTableCell cellAt(const QTextCursor &c) const;

    QTextCursor rowStart(const QTextCursor &c) const;
    QTextCursor rowEnd(const QTextCursor &c) const;

    void setFormat(const QTextTableFormat &format) { QTextObject::setFormat(format); }
    QTextTableFormat format() const { return QTextObject::format().toTableFormat(); }

private:
    friend class QTextTableCell;
#if defined(Q_DISABLE_COPY)
    QTextTable(const QTextTable &o);
    QTextTable & operator =(const QTextTable &o);
#endif
};

#endif
