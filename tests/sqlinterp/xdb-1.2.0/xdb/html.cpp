/*  $Id: html.cpp,v 1.6 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

    This file contains the basic Xbase routines for creating html

    Copyright (C) 1997  StarTech, Gary A. Kunkel
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

    V 1.0      10/10/97   - Initial release of software
    V 1.5      1/2/98     - Added memo field support
    V 1.6a     4/1/98     - Added expression support
    V 1.6b     4/8/98     - Numeric index keys
    V 1.7.4.c  7/7/98     - Continued development on GenFormFields method
*/

#include <xdb/xbase.h>
#ifdef XB_HTML

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>             /* BSDI BSD/OS 3.1 */

#include <xdb/xbexcept.h>

/************************************************************************/
void xbHtml::TextOut( const char * String )
{
   cout << String;
}
/************************************************************************/
void xbHtml::InitVals( void )
{
   FieldNameArray = NULL;
   DataValueArray = NULL;
   NoOfDataFields = 0;
   HtmlBufLen     = 0;
   HtmlWorkBuf    = NULL;
   LoadArray();
}
/************************************************************************/
xbHtml::xbHtml( void )
{
	InitVals();
}   
/************************************************************************/
void xbHtml::DumpArray( void )
{
   /* dont forget Content-type  */

   xbShort i;
   if( NoOfDataFields == 0 )
      cout << "No Input Data From Form\n";
   else
   {
      cout << "There are " << NoOfDataFields << " fields";
      cout << "<BR>" << NoOfDataFields << " Field Name / Data Values received\n";
      cout << "<BR>-----------------------------------\n";
      for( i = 0; i < NoOfDataFields; i++ )
      {
         cout << "<br>" << FieldNameArray[i] << " => ";
//       PrintEncodedString( DataValueArray[i] );
         if( DataValueArray[i] )
            cout << DataValueArray[i];
      }
   }
}
/***********************************************************************/
char * xbHtml::GetData( xbShort pos )
{
	if( pos < NoOfDataFields && pos >= 0 )
		return DataValueArray[pos];
	else
		return NULL;
}
/************************************************************************/
xbShort xbHtml::GetArrayNo( const char * FieldName )
{
   xbShort i;
   for( i = 0; i < NoOfDataFields; i++ )
      if( strcmp( FieldName, FieldNameArray[i] ) == 0 )
         return i;
   return -1;
}
/************************************************************************/
char * xbHtml::GetDataForField( const char * FieldName )
{
	return( GetData( GetArrayNo( FieldName )));
}
/************************************************************************/
void xbHtml::LoadArray( void )
{
   xbShort Len;               /* length of data string from form */
   xbShort Len1, Len2;        /* tokenized string lengths        */
   xbShort i;
   char  *FormData;         /* actual data from form           */
   char  *QueryStringPtr;
   char  *Token;
   char  **Names, **Values;
   
   if( !getenv( "REQUEST_METHOD" )) return;
 
   /* put the input data into field FormData */
   if( !strcmp( "POST", getenv( "REQUEST_METHOD" )))
   {
      if(( Len = atoi( getenv( "CONTENT_LENGTH" ))) == 0 )
          return;
      FormData = (char *) malloc( sizeof( char ) * ( Len + 1 ));
      fgets( FormData, Len+1, stdin );
   }
   else   /* GET */
   {
      if(( QueryStringPtr = getenv( "QUERY_STRING" )) == NULL ) return;
      Len = strlen( QueryStringPtr );
      if( Len == 0 ) return;
      FormData = (char *) malloc( sizeof( char ) * ( Len + 1 ));
      strcpy( FormData, QueryStringPtr );
   }

   /* count the number of data fields,  & is the field separator  */
   Token = strchr( FormData, '&' );
   NoOfDataFields++;
   while( Token != NULL )
   {
      NoOfDataFields++;
      Token++;
      Token = strchr( Token, '&' );
   }
    
   Names  = (char **)malloc(sizeof(char*)*NoOfDataFields);
   Values = (char **)malloc(sizeof(char*)*NoOfDataFields);

   for( i =0, Token=strtok(FormData, "&"); Token != NULL; i++ )
   {
      Len1 = strlen( Token );
      Len2 = strcspn( Token, "=" );
      Names[i] = (char *) malloc( sizeof(char) * (Len2 + 1));
      strncpy( Names[i], Token, Len2 );
      Names[i][Len2] = 0x00;
      DeleteEscChars( Names[i] );
      if( Len1 != Len2+1 )
      {
	 Values[i] = (char *)malloc(sizeof(char) * (Len1 - Len2));
         strcpy( Values[i], Token+Len2+1 );
         DeleteEscChars( Values[i] );
      }
      else  /* no data for field  name=& */
         Values[i] = NULL;
      Token = strtok( NULL, "&" );
   }
   free( FormData );
   FieldNameArray = Names;
   DataValueArray = Values;
}
/************************************************************************/
void xbHtml::DeleteEscChars( char * String )
{
   xbShort s,t;         /* source && target */
   char  HexVal[3];   
   xbShort EscCnt = 0;

   for( s=0, t=0; String[s]; s++, t++ )
   {
      if( String[s] == '+' )
	      String[t] = ' ';
      else if( String[s] == '%' )
      {
         HexVal[0] = String[s+1];
         HexVal[1] = String[s+2];
         HexVal[2] = 0x00;
         String[t] = strtol( HexVal, NULL, 16 );
         s+=2;
         EscCnt++;
      }
      else
			String[t] = String[s];
   }
   for( t = strlen( String ) - 1; t > 1 && EscCnt > 0; t-=2, EscCnt-- )
   {
      String[t]   = 0x20;
      String[t-1] = 0x20;
   }
}
/************************************************************************/
xbLong xbHtml::Tally( const char * File )
{
/* FIXME:  Locking works under Unix only: <fcntl.h> does not warrant fcntl() 
   Noticed with Borland 4.5.  Current workaround effectively disables locking
   even with XB_LOCKING_ON -- willy */
/* must have write access to the directory for this routine to work */

#if defined(HAVE_FCNTL) && defined(XB_LOCKING_ON)
	struct flock fl;
#endif

	xbLong cnt;
	FILE *f;
	xbShort rc;

	if(( f = fopen( File, "r+" )) == NULL )
	{
		/* land here if file does not exist - initialize it */
		if(( f = fopen( File, "w+" )) == NULL ) return 0L;
		rc = fprintf( f, "%08lu\n", 1L );
		fclose( f );
		if( rc == EOF )
			return 0L;
		else
			return 1L;
	}

/* lock the file */
#if defined(HAVE_FCNTL) && defined(XB_LOCKING_ON)
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0L;
	fl.l_len = 1L;
	fcntl( fileno( f ), F_SETLKW, &fl );
#endif

	fseek( f, 0L, SEEK_SET );
	fscanf( f, "%08lu", &cnt );
	fseek( f, 0L, SEEK_SET );
	rc = fprintf( f, "%08lu\n", ++cnt );

/* unlock the file */
#if defined(HAVE_FCNTL) && defined(XB_LOCKING_ON)
	fl.l_type = F_UNLCK;
	fcntl( fileno( f ), F_SETLKW, &fl );
#endif

	fclose( f );
	return cnt;
}
/************************************************************************/
xbShort xbHtml::PostMethod( void )
{
   char s[5];
   xbShort i;
   if( !getenv( "REQUEST_METHOD" )) 
      return 0;
   else
   {
      memset( s, 0x00, 5 );
      strncpy( s, getenv( "REQUEST_METHOD" ), 4 );
      for( i = 0; i < 5; i++) s[i] = toupper( s[i] );
      if( strcmp( s, "POST" ) == 0 ) return 1;
   }
   return 0;
}
/************************************************************************/
xbShort xbHtml::GetMethod( void )
{
   char s[4];
   xbShort i;

   if( !getenv( "REQUEST_METHOD" )) 
      return 0;
   else
   {
      memset( s, 0x00, 4 );
      strncpy( s, getenv( "REQUEST_METHOD" ), 3 );
      for( i = 0; i < 4; i++) s[i] = toupper( s[i] );
      if( strcmp( s, "GET" ) == 0 ) return 1;
   }
   return 0;
}
/************************************************************************/
xbShort xbHtml::GenFormFields(xbDbf *d, xbShort Option, const char *Title, 
															xbFieldList *fl) {
   xbShort i;
   char  buf[255];

   cout << "\n<TABLE>";
   if( Title )
      cout << "\n<CAPTION ALIGN=\"TOP\">" << Title << "</CAPTION><BR>";
   
   i = 0;
   while( fl[i].FieldLen != 0 )
   {
      cout << "\n<BR><TR><TH ALIGN=\"LEFT\">" << fl[i].Label;
      if( fl[i].Option == 2 )
      {
         if( !d )
					 xb_error(XB_NOT_OPEN);
         d->GetField( fl[i].FieldNo, buf );
         cout << "<TD>" << buf;
      }
      else
      {
         cout << "<TD><INPUT TEXT NAME = \"" << fl[i].FieldName << "\"";
         cout << " size = " << fl[i].FieldLen;
     
         if( fl[i].Option == 1 )
            cout << " TYPE=\"password\" ";

         cout << " value = ";
         if( Option == 1 )
         {
	    if( !d )
	       xb_error(XB_NOT_OPEN);
            d->GetField( fl[i].FieldNo, buf );
            cout << "\"" << buf << "\"";
         }
         cout << ">";
      }  
      i++;
   }
   cout << "\n</TABLE>";
   return XB_NO_ERROR;
}
/************************************************************************/
void xbHtml::StartHtmlPage( const char * Title )
{
   cout << "Content-type: text/html\n\n";
   cout << "\n<HTML><HEAD><TITLE>" << Title << "</TITLE></HEAD><BODY>";
}
/************************************************************************/
void xbHtml::PrintEncodedChar( char c )
{
   switch( c )
   {
      case '<':  cout << "&lt;"; break;
      case '>':  cout << "&gt;"; break;
      case '&':  cout << "&amp;"; break;
      case '"':  cout << "&quot;"; break;
      default: cout << c; break;  
   }
   return;
}
/************************************************************************/
void xbHtml::PrintEncodedString( const char * s )
{
   const char *p;
   p = s;
   while( *p ) PrintEncodedChar( *p++ );
   return;
}
/************************************************************************/
xbShort xbHtml::SetCookie( const char * Name,const char * Value, 
   const char * ExpDate, const char * ExpTime, const char * TimeZone, 
   const char * Path, const char * Domain, xbShort Secure )
{
   if(
      ( !Name || !Value ) ||
      ( ExpDate && !TimeZone)
     )
	   xb_error(XB_INVALID_OPTION);

   cout << "\nSet-Cookie: " << Name << "=" << Value << ";";
   if( ExpDate )
   {
      cout << ExpDate << ";";
      if( ExpTime )
         cout << ExpTime;
      else
         cout << "00:00:00";
      cout << TimeZone << ";";
   }
   if( Path )
      cout << "\nPath=" << Path << ";";
   if( Domain )
      cout << "domain=" << Domain << ";";
   if( Secure )
      cout << "Secure";
   cout << "\n";
   return XB_NO_ERROR;
}
/************************************************************************/
void xbHtml::SpaceToPlus( char * String )
{
   char * p;
   p = String;
   while( *p )
   {
      if( *p == ' ' ) *p = '+';
      p++;
   }
   /* eliminate trailing white space */
   p--;
   while( *p == '+' && p > String )
   {
      *p = 0x00;
      p--;
   }
}
/************************************************************************/
void xbHtml::SendRedirect( char * url ) const
{  
   cout << "Location: " << url << endl << endl; 
}
/************************************************************************/
void xbHtml::PlusToSpace( char * String )
{
   char * p;
   p = String;
   while( *p )
   {
      if( *p == '+' ) *p = ' ';
      p++;
   }
}
/************************************************************************/
const char * xbHtml::GetCookie( const char * CookieName )
{
   char * CookieData;
   char * TempPtr;
   char * q;
   xbShort len;
   if(( CookieData = GetEnv( "HTTP_COOKIE" )) == NULL )
      return NULL;
   len = strlen( CookieName );
   len += 2;
   if(( TempPtr = (char *) malloc(len)) == NULL )
      return NULL;
   strcpy( TempPtr, CookieName );
   strcat( TempPtr, "=" );
   if(( q = strstr( CookieData, TempPtr )) == NULL )
   {
      free( TempPtr );
      return NULL;
   }
   free( TempPtr );
   len--;
   q += len;
   TempPtr = q;
   len = 0;
   while( *q && *q != ';' )
   {
      len++;
      q++;
   }
   len++;
   if( len > HtmlBufLen )
   {
      if( HtmlBufLen )
         free( HtmlWorkBuf );
      if(( HtmlWorkBuf = (char *) malloc( len )) == NULL )
         return NULL;
   }
   memset( HtmlWorkBuf, 0x00, len );
   q = HtmlWorkBuf;
   while( *TempPtr && *TempPtr != ';' )
      *q++ = *TempPtr++;
   return HtmlWorkBuf;
}
/************************************************************************/
#endif             /* XB_HTML */
