/****************************************************************************
** $Id$
**
** Implementation of QTestListBox class
**
** Created : 010301
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt TestFramework of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtestlistbox_p.h"
#include <stdlib.h>
#include <qregexp.h>
#include "qtestcontrol_p.h"
#include <assert.h>

/*!
  \class QTestListBox qtestlistbox.h
  \brief The QTestListBox class is an extension to QListBox and introduces
  functionality for testing a QListBox.
  	
   <strong>Groups of functions:</strong>
  <ul>

  <li> Construction:
	index2MousePos().

  </ul>
*/

/*!
    Converts a ListBox item specified by \a listItem to a 
    position \a mouse_pos.  
    The function returns TRUE if the conversion is valid.
*/

bool QTestListBox::index2MousePos (int listItem, QPoint& mouse_pos)
{
    assert (listItem >= 0);

    if ( ! itemVisible(listItem))
        setTopItem(listItem);

    QListBoxItem* i = item(listItem);
    if ( i ) {

	// return the top coordinate of the rectangle that is occupied by the listbox item
	int yPos = itemRect(i).y();
	mouse_pos.setY(yPos + 2);
	return TRUE;

    } else {
	return FALSE;
    }
}

/*!
    \overload
    The mouse position returned is the line closest to \a listItem that matches 
    \a itemText most closely.
*/

bool QTestListBox::index2MousePos(const QString& itemText, int listItem, QPoint& mouse_pos)
{
    assert ( ! itemText.isEmpty ());
    assert (listItem >= 0);

    bool was_text_found = FALSE;
    if ( ! itemText.isEmpty ())
    {
        bool use_regexp = FALSE;
        QRegExp text_pattern;
        if (itemText.find (regexp_magic) == 0)
        {
            text_pattern = QRegExp (itemText.mid (sizeof regexp_magic - 1));
            use_regexp = TRUE;
        }

	// First do a test on the specified item directly
	if (listItem >= 0 && listItem < (int)count()) {
	    if (use_regexp)
		was_text_found = (text_pattern.search(text(listItem),0) >= 0);
	    else
		was_text_found = (itemText == text (listItem));
	    if (was_text_found)
		return index2MousePos(listItem,mouse_pos);
	}

	// If the item didn't match, loop trough the whole list until we find a match
	// that is closest to the specified item's position
        int min_diff = INT_MAX;
	for (uint i=0; i < count(); ++i)
	{
	    bool does_text_match;
	    if (use_regexp)
		does_text_match = (text_pattern.search(text(i),0) >= 0);
	    else
		does_text_match = (itemText == text (i));

	    if (does_text_match) {
		if (min_diff > abs ((int)(i - listItem))) // we want the one closest to 'item'
		{
		    if (!index2MousePos(i,mouse_pos))
			return FALSE;

		    was_text_found = TRUE;
		    min_diff = abs ((int)(i - listItem));
		} else {
		    if (was_text_found)
			break; // we have found the closest matching item so quit the loop
		}
	    }
	}
    }

    return was_text_found;
}
