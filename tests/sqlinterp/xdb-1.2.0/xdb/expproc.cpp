/*  $Id: expproc.cpp,v 1.6 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

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
    V 1.7.1 5/25/98    - Enhanced expression support
*/

#include <xdb/xbase.h>

#ifdef XB_EXPRESSIONS

#include <ctype.h>
#include <math.h>
#include <string.h>

#include <xdb/xbexcept.h>

/*************************************************************************/
xbExpNode * xbExpn::GetFirstTreeNode( xbExpNode * e )
{
   xbExpNode * WorkNode;
   if( !e ) return e;
   WorkNode = e;
   while( WorkNode->Sibling1 )
      WorkNode = WorkNode->Sibling1;
   return WorkNode;
}
/*************************************************************************/
xbShort xbExpn::ValidOperation( char * Operand, char Op1, char Op2 )
{
   /*  Valid operation table

       operator  Field1   Field2          operator Field1   Field2

        **        N        N                 =        N        N
        *         N        N                 =        C        C
        /         N        N                 <>,#     N        N
        +         N        N                 <>,#     C        C
        +         C        C                 <=       N        N
        -         N        N                 <=       C        C
        -         C        C                 >=       N        N
        <         N        N                 >=       C        C
        <         C        C                 
        >         N        N
        >         C        C

    */


   if( Operand[0] == '*' && Operand[1] == '*' && Op1 == 'N' && Op2 == 'N' )
      return 1;

   switch( Operand[0] )
   {
      case '*': 
      case '/':
         if( Op1 == 'N' && Op2 == 'N' ) 
            return 1; 
         else 
            return 0;
//         break;

      case '+':
      case '-':
      case '<':
      case '>':
      case '=':
      case '#':
      case '$': // added 3/26/00 dtb
         if(( Op1 == 'N' && Op2 == 'N' ) || ( Op1 == 'C' && Op2 == 'C' ))
             return 1;
         else
             return 0;
             
      case '.' : // logical operators, added 3/26/00 dtb
         if((Operand[1] == 'A' || Operand[1] == 'N' || Operand[1] == 'O')) /* &&
            (Op1 == 'N' && Op2 == 'N')) */
            return 1;
      return 0;

      default:
         return 0;
   }
}
/*************************************************************************/
xbExpNode * xbExpn::GetNextTreeNode( xbExpNode * e )
{
   if( !e->Node ) return NULL;

   /* sibling 1 && sibling 2 exists */
   if( e == e->Node->Sibling1 && e->Node->Sibling2 )
      return GetFirstTreeNode( e->Node->Sibling2 );

   /* sibling2 && sibling3 exists */
   else if( e == e->Node->Sibling2 && e->Node->Sibling3 )
      return GetFirstTreeNode( e->Node->Sibling3 );

   else
      return e->Node;
}
/*************************************************************************/
xbShort xbExpn::ProcessExpression( xbExpNode * e )
{
   return ProcessExpression( e, 0 );
}
/*************************************************************************/
xbShort xbExpn::ProcessExpression( xbExpNode * Wtree, xbShort RecBufSw )
{
   xbExpNode * WorkNode;
   xbShort rc = 0;

   if( Wtree == 0 )
      Wtree = Tree;

   memset(WorkBuf, 0x00, WorkBufMaxLen+1 );
   /* initialize the stack - free any expnodes */
   while( GetStackDepth() > 0 )
   {
      WorkNode = (xbExpNode *) Pop();
      if( !WorkNode->InTree )
         FreeExpNode( WorkNode );
   }   

   if(( WorkNode = GetFirstTreeNode( Wtree )) == NULL )
		 xb_error(XB_NO_DATA);

   while( WorkNode )
   {
//printf("WorkNode->Type = %c\n", WorkNode->Type);
      Push( (void *) WorkNode );
      if( WorkNode->Type == 'D' && WorkNode->dbf )
      {
         WorkNode->dbf->GetField( WorkNode->FieldNo, (char *) WorkNode->Result, RecBufSw );
         if( WorkNode->dbf->GetFieldType( WorkNode->FieldNo ) == 'N' || 
            WorkNode->dbf->GetFieldType( WorkNode->FieldNo ) == 'F' )
            WorkNode->DoubResult = WorkNode->dbf->GetDoubleField( WorkNode->FieldNo );
      }
      else if( WorkNode->Type == 'O' )
      {
         if(( rc = ProcessOperator( RecBufSw )) != XB_NO_ERROR )
           return rc;
      }
      else if( WorkNode->Type == 'F' )
         if(( rc = ProcessFunction( WorkNode->NodeText )) != XB_NO_ERROR )
    			return rc;
      WorkNode = GetNextTreeNode( WorkNode );
   }
   if( GetStackDepth() != 1 )    /* should only have result left in stack */
		 xb_error(XB_PARSE_ERROR);

   return XB_NO_ERROR;
}
/*************************************************************************/
char xbExpn::GetOperandType( xbExpNode * e )
{
   /* this routine returns
         L - logical
         N - Numeric
         C - Character
         0 - error
   */
 
   char WorkType;

//printf("e->Type = '%c'\n", e->Type);

   if( e->Type == 'd' || e->Type == 'N' || e->Type == 'i' ) return 'N';
   if( e->Type == 'l' ) return 'L';
   if( e->Type == 's' ) return 'C';

   if( e->Type == 'C' )
   {
      if(e->NodeText[0]=='-' || e->NodeText[0]=='+' || 
         (isdigit(e->NodeText[0]) &&
	  !(e->NodeText[e->DataLen] == '\'' || e->NodeText[e->DataLen] == '"'))
	 )
        return 'N';
      else
        return 'C';
   }
   else if( e->Type == 'D' && e->dbf )
   {
      WorkType = e->dbf->GetFieldType( e->FieldNo );

      if( WorkType == 'C' ) return 'C';
      else if( WorkType == 'F' || WorkType == 'N' ) return 'N';
      else if( WorkType == 'L' ) return 'L';
      else return 0;
   }
   else
      return 0;
}
/*************************************************************************/
xbShort xbExpn::ProcessOperator( xbShort RecBufSw )
{
   xbExpNode * WorkNode;
   char Operator[6 /*3*/];  // changed for logical ops 3/26/00 dtb
   char t;

   if( GetStackDepth() < 3 ) 
{
//printf("stack depth < 3\n");   
		 xb_error(XB_PARSE_ERROR);
}

   WorkNode = (xbExpNode *) Pop();
   if( WorkNode->Len > 5 /*2*/) // changed for logical ops 3/26/00 dtb
{
//printf("WorkNode->Len = %d\n", WorkNode->Len);
		 xb_error(XB_PARSE_ERROR);
}

   memset( Operator, 0x00, 6 /*3*/ );  // changed for logical ops 3/26/00 dtb
   strncpy( Operator, WorkNode->NodeText, WorkNode->Len );
//printf("Operator = '%s'\n", Operator);
   if( !WorkNode->InTree ) 
      FreeExpNode( WorkNode );

   /* load up operand 1 */
   WorkNode = (xbExpNode *) Pop();
//printf("Operand 1 WorkNode->Type = '%c'\n", WorkNode->Type);
   if(( OpType1 = GetOperandType( WorkNode )) == 0 )
		 xb_error(XB_PARSE_ERROR);

   if( OpLen1 < WorkNode->DataLen+1 && WorkNode->Type != 'd' )
   {
      if( OpLen1 > 0 ) free( Op1 );
      if(( Op1 = (char *) malloc( WorkNode->DataLen+1 )) == NULL ) {
	      xb_memory_error;
      }
      OpLen1 = WorkNode->DataLen+1;
   }
   OpDataLen1 = WorkNode->DataLen;
   memset( Op1, 0x00, WorkNode->DataLen+1 );
   if( WorkNode->Type == 'D' && WorkNode->dbf )     /* database field  */
   {
      WorkNode->dbf->GetField( WorkNode->FieldNo, Op1, RecBufSw );
      t = WorkNode->dbf->GetFieldType( WorkNode->FieldNo );
      if( t == 'N' || t == 'F' )
         Opd1 = strtod( WorkNode->Result, 0 );
   }
   else if( WorkNode->Type == 'C' )     /* constant        */
      memcpy( Op1, WorkNode->NodeText, WorkNode->DataLen );
   else if( WorkNode->Type == 's' )     /* previous result */
      memcpy( Op1, WorkNode->Result, WorkNode->DataLen+1 );
   else if( WorkNode->Type == 'd' )     /* previous numeric result */   
      Opd1 = WorkNode->DoubResult;
   else if( WorkNode->Type == 'N' )     /* previous numeric result */   
      Opd1 = strtod( WorkNode->Result, 0 );
   else if(WorkNode->Type == 'l')       /* previous logical result 3/26/00 dtb */
      Opd1 = WorkNode->IntResult;
   if( !WorkNode->InTree ) 
      FreeExpNode( WorkNode );

   /* load up operand 2 */
   WorkNode = (xbExpNode *) Pop();
//printf("Operand 2 WorkNode->Type = '%c'\n", WorkNode->Type);
   if(( OpType2 = GetOperandType( WorkNode )) == 0 )
		 xb_error(XB_PARSE_ERROR);

   if( OpLen2 < WorkNode->DataLen+1 && WorkNode->Type != 'd' )
   {
      if( OpLen2 > 0 ) free( Op2 );
      if(( Op2 = (char *) malloc( WorkNode->DataLen+1 )) == NULL ) {
	      xb_memory_error;
      }
      OpLen2 = WorkNode->DataLen+1;
   }
   OpDataLen2 = WorkNode->DataLen;
   memset( Op2, 0x00, WorkNode->DataLen+1 );
   if( WorkNode->Type == 'D' && WorkNode->dbf )      /* database field  */
   {
      WorkNode->dbf->GetField( WorkNode->FieldNo, Op2, RecBufSw );
      t = WorkNode->dbf->GetFieldType( WorkNode->FieldNo );
      if( t == 'N' || t == 'F' )
         Opd2 = strtod( WorkNode->Result, 0 );
   }
   else if( WorkNode->Type == 'C' )     /* constant        */
      memcpy( Op2, WorkNode->NodeText, WorkNode->DataLen );
   else if( WorkNode->Type == 's' )     /* previous result */
      memcpy( Op2, WorkNode->Result, WorkNode->DataLen+1 );
   else if( WorkNode->Type == 'd' )     /* previous numeric result */
      Opd2 = WorkNode->DoubResult;
   else if( WorkNode->Type == 'N' )     /* previous numeric result */   
      Opd2 = strtod( WorkNode->Result, 0 );
   else if(WorkNode->Type == 'l')       /* previous logical result 3/26/00 dtb*/
      Opd2 = WorkNode->IntResult;
   if( !WorkNode->InTree )
      FreeExpNode( WorkNode );

//printf("Operator '%s', OpType1 = '%c', OpType2 = '%c'\n", Operator, OpType1, OpType2);
   if( !ValidOperation( Operator, OpType1, OpType2 ))
   {
//printf("Invalid operator '%s', OpType1 = '%c', OpType2 = '%c'\n", Operator, OpType1, OpType2);
      xb_error(XB_PARSE_ERROR);
   }

   if( OpType1 == 'N' || OpType1 == 'L')    /* numeric procesing */
   {
      return NumericOperation( Operator );
   }
   else                    /* must be character */
      return  AlphaOperation( Operator );
}
/*************************************************************************/
xbShort xbExpn::NumericOperation( char * Operator )
{
   xbDouble  Operand1, Operand2;
   xbExpNode * WorkNode;
   xbShort   ResultLen;
   char    SaveType;

   ResultLen = 0;

//printf("NumericOperation(%s)\n", Operator);

   if( Operator[0] == '=' || Operator[0] == '<' || 
       Operator[0] == '>' || Operator[0] == '#'  ||
       Operator[0] == '.')
      SaveType = 'l';
   else
      SaveType = 'd';

   if(( WorkNode = GetExpNode( ResultLen )) == NULL )
		 xb_error(XB_PARSE_ERROR);

   WorkNode->Type = SaveType;
   WorkNode->DataLen = ResultLen;

   if( OpType1 == 'd' || OpType1 == 'N' )
      Operand1 = Opd1;
   else
      Operand1 = strtod( Op1, NULL );

   if( OpType2 == 'd' || OpType2 == 'N' )
      Operand2 =  Opd2;
   else
      Operand2 = strtod( Op2, NULL );

   if( Operator[0] == '*' && Operator[1] == '*' )
      WorkNode->DoubResult = pow( Operand2, Operand1 );
   else if( Operator[0] == '*' )
      WorkNode->DoubResult = Operand2 * Operand1;
   else if( Operator[0] == '/') 
      WorkNode->DoubResult = Operand2 / Operand1;
   else if( Operator[0] == '+' )
      WorkNode->DoubResult = Operand2 + Operand1;
   else if( Operator[0] == '-' )
      WorkNode->DoubResult = Operand2 - Operand1;

   /* = */
   else if((Operator[0]== '=' || Operator[1]== '=' ) && Operand1 == Operand2)
      WorkNode->IntResult = 1;
   else if( Operator[0] == '=' )
      WorkNode->IntResult = 0;
   /* not = */
   else if((Operator[0] == '<' && Operator[1] == '>')||Operator[0] == '#')
   {
      if( Operand1 != Operand2 )
         WorkNode->IntResult = 1;
      else
         WorkNode->IntResult = 0;
   }
   /* less than */
   else if( Operator[0] == '<' )
   {
      if( Operand2 < Operand1 )
         WorkNode->IntResult = 1;
      else
         WorkNode->IntResult = 0;
   }
   /* greater than */
   else if( Operator[0] == '>' )
   {
      if( Operand2 > Operand1 )
         WorkNode->IntResult = 1;
      else
         WorkNode->IntResult = 0;
   }
   else if(Operator[0] == '.') // logical operators, added 3/26/00 dtb
   {
//printf("Operand1 = %d, Operand2 = %d\n", Operand1, Operand2);
     switch(Operator[1])
     {
       case 'A' : // and
         WorkNode->IntResult = (Opd1 && Opd2) ? 1 : 0;
       break;
       
       case 'N' : // not
         WorkNode->IntResult = (!(Opd1 && Opd2)) ? 1 : 0;
       break;
       
       case 'O' : // or
         WorkNode->IntResult = (Opd1 || Opd2) ? 1 : 0;
       break;
       
       default :
         xb_error(XB_PARSE_ERROR);
     }
   }
   else
      xb_error(XB_PARSE_ERROR);

   Push((void *) WorkNode );
   return 0;
}
/*************************************************************************/
xbShort xbExpn::AlphaOperation( char * Operator )
{
   xbShort ResultLen, i;
   char *p, SaveType;
   xbExpNode * WorkNode;

//printf("AlphaOperation(%s): Op1 = '%*s', Op2 = '%*s'\n", Operator,
//       OpDataLen1, Op1, OpDataLen2, Op2);

   if( Operator[0] == '=' || Operator[0] == '<' || 
       Operator[0] == '>' || Operator[0] == '#'  ||
       Operator[0] == '$')
   {
      ResultLen = 0;
      SaveType = 'l';
   }
   else
   {
      ResultLen = OpDataLen1 + OpDataLen2 + 1;
      SaveType = 's';
   }
   if(( WorkNode = GetExpNode( ResultLen )) == NULL )
		 xb_error(XB_PARSE_ERROR);

   memset( WorkNode->Result, 0x00, ResultLen );
   WorkNode->Type = SaveType;
   if( WorkNode->Type == 'l' )
      WorkNode->DataLen = 0;
   else
      WorkNode->DataLen = ResultLen - 1;

   if( Operator[0] == '+' )
   {
      strncpy( WorkNode->Result, Op2, OpDataLen2 );
      strncat( WorkNode->Result, Op1, OpDataLen1 );
   }
   else if( Operator[0] == '-' )
   {
      strncpy( WorkNode->Result, Op2, OpDataLen2 );
      p =  WorkNode->Result;
      p += OpDataLen2-1;
      i =  OpDataLen2-1;
      while( *p == 0x20 && i > 0 )
      {
         *p = 0x00;
         p--;
         i--;
      }
      strncat( WorkNode->Result, Op1, OpDataLen1 );
      p =  WorkNode->Result;
      i = strlen( WorkNode->Result );
      p += i; 
      for( ; i < ResultLen-1; i++)
      {
         *p = 0x20;   /* pad with spaces */
         p++;
      }
   }
   /* = */
   else if((Operator[0]== '=' || Operator[1]== '=' ) && strcmp(Op1,Op2) == 0)
      WorkNode->IntResult = 1;
   else if( Operator[0] == '=' )
      WorkNode->IntResult = 0;
   /* not = */
   else if((Operator[0] == '<' && Operator[1] == '>')||Operator[0] == '#')
   {
      if( strcmp( Op1, Op2 ) != 0 )
         WorkNode->IntResult = 1;
      else
         WorkNode->IntResult = 0;
   }
   /* less than */
   else if( Operator[0] == '<' )
   {
      if( strcmp( Op2, Op1 ) < 0 )
         WorkNode->IntResult = 1;
      else
         WorkNode->IntResult = 0;
   }
   /* greater than */
   else if( Operator[0] == '>' )
   {
      if( strcmp( Op2, Op1 ) > 0 )
         WorkNode->IntResult = 1;
      else
         WorkNode->IntResult = 0;
   }
   //  contains, added 3/26/00 dtb
   else if(Operator[0] == '$')
   {
     if(strstr(Op2, Op1))
       WorkNode->IntResult = 1;
     else
       WorkNode->IntResult = 0;
   }
   else
     xb_error(XB_PARSE_ERROR);

//printf("WorkNode->IntResult = %d\n", WorkNode->IntResult);
   Push((void *) WorkNode);
   return XB_NO_ERROR;
}
/*************************************************************************/
#endif  	//	XB_EXPRESSIONS
