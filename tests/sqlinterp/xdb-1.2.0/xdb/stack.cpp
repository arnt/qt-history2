/*  $Id: stack.cpp,v 1.3 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

    This file contains logic for handling basic stack functions.

    Copyright (C) 1997  Crypton Technologies, Gary A. Kunkel   
    email - xbase@startech.keller.tx.us
    www   - http://www.startech.keller.tx.us/xbase.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    V 1.0   10/10/97   - Initial release of software
    V 1.5   1/2/97     - Added memo field support
    V 1.6a  4/1/98     - Added expression support
    V 1.6b  4/8/98     - Added expression support
*/

#include <xdb/xbconfig.h>
#include <xdb/xbase.h>

#include <string.h>
#include <stdlib.h>
#include <iostream.h>

#include <xdb/xstack.h>

/*************************************************************************/
xbStack::xbStack( void )
{
   First      = NULL;
   Last       = NULL;
   Free       = NULL;
   StackDepth = 0;;
}
/*************************************************************************/
void xbStack::InitStack( void )
{
   if( !First || !Last ) return;
   if( Free )
   {
      Last->Next     = Free;
      Free->Previous = Last;
   }
   Free  = First;
   First = NULL;
   Last  = NULL;
   StackDepth = 0;
   return;
}
/*************************************************************************/
xbStackElement * xbStack::GetStackElement( void )
{
   xbStackElement * Temp;

   /* check the free chain for an empty Stack element, if not found,
      allocate one from memory */

   if( Free )
   {
      Temp = Free;
      Free = Free->Next;
   }
   else 

    if((( Temp = (xbStackElement *) malloc( sizeof( xbStackElement )))==NULL))
      return NULL;

   memset( Temp, 0x00, sizeof( xbStackElement ));
   return Temp;
}
/*************************************************************************/
void xbStack::FreeStackElement( xbStackElement * e )
{
   e->Previous = NULL;
   e->Next = Free;
   if( Free ) Free->Previous = e;
   Free = e;
}
/*************************************************************************/
xbShort xbStack::Push( void * p )
{
   xbStackElement * Temp;
   if(( Temp = GetStackElement()) == NULL )
      return 102;

   Temp->UserPtr = p;
   if( !First )
   {
      First = Temp;
      Last  = Temp;
      StackDepth = 1;
   }
   else
   {
      Last->Next = Temp;
      Temp->Previous = Last;
      Last = Temp;
      StackDepth++;
   }
   return 0;
}
/*************************************************************************/
void * xbStack::Pop( void )
{
   void * p;
   xbStackElement * Save;

   if( StackDepth == 0 ) 
      return NULL;
   else
   {
      p = Last->UserPtr;
      if( StackDepth == 1 )
      {
         FreeStackElement( First );
         First = NULL;
         Last  = NULL;
      }
      else  /* number of items in Stack must be > 1 */
      {
         Last->Previous->Next = NULL;
         Save = Last;
         Last = Last->Previous;
         FreeStackElement( Save );
      }
      StackDepth--;
      return p;
   }
}
/*************************************************************************/
#ifdef XBASE_DEBUG
void xbStack::DumpStack( void )
{
   xbStackElement * e;
   if( StackDepth == 0 )
   {
      cout << "\nStack is empty...";
      return;
   }

   cout << "\nThere are " << StackDepth << " entries.";
   cout << "\nFirst = " << First << "  Last = " << Last;

   e = First;
   while( e )
   {
      cout << "\n*****************************";
      cout << "\nThis      = " << e;
      cout << "\nNext      = " << e->Next;
      cout << "\nPrevious  = " << e->Previous;
      cout << "\nUser Pointer = " << e->UserPtr;
      e = e->Next;
   }
   cout << "\nFree list follows...";
   e = Free;
   while( e )
   {
      cout << "\n*****************************";
      cout << "\nThis      = " << e;
      cout << "\nNext      = " << e->Next;
      cout << "\nPrevious  = " << e->Previous;
      cout << "\nUser Pointer = " << e->UserPtr;
      e = e->Next;
   }
   return;
}
#endif
/*************************************************************************/
