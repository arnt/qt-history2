#ifndef QTEXTTABLE_H
#define QTEXTTABLE_H

#ifndef QT_H
#include <qglobal.h>
#include <qshareddata.h>
#include <qobject.h>
#include "qtextformat.h"
#endif // QT_H

class QTextCursor;
class QTextTablePrivate;

class Q_GUI_EXPORT QTextTableCell
{
public:
    QTextTableCell() : d(0) {}
    ~QTextTableCell() {}
    QTextTableCell(const QTextTableCell &o) : d(o.d), fragment(o.fragment) {}
    QTextTableCell &operator=(const QTextTableCell &o)
    { d = o.d; fragment = o.fragment; return *this; }

    QTextCharFormat format() const;

    int row() const;
    int column() const;

    int rowSpan() const;
    int columnSpan() const;

    inline bool isValid() const { return d != 0; }

    QTextCursor start() const;
    QTextCursor end() const;
    int startPosition() const;
    int endPosition() const;

    inline bool operator==(const QTextTableCell &other) const
    { return d == other.d && fragment == other.fragment; }
    inline bool operator!=(const QTextTableCell &other) const
    { return !operator==(other); }

private:
    friend class QTextTable;
    QTextTableCell(const QTextTablePrivate *p, int f)
        : d(p), fragment(f) {}

    const QTextTablePrivate *d;
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
#if defined(Q_DISABLE_COPY)
    QTextTable(const QTextTable &o);
    QTextTable & operator =(const QTextTable &o);
#endif
};

#endif
