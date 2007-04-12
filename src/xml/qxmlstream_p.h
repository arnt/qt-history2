/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QXMLSTREAM_P_H
#define QXMLSTREAM_P_H

class QXmlStreamReader_Table
{
public:
  enum {
    EOF_SYMBOL = 0,
    AMPERSAND = 5,
    ANY = 41,
    ATTLIST = 31,
    BANG = 25,
    CDATA = 46,
    CDATA_START = 28,
    COLON = 17,
    COMMA = 19,
    DASH = 20,
    DBLQUOTE = 8,
    DIGIT = 27,
    DOCTYPE = 29,
    DOT = 23,
    ELEMENT = 30,
    EMPTY = 40,
    ENTITIES = 50,
    ENTITY = 32,
    ENTITY_DONE = 45,
    EQ = 14,
    ERROR = 43,
    FIXED = 39,
    HASH = 6,
    ID = 47,
    IDREF = 48,
    IDREFS = 49,
    IMPLIED = 38,
    LANGLE = 3,
    LBRACK = 9,
    LETTER = 26,
    LPAREN = 11,
    NDATA = 36,
    NMTOKEN = 51,
    NMTOKENS = 52,
    NOTATION = 33,
    NOTOKEN = 1,
    PARSE_ENTITY = 44,
    PCDATA = 42,
    PERCENT = 15,
    PIPE = 13,
    PLUS = 21,
    PUBLIC = 35,
    QUESTIONMARK = 24,
    QUOTE = 7,
    RANGLE = 4,
    RBRACK = 10,
    REQUIRED = 37,
    RPAREN = 12,
    SEMICOLON = 18,
    SLASH = 16,
    SPACE = 2,
    STAR = 22,
    SYSTEM = 34,
    VERSION = 54,
    XML = 53,

    ACCEPT_STATE = 409,
    RULE_COUNT = 262,
    STATE_COUNT = 420,
    TERMINAL_COUNT = 55,
    NON_TERMINAL_COUNT = 81,

    GOTO_INDEX_OFFSET = 420,
    GOTO_INFO_OFFSET = 980,
    GOTO_CHECK_OFFSET = 980,
  };

  static const char  *const spell [];
  static const int            lhs [];
  static const int            rhs [];
  static const int   goto_default [];
  static const int action_default [];
  static const int   action_index [];
  static const int    action_info [];
  static const int   action_check [];

  inline int nt_action (int state, int nt) const
  {
    const int *const goto_index = &action_index [GOTO_INDEX_OFFSET];
    const int *const goto_check = &action_check [GOTO_CHECK_OFFSET];

    const int yyn = goto_index [state] + nt;

    if (yyn < 0 || goto_check [yyn] != nt)
      return goto_default [nt];

    const int *const goto_info = &action_info [GOTO_INFO_OFFSET];
    return goto_info [yyn];
  }

  inline int t_action (int state, int token) const
  {
    const int yyn = action_index [state] + token;

    if (yyn < 0 || action_check [yyn] != token)
      return - action_default [state];

    return action_info [yyn];
  }
};


const char *const QXmlStreamReader_Table::spell [] = {
  "end of file", 0, " ", "<", ">", "&", "#", "\'", "\"", "[", 
  "]", "(", ")", "|", "=", "%", "/", ":", ";", ",", 
  "-", "+", "*", ".", "?", "!", "[a-zA-Z]", "[0-9]", "[CDATA[", "DOCTYPE", 
  "ELEMENT", "ATTLIST", "ENTITY", "NOTATION", "SYSTEM", "PUBLIC", "NDATA", "REQUIRED", "IMPLIED", "FIXED", 
  "EMPTY", "ANY", "PCDATA", 0, 0, 0, "CDATA", "ID", "IDREF", "IDREFS", 
  "ENTITIES", "NMTOKEN", "NMTOKENS", "<?xml", "version"};

const int QXmlStreamReader_Table::lhs [] = {
  55, 55, 57, 57, 57, 57, 57, 57, 57, 57, 
  65, 66, 62, 70, 70, 70, 72, 64, 64, 64, 
  64, 76, 75, 77, 77, 77, 77, 77, 77, 78, 
  78, 78, 78, 78, 78, 78, 84, 80, 85, 85, 
  85, 85, 88, 89, 90, 90, 90, 90, 91, 91, 
  93, 93, 93, 94, 94, 95, 95, 96, 96, 97, 
  97, 86, 86, 92, 87, 98, 98, 100, 100, 100, 
  100, 100, 100, 100, 100, 100, 100, 100, 101, 102, 
  102, 102, 102, 104, 106, 107, 107, 81, 81, 108, 
  108, 109, 109, 82, 82, 82, 63, 63, 73, 111, 
  61, 112, 113, 83, 83, 83, 114, 114, 114, 114, 
  114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 
  114, 114, 114, 114, 114, 114, 114, 115, 115, 115, 
  115, 68, 68, 68, 68, 116, 117, 116, 117, 116, 
  117, 116, 117, 119, 119, 119, 119, 119, 119, 119, 
  119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 
  119, 119, 119, 119, 119, 118, 110, 110, 110, 110, 
  120, 121, 120, 121, 120, 121, 120, 121, 122, 122, 
  122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 
  122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 
  122, 122, 122, 103, 103, 103, 103, 125, 126, 125, 
  126, 125, 125, 126, 126, 127, 127, 127, 127, 130, 
  69, 69, 69, 131, 131, 132, 60, 58, 59, 133, 
  79, 124, 129, 128, 123, 134, 134, 134, 134, 56, 
  56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 
  56, 71, 67, 67, 105, 74, 99, 99, 99, 99, 
  99, 135};

const int QXmlStreamReader_Table:: rhs[] = {
  2, 1, 4, 2, 2, 2, 2, 2, 2, 0, 
  1, 1, 9, 2, 4, 0, 4, 6, 4, 4, 
  6, 1, 3, 1, 1, 1, 2, 2, 2, 1, 
  1, 1, 1, 1, 1, 1, 4, 4, 1, 1, 
  1, 1, 1, 2, 1, 1, 1, 0, 2, 2, 
  2, 6, 6, 1, 5, 1, 5, 3, 5, 0, 
  1, 6, 8, 4, 2, 1, 5, 1, 0, 1, 
  1, 1, 1, 1, 1, 1, 6, 7, 1, 2, 
  2, 1, 4, 3, 3, 1, 2, 5, 6, 4, 
  6, 3, 5, 5, 3, 4, 4, 5, 2, 3, 
  2, 2, 4, 5, 5, 7, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 2, 2, 3, 3, 2, 2, 2, 2, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 2, 2, 3, 3, 
  2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 2, 2, 3, 3, 2, 2, 2, 
  2, 1, 1, 1, 1, 1, 1, 1, 1, 5, 
  0, 1, 3, 1, 3, 2, 4, 3, 5, 3, 
  3, 3, 3, 4, 4, 1, 1, 2, 2, 2, 
  4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 
  1, 2};

