/****************************************************************************
** $Id$
**
** Implementation of QTestPopupMenu class
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

#define INCLUDE_MENUITEM_DEF
#include "qtestpopupmenu_p.h"
#include <qregexp.h>
#include "qtestcontrol_p.h"

/*!
  \class QTestPopupMenu qtestpopupmenu.h
  \brief The QTestPopupMenu class is an extension to QPopupMenu and introduces
  functionality for testing a QPopupMenu.
  
   <strong>Groups of functions:</strong>
  <ul>

  <li> Conversion:
	index2MousePos(),
	mousePos2Index().

  </ul>
*/

/*!
	Converts a string \a menu_text into a mouse position \a mouse_pos on which that 
	text can be found.
	Returns TRUE if the conversion is valid.
*/

bool QTestPopupMenu::index2MousePos(const QString& menu_text, QPoint &mouse_pos)
{
    QMenuItem *mi;
    int y = 0;
    bool use_regexp = FALSE;
    QRegExp menu_text_pattern;

    if (menu_text.find (regexp_magic) == 0) {
        menu_text_pattern = QRegExp (menu_text.mid (sizeof regexp_magic - 1));
        use_regexp = TRUE;
    }
    
	QMenuItemListIt it( *mitems );
    while ( (mi=it.current()) ) {

		++it;
		if (use_regexp)
        {
			if (menu_text_pattern.search(mi->text(), 0) >= 0)
				return index2MousePos(y,mouse_pos);
        } else {
            if (menu_text == mi->text ())
				return index2MousePos(y,mouse_pos);
        }
		
		y++;
    }

	return FALSE;
}

/*!
	Converts a menu item referred to by \a index into a mouse position \a mouse_pos.
	Returns TRUE if the conversion is valid.
*/

bool QTestPopupMenu::index2MousePos(int index, QPoint &mouse_pos)
{
    QMenuItem *mi;
    int row = 0;
    int x = contentsRect().x();
    int y = contentsRect().y();
    int itemw = contentsRect().width() / columns ();
    QMenuItemListIt it( *mitems );

    while ( (mi=it.current()) ) {
		++it;
		int itemh = itemHeight( mi );
		if ( columns () > 1 && y + itemh > contentsRect().bottom() ) {
			y = contentsRect().y();
			x +=itemw;
		}
		if ( row == index ) {
			mouse_pos.setY(y + 3);
            int pm_inner_width = contentsRect().width() - 2 - frameWidth();
            if (mouse_pos.x() > pm_inner_width)
                mouse_pos.setX(pm_inner_width);
			return TRUE;
		}

		y += itemh;
		++row;
    }

    return FALSE;
}

/*!
	Converts a mouse position \a pos into a \a index referencing a menu item.
	Returns TRUE if the conversion is valid.
*/

bool QTestPopupMenu::mousePos2Index( const QPoint &pos, int &index )
{
    if ( !contentsRect().contains(pos) )
		return FALSE;

    int row = 0;
    int x = contentsRect().x();
    int y = contentsRect().y();
    int itemw = contentsRect().width() / columns ();

    QMenuItem *mi;
    QMenuItemListIt it( *mitems );
    while ( (mi=it.current()) ) {
		++it;
		int itemh = itemHeight( row );
		if ( columns () > 1 && y + itemh > contentsRect().bottom() ) {
			y = contentsRect().y();
			x +=itemw;
		}
		if ( QRect( x, y, itemw, itemh ).contains( pos ) )
			break;
		y += itemh;
		++row;
    }

    if ( mi && !mi->isSeparator() ) {
		index = row;
		return TRUE;
	}

    return FALSE;
}
