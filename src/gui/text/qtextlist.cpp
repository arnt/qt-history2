
#include "qtextlist.h"
#include "qtextlist_p.h"
#include "qtextcursor.h"
#include <private/qtextformat_p.h>
#include <qdebug.h>

#define d d_func()

int QTextListPrivate::itemNumber(const QTextBlockIterator &block) const
{
// #####
    return 1;
}

/*!
    \class QTextList qtextlist.h
    \brief A list in a QTextDocument

    \ingroup text

    QTextList represents a list object in a QTextDocument. Lists can
    be created through QTextCursor::createList and queried with
    QTextCursor::currentList.

*/

/*! \internal
 */
QTextList::QTextList()
    : QTextFormatGroup(*(new QTextListPrivate))
{
}

/*!
  \internal
*/
QTextList::~QTextList()
{
}

/*!
  \returns the number of items in the list.
*/
int QTextList::count() const
{
    return d->blocks.count();
}

/*!
  \returns a QTextCursor positioned at the \a i 'th item in this list.
*/
QTextBlockIterator QTextList::item(int i) const
{
    // ###########
    return QTextBlockIterator();
}

/*!
  \fn void QTextList::setFormat(const QTextListFormat &format)

  sets the format of the list to \a format.
*/

/*!
  \fn QTextListFormat QTextList::format() const

  \returns the format of the list.
*/


QString QTextList::itemText(const QTextBlockIterator &blockIt) const
{
    return QString();
#if 0
    if (!list || item < 0)
	return QString::null;

    QTextBlockIterator block = list->d->blocks.at(item - 1);
    if (block.atEnd())
	return QString::null;

    QTextBlockFormat blockFormat = block.blockFormat();
    QTextListFormat listFmt = blockFormat.listFormat();
    if (!listFmt.isValid())
	return QString::null;

    QString result;

    const int style = listFmt.style();

    switch (style) {
	case QTextListFormat::ListDecimal:
	    result = QString::number(item);
	    break;
	    // from the old richtext
	case QTextListFormat::ListLowerAlpha:
	case QTextListFormat::ListUpperAlpha:
	    {
		const char baseChar = style == QTextListFormat::ListUpperAlpha ? 'A' : 'a';

		int c = item;
		while (c > 0) {
		    c--;
		    result.prepend(QChar(baseChar + (c % 26)));
		    c /= 26;
		}
	    }
	    break;
	default:
	    Q_ASSERT(false);
    }
    if (blockFormat.direction() == QTextBlockFormat::RightToLeft)
	return result.prepend(QChar('.'));
    return result + QChar('.');
#endif
}