const int QXmlStreamReader_Table::action_default [] = {
  10, 251, 0, 2, 1, 0, 125, 117, 119, 120, 
  127, 130, 123, 11, 114, 108, 0, 109, 129, 111, 
  115, 113, 121, 124, 126, 107, 110, 112, 118, 116, 
  128, 122, 12, 244, 248, 243, 0, 131, 240, 247, 
  16, 242, 250, 249, 0, 246, 251, 221, 245, 0, 
  0, 256, 0, 237, 236, 0, 239, 238, 234, 230, 
  99, 255, 0, 226, 0, 0, 252, 97, 98, 101, 
  0, 0, 253, 0, 0, 0, 164, 156, 158, 159, 
  133, 145, 162, 153, 147, 148, 144, 150, 154, 152, 
  160, 163, 143, 146, 149, 151, 157, 155, 165, 161, 
  141, 166, 0, 135, 139, 137, 142, 132, 140, 0, 
  138, 134, 136, 0, 15, 14, 254, 0, 22, 19, 
  253, 0, 0, 18, 0, 0, 31, 36, 30, 0, 
  32, 253, 0, 33, 0, 24, 0, 34, 0, 26, 
  35, 25, 0, 231, 40, 39, 253, 42, 48, 253, 
  41, 0, 43, 253, 48, 253, 0, 48, 253, 0, 
  0, 47, 45, 46, 50, 51, 253, 253, 0, 56, 
  253, 53, 253, 0, 57, 0, 54, 253, 52, 253, 
  0, 55, 64, 49, 0, 253, 60, 253, 0, 58, 
  61, 62, 0, 253, 60, 0, 59, 63, 44, 65, 
  0, 38, 0, 0, 253, 0, 94, 95, 0, 0, 
  0, 0, 253, 0, 200, 191, 193, 195, 168, 180, 
  198, 189, 183, 181, 184, 179, 186, 188, 196, 199, 
  178, 182, 185, 187, 192, 190, 194, 197, 201, 203, 
  202, 176, 0, 0, 0, 0, 235, 232, 170, 174, 
  172, 0, 0, 93, 177, 167, 175, 0, 173, 169, 
  171, 92, 0, 96, 0, 0, 0, 0, 0, 253, 
  86, 253, 69, 254, 0, 87, 0, 89, 68, 74, 
  73, 70, 71, 72, 253, 75, 76, 0, 0, 0, 
  261, 260, 258, 259, 257, 66, 253, 0, 253, 0, 
  0, 67, 77, 253, 0, 253, 0, 0, 78, 0, 
  79, 0, 82, 85, 0, 0, 205, 215, 214, 0, 
  217, 219, 218, 216, 0, 233, 207, 211, 209, 213, 
  204, 212, 0, 210, 206, 208, 0, 81, 80, 0, 
  83, 0, 84, 88, 100, 0, 37, 0, 0, 0, 
  0, 91, 90, 0, 103, 23, 27, 29, 28, 0, 
  0, 253, 254, 0, 253, 0, 106, 105, 253, 0, 
  104, 102, 0, 0, 20, 253, 17, 0, 21, 0, 
  0, 241, 0, 253, 0, 229, 0, 222, 228, 0, 
  227, 224, 253, 253, 254, 223, 225, 0, 253, 0, 
  220, 253, 0, 253, 0, 221, 0, 0, 13, 262, 
  9, 5, 8, 4, 0, 7, 251, 6, 0, 3};

const int QXmlStreamReader_Table::goto_default [] = {
  2, 4, 3, 46, 381, 41, 35, 48, 45, 39, 
  239, 49, 117, 75, 386, 72, 116, 40, 44, 157, 
  120, 121, 136, 135, 139, 128, 126, 130, 137, 129, 
  149, 150, 147, 159, 158, 199, 155, 154, 156, 177, 
  170, 187, 191, 296, 295, 288, 314, 313, 312, 272, 
  63, 270, 271, 132, 131, 212, 36, 33, 138, 37, 
  38, 109, 102, 323, 101, 257, 242, 241, 238, 240, 
  332, 319, 318, 320, 322, 391, 392, 47, 43, 55, 
  0};

const int QXmlStreamReader_Table::action_index [] = {
  -9, -55, 54, 198, 925, 103, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, 129, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, 66, -55, -55, -55, 
  79, -55, -55, -55, 132, -55, -55, 102, -55, -18, 
  116, -55, 25, -55, -55, 138, -55, -55, -55, -55, 
  -55, -55, 34, -55, 71, 42, -55, -55, -55, -55, 
  86, 82, 102, 289, 342, 102, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, 395, -55, -55, -55, -55, -55, -55, 315, 
  -55, -55, -55, 84, -55, -55, -55, 92, -55, -55, 
  102, 194, 57, -55, 63, 55, -55, -55, -55, 152, 
  -55, 62, 195, -55, 191, -55, 202, -55, 65, -55, 
  -55, -55, 60, -55, -55, -55, 102, -55, 148, 40, 
  -55, 165, -55, 36, 157, 102, 43, 161, 47, 24, 
  99, -55, -55, -55, -55, 114, 102, 102, 115, -55, 
  102, 33, 102, 113, -55, 111, -55, 102, 38, 102, 
  112, -55, -55, -55, 134, 102, 37, 102, 32, -55, 
  -55, -55, 136, 35, 23, 13, -55, -55, -55, -55, 
  30, -55, 14, 44, 45, 46, -55, -55, 607, 137, 
  819, 98, 39, 109, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, 501, 85, 61, 168, -55, -55, -55, -55, 
  -55, 102, 100, -55, -55, -55, -55, 766, -55, -55, 
  -55, -55, 59, -55, 56, 48, 41, 105, 51, 102, 
  -55, 102, 222, 50, 52, -55, 53, -55, -55, -55, 
  -55, -55, -55, -55, 102, -55, -55, 58, 160, 236, 
  -55, -55, -55, -55, -55, -55, -2, 145, -2, -2, 
  193, -55, -55, -2, 236, -2, 106, -2, -55, 448, 
  -55, 713, -55, -55, 150, 91, -55, -55, -55, 660, 
  -55, -55, -55, -55, -17, -55, -55, -55, -55, -55, 
  -55, -55, 554, -55, -55, -55, 29, -55, -55, 95, 
  -55, 28, -55, -55, -55, 27, -55, 26, 22, -6, 
  4, -55, -55, 2, -55, -55, -55, -55, -55, 76, 
  64, 5, 67, 1, 0, 49, -55, -55, 6, 11, 
  -55, -55, -13, 171, -55, 7, -55, 21, -55, 872, 
  182, -55, -16, 9, 10, -55, 124, -10, -55, 8, 
  -55, -55, 15, 31, -3, -55, -55, 18, 16, 78, 
  -55, 17, 12, 20, 143, 19, 3, -1, -55, -55, 
  -55, -55, -55, -55, 101, -55, -55, -55, 872, -55, 

  -81, -81, -81, 143, 80, -14, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, 74, 55, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, 63, -81, 65, -81, -81, -81, -81, -81, 
  -81, 30, -81, -8, -15, 28, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, 48, -81, -81, -81, -81, -81, -81, 41, 
  -81, -81, -81, 54, -81, -81, -81, -81, -81, -81, 
  44, 110, -81, -81, -81, 66, -81, -81, -81, 64, 
  -81, 68, -81, -81, -81, -81, 115, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, 9, -81, -81, 33, 
  -81, -81, -81, -6, -34, 5, -81, 6, 17, -81, 
  -81, -81, -81, -81, -81, -81, 4, -10, -36, -81, 
  42, -81, 3, -26, -81, -33, -81, 21, -81, 24, 
  -11, -81, -81, -81, -81, 12, -81, 10, 0, -81, 
  -81, -81, -81, 27, 7, 19, -81, -81, -81, -81, 
  -81, -81, -12, -81, 1, -81, -81, -81, -81, 60, 
  50, 59, 56, 51, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, 32, -20, -81, -81, -81, -81, -81, -81, 
  -81, 47, 52, -81, -81, -81, -81, -7, -81, -81, 
  -81, -81, -81, -81, 16, -81, 43, 45, 39, 82, 
  -81, 96, -81, 29, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, 49, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, 34, -81, 102, 31, 
  -4, -81, -81, 38, 46, 40, -81, 35, -81, 20, 
  -81, 87, -81, -81, -81, 67, -81, -81, -81, 73, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, 93, -81, -81, -81, 71, -81, -81, 36, 
  -81, 26, -81, -81, -81, 53, -81, 58, 61, 57, 
  62, -81, -81, 37, -81, -81, -81, -81, -81, -2, 
  -5, 84, -9, -81, -3, -81, -81, -81, 2, -81, 
  -81, -81, 13, 86, -81, 23, -81, -81, -81, 81, 
  -81, -81, -22, 8, -81, -81, -81, -32, -81, -81, 
  -81, -81, 76, 15, 69, -81, -81, -81, 22, -25, 
  -81, 14, -81, 25, 18, 79, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, 11, -81, 179, -81};

