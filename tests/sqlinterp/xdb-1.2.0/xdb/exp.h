/*  $Id: exp.h,v 1.7 1999/03/19 10:56:33 willy Exp $

    Xbase project source code 

    This file contains a header file for the EXP object, which is
    used for expression processing.

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

    V 1.0   10/10/97   - Initial release of software
    V 1.5   1/2/97     - Added memo field support
    V 1.6a  4/1/98     - Added expression support
    V 1.6b  4/8/98     - Numeric index keys
    V 1.7.1 5/25/98    - Expression support enhancements
*/

#ifndef __XB_EXP_H__
#define __XB_EXP_H__

#include <xdb/xbase.h>

#ifdef XB_EXPRESSIONS             /* compile if expression logic on */

#define XB_EXPRESSION xbExpNode

#include <xdb/xtypes.h>
#include <xdb/xstack.h>

#undef ABS
#undef MIN
#undef MAX

class XBDLLEXPORT xbDbf;

struct xbFuncDtl {
   char * FuncName;                 /* function name               */
   xbShort  ParmCnt;                  /* no of parms it needs        */
   char   ReturnType;               /* return type of function     */
   void   (*ExpFuncPtr)();          /* pointer to function routine */
};

struct xbExpNode {
   char * NodeText;           /* expression text                 */
   char Type;                 /* same as TokenType below         */
   xbShort Len;                 /* length of expression text       */
   xbShort InTree;              /* this node in the tree? 1=yes    */
   xbExpNode * Node;            /* pointer to parent               */
   xbExpNode * Sibling1;        /* pointer to sibling 1            */
   xbExpNode * Sibling2;        /* pointer to sibling 2            */
   xbExpNode * Sibling3;        /* pointer to sibling 3            */

   xbShort  DataLen;            /* length of data in result buffer */
   xbShort  ResultLen;          /* length of result buffer         */
   char * Result;             /* result buffer - ptr to result   */
   xbDouble DoubResult;         /* Numeric Result                  */
   xbShort  IntResult;          /* logical result                  */

   xbDbf *  dbf;                /* pointer to datafile             */
   xbShort  FieldNo;            /* field no if DBF field           */
   char   ExpressionType;     /* used in head node C,N,L or D    */
};

/* Expression handler */

class XBDLLEXPORT xbExpn : public xbStack, public xbDate {
public:
   void  FreeExpNode( xbExpNode * );
   xbShort ProcessExpression( xbExpNode *, xbShort );
   xbExpNode * GetTree( void ) { return Tree; }
   void SetTreeToNull( void ) { Tree = NULL; }
   xbExpNode * GetFirstTreeNode( xbExpNode * );

   xbExpn( void );
   xbShort  GetNextToken( const char *s, xbShort MaxLen);

   /* expression methods */
   xbDouble ABS( xbDouble );
   xbLong   ASC( char * );
   xbLong   AT( char *, char * );
   char *   CDOW( char * );
   char *   CHR( xbLong );
   char *   CMONTH( char * );
   char *   DATE( void );
   xbLong   DAY( char * );
   xbLong   DESCEND( char * );
   xbLong   DOW( char * );
   char *   DTOC( char * );
   char *   DTOS( char * );
   xbDouble EXP( xbDouble );
   xbLong   INT( xbDouble );
   xbLong   ISALPHA( char * );
   xbLong   ISLOWER( char * );
   xbLong   ISUPPER( char * );
   char *   LEFT( char *, xbShort );
   xbLong   LEN( char * );
   xbDouble LOG( xbDouble );
   char *   LOWER( char * );
   char *   LTRIM( char * );
   xbDouble MAX( xbDouble, xbDouble );
   xbLong   MONTH( char * );         /* MONTH() */
   xbDouble MIN( xbDouble, xbDouble );
   char *   RECNO( xbULong );
   xbLong   RECNO( xbDbf * );
   char *   REPLICATE( char *, xbShort );
   char *   RIGHT( char *, xbShort );
   char *   RTRIM( char * );
   char *   SPACE( xbShort );
   xbDouble SQRT( xbDouble );
   char *   STR( char * );
   char *   STR( char *, xbShort );
   char *   STR( char *, xbShort, xbShort );
   char *   STR( xbDouble );
   char *   STR( xbDouble, xbShort );
   char *   STR(xbDouble, xbUShort length, xbShort numDecimals );
   char *   STRZERO( char * );
   char *   STRZERO( char *, xbShort );
   char *   STRZERO( char *, xbShort, xbShort );
   char *   STRZERO( xbDouble );
   char *   STRZERO( xbDouble, xbShort );
   char *   STRZERO( xbDouble, xbShort, xbShort );
   char *   SUBSTR( char *, xbShort, xbShort );
   char *   TRIM( char * );
   char *   UPPER( char * );
   xbLong   VAL( char * );
   xbLong   YEAR( char * );
   void     SetDefaultDateFormat(const xbString f){ DefaultDateFormat = f; }

