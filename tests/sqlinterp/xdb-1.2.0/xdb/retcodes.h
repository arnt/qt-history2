/*  $Id: retcodes.h,v 1.5 1999/02/15 22:18:21 willy Exp $

    Xbase project source code

    This file contains a listing of all the Xbase return codes.

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

    V 1.0    10/10/97   - Initial release of software
    V 1.5    1/2/98     - Added memo field support
    V 1.6a   4/1/98     - Added expression support
    V 1.6b   4/8/98     - Numeric index keys
    V 1.8.0a 1/27/99    - Changed return codes to negative values
*/

#ifndef __XB_RETCODES_H__
#define __XB_RETCODES_H__

/***********************************************/
/* Return Codes and Error Messages             */

#define XB_NO_ERROR                 0
#define XB_EOF                    -100
#define XB_BOF                    -101
#define XB_NO_MEMORY              -102
#define XB_FILE_EXISTS            -103
#define XB_OPEN_ERROR             -104
#define XB_WRITE_ERROR            -105
#define XB_UNKNOWN_FIELD_TYPE     -106
#define XB_ALREADY_OPEN           -107
#define XB_NOT_XBASE              -108
#define XB_INVALID_RECORD         -109
#define XB_INVALID_OPTION         -110
#define XB_NOT_OPEN               -111
#define XB_SEEK_ERROR             -112
#define XB_READ_ERROR             -113
#define XB_NOT_FOUND              -114
#define XB_FOUND                  -115
#define XB_INVALID_KEY            -116
#define XB_INVALID_NODELINK       -117
#define XB_KEY_NOT_UNIQUE         -118
#define XB_INVALID_KEY_EXPRESSION -119
#define XB_DBF_FILE_NOT_OPEN      -120
#define XB_INVALID_KEY_TYPE       -121
#define XB_INVALID_NODE_NO        -122
#define XB_NODE_FULL              -123
#define XB_INVALID_FIELDNO        -124
#define XB_INVALID_DATA           -125
#define XB_NOT_LEAFNODE           -126
#define XB_LOCK_FAILED            -127
#define XB_CLOSE_ERROR            -128
#define XB_INVALID_SCHEMA         -129
#define XB_INVALID_NAME           -130
#define XB_INVALID_BLOCK_SIZE     -131
#define XB_INVALID_BLOCK_NO       -132
#define XB_NOT_MEMO_FIELD         -133
#define XB_NO_MEMO_DATA           -134
#define XB_EXP_SYNTAX_ERROR       -135
#define XB_PARSE_ERROR            -136
#define XB_NO_DATA                -137
#define XB_UNKNOWN_TOKEN_TYPE     -138
#define XB_INVALID_FIELD          -140
#define XB_INSUFFICIENT_PARMS     -141
#define XB_INVALID_FUNCTION       -142
#define XB_INVALID_FIELD_LEN      -143
#endif	/* __XB_RETCODES_H__ */