const int QXmlStreamReader_Table::action_info [] = {
  66, 325, 66, 408, 66, 367, 66, 66, 66, 66, 
  61, 66, 390, 51, 385, 370, 61, 66, 66, 66, 
  51, 66, 66, 61, 66, 378, 403, 407, 66, 66, 
  66, 66, 398, 66, 201, 1, 401, 66, 66, 51, 
  51, 66, 66, 59, 0, 190, 68, 66, 207, 66, 
  206, 179, 172, 366, 409, 182, 343, 277, 51, 190, 
  51, 123, 0, 263, 66, 0, 198, 51, 344, 303, 
  69, 74, 73, 66, 74, 73, 61, 51, 143, 247, 
  0, 51, 61, 74, 73, 311, 309, 62, 60, 74, 
  73, 74, 73, 74, 73, 65, 119, 50, 202, 360, 
  359, 118, 311, 309, 66, 74, 73, 74, 73, 50, 
  153, 54, 53, 71, 70, 243, 0, 51, 307, 298, 
  347, 265, 153, 153, 153, 51, 153, 167, 388, 51, 
  372, 51, 0, 166, 0, 51, 0, 51, 51, 51, 
  389, 51, 54, 53, 74, 73, 186, 185, 194, 193, 
  74, 73, 265, 62, 60, 61, 58, 299, 298, 0, 
  371, 372, 0, 146, 57, 56, 310, 311, 309, 161, 
  163, 152, 162, 66, 0, 374, 153, 0, 161, 163, 
  118, 162, 161, 163, 0, 162, 246, 338, 337, 336, 
  0, 51, 145, 144, 57, 56, 66, 124, 382, 0, 
  410, 16, 210, 208, 66, 124, 62, 60, 61, 125, 
  290, 265, 355, 291, 0, 0, 293, 125, 0, 294, 
  292, 266, 264, 267, 268, 0, 0, 0, 0, 211, 
  209, 0, 0, 284, 0, 0, 0, 0, 0, 13, 
  0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 
  0, 32, 0, 290, 280, 287, 291, 0, 0, 293, 
  0, 0, 294, 292, 0, 0, 0, 0, 278, 281, 
  282, 283, 279, 285, 286, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 98, 0, 93, 0, 84, 92, 80, 85, 94, 
  87, 95, 89, 83, 88, 97, 77, 96, 78, 79, 
  90, 99, 82, 91, 76, 86, 81, 98, 0, 93, 
  0, 84, 111, 110, 85, 94, 87, 95, 89, 83, 
  88, 97, 77, 96, 78, 79, 90, 99, 82, 91, 
  76, 86, 81, 0, 98, 0, 93, 0, 84, 107, 
  106, 85, 94, 87, 95, 89, 83, 88, 97, 77, 
  96, 78, 79, 90, 99, 82, 91, 76, 86, 81, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 98, 0, 93, 
  0, 84, 104, 103, 85, 94, 87, 95, 89, 83, 
  88, 97, 77, 96, 78, 79, 90, 99, 82, 91, 
  76, 86, 81, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  98, 0, 93, 315, 84, 317, 316, 85, 94, 87, 
  95, 89, 83, 88, 97, 77, 96, 78, 79, 90, 
  99, 82, 91, 76, 86, 81, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 236, 223, 231, 213, 222, 249, 248, 
  224, 232, 226, 233, 227, 221, 0, 235, 215, 234, 
  216, 217, 228, 237, 220, 229, 214, 225, 219, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 98, 0, 93, 315, 
  84, 334, 333, 85, 94, 87, 95, 89, 83, 88, 
  97, 77, 96, 78, 79, 90, 99, 82, 91, 76, 
  86, 81, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 236, 
  223, 231, 213, 222, 230, 218, 224, 232, 226, 233, 
  227, 221, 0, 235, 215, 234, 216, 217, 228, 237, 
  220, 229, 214, 225, 219, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 98, 0, 93, 315, 84, 327, 326, 85, 
  94, 87, 95, 89, 83, 88, 97, 77, 96, 78, 
  79, 90, 99, 82, 91, 76, 86, 81, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 98, 0, 93, 315, 84, 
  330, 329, 85, 94, 87, 95, 89, 83, 88, 97, 
  77, 96, 78, 79, 90, 99, 82, 91, 76, 86, 
  81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 236, 223, 
  231, 213, 222, 259, 258, 224, 232, 226, 233, 227, 
  221, 0, 235, 215, 234, 216, 217, 228, 237, 220, 
  229, 214, 225, 219, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 236, 223, 231, 213, 222, 255, 254, 224, 232, 
  226, 233, 227, 221, 0, 235, 215, 234, 216, 217, 
  228, 237, 220, 229, 214, 225, 219, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 30, 380, 25, 5, 15, 24, 
  10, 17, 26, 19, 27, 21, 14, 20, 29, 7, 
  28, 8, 9, 22, 31, 12, 23, 6, 18, 11, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 
  0, 0, 0, 0, 0, 32, 0, 30, 16, 25, 
  5, 15, 24, 10, 17, 26, 19, 27, 21, 14, 
  20, 29, 7, 28, 8, 9, 22, 31, 12, 23, 
  6, 18, 11, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  13, 0, 0, 0, 0, 0, 0, 0, 32, 0, 

  169, 164, 175, 176, 364, 52, 160, 204, 368, 365, 
  174, 361, 418, 205, 369, 173, 168, 165, 393, 189, 
  384, 151, 192, 400, 188, 181, 402, 397, 383, 184, 
  321, 405, 373, 178, 399, 377, 180, 404, 196, 195, 
  301, 183, 342, 115, 113, 200, 297, 302, 108, 197, 
  304, 308, 306, 354, 171, 100, 122, 0, 353, 245, 
  260, 289, 345, 252, 348, 253, 269, 114, 262, 346, 
  244, 387, 261, 251, 349, 379, 350, 352, 351, 341, 
  203, 67, 64, 321, 340, 142, 324, 339, 395, 305, 
  42, 42, 394, 406, 274, 387, 363, 321, 273, 250, 
  362, 148, 376, 321, 112, 0, 375, 0, 276, 0, 
  0, 105, 273, 0, 300, 0, 127, 256, 140, 393, 
  133, 127, 0, 140, 0, 133, 141, 0, 134, 0, 
  0, 358, 0, 134, 0, 0, 0, 0, 356, 357, 
  0, 0, 0, 0, 396, 328, 416, 275, 413, 411, 
  417, 415, 412, 34, 34, 0, 0, 0, 0, 331, 
  0, 414, 0, 0, 0, 335, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 419, 0, 0, 0, 0, 0, 42, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 34, 0, 0, 0, 0, 0, 0, 0};