   xbString GetDefaultDateFormat() const { return DefaultDateFormat; }
   xbShort  ProcessExpression( const char *exp, xbDbf * d );
   xbShort  ParseExpression( const char *exp, xbDbf * d );
   XB_EXPRESSION * GetExpressionHandle( void );
   char   GetExpressionResultType(XB_EXPRESSION * );
   char * GetCharResult( void );
   xbDouble GetDoubleResult( void );
   xbLong   GetIntResult( void );
   xbShort  ProcessExpression( xbExpNode * );
   xbShort  BuildExpressionTree( const char * Expression, xbShort MaxTokenLen,
            xbDbf *d );


#ifdef XBASE_DEBUG
   void DumpExpressionTree( xbExpNode * );
   void DumpExpFreeChain( void );
   void DumpExpNode( xbExpNode * );
#endif

private:
   xbFuncDtl *XbaseFuncList;    /* pointer to list of Xbase functions    */
   xbExpNode *NextFreeExpNode;  /* pointer to chain of free nodes        */
   xbExpNode *Tree;
   xbShort LogicalType;         /* set to 1 for logical type nodes       */

   char TokenType;            /* E - Expression, not in simplest form  */
                              /* C - Constant                          */
                              /* N - Numeric Constant                  */
                              /* O - Operator                          */
                              /* F - Function                          */
                              /* D - Database Field                    */
                              /* s - character string result           */
                              /* l - logical or short int result       */
                              /* d - double result                     */

   char  PreviousType;         /* used to see if "-" follows operator     */
   char  *  Op1;               /* pointer to operand 1                    */
   char  *  Op2;               /* pointer to operand 2                    */
   xbDouble Opd1;              /* double result 1                         */
   xbDouble Opd2;              /* double result 2                         */
   xbShort OpLen1;             /* length of memory allocated to operand 1 */
   xbShort OpLen2;             /* length of memory allocated to operand 2 */
   xbShort OpDataLen1;         /* length of data in op1                   */
   xbShort OpDataLen2;         /* length of data in op2                   */

   char    OpType1;            /* type of operand 1                       */
   char    OpType2;            /* type of operand 2                       */
   xbShort TokenLen;           /* length of token                         */

   static xbString DefaultDateFormat;  /*default date format for DTOC func*/

   enum { WorkBufMaxLen = 200 };
   char  WorkBuf[WorkBufMaxLen+1];

   xbShort  IsWhiteSpace( char );
   char   IsSeparator( char );
   xbExpNode * LoadExpNode( const char * ENodeText, const char EType,
           const xbShort ELen, const xbShort BufLen );
   xbShort  OperatorWeight( const char *Oper, xbShort len );
   xbShort  ReduceComplexExpression( const char * NextToken, xbShort Len,
            xbExpNode * cn, xbDbf *d );
   xbShort  GetFunctionTokenLen( const char *s );
   xbShort  ReduceFunction( const char *NextToken, xbExpNode *cn, xbDbf *d );
   xbExpNode * GetNextTreeNode( xbExpNode * );
   xbShort  ProcessOperator( xbShort );
   xbShort  ProcessFunction( char * );
   xbShort  ValidOperation( char *, char, char );
   char   GetOperandType( xbExpNode * );
   xbShort  AlphaOperation( char * );
   xbShort  NumericOperation( char * );
   xbExpNode * GetExpNode( xbShort );
   xbShort  GetFuncInfo( const char *Function, xbShort Option );
   xbDouble GetDoub( xbExpNode * );
   xbLong   GetInt( xbExpNode * );
};
#endif               // XB_EXPRESSIONS
#endif               // __XB_EXP_H__
