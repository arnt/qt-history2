#include "qtcursordatabase_p.h"
#include <QApplication>

QtCursorDatabase::QtCursorDatabase()
{
    appendCursor(Qt::ArrowCursor, QApplication::translate("QtCursorDatabase", "Arrow", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-arrow.png"));
    appendCursor(Qt::UpArrowCursor, QApplication::translate("QtCursorDatabase", "Up Arrow", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-uparrow.png"));
    appendCursor(Qt::CrossCursor, QApplication::translate("QtCursorDatabase", "Cross", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-cross.png"));
    appendCursor(Qt::WaitCursor, QApplication::translate("QtCursorDatabase", "Wait", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-wait.png"));
    appendCursor(Qt::IBeamCursor, QApplication::translate("QtCursorDatabase", "IBeam", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-ibeam.png"));
    appendCursor(Qt::SizeVerCursor, QApplication::translate("QtCursorDatabase", "Size Vertical", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-sizev.png"));
    appendCursor(Qt::SizeHorCursor, QApplication::translate("QtCursorDatabase", "Size Horizontal", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-sizeh.png"));
    appendCursor(Qt::SizeFDiagCursor, QApplication::translate("QtCursorDatabase", "Size Backslash", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-sizef.png"));
    appendCursor(Qt::SizeBDiagCursor, QApplication::translate("QtCursorDatabase", "Size Slash", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-sizeb.png"));
    appendCursor(Qt::SizeAllCursor, QApplication::translate("QtCursorDatabase", "Size All", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-sizeall.png"));
    appendCursor(Qt::BlankCursor, QApplication::translate("QtCursorDatabase", "Blank", 0,
                        QApplication::UnicodeUTF8), QIcon());
    appendCursor(Qt::SplitVCursor, QApplication::translate("QtCursorDatabase", "Split Vertical", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-vsplit.png"));
    appendCursor(Qt::SplitHCursor, QApplication::translate("QtCursorDatabase", "Split Horizontal", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-hsplit.png"));
    appendCursor(Qt::PointingHandCursor, QApplication::translate("QtCursorDatabase", "Pointing Hand", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-hand.png"));
    appendCursor(Qt::ForbiddenCursor, QApplication::translate("QtCursorDatabase", "Forbidden", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-forbidden.png"));
    appendCursor(Qt::OpenHandCursor, QApplication::translate("QtCursorDatabase", "Open Hand", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-openhand.png"));
    appendCursor(Qt::ClosedHandCursor, QApplication::translate("QtCursorDatabase", "Closed Hand", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-closedhand.png"));
    appendCursor(Qt::WhatsThisCursor, QApplication::translate("QtCursorDatabase", "What's This", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-whatsthis.png"));
    appendCursor(Qt::BusyCursor, QApplication::translate("QtCursorDatabase", "Busy", 0,
                        QApplication::UnicodeUTF8), QIcon(":/qtpropertybrowser/images/cursor-busy.png"));
}

void QtCursorDatabase::appendCursor(Qt::CursorShape shape, const QString &name, const QIcon &icon)
{
    if (m_cursorShapeToValue.contains(shape))
        return;
    int value = m_cursorNames.count();
    m_cursorNames.append(name);
    m_cursorIcons[value] = icon;
    m_valueToCursorShape[value] = shape;
    m_cursorShapeToValue[shape] = value;
}

QStringList QtCursorDatabase::cursorShapeNames() const
{
    return m_cursorNames;
}

QMap<int, QIcon> QtCursorDatabase::cursorShapeIcons() const
{
    return m_cursorIcons;
}

QString QtCursorDatabase::cursorToShapeName(const QCursor &cursor) const
{
    int val = cursorToValue(cursor);
    if (val >= 0)
        return m_cursorNames.at(val);
    return QString();
}

QIcon QtCursorDatabase::cursorToShapeIcon(const QCursor &cursor) const
{
    int val = cursorToValue(cursor);
    return m_cursorIcons.value(val);
}

int QtCursorDatabase::cursorToValue(const QCursor &cursor) const
{
    Qt::CursorShape shape = cursor.shape();
    if (m_cursorShapeToValue.contains(shape))
        return m_cursorShapeToValue[shape];
    return -1;
}

QCursor QtCursorDatabase::valueToCursor(int value) const
{
    if (m_valueToCursorShape.contains(value))
        return QCursor(m_valueToCursorShape[value]);
    return QCursor();
}