const int QXmlStreamReader_Table::action_check [] = {
  2, 18, 2, 4, 2, 4, 2, 2, 2, 2, 
  26, 2, 4, 26, 4, 4, 26, 2, 2, 2, 
  26, 2, 2, 26, 2, 4, 14, 24, 2, 2, 
  2, 2, 14, 2, 4, 44, 54, 2, 2, 26, 
  26, 2, 2, 18, -1, 22, 4, 2, 4, 2, 
  4, 13, 19, 4, 0, 12, 4, 4, 26, 22, 
  26, 4, -1, 4, 2, -1, 42, 26, 20, 11, 
  4, 7, 8, 2, 7, 8, 26, 26, 18, 18, 
  -1, 26, 26, 7, 8, 7, 8, 24, 25, 7, 
  8, 7, 8, 7, 8, 24, 4, 6, 36, 34, 
  35, 9, 7, 8, 2, 7, 8, 7, 8, 6, 
  11, 26, 27, 34, 35, 6, -1, 26, 12, 13, 
  15, 20, 11, 11, 11, 26, 11, 13, 4, 26, 
  29, 26, -1, 19, -1, 26, -1, 26, 26, 26, 
  16, 26, 26, 27, 7, 8, 12, 13, 12, 13, 
  7, 8, 20, 24, 25, 26, 18, 12, 13, -1, 
  28, 29, -1, 11, 26, 27, 6, 7, 8, 21, 
  22, 6, 24, 2, -1, 4, 11, -1, 21, 22, 
  9, 24, 21, 22, -1, 24, 18, 37, 38, 39, 
  -1, 26, 40, 41, 26, 27, 2, 3, 16, -1, 
  2, 3, 7, 8, 2, 3, 24, 25, 26, 15, 
  17, 20, 10, 20, -1, -1, 23, 15, -1, 26, 
  27, 30, 31, 32, 33, -1, -1, -1, -1, 34, 
  35, -1, -1, 11, -1, -1, -1, -1, -1, 45, 
  -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, 
  -1, 53, -1, 17, 32, 33, 20, -1, -1, 23, 
  -1, -1, 26, 27, -1, -1, -1, -1, 46, 47, 
  48, 49, 50, 51, 52, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 2, -1, 4, -1, 6, 7, 8, 9, 10, 
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
  21, 22, 23, 24, 25, 26, 27, 2, -1, 4, 
  -1, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, -1, 2, -1, 4, -1, 6, 7, 
  8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 2, -1, 4, 
  -1, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 
  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
  22, 23, 24, 25, 26, 27, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, 45, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, 2, 3, 4, 5, 6, 7, 8, 
  9, 10, 11, 12, 13, 14, -1, 16, 17, 18, 
  19, 20, 21, 22, 23, 24, 25, 26, 27, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, 45, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, 2, -1, 4, 5, 
  6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 
  26, 27, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 45, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 
  3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
  13, 14, -1, 16, 17, 18, 19, 20, 21, 22, 
  23, 24, 25, 26, 27, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, 45, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, 2, -1, 4, 5, 6, 7, 8, 9, 
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 
  20, 21, 22, 23, 24, 25, 26, 27, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 45, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 2, -1, 4, 5, 6, 
  7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 
  27, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, 45, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, 2, 3, 
  4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 
  14, -1, 16, 17, 18, 19, 20, 21, 22, 23, 
  24, 25, 26, 27, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 45, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
  11, 12, 13, 14, -1, 16, 17, 18, 19, 20, 
  21, 22, 23, 24, 25, 26, 27, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, 45, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, 2, 3, 4, 5, 6, 7, 
  8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, 
  -1, -1, -1, -1, -1, 53, -1, 2, 3, 4, 
  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  45, -1, -1, -1, -1, -1, -1, -1, 53, -1, 

  36, 35, 12, 36, 13, 19, 12, 19, 13, 12, 
  36, 13, 1, 12, 12, 12, 12, 12, 50, 19, 
  12, 12, 12, 48, 12, 36, 12, 12, 50, 12, 
  10, 13, 19, 12, 12, 12, 12, 12, 19, 12, 
  44, 35, 16, 13, 16, 12, 12, 16, 63, 42, 
  12, 16, 12, 16, 12, 63, 12, -1, 19, 79, 
  67, 12, 19, 16, 19, 13, 50, 13, 12, 16, 
  19, 16, 13, 13, 16, 1, 19, 16, 16, 50, 
  12, 16, 19, 10, 48, 19, 19, 16, 12, 43, 
  10, 10, 16, 14, 12, 16, 12, 10, 16, 67, 
  16, 37, 16, 10, 63, -1, 20, -1, 12, -1, 
  -1, 63, 16, -1, 12, -1, 6, 67, 8, 50, 
  10, 6, -1, 8, -1, 10, 16, -1, 18, -1, 
  -1, 16, -1, 18, -1, -1, -1, -1, 23, 24, 
  -1, -1, -1, -1, 75, 72, 3, 51, 5, 6, 
  7, 8, 9, 73, 73, -1, -1, -1, -1, 72, 
  -1, 18, -1, -1, -1, 72, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, 4, -1, -1, -1, -1, -1, 10, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, 73, -1, -1, -1, -1, -1, -1, -1};


#include <QCoreApplication>
template <typename T> class QXmlStreamSimpleStack {
    T *data;
    int tos, cap;
public:
    inline QXmlStreamSimpleStack():data(0), tos(-1), cap(0){}
    inline ~QXmlStreamSimpleStack(){ if (data) qFree(data); }

    inline void reserve(int extraCapacity) {
        if (tos + extraCapacity + 1 > cap) {
            cap = qMax(tos + extraCapacity + 1, cap << 1 );
            data = reinterpret_cast<T *>(qRealloc(data, cap * sizeof(T)));
        }
    }

    inline T &push() { reserve(1); return data[++tos]; }
    inline T &rawPush() { return data[++tos]; }
    inline const T &top() const { return data[tos]; }
    inline T &top() { return data[tos]; }
    inline T &pop() { return data[tos--]; }
    inline T &operator[](int index) { return data[index]; }
    inline const T &at(int index) const { return data[index]; }
    inline int size() const { return tos + 1; }
    inline void resize(int s) { tos = s - 1; }
    inline bool isEmpty() const { return tos < 0; }
    inline void clear() { tos = -1; }
};


class QXmlStream
{
    Q_DECLARE_TR_FUNCTIONS(QXmlStream)
};

class QXmlStreamPrivateTagStack {
public:
    struct NamespaceDeclaration
    {
        QStringRef prefix;
        QStringRef namespaceUri;
    };

    struct Tag
    {
        QStringRef name;
        QStringRef qualifiedName;
        NamespaceDeclaration namespaceDeclaration;
        int tagStackStringStorageSize;
        int namespaceDeclarationsSize;
    };


    QXmlStreamPrivateTagStack();
    QXmlStreamSimpleStack<NamespaceDeclaration> namespaceDeclarations;
    QString tagStackStringStorage;
    int tagStackStringStorageSize;
    int tagStackDefaultStringStorageSize;
    bool tagsDone;

    inline QStringRef addToStringStorage(const QStringRef &s) {
        int pos = tagStackStringStorageSize;
        tagStackStringStorage.insert(pos, s.unicode(), s.size());
        tagStackStringStorageSize = tagStackStringStorage.size();
        return QStringRef(&tagStackStringStorage, pos, s.size());
    }
    inline QStringRef addToStringStorage(const QString &s) {
        int pos = tagStackStringStorageSize;
        tagStackStringStorage.insert(pos, s.unicode(), s.size());
        tagStackStringStorageSize = tagStackStringStorage.size();
        return QStringRef(&tagStackStringStorage, pos, s.size());
    }

    QXmlStreamSimpleStack<Tag> tagStack;

    inline void initTagStack() {
        tagStackStringStorageSize = tagStackDefaultStringStorageSize;
        namespaceDeclarations.resize(1);
    }

