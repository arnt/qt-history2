/****************************************************************************
**
** copyright (c) 1992-$thisyear$ $trolltech$. all rights reserved.
**
** this file is part of the $module$ of the qt toolkit.
**
** $trolltech_dual_license$
**
** this file is provided as is with no warranty of any kind, including the
** warranty of design, merchantability and fitness for a particular purpose.
**
****************************************************************************/

#ifndef QTCURSORDATABASE_H
#define QTCURSORDATABASE_H

#include <QMap>
#include <QIcon>
#include <QStringList>

class QtCursorDatabase
{
public:
    QtCursorDatabase();

    QStringList cursorShapeNames() const;
    QMap<int, QIcon> cursorShapeIcons() const;
    QString cursorToShapeName(const QCursor &cursor) const;
    QIcon cursorToShapeIcon(const QCursor &cursor) const;
    int cursorToValue(const QCursor &cursor) const;
    QCursor valueToCursor(int value) const;
private:
    void appendCursor(Qt::CursorShape shape, const QString &name, const QIcon &icon);
    QStringList m_cursorNames;
    QMap<int, QIcon> m_cursorIcons;
    QMap<int, Qt::CursorShape> m_valueToCursorShape;
    QMap<Qt::CursorShape, int> m_cursorShapeToValue;
};

#endif

