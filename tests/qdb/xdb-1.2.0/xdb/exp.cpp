/*  $Id: exp.cpp,v 1.7 1999/03/19 10:56:33 willy Exp $

    Xbase project source code

    This file contains logic for handling Xbase expressions.

    Copyright (C) 1997  Startech, Gary A. Kunkel   
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

    V 1.0    10/10/97   - Initial release of software
    V 1.5    1/2/97     - Added memo field support
    V 1.6a   4/1/98     - Added expression support
    V 1.6b   4/8/98     - Numeric index keys
    V 1.7.1  5/25/98    - Enhanced expression support
    V 1.8.0a 1/27/99    - Release v1.8 upgrade
*/

#include <xdb/xbase.h>
#ifdef XB_EXPRESSIONS
#include <ctype.h>
#include <string.h>

#include <xdb/xbexcept.h>

// set the default date format
xbString xbExpn::DefaultDateFormat = "MM/DD/YY";

/************************************************************************/
/* putting this part in EXP did not work  */

static xbFuncDtl FuncList[] =
{
   /* Func      # of   Return
      Name      parms   Type */  
   { "ABS",       1,    'N' },
   { "ASC",       1,    'N' },
   { "AT",        2,    'N' },
   { "CDOW",      1,    'C' },
   { "CHR",       1,    'C' },
   { "CMONTH",    1,    'C' },
   
   { "DATE",      0,    'C' },
   { "DAY",       1,    'N' },
   { "DESCEND",   1,    'C' },
   { "DOW",       1,    'N' },
   { "DTOC",      1,    'C' },
   { "DTOS",      1,    'C' },
     
   { "EXP",       1,    'N' },
   { "IIF",       3,    'C' },
   { "INT",       1,    'N' },
   { "ISALPHA",   1,    'L' },
   { "ISLOWER",   1,    'L' },
   { "ISUPPER",   1,    'L' },
   { "LEFT",      2,    'C' },
   { "LEN",       1,    'N' },
   { "LOG",       1,    'N' },
   { "LOWER",     1,    'C' },
   { "LTRIM",     1,    'C' },
   { "MAX",       2,    'N' },
   { "MIN",       2,    'N' },
   { "MONTH",     1,    'N' },  
   { "RECNO",     0,    'N' }, 
   { "RECNO",     0,    'L' }, 
   { "REPLICATE", 2,    'C' },
   { "RIGHT",     2,    'C' },
   { "RTRIM",     1,    'C' },
   { "SPACE",     1,    'C' },
   { "SQRT",      1,    'N' },
   { "STR",       1,    'C' },
   { "STR",       2,    'C' },
   { "STR",       3,    'C' },
   { "STRZERO",   1,    'C' },
   { "STRZERO",   2,    'C' },
   { "STRZERO",   3,    'C' },
   { "SUBSTR",    3,    'C' },
   { "TRIM",      1,    'C' },
   { "UPPER",     1,    'C' },
   { "VAL",       1,    'N' },
   { "YEAR",      1,    'N' },
   { 0, 0, 0 },
};
/*************************************************************************/
xbExpn::xbExpn( void )
{
   TokenType       = 0x00;
   NextFreeExpNode = NULL;
   Tree            = NULL;
   TokenLen        = 0;
   OpLen1          = 0;
   OpLen2          = 0;
   OpDataLen1      = 0;
   OpDataLen2      = 0;
   Op1             = NULL;
   Op2             = NULL;
   XbaseFuncList   = FuncList;
   memset( WorkBuf, 0x00, WorkBufMaxLen+1 );
}
/*************************************************************************/
xbShort xbExpn::GetFuncInfo( const char * Function, xbShort Option )
{
/*  Option = 0 - return array offset no
             1 - return number of needed parms
             2 - return function result type
             ? - return 0 if valid function
*/
   xbFuncDtl * f;
   xbShort i, len;
   const char *s;

   s = Function;
   len = 0;
   while( *s && *s != '(' ) { s++; len++; }

   f = XbaseFuncList;
   i = 0;
   while( f[i].FuncName )
   {
      if( strncmp( f[i].FuncName, Function, len ) == 0 )
      {
         if( Option == 0 ) return i;
         if( Option == 1 ) return f[i].ParmCnt;
         if( Option == 2 ) return f[i].ReturnType;
         return 0;
      }
      i++;
   }

   cout << "\nGetFuncInfo error function = " << Function << "\n";
   return -1;
}
/*************************************************************************/
xbShort xbExpn::IsWhiteSpace( char c )
{
   if( c == 0x20 ) 
      return 1;
   else
      return 0;
}
/*************************************************************************/
xbShort xbExpn::GetNextToken( const char * s, xbShort MaxLen )
{
   /* TreeResultType Settings 
          Token        Action/
     Was   Type        Result
     Unv    N            N
     Unv    C            C
     Unv    Function     Table Lookup
     Unv    Field        Field Type
     Not L  Any Logical  L
   */

   xbShort Wctr, Wtype, Wsw, EmptyCtr, MaxCtr, MaxCtrSave;
   const char *sp, *np, *pp;   /* save, next and previous pointer */

   LogicalType = 0;
   TokenType   = 0;
   TokenLen    = 0;
   EmptyCtr    = 0;
   MaxCtr      = 0;

   if( !s || ! *s )
		 xb_error(XB_NO_DATA);

   /* go past any initial white space */
   while( s && *s && IsWhiteSpace( *s ))
   {
      s++;
      MaxCtr++;
      if (MaxCtr >= MaxLen)
				return XB_NO_ERROR;
   }

/* 1 check for parens */
/* '(', if found go to corresponding ')', if no ')', return -1 */
   if( *s == '(' || *s == '{' )
   {
      if( *s == '{' ) Wtype = 0; else Wtype = 1;
      Wctr = 1;
      s++;

      MaxCtr++;
      if( MaxCtr >= MaxLen )
				xb_error(XB_PARSE_ERROR);

      while( s && *s )
      {
         if(( *s == ')' && Wtype == 1 ) || (*s == '}' && Wtype == 0 ))
         {
            Wctr--;
            if( Wctr == 0 )
            {
               if( EmptyCtr != 0 ) {
                  TokenType = 'E';
                  PreviousType = 'E';
               } else
	          xb_error(XB_PARSE_ERROR);

               TokenLen += 2;
               return XB_NO_ERROR;
            }
         }
         else if(( *s == '(' && Wtype == 1 ) || (*s == '{' && Wtype == 0 ))
         {
            Wctr++;
            EmptyCtr++;
         }
         else if( *s != ' ' )
            EmptyCtr++;
         s++;
         TokenLen++;
         MaxCtr++;
         if( MaxCtr >= MaxLen )
           xb_error(XB_PARSE_ERROR);
      }
      xb_error(XB_PARSE_ERROR);
   }

/* 2 - Check for Constants */
/* check for "'" or """, if no corresponding quote return -1 */
   if( *s == '"' || *s == '\'' )
   {
      if( *s == '"' ) Wtype = 0; else Wtype = 1;
      TokenType  = 'C';               /* set to constant  */
      PreviousType = 'C';
//      if( !TreeResultType ) TreeResultType = 'C';
      s++;

      MaxCtr++;
      if (MaxCtr >= MaxLen)
        return XB_NO_ERROR;

      while( s && *s )
      {
         if(( *s == '"' && Wtype == 0 ) || (*s == '\'' && Wtype == 1 ))
            return XB_NO_ERROR;
         s++;
         TokenLen++;
         MaxCtr++;
         if (MaxCtr >= MaxLen)
           return XB_NO_ERROR;
      }
      xb_error(XB_PARSE_ERROR);
   }

/* check for .T. .F. .TRUE. or .FALSE.  */

   if( s && *s && *s == '.' )
   {
      if(( strncmp( s, ".T.", 3 ) == 0 ) || ( strncmp( s, ".F.", 3 ) == 0 ))
      {
         TokenLen   = 3;
         TokenType  = 'C';      /* constant */
         PreviousType = 'C';
         LogicalType = 1;
//         if( !TreeResultType ) TreeResultType = 'L';
         return XB_NO_ERROR;
      }
      else if( strncmp( s, ".TRUE.", 6 ) == 0 )
      {
         TokenLen   = 6;
         TokenType  = 'C';      /* constant */
         PreviousType = 'C';
         LogicalType = 1;
//         if( !TreeResultType ) TreeResultType = 'L';
         return XB_NO_ERROR;
      }
      else if( strncmp( s, ".FALSE.", 7 ) == 0 )
      {
         TokenLen   = 7;
         TokenType  = 'C';      /* constant */
         PreviousType = 'C';
         LogicalType = 1;
//         if( !TreeResultType ) TreeResultType = 'L';
         return XB_NO_ERROR;
      }
   }

/* check for positive, negative or decimal number constants */
   if(( *s == '-' && ( PreviousType == 'O' || PreviousType == 0 )) ||
      ( *s == '+' && ( PreviousType == 'O' || PreviousType == 0 )) ||
        *s == '.' || isdigit( *s ))
   {
      sp = s;
      MaxCtrSave = MaxCtr;
      Wsw = Wctr = 0;
      if( *s == '.' )
      {
         Wctr++;
         s++;
         MaxCtr++;
         if (MaxCtr >= MaxLen)
           xb_error(XB_PARSE_ERROR);

         if( s && *s && isdigit( *s ))
            TokenLen++;
         else
            Wsw++;
      }
      else if( *s == '-' )
      {
         s++;
         TokenLen++;
         MaxCtr++;
         if( MaxCtr >= MaxLen )
           xb_error(XB_PARSE_ERROR);

         /* go past any white space between sign and number */
         while( s && *s && IsWhiteSpace( *s )) 
         {
            s++;
            TokenLen++;
            MaxCtr++;
            if( MaxCtr >= MaxLen )
              xb_error(XB_PARSE_ERROR);
         }
      }

      if( isdigit( *s ) || (*s == '.' && !Wsw ))
      {
         while(s && *s && ((*s == '.' && Wctr < 2 ) || isdigit(*s)) && !Wsw )
         {
            if( *s == '.' ) 
            {
               Wctr++; 
               if( Wctr > 1 ) break;
               s++;

               MaxCtr++;
               if( MaxCtr >= MaxLen ) 
               {
                  TokenType = 'N';
                  PreviousType = 'N';
 //                 if( !TreeResultType ) TreeResultType = 'N';
                  return XB_NO_ERROR;
               }

               if( s && *s && isdigit( *s ))
                  TokenLen++;
               else
                  Wsw++;
            }
            else
            {
               s++;
               TokenLen++;
               MaxCtr++;
               if( MaxCtr >= MaxLen ) 
               {
                  TokenType = 'N';
                  PreviousType = 'N';
   //               if( !TreeResultType ) TreeResultType = 'N';
                  return XB_NO_ERROR;
               }
            }
         }
         TokenType  = 'N';      /* constant */
         PreviousType = 'N'; 
     //    if( !TreeResultType ) TreeResultType = 'N';
         return XB_NO_ERROR;
      }
      else
      {
         s = sp;
         MaxCtr = MaxCtrSave;
      }
   }

/* 2) Check for operators */
#if 0
   if( *s == '+' || *s == '-' || *s == '/' || *s == '=' ||
       *s == '$' || *s == '^' || *s == '#' )
   {
      TokenLen = 1;
      TokenType = 'O';
      PreviousType = 'O';
      return XB_NO_ERROR;
   }
#else  // changed 3/25/00 dtb
   if( *s == '+' || *s == '-' || *s == '/' || *s == '^')
   {
      TokenLen = 1;
      TokenType = 'O';
      PreviousType = 'O';
      return XB_NO_ERROR;
   }
   if(*s == '=' || *s == '$' || *s == '#' )
   {
      LogicalType = 1;
      TokenLen = 1;
      TokenType = 'O';
      PreviousType = 'O';
      return XB_NO_ERROR;
   }
#endif
   if( *s == '*' )
   {
      s++;

      MaxCtr++;
      if( MaxCtr >= MaxLen )
        xb_error(XB_PARSE_ERROR);

      TokenType = 'O';
      PreviousType = 'O';
      if( *s == '*' )
      {
         TokenLen = 2;
         return XB_NO_ERROR;
      }
      else
      {
         TokenLen = 1;
         return XB_NO_ERROR;
      }
   }
   if( *s == '<' || *s == '>' )
   {
      s++;

      MaxCtr++;
      if( MaxCtr >= MaxLen )
        xb_error(XB_PARSE_ERROR);

      LogicalType = 1;  // added 3/25/00 dtb
      TokenType = 'O';
      PreviousType = 'O';
      if( *s == '<' || *s == '>' || *s == '=' )
      {
         TokenLen = 2;
         return XB_NO_ERROR;
      }
      else
      {
         TokenLen = 1;
         return XB_NO_ERROR;
      }
  } 

/* check for .NOT. .OR. .AND. */

   if( s && *s && *s == '.' )
   {
      if( strncmp( s, ".NOT.", 5 ) == 0 )
      {
         TokenLen   = 5;
         TokenType  = 'O';      /* constant */
         PreviousType = 'O';
         LogicalType = 1;
//         if( TreeResultType != 'L' ) TreeResultType = 'L';
         return XB_NO_ERROR;
      }
      else if( strncmp( s, ".AND.", 5 ) == 0 )
      {
         TokenLen   = 5;
         TokenType  = 'O';      /* constant */
         PreviousType = 'O';
         LogicalType = 1;
//         if( TreeResultType != 'L' ) TreeResultType = 'L';
         return XB_NO_ERROR;
      }
      else if( strncmp( s, ".OR.", 4 ) == 0 )
      {
         TokenLen   = 4;
         TokenType  = 'O';      /* constant */
         PreviousType = 'O';
         LogicalType = 1;
//         if( TreeResultType != 'L' ) TreeResultType = 'L';
         return XB_NO_ERROR;
      }
   } 

   /* If get this far, must be function or database field */
//   sp = s;   /* for GetFuncInfo */
   while( s && *s )
   {
      s++;
      TokenLen++;
      MaxCtr++;
      if( MaxCtr >= MaxLen )
      {
         TokenType = 'D';
         PreviousType = 'D';
         return XB_NO_ERROR;
      }

      if( s && *s && *s == '(' )
      {
         /* look for corresponding ) */
         Wctr = 1;
         s++;
         TokenLen++;
         MaxCtr++;
         if( MaxCtr >= MaxLen )
           xb_error(XB_PARSE_ERROR);
      
         while( s && *s )
         {
            if( *s == ')' )
            {
               Wctr--;
               if( !Wctr )
               {
                  TokenType = 'F';     /* function */
                  PreviousType = 'F';
                  TokenLen++;
 //                 TreeResultType = GetFuncInfo( sp, 2 );
                  return XB_NO_ERROR;
               }
            }
            if( *s == '(' ) Wctr++;
            s++;
            TokenLen++;
            MaxCtr++;
            if( MaxCtr >= MaxLen )
              xb_error(XB_PARSE_ERROR);
         }
         xb_error(XB_PARSE_ERROR);
      }
      else 
      {
         np = s + 1;
         pp = s - 1;
         if( !s || !*s || 
             (IsSeparator( *s ) && 
             !(*s == '-' && *np == '>' )  &&
             !(*s == '>' && *pp == '-' )))
         {
            if(TokenLen > 0) 
            {
               TokenType = 'D';      /* database field */
               PreviousType = 'D';
               return XB_NO_ERROR;
            }
         }
      }
   }
   return XB_NO_ERROR;
}
/*************************************************************************/
char xbExpn::IsSeparator( char c )
{
   if( c == '-' || c == '+' || c == '*' || c == '/' || c == '$' ||
       c == ' ' || c == '#' || c == '<' || c == '>' || c == '^' ||
       c == '=' || c == '.' /* || c == ')' */ )
      return c;
   else
      return 0;
}
/*************************************************************************/
xbExpNode * xbExpn::GetExpNode(xbShort Len) {
   xbExpNode * Save;
   xbExpNode * Temp;
   xbShort   SaveLen;
   char    * SavePtr;


   if( NextFreeExpNode )
   {
      Save = NULL;
      Temp = NextFreeExpNode;
      while( Temp->Node && Len > Temp->ResultLen )
      {
         Save = Temp;
         Temp = Temp->Node;
      }
      /* check for end of chain */
      if( Len <= Temp->ResultLen )
      {
         if( Save )
            Save->Node = Temp->Node;
         else
            NextFreeExpNode = Temp->Node;

         SaveLen = Temp->ResultLen;
         SavePtr = Temp->Result;
         memset( Temp, 0x00, sizeof( xbExpNode ));
         Temp->ResultLen = SaveLen;
         Temp->Result = SavePtr;

         return Temp;
      }
      /* if none loaded w/ big enough result areas */
      if( NextFreeExpNode->ResultLen == 0 )
      {
         Temp = NextFreeExpNode;
         NextFreeExpNode = Temp->Node;
         memset( Temp, 0x00, sizeof( xbExpNode ));
         if(( Temp->Result = (char *) malloc( Len )) == NULL ) return NULL;
         memset( Temp->Result, 0x00, Len );
         Temp->ResultLen = Len;
         return Temp;
      }
   }

   /* get here if it couldn't use an already allocated but unused node */
   if(( Temp = (xbExpNode *) malloc( sizeof( xbExpNode ))) == NULL )
      return NULL;
   memset( Temp, 0x00, sizeof( xbExpNode ));

   if( Len > 0 )
   {
      if(( Temp->Result = (char *) malloc( Len )) == NULL ) 
      {
         free( Temp );
         return NULL;
      }
      memset( Temp->Result, 0x00, Len );
      *Temp->Result = 0x00;
      Temp->ResultLen = Len;
   }
   return Temp;
}
/*************************************************************************/
xbExpNode * xbExpn::LoadExpNode( 
         const char *ENodeText,    /* pointer to text data       */
         const char EType,         /* Operand type               */
	 const xbShort ELen,       /* length of node text data   */
         const xbShort BufLen )    /* length needed in the buffer*/
{
   xbExpNode * CurNode;
   if(( CurNode = GetExpNode(BufLen)) == NULL ) return NULL;

   CurNode->NodeText       = strdup( ENodeText );
   CurNode->Type           = EType;
   CurNode->Len            = ELen;
   CurNode->Node           = NULL;
   CurNode->Sibling1       = NULL;
   CurNode->Sibling2       = NULL;
   CurNode->Sibling3       = NULL;
   CurNode->FieldNo        = -1;
   CurNode->dbf            = NULL;
   CurNode->InTree         = 1;
   CurNode->ResultLen      = BufLen;
   CurNode->dbf            = NULL;
   CurNode->ExpressionType = 0;
   return CurNode;
}
/*************************************************************************/
void xbExpn::FreeExpNode( xbExpNode * n )
{
   xbExpNode * Temp;
   xbExpNode * Save;

   if( n->Sibling1 )
   {
      FreeExpNode( n->Sibling1 );
      n->Sibling1 = NULL;
   }
   if( n->Sibling2 )
   {
      FreeExpNode( n->Sibling2 );
      n->Sibling2 = NULL;
   }
   if( n->Sibling3 )
   {
      FreeExpNode( n->Sibling3 );
      n->Sibling3 = NULL;
   }
   if( !NextFreeExpNode )
      NextFreeExpNode = n;
   else if ( n->ResultLen == 0 )
   {
      n->Node = NextFreeExpNode;
      NextFreeExpNode = n;
   }
   else
   {
      Temp = NextFreeExpNode;
      Save = NULL;
      while( Temp->Node && ( n->ResultLen > Temp->Node->ResultLen ))
      {
         Save = Temp;
         Temp = Temp->Node;
      }
      if( n->ResultLen > Temp->ResultLen )
      {
         n->Node = Temp->Node;
         Temp->Node = n;
      }
      else
      {
         n->Node = Temp;
         if( Save )
            Save->Node = n;
         else
            NextFreeExpNode = n;
      }
   } 
   return;
}
/*************************************************************************/
xbShort xbExpn::BuildExpressionTree( const char * Expression, 
          xbShort MaxTokenLen, xbDbf * d )
{
   /* previous node is the node to insert under */
   xbExpNode * CurNode = 0;
   xbExpNode * PreviousNode;
   xbShort rc, FieldNo=0, BufLen;
   xbShort TokenLenCtr;
   char  c;
   const char *p;
   char  TempField[11];
   char  TableName[31];
   xbDbf * TempDbf=0;

   if( Tree )
   {
      FreeExpNode( Tree );
      Tree = NULL;
   }

   p = Expression;
   PreviousNode = NULL;
   PreviousType = TokenLenCtr = 0;

   while( IsWhiteSpace( *p ))
   {
      p++;
      TokenLenCtr++;
      if(TokenLenCtr >= MaxTokenLen)
				return XB_NO_ERROR;
   }

   rc = GetNextToken( p, MaxTokenLen-TokenLenCtr );
   if( rc != XB_NO_DATA && rc != XB_NO_ERROR )
	   return rc;

   while( rc == 0 )
   {
      if( TokenType == 'D' && d )
      {
         if( TokenLen > 30 )
            strncpy( TableName, p, 30 );
         else
            strncpy( TableName, p, TokenLen );

         memset( TempField, 0x00, 11 );

         if( strstr( p, "->" ) != NULL )
         {
            if(( TempDbf = d->xbase->GetDbfPtr( TableName )) == NULL )
              xb_error(XB_INVALID_FIELD);

            strncpy( TempField, strtok( NULL, "->" ), TokenLen-strlen(TableName)-2 );
         }
         else
         {
            TempDbf = d;
            if( TokenLen > 10 )
            xb_error(XB_INVALID_FIELD);
            strncpy( TempField, p, TokenLen );
         }
         if(( FieldNo = TempDbf->GetFieldNo( TempField )) == -1 ) 
           xb_error(XB_INVALID_FIELD);
         BufLen = TempDbf->GetFieldLen( FieldNo ) + 1;
      }
      else if( TokenType == 'C' || TokenType == 'N' )
         BufLen = TokenLen + 1;
      else
         BufLen = 0;

      if( TokenType == 'C' ) p++;   /* go past first ' */
             
      if( TokenType != 'O' )
      {
         if( !Tree )   /* create root node with this token */
         {
            CurNode = LoadExpNode( p, TokenType, TokenLen, BufLen );
            Tree = CurNode;
         }
         else          /* put as child 2 of previous node  */
         {
            CurNode = LoadExpNode( p, TokenType, TokenLen, BufLen );
            PreviousNode->Sibling2 = CurNode;
            CurNode->Node = PreviousNode;
         }

         if( TokenType == 'E' )
         {
            if((rc=ReduceComplexExpression(p,TokenLen,CurNode,d))!=0)
               return rc;
         }
         else if( TokenType == 'F' )
         {
            if(( rc = ReduceFunction( p, CurNode, d)) != 0 )
               return rc;
            CurNode->ExpressionType = GetFuncInfo( p, 2 );
            CurNode->dbf = d; 
         }
         else if( TokenType == 'D' && d )
         {
            CurNode->DataLen = BufLen - 1;
            CurNode->FieldNo = FieldNo;
            CurNode->dbf = TempDbf;
            c = TempDbf->GetFieldType( FieldNo );
            if( c == 'C' || c == 'M' ) CurNode->ExpressionType = 'C';
            else if( c == 'L' ) CurNode->ExpressionType = 'L';
            else if( c == 'N' || c == 'F' ) CurNode->ExpressionType = 'N';
            else if( c == 'D' ) CurNode->ExpressionType = 'D';
         }
         else if( TokenType == 'C' || TokenType == 'N' )
         {
            CurNode->DataLen = CurNode->Len;
            memset( CurNode->Result, 0x00, CurNode->DataLen + 1 );
            strncpy( CurNode->Result, CurNode->NodeText, CurNode->DataLen );
            if( TokenType == 'N' )
            { 
               CurNode->DoubResult = strtod(  CurNode->Result, 0 );
               CurNode->ExpressionType = 'N';
            }
            else
               CurNode->ExpressionType = 'C';
         }
      }
      else /* it is an operator */
      {
         if(!Tree) {
					 if(*p == '-') {
               CurNode = LoadExpNode( p, TokenType, TokenLen, 0 );
               CurNode->ExpressionType = 'C';
            } else
            xb_error(XB_EXP_SYNTAX_ERROR);
         }
         else
         {
            if( Tree->Type != 'O' )
            {
               CurNode = LoadExpNode( p, TokenType, TokenLen, 0 );
               Tree->Node = CurNode;       /* link the new parent to old tree */
               CurNode->Sibling1 = Tree;   /* connect the sibling             */
               Tree = CurNode;             /*  root in tree                 */
            }
            else
            {
               PreviousNode = CurNode->Node;
               CurNode = LoadExpNode( p, TokenType, TokenLen, 0 );
               while( PreviousNode &&
               (( OperatorWeight( PreviousNode->NodeText, TokenLen ) == 0 ) ||
                ( OperatorWeight( CurNode->NodeText, TokenLen ) >= /*<=*/
                  OperatorWeight( PreviousNode->NodeText, TokenLen ))))
                    PreviousNode = PreviousNode->Node;

               if( PreviousNode )
               {
                  CurNode->Node = PreviousNode;
                  CurNode->Sibling1 = PreviousNode->Sibling2;
                  PreviousNode->Sibling2 = CurNode;
                  CurNode->Sibling1->Node = CurNode;
               }
               else
               { /* insert at root */
                  CurNode->Sibling1 = Tree;
                  Tree = CurNode;
                  CurNode->Sibling1->Node = CurNode;
               }
            }
            if( LogicalType )
               CurNode->ExpressionType = 'L';
         }  
      }
      PreviousNode = CurNode;
      p += CurNode->Len; 
      if( TokenType == 'C' )
      {
         p++;              /* go past last ' */
         TokenLenCtr+=2;   /* add the quotes */
      }

      TokenLenCtr += CurNode->Len;
      if( TokenLenCtr >= MaxTokenLen )
         return XB_NO_ERROR;
      if( p && *p && TokenType == 'E' )
      {
         p++;
         TokenLenCtr++;
      }

      while( IsWhiteSpace( *p ))
      {
         p++;
         TokenLenCtr++;
         if( TokenLenCtr >= MaxTokenLen ) 
            return XB_NO_ERROR;
      }
      rc = GetNextToken( p, MaxTokenLen-TokenLenCtr );
      if( rc != XB_NO_DATA && rc != XB_NO_ERROR )
         return rc;
   }
   return XB_NO_ERROR;
}
/*************************************************************************/
char xbExpn::GetExpressionResultType( XB_EXPRESSION * e ) {
   xbExpNode * Temp = 0;
   if( e )
      Temp = e;
   else if( !Temp ) 
      Temp = Tree;
   else
      return 0;
 
   while( Temp && !Temp->ExpressionType && Temp->Sibling1 )
      Temp = Temp->Sibling1;
   return Temp->ExpressionType;
}
/*************************************************************************/
XB_EXPRESSION * xbExpn::GetExpressionHandle( void ) {
   XB_EXPRESSION * e;
   e = Tree;
   Tree = NULL;
   return e;
}
/*************************************************************************/
xbShort xbExpn::OperatorWeight( const char * Oper, xbShort len )
{
   if( len < 1 || len > 5 ) return 0;
   if( len == 1 && ( Oper[0] == '*' && Oper[1] == '*' )) return 3;
   if( Oper[0] == '+' || Oper[0] == '-' ) return 1;
   if( Oper[0] == '*' || Oper[0] == '/' ) return 2;
   if(Oper[0] == '.') return 1;
   return 0;
}
/*************************************************************************/
xbShort xbExpn::ReduceComplexExpression(const char *NextToken, xbShort Len, 
																				xbExpNode *cn, xbDbf *d) {
   const char *p;
   xbShort rc;
   xbExpNode * SaveTree;

   SaveTree = Tree;
   Tree = NULL;

   p = NextToken;
   p++;
   
   if(( rc = BuildExpressionTree( p, Len-2, d )) != XB_NO_ERROR )
     return rc;
 
   if(cn->Node)   /* then this is the base tree */
   {
      cn->Node->Sibling2 = Tree;
      Tree->Node = cn->Node;
      FreeExpNode( cn );
      Tree = SaveTree;
   }
   else
      FreeExpNode( cn );

   return XB_NO_ERROR;
}
/*************************************************************************/
xbShort xbExpn::GetFunctionTokenLen( const char * s )
{
   xbShort cnt, LeftParenCtr;
   const char *p;

   cnt = LeftParenCtr = 0;
   p = s;

   while( p &&
         ( *p != ',' || ( *p == ',' && LeftParenCtr > 0 )) &&
	 !( LeftParenCtr == 0 && *p == ')'))
   {
      if( *p == '(' ) 
         LeftParenCtr++;
      else if( *p == ')' ) 
         LeftParenCtr--;
      p++;
      cnt++;
   }
   return cnt;
}
/*************************************************************************/
xbShort xbExpn::ReduceFunction(const char *NextToken, xbExpNode *cn, xbDbf *d) 
{
   const char *p;
   xbShort rc;
   xbShort FuncTokenLen;
   xbExpNode * SaveTree;

   p = strchr( NextToken, '(' );
   if (!p)
    xb_error(XB_PARSE_ERROR);

   p++;
   while( IsWhiteSpace( *p )) p++;
   if (*p == ')')
		 return XB_NO_ERROR;

   /* do function paramater 1 */
   FuncTokenLen = GetFunctionTokenLen( p );
   SaveTree = Tree;
   Tree = NULL;
   if(( rc = BuildExpressionTree( p, FuncTokenLen, d )) != XB_NO_ERROR )
     return rc;
   cn->Sibling1 = Tree;
   Tree->Node = cn;
   Tree = SaveTree;

   /* do function paramater 2 */

   p += FuncTokenLen;
   while( IsWhiteSpace( *p )) p++;
   if(*p == ')')
     return XB_NO_ERROR;
   if( *p != ',' )
     xb_error(XB_PARSE_ERROR);

   p++; 
   while( IsWhiteSpace( *p )) p++;
   FuncTokenLen = GetFunctionTokenLen( p );
   SaveTree = Tree;
   Tree = NULL;
   if(( rc = BuildExpressionTree( p, FuncTokenLen, d )) != XB_NO_ERROR )
     return rc;

   cn->Sibling2 = Tree;
   Tree->Node = cn;
   Tree = SaveTree;

   /* do function paramater 3 */
   p += FuncTokenLen;
   while( IsWhiteSpace( *p )) p++;
   if (*p == ')')
     return XB_NO_ERROR;
   if( *p != ',' )
     xb_error(XB_PARSE_ERROR);

   p++; 
   while( IsWhiteSpace( *p )) p++;
   FuncTokenLen = GetFunctionTokenLen( p );
   SaveTree = Tree;
   Tree = NULL;
   if(( rc = BuildExpressionTree( p, FuncTokenLen, d )) != XB_NO_ERROR )
     xb_error(rc);

   cn->Sibling3 = Tree;
   Tree->Node = cn;
   Tree = SaveTree;

   return XB_NO_ERROR;
}
/*************************************************************************/
xbShort xbExpn::ParseExpression(const char *exp, xbDbf *d) {
   return BuildExpressionTree(exp, strlen(exp), d);
}
/*************************************************************************/
xbShort xbExpn::ProcessExpression(const char *e, xbDbf *d) {
   xbShort rc;
   if(( rc = BuildExpressionTree( e, strlen( e ), d )) != XB_NO_ERROR )
		 return rc;

   if(( rc = ProcessExpression( Tree )) != XB_NO_ERROR )
  	 return rc;

   return XB_NO_ERROR;
}
/*************************************************************************/
#ifdef XBASE_DEBUG
void xbExpn::DumpExpressionTree( xbExpNode * E )
{
   if( !E ) E = Tree;
   if( !E ) return;
   DumpExpNode( E );

   if( E->Sibling1 ) DumpExpressionTree( E->Sibling1 );
   if( E->Sibling2 ) DumpExpressionTree( E->Sibling2 );
   if( E->Sibling3 ) DumpExpressionTree( E->Sibling3 );
   return;
}
/*************************************************************************/
void xbExpn::DumpExpFreeChain( void )
{
   xbExpNode * Temp;
   Temp = NextFreeExpNode;

   cout << "\nFree Exp Node chain...";
   while( Temp )
   {
      DumpExpNode( Temp );
      Temp = Temp->Node;
   }
   return;
}
/************************************************************************/
void xbExpn::DumpExpNode(xbExpNode *e) {
   cout << "\n\n****** Exp Node ******";
   cout << "\nExp Node Address " << e;
   cout << "  Node Text = "  << e->NodeText;
   cout << "\nType = "       << e->Type;
   cout << " Len = "         << e->Len;
   cout << " InTree = "      << e->InTree;
   cout << "\nParent = "     << e->Node;
   cout << "\nField No = "   << e->FieldNo;
   cout << "\nExpressionType = " << e->ExpressionType;
   cout << "\nResult = "     << e->Result;
   cout << " DoubResult = "  << e->DoubResult;
   cout << " IntResult = "   << e->IntResult;
   cout << " ResultLen = "   << e->ResultLen;
   cout << " DataLen = "     << e->DataLen;
   cout << " Sibling 1 = "   << e->Sibling1;
   cout << " Sibling 2 = "   << e->Sibling2;
   if( e->Sibling3 ) cout << " Sibling3 = " << e->Sibling3;
   return;
}
#endif
#endif     // XB_EXPRESSIONS
/*************************************************************************/