    inline Tag &tagStack_pop() {
        Tag& tag = tagStack.pop();
        tagStackStringStorageSize = tag.tagStackStringStorageSize;
        namespaceDeclarations.resize(tag.namespaceDeclarationsSize);
        tagsDone = tagStack.isEmpty();
        return tag;
    }
    inline Tag &tagStack_push() {
        Tag &tag = tagStack.push();
        tag.tagStackStringStorageSize = tagStackStringStorageSize;
        tag.namespaceDeclarationsSize = namespaceDeclarations.size();
        return tag;
    }
};

class QXmlStreamReaderPrivate : public QXmlStreamReader_Table, public QXmlStreamPrivateTagStack{
    QXmlStreamReader *q_ptr;
    Q_DECLARE_PUBLIC(QXmlStreamReader)
public:
    QXmlStreamReaderPrivate(QXmlStreamReader *q);
    ~QXmlStreamReaderPrivate();
    void init();

    QByteArray rawReadBuffer;
    QByteArray dataBuffer;
    uchar firstByte;
    qint64 nbytesread;
    QString readBuffer;
    int readBufferPos;
    QXmlStreamSimpleStack<uint> putStack;
    struct Entity {
        Entity(const QString& str = QString())
            :value(str), external(false), unparsed(false), literal(false),
             hasBeenParsed(false), isCurrentlyReferenced(false){}
        static inline Entity createLiteral(const QString &entity)
            { Entity result(entity); result.literal = result.hasBeenParsed = true; return result; }
        QString value;
        uint external : 1;
        uint unparsed : 1;
        uint literal : 1;
        uint hasBeenParsed : 1;
        uint isCurrentlyReferenced : 1;
    };
    QHash<QString, Entity> entityHash;
    QHash<QString, Entity> parameterEntityHash;
    QXmlStreamSimpleStack<Entity *>entityReferenceStack;
    inline bool referenceEntity(Entity &entity) {
        if (entity.isCurrentlyReferenced) {
            raiseWellFormedError(QXmlStream::tr("Recursive entity detected."));
            return false;
        }
        entity.isCurrentlyReferenced = true;
        entityReferenceStack.push() = &entity;
        injectToken(ENTITY_DONE);
        return true;
    }


    QIODevice *device;
    bool deleteDevice;
    QTextCodec *codec;
    QTextDecoder *decoder;
    bool atEnd;
    QXmlStreamReader::TokenType type;
    QXmlStreamReader::Error error;
    QString errorString;

    qint64 lineNumber, lastLineStart, characterOffset;


    void write(const QString &);
    void write(const char *);


    QXmlStreamAttributes attributes;
    QStringRef namespaceForPrefix(const QStringRef &prefix);
    void resolveTag();
    void resolvePublicNamespaces();
    void resolveDtd();
    uint resolveCharRef(int symbolIndex);
    bool checkStartDocument();
    void startDocument(const QStringRef &version);
    void parseError();
    void checkPublicLiteral(const QStringRef &publicId);

    bool scanDtd;
    QStringRef lastAttributeValue;
    bool lastAttributeIsCData;
    struct DtdAttribute {
        QStringRef tagName;
        QStringRef attributeQualifiedName;
        QStringRef attributePrefix;
        QStringRef attributeName;
        QStringRef defaultValue;
        bool isCDATA;
    };
    QXmlStreamSimpleStack<DtdAttribute> dtdAttributes;
    struct NotationDeclaration {
        QStringRef name;
        QStringRef publicId;
        QStringRef systemId;
    };
    QXmlStreamSimpleStack<NotationDeclaration> notationDeclarations;
    QXmlStreamNotationDeclarations publicNotationDeclarations;
    QXmlStreamNamespaceDeclarations publicNamespaceDeclarations;

    struct EntityDeclaration {
        QStringRef name;
        QStringRef notationName;
        QStringRef publicId;
        QStringRef systemId;
        QStringRef value;
        bool parameter;
        bool external;
        inline void clear() {
            name.clear();
            notationName.clear();
            publicId.clear();
            systemId.clear();
            value.clear();
            parameter = external = false;
        }
    };
    QXmlStreamSimpleStack<EntityDeclaration> entityDeclarations;
    QXmlStreamEntityDeclarations publicEntityDeclarations;

    QStringRef text;

    QStringRef prefix, namespaceUri, qualifiedName, name;
    QStringRef processingInstructionTarget, processingInstructionData;
    uint isEmptyElement : 1;
    uint isWhitespace : 1;
    uint isCDATA : 1;
    uint standalone : 1;
    uint hasCheckedStartDocument : 1;
    uint hasSeenTag : 1;
    uint inParseEntity : 1;
    uint referenceToUnparsedEntityDetected : 1;
    uint hasExternalDtdSubset : 1;
    uint lockEncoding : 1;
    uint namespaceProcessing : 1;

    int resumeReduction;
    void resume(int rule);

    inline bool entitiesMustBeDeclared() const {
        return (!inParseEntity
                && (standalone
                    || (!referenceToUnparsedEntityDetected
                        && !hasExternalDtdSubset)));
    }

    // qlalr parser
    int tos;
    int stack_size;
    struct Value {
        int pos;
        int len;
        int prefix;
        ushort c;
    };

    Value *sym_stack;
    int *state_stack;
    inline void reallocateStack();
    inline Value &sym(int index) const
    { return sym_stack[tos + index - 1]; }
    QString textBuffer, dtdBuffer;
    inline void clearTextBuffer() {
        if (!scanDtd) {
            textBuffer.resize(0);
            textBuffer.reserve(256);
        }
    }
    struct Attribute {
        Value key;
        Value value;
    };
    QXmlStreamSimpleStack<Attribute> attributeStack;

    inline QStringRef symString(int index) {
        const Value &symbol = sym(index);
        return QStringRef(&textBuffer, symbol.pos + symbol.prefix, symbol.len - symbol.prefix);
    }
    inline QStringRef symName(int index) {
        const Value &symbol = sym(index);
        return QStringRef(&textBuffer, symbol.pos, symbol.len);
    }
    inline QStringRef symString(int index, int offset) {
        const Value &symbol = sym(index);
        return QStringRef(&textBuffer, symbol.pos + symbol.prefix + offset, symbol.len - symbol.prefix -  offset);
    }
    inline QStringRef symPrefix(int index) {
        const Value &symbol = sym(index);
        if (symbol.prefix)
            return QStringRef(&textBuffer, symbol.pos, symbol.prefix - 1);
        return QStringRef();
    }
    inline QStringRef symString(const Value &symbol) {
        return QStringRef(&textBuffer, symbol.pos + symbol.prefix, symbol.len - symbol.prefix);
    }
    inline QStringRef symName(const Value &symbol) {
        return QStringRef(&textBuffer, symbol.pos, symbol.len);
    }
    inline QStringRef symPrefix(const Value &symbol) {
        if (symbol.prefix)
            return QStringRef(&textBuffer, symbol.pos, symbol.prefix - 1);
        return QStringRef();
    }

    inline void clearSym() { Value &val = sym(1); val.pos = textBuffer.size(); val.len = 0; }


    short token;
    ushort token_char;

    uint filterCarriageReturn();
    inline uint getChar();
    inline ushort peekChar();
    inline void putChar(uint c) { putStack.push() = c; }
    inline void putChar(QChar c) { putStack.push() =  c.unicode(); }
    void putString(const QString &s, int from = 0);
    void putStringLiteral(const QString &s);
    void putStringWithLiteralQuotes(const QString &s);
    ushort getChar_helper();

    bool scanUntil(const char *str, short tokenToInject = -1);
    bool scanString(const char *str, short tokenToInject, bool requireSpace = true);
    inline void injectToken(ushort tokenToInject) {
        putChar(int(tokenToInject) << 16);
    }

    static bool validateName(const QStringRef &name);

    void parseEntity(const QString &value);
    QXmlStreamReaderPrivate *entityParser;

