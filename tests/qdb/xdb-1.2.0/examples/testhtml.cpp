/*  $Id: testhtml.cpp,v 1.5 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    This sample program demonstrates the use of the HTML class

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

    V 1.2   11/20/97   -  Initial creation of program
    V 1.5   1/2/98     -  Added memo field support
    V 1.6a  5/1/98     -  Added expression support
*/

#define HTML_PAGE 

#include <xdb/xbase.h>

int main()
{
#ifdef XB_HTML
   xbLong cnt;
   xbHtml Page;

#ifdef HTML_PAGE 
      Page.StartHtmlPage( "Test Web Page Here" );
#else
      Page.StartTextPage();
#endif

   if( Page.PostMethod())
   {
      Page.PrintHtml( "Post Method Used" );
      Page.NewLine();
   }
   else if( Page.GetMethod())
   {
      Page.PrintHtml( "Get Method Used" );
      Page.NewLine();
   }
   else
   {
      Page.PrintHtml( "No Get Method or Post Method used" );
      Page.NewLine();
   }

   Page.ItalicOn();
   Page.PrintHtml( "Sample Italic Data" );
   Page.NewLine();
   Page.ItalicOff();

   Page.BoldOn();
   Page.PrintHtml( "Sample Bold Data" );
   Page.NewLine();
   Page.BoldOff();

   Page.HeaderOn( 1 );
   Page.PrintHtml( "Sample Header Level 1" );
   Page.NewLine();
   Page.HeaderOff( 1 );

   Page.HeaderOn( 2 );
   Page.PrintHtml( "Sample Header Level 2" );
   Page.NewLine();
   Page.HeaderOff( 2 );

   Page.HeaderOn( 3 );
   Page.PrintHtml( "Sample Header Level 3" );
   Page.NewLine();
   Page.HeaderOff( 3 );

   Page.HeaderOn( 4 );
   Page.PrintHtml( "Sample Header Level 4" );
   Page.NewLine();
   Page.HeaderOff( 4 );

   Page.HeaderOn( 5 );
   Page.PrintHtml( "Sample Header Level 5" );
   Page.NewLine();
   Page.HeaderOff( 5 );

   Page.HeaderOn( 6 );
   Page.PrintHtml( "Sample Header Level 6" );
   Page.NewLine();
   Page.HeaderOff( 6 );

   Page.EmphasizeOn();
   Page.PrintHtml( "Sample Emphasize Data" );
   Page.NewLine();
   Page.EmphasizeOff();

   Page.Bullet();
   Page.PrintHtml( "Bullet # 1" );

   Page.Bullet();
   Page.PrintHtml( "Bullet # 2" );

   Page.Bullet();
   Page.PrintHtml( "Bullet # 3" );
   Page.NewLine();
   Page.NewLine();
 
   cnt = Page.Tally( "count.dat" );
   Page.PrintHtml( "This Page has been accessed " );
   Page.PrintHtml( cnt );
   Page.PrintHtml( " times.\n" );
   Page.NewLine();

   Page.PrintHtml( "\n<BR>Auth Type: " );
   Page.PrintHtml( Page.GetEnv( "AUTH_TYPE" ));

   Page.PrintHtml( "\n<BR>Content Length: " );
   Page.PrintHtml( Page.GetEnv( "CONTENT_LENGTH" ));

   Page.PrintHtml( "\n<BR>Content Type: " );
   Page.PrintHtml( Page.GetEnv( "CONTENT_TYPE" ));

   Page.PrintHtml( "\n<BR>HTTP Request Method: " );
   Page.PrintHtml( Page.GetEnv( "HTTP_REQUEST_METHOD" ));

   Page.PrintHtml( "\n<BR>Query String: " );
   Page.PrintHtml( Page.GetEnv( "QUERY_STRING" ));

   Page.PrintHtml( "\n<BR>Remote Addr: " );
   Page.PrintHtml( Page.GetEnv( "REMOTE_ADDR" ));

   Page.PrintHtml( "\n<BR>Remote Host: " );
   Page.PrintHtml( Page.GetEnv( "REMOTE_HOST" ));

   Page.PrintHtml( "\n<BR>HTTP Remote User: " );
   Page.PrintHtml( Page.GetEnv( "REMOTE_USER" ));

   Page.PrintHtml( "\n<BR>Script Filename: " );
   Page.PrintHtml( Page.GetEnv( "SCRIPT_FILENAME" ));

   Page.PrintHtml( "\n<BR>Script Name: " );
   Page.PrintHtml( Page.GetEnv( "SCRIPT_NAME" ));

   Page.PrintHtml( "\n<BR>Server Port: " );
   Page.PrintHtml( Page.GetEnv( "SERVER_PORT" ));

   Page.PrintHtml( "\n<BR>Server Protocol: " );
   Page.PrintHtml( Page.GetEnv( "SERVER_PROTOCOL" ));

   Page.PrintHtml( "\n<BR>Script Path: " );
   Page.PrintHtml( Page.GetEnv( "SCRIPT_PATH" ));

   Page.PrintHtml( "\n<BR>CGI stdin: " );
   Page.PrintHtml( Page.GetEnv( "CGI_STDIN" ));

   Page.PrintHtml( "\n<BR>CGI stdout: " );
   Page.PrintHtml( Page.GetEnv( "CGI_STDOUT" ));

   Page.PrintHtml( "\n<BR>CGI stderr: " );
   Page.PrintHtml( Page.GetEnv( "CGI_STDERR" ));

   Page.PrintHtml( "\n<BR>HTTP User Agent: " );
   Page.PrintHtml( Page.GetEnv( "HTTP_USER_AGENT" ));

   Page.PrintHtml( "\n<BR>HTTP Content Length: " );
   Page.PrintHtml( Page.GetEnv( "HTTP_CONTENT_LENGTH" ));

   Page.PrintHtml( "\n<BR>HTTP Accept: " );
   Page.PrintHtml( Page.GetEnv( "HTTP_ACCEPT" ));

   Page.PrintHtml( "\n<BR>HTTP Host: " );
   Page.PrintHtml( Page.GetEnv( "HTTP_HOST" ));

   Page.PrintHtml( "\n<BR>Request Method: " );
   Page.PrintHtml( Page.GetEnv( "REQUEST_METHOD" ));
   Page.NewLine();
   Page.NewLine();

   Page.PrintHtml( "\n<BR>Dump Array..." );
   Page.DumpArray();
   Page.NewLine();
   Page.NewLine();

   Page.PrintHtml( "\n<BR>Retrieve Data by Field.." );
   /* one way to get the data from a field - field1 */
   Page.PrintHtml( "\n<BR>Field1 = " );
   Page.PrintHtml( Page.GetDataForField( "field1" ));

   /* another way to get the data from a field - field2 */
   Page.PrintHtml( "\n<BR>Field2 = " );
   Page.PrintHtml( Page.GetData( Page.GetArrayNo( "field2" )));
   Page.NewLine();

   Page.EndHtmlPage();

   return 0;
#else
   cout << "\nXB_HTML is not compiled in\n";
#endif
   return 0;
}     