    bool scanAfterLangleBang();
    bool scanPublicOrSystem();
    bool scanNData();
    bool scanAfterDefaultDecl();
    bool scanAttType();


    // scan optimization functions. Not strictly necessary but LALR is
    // not very well suited for scanning fast
    int fastScanLiteralContent();
    int fastScanSpace();
    int fastScanContentCharList();
    int fastScanName(int *prefix = 0);
    inline int fastScanNMTOKEN();


    bool parse();
    inline void consumeRule(int);

    void raiseError(QXmlStreamReader::Error error, const QString& message = QString());
    void raiseWellFormedError(const QString &message);

};

bool QXmlStreamReaderPrivate::parse()
{
    // cleanup currently reported token

    switch (type) {
    case QXmlStreamReader::StartElement:
        name.clear();
        prefix.clear();
	qualifiedName.clear();
        namespaceUri.clear();
        if (publicNamespaceDeclarations.size())
            publicNamespaceDeclarations.clear();
        if (attributes.size())
            attributes.resize(0);
        if (isEmptyElement) {
            type = QXmlStreamReader::EndElement;
            Tag &tag = tagStack_pop();
            namespaceUri = tag.namespaceDeclaration.namespaceUri;
            name = tag.name;
	    qualifiedName = tag.qualifiedName;
            isEmptyElement = false;
            return true;
        }
        clearTextBuffer();
        break;
    case QXmlStreamReader::EndElement:
        name.clear();
        prefix.clear();
	qualifiedName.clear();
        namespaceUri.clear();
        clearTextBuffer();
        break;
    case QXmlStreamReader::DTD:
        publicNotationDeclarations.clear();
        publicEntityDeclarations.clear();
        // fall through
    case QXmlStreamReader::Comment:
    case QXmlStreamReader::Characters:
        isCDATA = isWhitespace = false;
        text.clear();
        clearTextBuffer();
        break;
    case QXmlStreamReader::EntityReference:
        text.clear();
        name.clear();
        clearTextBuffer();
        break;
    case QXmlStreamReader::ProcessingInstruction:
        processingInstructionTarget.clear();
        processingInstructionData.clear();
	clearTextBuffer();
        break;
    case QXmlStreamReader::NoToken:
    case QXmlStreamReader::Invalid:
        break;
    default:
        clearTextBuffer();
        ;
    }

    type = QXmlStreamReader::NoToken;


    // the main parse loop
    int act, r;

    if (resumeReduction) {
        act = state_stack[tos-1];
        r = resumeReduction;
        resumeReduction = 0;
        goto ResumeReduction;
    }

    act = state_stack[tos];

    forever {
        if (token == -1 && - TERMINAL_COUNT != action_index[act]) {
            uint cu = getChar();
            token = NOTOKEN;
            token_char = cu;
            if (cu & 0xff0000) {
                token = cu >> 16;
            } else switch (token_char) {
            case 0xfffe:
            case 0xffff:
                token = ERROR;
                break;
            case '\r':
                token = SPACE;
                if (cu == '\r') {
                    if ((token_char = filterCarriageReturn())) {
                        ++lineNumber;
                        lastLineStart = characterOffset + readBufferPos;
                        break;
                    }
                } else {
                    break;
                }
                // fall through
            case '\0': {
                token = EOF_SYMBOL;
                if (!tagsDone && !inParseEntity) {
                    int a = t_action(act, token);
                    if (a < 0) {
                        raiseError(QXmlStreamReader::PrematureEndOfDocumentError);
                        return false;
                    }
                }

            } break;
            case '\n':
                ++lineNumber;
                lastLineStart = characterOffset + readBufferPos;
            case ' ':
            case '\t':
                token = SPACE;
                break;
            case '&':
                token = AMPERSAND;
                break;
            case '#':
                token = HASH;
                break;
            case '\'':
                token = QUOTE;
                break;
            case '\"':
                token = DBLQUOTE;
                break;
            case '<':
                token = LANGLE;
                break;
            case '>':
                token = RANGLE;
                break;
            case '[':
                token = LBRACK;
                break;
            case ']':
                token = RBRACK;
                break;
            case '(':
                token = LPAREN;
                break;
            case ')':
                token = RPAREN;
                break;
            case '|':
                token = PIPE;
                break;
            case '=':
                token = EQ;
                break;
            case '%':
                token = PERCENT;
                break;
            case '/':
                token = SLASH;
                break;
            case ':':
                token = COLON;
                break;
            case ';':
                token = SEMICOLON;
                break;
            case ',':
                token = COMMA;
                break;
            case '-':
                token = DASH;
                break;
            case '+':
                token = PLUS;
                break;
            case '*':
                token = STAR;
                break;
            case '.':
                token = DOT;
                break;
            case '?':
                token = QUESTIONMARK;
                break;
            case '!':
                token = BANG;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                token = DIGIT;
                break;
            default:
                if (cu < 0x20)
                    token = NOTOKEN;
                else
                    token = LETTER;
                break;
            }
        }

        act = t_action (act, token);
        if (act == ACCEPT_STATE) {
            // reset the parser in case someone resumes (process instructions can follow a valid document)
            tos = 0;
            state_stack[tos++] = 0;
            state_stack[tos] = 0;
            return true;
        } else if (act > 0) {
            if (++tos == stack_size)
                reallocateStack();

            Value &val = sym_stack[tos];
            val.c = token_char;
            val.pos = textBuffer.size();
            val.prefix = 0;
            val.len = 1;
            if (token_char)
                textBuffer.inline_append(token_char);

            state_stack[tos] = act;
            token = -1;


        } else if (act < 0) {
            r = - act - 1;

#if defined (QLALR_DEBUG)
            int ridx = rule_index[r];
            printf ("%3d) %s ::=", r + 1, spell[rule_info[ridx]]);
            ++ridx;
            for (int i = ridx; i < ridx + rhs[r]; ++i) {
                int symbol = rule_info[i];
                if (const char *name = spell[symbol])
                    printf (" %s", name);
                else
                    printf (" #%d", symbol);
            }
            printf ("\n");
#endif

            tos -= rhs[r];
            act = state_stack[tos++];
        ResumeReduction:
            switch (r) {

        case 0:
            type = QXmlStreamReader::EndDocument;
        break;

        case 1:
            if (type != QXmlStreamReader::Invalid) {
                if (hasSeenTag || inParseEntity) {
                    type = QXmlStreamReader::EndDocument;
                } else {
                    raiseError(QXmlStreamReader::PrematureEndOfDocumentError, QXmlStream::tr("Start tag expected."));
                    // reset the parser
                    tos = 0;
                    state_stack[tos++] = 0;
                    state_stack[tos] = 0;
                    return false;
                }
            }
        break;

        case 10:
            entityReferenceStack.pop()->isCurrentlyReferenced = false;
            clearSym();
        break;

        case 11:
            if (!scanString(spell[VERSION], VERSION, false) && atEnd) {
                resume(11);
                return false;
            }
        break;

        case 12:
            type = QXmlStreamReader::StartDocument;
            startDocument(symString(6));
        break;

        case 13:
            hasExternalDtdSubset = true;
        break;

        case 14:
            checkPublicLiteral(symString(2));
            hasExternalDtdSubset = true;
        break;

        case 16:
            if (!scanPublicOrSystem() && atEnd) {
                resume(16);
                return false;
            }
        break;

        case 17:
        case 18:
        case 19:
        case 20:
            type = QXmlStreamReader::DTD;
            text = &textBuffer;
        break;

        case 21:
            scanDtd = true;
        break;

        case 22:
            scanDtd = false;
        break;

        case 36:
            if (!scanString(spell[EMPTY], EMPTY, false)
                && !scanString(spell[ANY], ANY, false)
                && atEnd) {
                resume(36);
                return false;
            }
        break;

        case 42:
            if (!scanString(spell[PCDATA], PCDATA, false) && atEnd) {
                resume(42);
                return false;
            }
        break;

        case 67: {
            lastAttributeIsCData = true;
        } break;

        case 78:
            if (!scanAfterDefaultDecl() && atEnd) {
                resume(78);
                return false;
            }
        break;

        case 83:
                sym(1) = sym(2);
                lastAttributeValue.clear();
                lastAttributeIsCData = false;
                if (!scanAttType() && atEnd) {
                    resume(83);
                    return false;
                }
        break;

        case 84: {
            DtdAttribute &dtdAttribute = dtdAttributes.push();
            dtdAttribute.tagName.clear();
            dtdAttribute.isCDATA = lastAttributeIsCData;
            dtdAttribute.attributePrefix = addToStringStorage(symPrefix(1));
            dtdAttribute.attributeName = addToStringStorage(symString(1));
            dtdAttribute.attributeQualifiedName = addToStringStorage(symName(1));
            if (lastAttributeValue.isNull()) {
                dtdAttribute.defaultValue.clear();
            } else {
                if (dtdAttribute.isCDATA)
                    dtdAttribute.defaultValue = addToStringStorage(lastAttributeValue);
                else
                    dtdAttribute.defaultValue = addToStringStorage(lastAttributeValue.toString().simplified());

            }
        } break;

        case 88: {
            if (referenceToUnparsedEntityDetected && !standalone)
                break;
            int n = dtdAttributes.size();
            QStringRef tagName = addToStringStorage(symString(3));
            while (n--) {
                DtdAttribute &dtdAttribute = dtdAttributes[n];
                if (!dtdAttribute.tagName.isNull())
                    break;
                dtdAttribute.tagName = tagName;
                for (int i = 0; i < n; ++i) {
                    if ((dtdAttributes[i].tagName.isNull() || dtdAttributes[i].tagName == tagName)
                        && dtdAttributes[i].attributeQualifiedName == dtdAttribute.attributeQualifiedName) {
                        dtdAttribute.attributeQualifiedName.clear(); // redefined, delete it
                        break;
                    }
                }
            }
        } break;

        case 89: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(89);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(3);
        } break;

        case 90: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(90);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(5);
            entityDeclaration.parameter = true;
        } break;

        case 91: {
            if (!scanNData() && atEnd) {
                resume(91);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.systemId = symString(3);
            entityDeclaration.external = true;
        } break;

        case 92: {
            if (!scanNData() && atEnd) {
                resume(92);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            checkPublicLiteral((entityDeclaration.publicId = symString(3)));
            entityDeclaration.systemId = symString(5);
            entityDeclaration.external = true;
        } break;

        case 93: {
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.notationName = symString(3);
            if (entityDeclaration.parameter)
                raiseWellFormedError(QXmlStream::tr("NDATA in parameter entity declaration."));
        }
        //fall through

        case 94:
        case 95: {
            if (referenceToUnparsedEntityDetected && !standalone) {
                entityDeclarations.pop();
                break;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            if (!entityDeclaration.external)
                entityDeclaration.value = symString(2);
            QString entityName = entityDeclaration.name.toString();
            QHash<QString, Entity> &hash = entityDeclaration.parameter ? parameterEntityHash : entityHash;
            if (!hash.contains(entityName)) {
                Entity entity(entityDeclaration.value.toString());
                entity.unparsed = (!entityDeclaration.notationName.isNull());
                entity.external = entityDeclaration.external;
                hash.insert(entityName, entity);
            }
        } break;

        case 96: {
            type = QXmlStreamReader::ProcessingInstruction;
            int pos = sym(4).pos + sym(4).len;
            processingInstructionTarget = symString(3);
            if (scanUntil("?>")) {
                processingInstructionData = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 2);
                const QString piTarget(processingInstructionTarget.toString());
                if (!piTarget.compare(QLatin1String("xml"), Qt::CaseInsensitive)) {
                    raiseWellFormedError(QXmlStream::tr("XML declaration not at start of document."));
                }
                else if(!QXmlUtils::isNCName(piTarget))
                    raiseWellFormedError(QXmlStream::tr("%1 is an invalid processing instruction name.").arg(piTarget));
            } else if (type != QXmlStreamReader::Invalid){
                resume(96);
                return false;
            }
        } break;

        case 97:
            type = QXmlStreamReader::ProcessingInstruction;
            processingInstructionTarget = symString(3);
            if (!processingInstructionTarget.toString().compare(QLatin1String("xml"), Qt::CaseInsensitive))
                raiseWellFormedError(QXmlStream::tr("Invalid processing instruction name."));
        break;

        case 98:
            if (!scanAfterLangleBang() && atEnd) {
                resume(98);
                return false;
            }
        break;

        case 99:
            if (!scanUntil("--")) {
                resume(99);
                return false;
            }
        break;

        case 100: {
            type = QXmlStreamReader::Comment;
            int pos = sym(1).pos + 4;
            text = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 3);
        } break;

        case 101: {
            type = QXmlStreamReader::Characters;
            isCDATA = true;
            int pos = sym(2).pos;
            if (scanUntil("]]>", -1)) {
                text = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 3);
            } else {
                resume(101);
                return false;
            }
        } break;

        case 102: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(102);
                return false;
            }
            NotationDeclaration &notationDeclaration = notationDeclarations.push();
            notationDeclaration.name = symString(3);
        } break;

        case 103: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId = symString(3);
            notationDeclaration.publicId.clear();
        } break;

        case 104: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId.clear();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
        } break;

        case 105: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
            notationDeclaration.systemId = symString(5);
        } break;

        case 127:
            isWhitespace = true;
            // fall through

        case 128:
        case 129:
        case 130:
            type = QXmlStreamReader::Characters;
            sym(1).len += fastScanContentCharList();
            if (atEnd && !inParseEntity) {
                resume(130);
                return false;
            }
            text = &textBuffer;
        break;

        case 131:
        case 132:
            clearSym();
        break;

        case 133:
        case 134:
            sym(1) = sym(2);
        break;

        case 135:
        case 136:
        case 137:
        case 138:
            sym(1).len += sym(2).len;
        break;

        case 164:
            textBuffer.data()[textBuffer.size()-1] = QLatin1Char(' ');
        break;

        case 165:
            sym(1).len += fastScanLiteralContent();
            if (atEnd) {
                resume(165);
                return false;
            }
        break;

        case 166:
        case 167:
            clearSym();
        break;

        case 168:
        case 169:
	    sym(1) = sym(2);
        break;

        case 170:
        case 171:
        case 172:
        case 173:
            sym(1).len += sym(2).len;
        break;

        case 203:
        case 204:
            clearSym();
        break;

        case 205:
        case 206:
            sym(1) = sym(2);
            lastAttributeValue = symString(1);
        break;

        case 207:
        case 208:
        case 209:
        case 210:
            sym(1).len += sym(2).len;
        break;

        case 219: {
            QStringRef prefix = symPrefix(1);
            if (prefix.isEmpty() && symString(1) == QLatin1String("xmlns") && namespaceProcessing) {
                NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.push();
                namespaceDeclaration.prefix.clear();

                const QStringRef ns(symString(5));
                if(ns == QLatin1String("http://www.w3.org/2000/xmlns/") ||
                   ns == QLatin1String("http://www.w3.org/XML/1998/namespace"))
                    raiseWellFormedError(QXmlStream::tr("Illegal namespace declaration."));
                else
                    namespaceDeclaration.namespaceUri = addToStringStorage(ns);
            } else {
                Attribute &attribute = attributeStack.push();
                attribute.key = sym(1);
                attribute.value = sym(5);

                QStringRef attributeQualifiedName = symName(1);
                bool normalize = false;
                for (int a = 0; a < dtdAttributes.size(); ++a) {
                    DtdAttribute &dtdAttribute = dtdAttributes[a];
                    if (!dtdAttribute.isCDATA
                        && dtdAttribute.tagName == qualifiedName
                        && dtdAttribute.attributeQualifiedName == attributeQualifiedName
                        ) {
                        normalize = true;
                        break;
                    }
                }
                if (normalize) {
                    // normalize attribute value (simplify and trim)
                    int pos = textBuffer.size();
                    int n = 0;
                    bool wasSpace = true;
                    for (int i = 0; i < attribute.value.len; ++i) {
                        QChar c = textBuffer.at(attribute.value.pos + i);
                        if (c.unicode() == ' ') {
                            if (wasSpace)
                                continue;
                            wasSpace = true;
                        } else {
                            wasSpace = false;
                        }
                        textBuffer.inline_append(textBuffer.at(attribute.value.pos + i));
                        ++n;
                    }
                    if (wasSpace)
                        while (n && textBuffer.at(pos + n - 1).unicode() == ' ')
                            --n;
                    attribute.value.pos = pos;
                    attribute.value.len = n;
                }
                if (prefix == QLatin1String("xmlns") && namespaceProcessing) {
                    NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.push();
                    QStringRef namespacePrefix = symString(attribute.key);
                    QStringRef namespaceUri = symString(attribute.value);
                    attributeStack.pop();
                    if ((namespacePrefix == QLatin1String("xml")
                         ^ namespaceUri == QLatin1String("http://www.w3.org/XML/1998/namespace"))
                        || namespaceUri == QLatin1String("http://www.w3.org/2000/xmlns/")
                        || namespaceUri.isEmpty()
                        || namespacePrefix == QLatin1String("xmlns"))
                        raiseWellFormedError(QXmlStream::tr("Illegal namespace declaration."));

                    namespaceDeclaration.prefix = addToStringStorage(namespacePrefix);
                    namespaceDeclaration.namespaceUri = addToStringStorage(namespaceUri);
                }
            }
        } break;

        case 225: {
            Tag &tag = tagStack_push();
            prefix = tag.namespaceDeclaration.prefix  = addToStringStorage(symPrefix(2));
            name = tag.name = addToStringStorage(symString(2));
            qualifiedName = tag.qualifiedName = addToStringStorage(symName(2));
            if (!validateName(qualifiedName))
                raiseWellFormedError(QXmlStream::tr("Invalid XML name."));
        } break;

        case 226:
            isEmptyElement = true;
        // fall through

        case 227:
            type = QXmlStreamReader::StartElement;
            resolveTag();
            if (tagStack.size() == 1 && hasSeenTag && !inParseEntity)
                raiseWellFormedError(QXmlStream::tr("Extra content at end of document."));
            hasSeenTag = true;
        break;

        case 228: {
            type = QXmlStreamReader::EndElement;
            Tag &tag = tagStack_pop();

            namespaceUri = tag.namespaceDeclaration.namespaceUri;
            name = tag.name;
            qualifiedName = tag.qualifiedName;
            if (qualifiedName != symName(3))
                raiseWellFormedError(QXmlStream::tr("Opening and ending tag mismatch."));
        } break;

        case 229: {
            sym(1).len += sym(2).len + 1;
            QString reference = symString(2).toString();
            if (entityHash.contains(reference)) {
                Entity &entity = entityHash[reference];
                if (entity.unparsed) {
                    raiseWellFormedError(QXmlStream::tr("Reference to unparsed entity '%1'.").arg(reference));
                } else {
                    if (!entity.hasBeenParsed) {
                        parseEntity(entity.value);
                        entity.hasBeenParsed = true;
                    }
                    if (entity.literal)
                        putStringLiteral(entity.value);
                    else if (referenceEntity(entity))
                        putString(entity.value);
                    textBuffer.chop(2 + sym(2).len);
                    clearSym();
                }
                break;
            } else if (entitiesMustBeDeclared()) {
                raiseWellFormedError(QXmlStream::tr("Entity '%1' not declared.").arg(reference));
                break;
            }
            type = QXmlStreamReader::EntityReference;
            name = symString(2);

        } break;

        case 230: {
            sym(1).len += sym(2).len + 1;
            QString reference = symString(2).toString();
            if (parameterEntityHash.contains(reference)) {
                Entity &entity = parameterEntityHash[reference];
                if (entity.unparsed || entity.external) {
                    referenceToUnparsedEntityDetected = true;
                } else {
                    if (referenceEntity(entity))
                        putString(entity.value);
                    textBuffer.chop(2 + sym(2).len);
                    clearSym();
                }
            } else if (entitiesMustBeDeclared()) {
                raiseWellFormedError(QXmlStream::tr("Entity '%1' not declared.").arg(symString(2).toString()));
            }
        } break;

        case 231:
            sym(1).len += sym(2).len + 1;
        break;

        case 232: {
            sym(1).len += sym(2).len + 1;
            QString reference = symString(2).toString();
            if (entityHash.contains(reference)) {
                Entity &entity = entityHash[reference];
                if (entity.unparsed || entity.value.isNull()) {
                    raiseWellFormedError(QXmlStream::tr("Reference to external entity '%1' in attribute value.").arg(reference));
                    break;
                }
                if (!entity.hasBeenParsed) {
                    parseEntity(entity.value);
                    entity.hasBeenParsed = true;
                }
                if (entity.literal)
                    putStringLiteral(entity.value);
                else if (referenceEntity(entity))
                    putStringWithLiteralQuotes(entity.value);
                textBuffer.chop(2 + sym(2).len);
                clearSym();
            } else if (entitiesMustBeDeclared()) {
                raiseWellFormedError(QXmlStream::tr("Entity '%1' not declared.").arg(reference));
            }
        } break;

        case 233: {
            if (uint s = resolveCharRef(3)) {
                if (s >= 0xffff)
                    putStringLiteral(QString::fromUcs4(&s, 1));
                else
                    putChar((LETTER << 16) | s);

                textBuffer.chop(3 + sym(3).len);
                clearSym();
            } else {
                raiseWellFormedError(QXmlStream::tr("Invalid character reference."));
            }
        } break;

        case 234: {
            if (uint s = resolveCharRef(3)) {
                if ( s == '\n' || s == '\r')
                    sym(1).len += 2 + sym(3).len;
                else {
                    if (s >= 0xffff)
                        putStringLiteral(QString::fromUcs4(&s, 1));
                    else
                        putChar((LETTER << 16) | s);

                    textBuffer.chop(3 + sym(3).len);
                    clearSym();
                }
            } else {
                raiseWellFormedError(QXmlStream::tr("Invalid character reference."));
            }
        } break;

        case 237:
        case 238:
            sym(1).len += sym(2).len;
        break;

        case 251:
            sym(1).len += fastScanSpace();
            if (atEnd) {
                resume(251);
                return false;
            }
        break;

        case 254: {
            sym(1).len += fastScanName(&sym(1).prefix);
            if (atEnd) {
                resume(254);
                return false;
            }
        } break;

        case 255:
            sym(1).len += fastScanName();
            if (atEnd) {
                resume(255);
                return false;
            }
        break;

        case 256:
        case 257:
        case 258:
        case 259:
        case 260:
            sym(1).len += fastScanNMTOKEN();
            if (atEnd) {
                resume(260);
                return false;
            }
        
        break;

    default:
        ;
    } // switch
            act = state_stack[tos] = nt_action (act, lhs[r] - TERMINAL_COUNT);
            if (type != QXmlStreamReader::NoToken)
                return true;
        } else {
            parseError();
            break;
        }
    }
    return false;
}

#endif // QXMLSTREAM_P_H

