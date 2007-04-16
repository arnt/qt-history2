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

    ACCEPT_STATE = 406,
    RULE_COUNT = 260,
    STATE_COUNT = 417,
    TERMINAL_COUNT = 55,
    NON_TERMINAL_COUNT = 80,

    GOTO_INDEX_OFFSET = 417,
    GOTO_INFO_OFFSET = 944,
    GOTO_CHECK_OFFSET = 944,
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
  100, 100, 100, 100, 100, 100, 100, 101, 102, 102, 
  102, 102, 104, 106, 107, 107, 81, 81, 108, 108, 
  109, 109, 82, 82, 82, 63, 63, 73, 111, 61, 
  112, 113, 83, 83, 83, 114, 114, 114, 114, 114, 
  114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 
  114, 114, 114, 114, 114, 114, 115, 115, 115, 115, 
  68, 68, 68, 68, 116, 117, 116, 117, 116, 117, 
  116, 117, 119, 119, 119, 119, 119, 119, 119, 119, 
  119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 
  119, 119, 119, 119, 118, 110, 110, 110, 110, 120, 
  121, 120, 121, 120, 121, 120, 121, 122, 122, 122, 
  122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 
  122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 
  122, 122, 103, 103, 103, 103, 125, 126, 125, 126, 
  125, 125, 126, 126, 127, 127, 127, 127, 129, 69, 
  69, 69, 130, 130, 131, 60, 58, 59, 132, 79, 
  124, 128, 123, 133, 133, 133, 133, 56, 56, 56, 
  56, 56, 56, 56, 56, 56, 56, 56, 56, 71, 
  67, 67, 105, 74, 99, 99, 99, 99, 99, 134};

const int QXmlStreamReader_Table:: rhs[] = {
  2, 1, 4, 2, 2, 2, 2, 2, 2, 0, 
  1, 1, 9, 2, 4, 0, 4, 6, 4, 4, 
  6, 1, 3, 1, 1, 1, 2, 2, 2, 1, 
  1, 1, 1, 1, 1, 1, 4, 4, 1, 1, 
  1, 1, 1, 2, 1, 1, 1, 0, 2, 2, 
  2, 6, 6, 1, 5, 1, 5, 3, 5, 0, 
  1, 6, 8, 4, 2, 1, 5, 1, 1, 1, 
  1, 1, 1, 1, 1, 6, 7, 1, 2, 2, 
  1, 4, 3, 3, 1, 2, 5, 6, 4, 6, 
  3, 5, 5, 3, 4, 4, 5, 2, 3, 2, 
  2, 4, 5, 5, 7, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  2, 2, 3, 3, 2, 2, 2, 2, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 2, 2, 3, 3, 2, 
  2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 2, 2, 3, 3, 2, 2, 2, 2, 
  1, 1, 1, 1, 1, 1, 1, 1, 5, 0, 
  1, 3, 1, 3, 2, 4, 3, 5, 3, 3, 
  3, 3, 4, 1, 1, 2, 2, 2, 4, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 0, 1, 
  0, 1, 1, 1, 1, 1, 1, 1, 1, 2};

const int QXmlStreamReader_Table::action_default [] = {
  10, 249, 0, 2, 1, 0, 124, 116, 118, 119, 
  126, 129, 122, 11, 113, 107, 0, 108, 128, 110, 
  114, 112, 120, 123, 125, 106, 109, 111, 117, 115, 
  127, 121, 12, 242, 246, 241, 0, 130, 238, 245, 
  16, 240, 248, 247, 0, 244, 249, 220, 243, 0, 
  0, 254, 0, 235, 234, 0, 237, 236, 233, 229, 
  98, 253, 0, 225, 0, 0, 250, 96, 97, 100, 
  0, 0, 251, 0, 0, 0, 163, 155, 157, 158, 
  132, 144, 161, 152, 146, 147, 143, 149, 153, 151, 
  159, 162, 142, 145, 148, 150, 156, 154, 164, 160, 
  140, 165, 0, 134, 138, 136, 141, 131, 139, 0, 
  137, 133, 135, 0, 15, 14, 252, 0, 22, 19, 
  251, 0, 0, 18, 0, 0, 31, 36, 30, 0, 
  32, 251, 0, 33, 0, 24, 0, 34, 0, 26, 
  35, 25, 0, 230, 40, 39, 251, 42, 48, 251, 
  41, 0, 43, 251, 48, 251, 0, 48, 251, 0, 
  0, 47, 45, 46, 50, 51, 251, 251, 0, 56, 
  251, 53, 251, 0, 57, 0, 54, 251, 52, 251, 
  0, 55, 64, 49, 0, 251, 60, 251, 0, 58, 
  61, 62, 0, 251, 0, 0, 59, 63, 44, 65, 
  0, 38, 0, 0, 251, 0, 93, 94, 0, 0, 
  0, 0, 251, 0, 199, 190, 192, 194, 167, 179, 
  197, 188, 182, 180, 183, 178, 185, 187, 195, 198, 
  177, 181, 184, 186, 191, 189, 193, 196, 200, 202, 
  201, 175, 0, 0, 231, 169, 173, 171, 0, 0, 
  92, 176, 166, 174, 0, 172, 168, 170, 91, 0, 
  95, 0, 0, 0, 0, 0, 251, 85, 251, 0, 
  252, 0, 86, 0, 88, 68, 73, 72, 69, 70, 
  71, 251, 74, 75, 0, 0, 0, 259, 258, 256, 
  257, 255, 66, 251, 0, 251, 0, 0, 67, 76, 
  251, 0, 251, 0, 0, 77, 0, 78, 0, 81, 
  84, 0, 0, 204, 214, 213, 0, 216, 218, 217, 
  215, 0, 232, 206, 210, 208, 212, 203, 211, 0, 
  209, 205, 207, 0, 80, 79, 0, 82, 0, 83, 
  87, 99, 0, 37, 0, 0, 0, 0, 90, 89, 
  0, 102, 23, 27, 29, 28, 0, 0, 251, 252, 
  0, 251, 0, 105, 104, 251, 0, 103, 101, 0, 
  0, 20, 251, 17, 0, 21, 0, 0, 239, 0, 
  251, 0, 228, 0, 221, 227, 0, 226, 223, 251, 
  251, 252, 222, 224, 0, 251, 0, 219, 251, 0, 
  251, 0, 220, 0, 0, 13, 260, 9, 5, 8, 
  4, 0, 7, 249, 6, 0, 3};

const int QXmlStreamReader_Table::goto_default [] = {
  2, 4, 3, 46, 378, 41, 35, 48, 45, 39, 
  239, 49, 117, 75, 383, 72, 116, 40, 44, 157, 
  120, 121, 136, 135, 139, 128, 126, 130, 137, 129, 
  149, 150, 147, 159, 158, 199, 155, 154, 156, 177, 
  170, 187, 191, 293, 292, 285, 311, 310, 309, 269, 
  63, 267, 268, 132, 131, 212, 36, 33, 138, 37, 
  38, 109, 102, 320, 101, 254, 242, 241, 238, 240, 
  329, 316, 315, 319, 388, 389, 47, 43, 55, 0};

const int QXmlStreamReader_Table::action_index [] = {
  -27, -55, 35, 123, 889, 103, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, 91, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, 36, -55, -55, -55, 
  52, -55, -55, -55, 104, -55, -55, 51, -55, -11, 
  74, -55, 5, -55, -55, 94, -55, -55, -55, -55, 
  -55, -55, 16, -55, 45, 24, -55, -55, -55, -55, 
  48, 63, 51, 253, 279, 51, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, 306, -55, -55, -55, -55, -55, -55, 359, 
  -55, -55, -55, 54, -55, -55, -55, 56, -55, -55, 
  51, 200, 25, -55, 39, 13, -55, -55, -55, 120, 
  -55, 46, 151, -55, 168, -55, 194, -55, 40, -55, 
  -55, -55, 14, -55, -55, -55, 51, -55, 106, 51, 
  -55, 131, -55, 51, 122, 51, 22, 129, 51, -12, 
  47, -55, -55, -55, -55, 64, 51, 51, 97, -55, 
  51, 12, 51, 93, -55, 88, -55, 51, 7, 51, 
  65, -55, -55, -55, 76, 51, -4, 51, -5, -55, 
  -55, -55, 84, 51, 2, 1, -55, -55, -55, -55, 
  21, -55, 0, 18, 51, 15, -55, -55, 677, 85, 
  518, 87, 51, 79, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, 624, 26, -55, -55, -55, -55, 51, 73, 
  -55, -55, -55, -55, 730, -55, -55, -55, -55, 29, 
  -55, 23, 17, 20, 96, 28, 51, -55, 51, 178, 
  19, 32, -55, 34, -55, -55, -55, -55, -55, -55, 
  -55, 51, -55, -55, 30, 128, 167, -55, -55, -55, 
  -55, -55, -55, 51, 55, 51, 51, 154, -55, -55, 
  51, 152, 51, 38, 51, -55, 465, -55, 412, -55, 
  -55, 110, 92, -55, -55, -55, 783, -55, -55, -55, 
  -55, -13, -55, -55, -55, -55, -55, -55, -55, 571, 
  -55, -55, -55, 51, -55, -55, 99, -55, 51, -55, 
  -55, -55, -2, -55, 51, 51, -16, 51, -55, -55, 
  51, -55, -55, -55, -55, -55, 147, 95, 51, 71, 
  8, 51, 11, -55, -55, 51, 10, -55, -55, -24, 
  136, -55, 51, -55, 3, -55, 836, 140, -55, -20, 
  51, -1, -55, 68, -25, -55, 9, -55, -55, 51, 
  51, -17, -55, -55, -10, 51, 99, -55, 51, -3, 
  51, 147, 51, -8, 4, -55, -55, -55, -55, -55, 
  -55, 37, -55, -55, -55, 836, -55, 

  -80, -80, -80, 224, 75, -16, -80, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, 0, -14, -80, -80, 
  -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, 55, -80, 57, -80, -80, -80, -80, -80, 
  -80, 63, -80, 58, -30, 49, -80, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, 65, -80, -80, -80, -80, -80, -80, 61, 
  -80, -80, -80, 21, -80, -80, -80, -80, -80, -80, 
  26, 107, -80, -80, -80, 22, -80, -80, -80, -20, 
  -80, 31, -80, -80, -80, -80, 126, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, 20, -80, -80, 24, 
  -80, -80, -80, 25, 9, 40, -80, 42, 44, -80, 
  -80, -80, -80, -80, -80, -80, 34, 37, 32, -80, 
  48, -80, 38, 35, -80, 19, -80, 33, -80, 39, 
  36, -80, -80, -80, -80, 45, -80, 46, 43, -80, 
  -80, -80, -80, 41, -80, 29, -80, -80, -80, -80, 
  -80, -80, -3, -80, 23, -80, -80, -80, -80, 18, 
  -52, 27, 30, 28, -80, -80, -80, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, -42, -80, -80, -80, -80, -80, 53, 71, 
  -80, -80, -80, -80, 68, -80, -80, -80, -80, -80, 
  -80, 66, -80, 64, 67, 60, 83, -80, 90, -80, 
  70, -80, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, 69, -80, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, -80, 17, -80, 51, 50, 59, -80, -80, 
  47, 11, 52, -80, 62, -80, 77, -80, 80, -80, 
  -80, -80, 56, -80, -80, -80, 97, -80, -80, -80, 
  -80, -80, -80, -80, -80, -80, -80, -80, -80, 87, 
  -80, -80, -80, 54, -80, -80, -9, -80, 14, -80, 
  -80, -80, 6, -80, 5, 3, -12, -4, -80, -80, 
  2, -80, -80, -80, -80, -80, 13, -5, 76, 81, 
  -80, -2, -80, -80, -80, 16, -80, -80, -80, -15, 
  73, -80, -6, -80, -80, -80, 72, -80, -80, -36, 
  1, -80, -80, -80, -50, -80, -80, -80, -80, 84, 
  12, 79, -80, -80, -80, -1, -39, -80, 15, -80, 
  8, 10, 147, -80, -80, -80, -80, -80, -80, -80, 
  -80, -80, -80, 4, -80, 94, -80};

const int QXmlStreamReader_Table::action_info [] = {
  66, 61, 51, 382, 395, 322, 61, 375, 405, 61, 
  51, 400, 364, 387, 367, 363, 404, 1, 190, 206, 
  179, 51, 207, 59, 197, 201, 51, 51, 68, 123, 
  198, 172, 143, 260, 182, 406, 340, 341, 274, 51, 
  69, 300, 51, 398, 244, 61, 51, 66, 66, 61, 
  304, 295, 0, 66, 51, 74, 73, 262, 153, 0, 
  119, 74, 73, 62, 60, 118, 369, 296, 295, 65, 
  74, 73, 385, 51, 357, 356, 153, 167, 74, 73, 
  74, 73, 202, 166, 386, 50, 71, 70, 186, 185, 
  0, 51, 74, 73, 74, 73, 194, 193, 50, 153, 
  54, 53, 74, 73, 153, 51, 308, 306, 153, 50, 
  0, 344, 58, 0, 51, 62, 60, 61, 51, 51, 
  57, 56, 51, 51, 262, 407, 16, 161, 163, 51, 
  162, 146, 368, 369, 307, 308, 306, 152, 66, 0, 
  371, 0, 153, 161, 163, 118, 162, 335, 334, 333, 
  161, 163, 0, 162, 74, 73, 379, 51, 210, 208, 
  145, 144, 0, 0, 62, 60, 61, 0, 0, 287, 
  0, 287, 288, 0, 288, 290, 32, 290, 291, 289, 
  291, 289, 0, 0, 287, 211, 209, 288, 262, 281, 
  290, 0, 0, 291, 289, 0, 66, 124, 263, 261, 
  264, 265, 66, 124, 352, 0, 0, 0, 0, 125, 
  277, 284, 0, 0, 0, 125, 0, 0, 0, 0, 
  0, 0, 0, 0, 275, 278, 279, 280, 276, 282, 
  283, 0, 0, 0, 0, 0, 0, 0, 0, 13, 
  0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 98, 0, 93, 0, 84, 
  92, 80, 85, 94, 87, 95, 89, 83, 88, 97, 
  77, 96, 78, 79, 90, 99, 82, 91, 76, 86, 
  81, 98, 0, 93, 0, 84, 107, 106, 85, 94, 
  87, 95, 89, 83, 88, 97, 77, 96, 78, 79, 
  90, 99, 82, 91, 76, 86, 81, 0, 98, 0, 
  93, 0, 84, 104, 103, 85, 94, 87, 95, 89, 
  83, 88, 97, 77, 96, 78, 79, 90, 99, 82, 
  91, 76, 86, 81, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 98, 0, 93, 0, 84, 111, 110, 85, 94, 
  87, 95, 89, 83, 88, 97, 77, 96, 78, 79, 
  90, 99, 82, 91, 76, 86, 81, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 98, 0, 93, 312, 84, 327, 
  326, 85, 94, 87, 95, 89, 83, 88, 97, 77, 
  96, 78, 79, 90, 99, 82, 91, 76, 86, 81, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 98, 0, 93, 
  312, 84, 314, 313, 85, 94, 87, 95, 89, 83, 
  88, 97, 77, 96, 78, 79, 90, 99, 82, 91, 
  76, 86, 81, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  236, 223, 231, 213, 222, 252, 251, 224, 232, 226, 
  233, 227, 221, 0, 235, 215, 234, 216, 217, 228, 
  237, 220, 229, 214, 225, 219, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 98, 0, 93, 312, 84, 331, 330, 
  85, 94, 87, 95, 89, 83, 88, 97, 77, 96, 
  78, 79, 90, 99, 82, 91, 76, 86, 81, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 236, 223, 231, 213, 
  222, 246, 245, 224, 232, 226, 233, 227, 221, 0, 
  235, 215, 234, 216, 217, 228, 237, 220, 229, 214, 
  225, 219, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 236, 
  223, 231, 213, 222, 230, 218, 224, 232, 226, 233, 
  227, 221, 0, 235, 215, 234, 216, 217, 228, 237, 
  220, 229, 214, 225, 219, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 236, 223, 231, 213, 222, 256, 255, 224, 
  232, 226, 233, 227, 221, 0, 235, 215, 234, 216, 
  217, 228, 237, 220, 229, 214, 225, 219, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 98, 0, 93, 312, 84, 
  324, 323, 85, 94, 87, 95, 89, 83, 88, 97, 
  77, 96, 78, 79, 90, 99, 82, 91, 76, 86, 
  81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 30, 377, 
  25, 5, 15, 24, 10, 17, 26, 19, 27, 21, 
  14, 20, 29, 7, 28, 8, 9, 22, 31, 12, 
  23, 6, 18, 11, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 13, 0, 0, 0, 0, 0, 0, 0, 32, 
  0, 30, 16, 25, 5, 15, 24, 10, 17, 26, 
  19, 27, 21, 14, 20, 29, 7, 28, 8, 9, 
  22, 31, 12, 23, 6, 18, 11, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 
  0, 0, 32, 0, 

  390, 376, 384, 52, 370, 415, 374, 347, 365, 397, 
  362, 396, 348, 381, 380, 253, 204, 148, 351, 349, 
  401, 346, 343, 402, 394, 247, 358, 399, 366, 294, 
  339, 248, 151, 108, 114, 205, 200, 160, 122, 337, 
  258, 142, 259, 203, 164, 178, 168, 243, 196, 175, 
  173, 180, 165, 195, 302, 176, 184, 188, 192, 301, 
  171, 0, 189, 297, 303, 113, 299, 0, 169, 249, 
  336, 174, 181, 67, 64, 321, 115, 183, 305, 350, 
  0, 286, 42, 342, 250, 42, 345, 318, 360, 373, 
  318, 0, 359, 372, 361, 271, 392, 318, 416, 270, 
  391, 0, 273, 298, 42, 0, 270, 318, 0, 0, 
  0, 0, 0, 127, 0, 140, 266, 133, 0, 0, 
  338, 100, 0, 141, 112, 134, 0, 0, 105, 390, 
  0, 0, 127, 0, 140, 257, 133, 0, 0, 0, 
  34, 272, 355, 34, 134, 317, 0, 0, 317, 353, 
  354, 0, 328, 393, 0, 317, 0, 0, 0, 332, 
  0, 403, 34, 384, 0, 317, 0, 0, 0, 325, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 413, 0, 410, 
  408, 414, 412, 409, 0, 0, 0, 0, 0, 0, 
  0, 0, 411, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0};

const int QXmlStreamReader_Table::action_check [] = {
  2, 26, 26, 4, 14, 18, 26, 4, 4, 26, 
  26, 14, 4, 4, 4, 4, 24, 44, 22, 4, 
  13, 26, 4, 18, 22, 4, 26, 26, 4, 4, 
  42, 19, 18, 4, 12, 0, 4, 20, 4, 26, 
  4, 11, 26, 54, 18, 26, 26, 2, 2, 26, 
  12, 13, -1, 2, 26, 7, 8, 20, 11, -1, 
  4, 7, 8, 24, 25, 9, 29, 12, 13, 24, 
  7, 8, 4, 26, 34, 35, 11, 13, 7, 8, 
  7, 8, 36, 19, 16, 6, 34, 35, 12, 13, 
  -1, 26, 7, 8, 7, 8, 12, 13, 6, 11, 
  26, 27, 7, 8, 11, 26, 7, 8, 11, 6, 
  -1, 15, 18, -1, 26, 24, 25, 26, 26, 26, 
  26, 27, 26, 26, 20, 2, 3, 21, 22, 26, 
  24, 11, 28, 29, 6, 7, 8, 6, 2, -1, 
  4, -1, 11, 21, 22, 9, 24, 37, 38, 39, 
  21, 22, -1, 24, 7, 8, 16, 26, 7, 8, 
  40, 41, -1, -1, 24, 25, 26, -1, -1, 17, 
  -1, 17, 20, -1, 20, 23, 53, 23, 26, 27, 
  26, 27, -1, -1, 17, 34, 35, 20, 20, 11, 
  23, -1, -1, 26, 27, -1, 2, 3, 30, 31, 
  32, 33, 2, 3, 10, -1, -1, -1, -1, 15, 
  32, 33, -1, -1, -1, 15, -1, -1, -1, -1, 
  -1, -1, -1, -1, 46, 47, 48, 49, 50, 51, 
  52, -1, -1, -1, -1, -1, -1, -1, -1, 45, 
  -1, -1, -1, -1, -1, 45, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 2, -1, 4, -1, 6, 
  7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 
  27, 2, -1, 4, -1, 6, 7, 8, 9, 10, 
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
  21, 22, 23, 24, 25, 26, 27, -1, 2, -1, 
  4, -1, 6, 7, 8, 9, 10, 11, 12, 13, 
  14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 
  24, 25, 26, 27, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 2, -1, 4, -1, 6, 7, 8, 9, 10, 
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
  21, 22, 23, 24, 25, 26, 27, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, 2, -1, 4, 5, 6, 7, 
  8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 2, -1, 4, 
  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  45, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 
  12, 13, 14, -1, 16, 17, 18, 19, 20, 21, 
  22, 23, 24, 25, 26, 27, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, 45, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, 2, -1, 4, 5, 6, 7, 8, 
  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 
  19, 20, 21, 22, 23, 24, 25, 26, 27, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, 45, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, 2, 3, 4, 5, 
  6, 7, 8, 9, 10, 11, 12, 13, 14, -1, 
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 
  26, 27, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 45, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 
  3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
  13, 14, -1, 16, 17, 18, 19, 20, 21, 22, 
  23, 24, 25, 26, 27, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, 45, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, 2, 3, 4, 5, 6, 7, 8, 9, 
  10, 11, 12, 13, 14, -1, 16, 17, 18, 19, 
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
  14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 
  24, 25, 26, 27, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 45, -1, -1, -1, -1, -1, -1, -1, 53, 
  -1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
  21, 22, 23, 24, 25, 26, 27, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, 45, -1, -1, -1, -1, -1, 
  -1, -1, 53, -1, 

  50, 1, 16, 19, 19, 1, 12, 19, 13, 48, 
  12, 12, 16, 12, 50, 67, 19, 37, 16, 16, 
  12, 16, 16, 13, 12, 67, 13, 12, 12, 12, 
  16, 13, 12, 63, 13, 12, 12, 12, 12, 48, 
  13, 19, 12, 12, 35, 12, 12, 19, 19, 12, 
  12, 12, 12, 12, 43, 36, 12, 12, 12, 12, 
  12, -1, 19, 12, 12, 16, 16, -1, 36, 16, 
  16, 36, 36, 16, 19, 19, 13, 35, 16, 19, 
  -1, 12, 10, 19, 13, 10, 19, 10, 12, 16, 
  10, -1, 16, 20, 13, 12, 12, 10, 4, 16, 
  16, -1, 12, 44, 10, -1, 16, 10, -1, -1, 
  -1, -1, -1, 6, -1, 8, 50, 10, -1, -1, 
  50, 63, -1, 16, 63, 18, -1, -1, 63, 50, 
  -1, -1, 6, -1, 8, 67, 10, -1, -1, -1, 
  68, 51, 16, 68, 18, 68, -1, -1, 68, 23, 
  24, -1, 72, 74, -1, 68, -1, -1, -1, 72, 
  -1, 14, 68, 16, -1, 68, -1, -1, -1, 72, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 3, -1, 5, 
  6, 7, 8, 9, -1, -1, -1, -1, -1, -1, 
  -1, -1, 18, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1};


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

    /*!
      \sa setType()
     */
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
    uint referenceToParameterEntityDetected : 1;
    uint hasExternalDtdSubset : 1;
    uint lockEncoding : 1;
    uint namespaceProcessing : 1;

    int resumeReduction;
    void resume(int rule);

    inline bool entitiesMustBeDeclared() const {
        return (!inParseEntity
                && (standalone
                    || (!referenceToUnparsedEntityDetected
                        && !referenceToParameterEntityDetected // Errata 13 as of 2006-04-25
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
    inline uint peekChar();
    inline void putChar(uint c) { putStack.push() = c; }
    inline void putChar(QChar c) { putStack.push() =  c.unicode(); }
    void putString(const QString &s, int from = 0);
    void putStringLiteral(const QString &s);
    void putReplacement(const QString &s);
    void putReplacementInAttributeValue(const QString &s);
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

private:
    /*! \internal
       Never assign to variable type directly. Instead use this function.

       This prevents errors from being ignored.
     */
    inline void setType(const QXmlStreamReader::TokenType t)
    {
        if(type != QXmlStreamReader::Invalid)
            type = t;
    }
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
            setType(QXmlStreamReader::EndElement);
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
    case QXmlStreamReader::StartDocument:
	lockEncoding = true;
	if(decoder->hasFailure()) {
	    raiseWellFormedError(QXmlStream::tr("Encountered incorrectly encoded content."));
	    readBuffer.clear();
	    return false;
	}
        // fall through
    default:
        clearTextBuffer();
        ;
    }

    setType(QXmlStreamReader::NoToken);


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
            setType(QXmlStreamReader::EndDocument);
        break;

        case 1:
            if (type != QXmlStreamReader::Invalid) {
                if (hasSeenTag || inParseEntity) {
                    setType(QXmlStreamReader::EndDocument);
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
            setType(QXmlStreamReader::StartDocument);
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
            setType(QXmlStreamReader::DTD);
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

        case 77:
            if (!scanAfterDefaultDecl() && atEnd) {
                resume(77);
                return false;
            }
        break;

        case 82:
                sym(1) = sym(2);
                lastAttributeValue.clear();
                lastAttributeIsCData = false;
                if (!scanAttType() && atEnd) {
                    resume(82);
                    return false;
                }
        break;

        case 83: {
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

        case 87: {
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

        case 88: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(88);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(3);
        } break;

        case 89: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(89);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(5);
            entityDeclaration.parameter = true;
        } break;

        case 90: {
            if (!scanNData() && atEnd) {
                resume(90);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.systemId = symString(3);
            entityDeclaration.external = true;
        } break;

        case 91: {
            if (!scanNData() && atEnd) {
                resume(91);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            checkPublicLiteral((entityDeclaration.publicId = symString(3)));
            entityDeclaration.systemId = symString(5);
            entityDeclaration.external = true;
        } break;

        case 92: {
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.notationName = symString(3);
            if (entityDeclaration.parameter)
                raiseWellFormedError(QXmlStream::tr("NDATA in parameter entity declaration."));
        }
        //fall through

        case 93:
        case 94: {
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

        case 95: {
            setType(QXmlStreamReader::ProcessingInstruction);
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
                resume(95);
                return false;
            }
        } break;

        case 96:
            setType(QXmlStreamReader::ProcessingInstruction);
            processingInstructionTarget = symString(3);
            if (!processingInstructionTarget.toString().compare(QLatin1String("xml"), Qt::CaseInsensitive))
                raiseWellFormedError(QXmlStream::tr("Invalid processing instruction name."));
        break;

        case 97:
            if (!scanAfterLangleBang() && atEnd) {
                resume(97);
                return false;
            }
        break;

        case 98:
            if (!scanUntil("--")) {
                resume(98);
                return false;
            }
        break;

        case 99: {
            setType(QXmlStreamReader::Comment);
            int pos = sym(1).pos + 4;
            text = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 3);
        } break;

        case 100: {
            setType(QXmlStreamReader::Characters);
            isCDATA = true;
            int pos = sym(2).pos;
            if (scanUntil("]]>", -1)) {
                text = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 3);
            } else {
                resume(100);
                return false;
            }
        } break;

        case 101: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(101);
                return false;
            }
            NotationDeclaration &notationDeclaration = notationDeclarations.push();
            notationDeclaration.name = symString(3);
        } break;

        case 102: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId = symString(3);
            notationDeclaration.publicId.clear();
        } break;

        case 103: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId.clear();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
        } break;

        case 104: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
            notationDeclaration.systemId = symString(5);
        } break;

        case 126:
            isWhitespace = true;
            // fall through

        case 127:
        case 128:
        case 129:
            setType(QXmlStreamReader::Characters);
            sym(1).len += fastScanContentCharList();
            if (atEnd && !inParseEntity) {
                resume(129);
                return false;
            }
            text = &textBuffer;
        break;

        case 130:
        case 131:
            clearSym();
        break;

        case 132:
        case 133:
            sym(1) = sym(2);
        break;

        case 134:
        case 135:
        case 136:
        case 137:
            sym(1).len += sym(2).len;
        break;

        case 163:
            textBuffer.data()[textBuffer.size()-1] = QLatin1Char(' ');
        break;

        case 164:
            sym(1).len += fastScanLiteralContent();
            if (atEnd) {
                resume(164);
                return false;
            }
        break;

        case 165:
        case 166:
            clearSym();
        break;

        case 167:
        case 168:
	    sym(1) = sym(2);
        break;

        case 169:
        case 170:
        case 171:
        case 172:
            sym(1).len += sym(2).len;
        break;

        case 202:
        case 203:
            clearSym();
        break;

        case 204:
        case 205:
            sym(1) = sym(2);
            lastAttributeValue = symString(1);
        break;

        case 206:
        case 207:
        case 208:
        case 209:
            sym(1).len += sym(2).len;
        break;

        case 218: {
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

        case 224: {
            Tag &tag = tagStack_push();
            prefix = tag.namespaceDeclaration.prefix  = addToStringStorage(symPrefix(2));
            name = tag.name = addToStringStorage(symString(2));
            qualifiedName = tag.qualifiedName = addToStringStorage(symName(2));
            if (!validateName(qualifiedName))
                raiseWellFormedError(QXmlStream::tr("Invalid XML name."));
        } break;

        case 225:
            isEmptyElement = true;
        // fall through

        case 226:
            setType(QXmlStreamReader::StartElement);
            resolveTag();
            if (tagStack.size() == 1 && hasSeenTag && !inParseEntity)
                raiseWellFormedError(QXmlStream::tr("Extra content at end of document."));
            hasSeenTag = true;
        break;

        case 227: {
            setType(QXmlStreamReader::EndElement);
            Tag &tag = tagStack_pop();

            namespaceUri = tag.namespaceDeclaration.namespaceUri;
            name = tag.name;
            qualifiedName = tag.qualifiedName;
            if (qualifiedName != symName(3))
                raiseWellFormedError(QXmlStream::tr("Opening and ending tag mismatch."));
        } break;

        case 228: {
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
                        putReplacement(entity.value);
                    textBuffer.chop(2 + sym(2).len);
                    clearSym();
                }
                break;
            } else if (entitiesMustBeDeclared()) {
                raiseWellFormedError(QXmlStream::tr("Entity '%1' not declared.").arg(reference));
                break;
            }
            setType(QXmlStreamReader::EntityReference);
            name = symString(2);

        } break;

        case 229: {
            sym(1).len += sym(2).len + 1;
            QString reference = symString(2).toString();
            if (parameterEntityHash.contains(reference)) {
                referenceToParameterEntityDetected = true;
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

        case 230:
            sym(1).len += sym(2).len + 1;
        break;

        case 231: {
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
                    putReplacementInAttributeValue(entity.value);
                textBuffer.chop(2 + sym(2).len);
                clearSym();
            } else if (entitiesMustBeDeclared()) {
                raiseWellFormedError(QXmlStream::tr("Entity '%1' not declared.").arg(reference));
            }
        } break;

        case 232: {
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

        case 235:
        case 236:
            sym(1).len += sym(2).len;
        break;

        case 249:
            sym(1).len += fastScanSpace();
            if (atEnd) {
                resume(249);
                return false;
            }
        break;

        case 252: {
            sym(1).len += fastScanName(&sym(1).prefix);
            if (atEnd) {
                resume(252);
                return false;
            }
        } break;

        case 253:
            sym(1).len += fastScanName();
            if (atEnd) {
                resume(253);
                return false;
            }
        break;

        case 254:
        case 255:
        case 256:
        case 257:
        case 258:
            sym(1).len += fastScanNMTOKEN();
            if (atEnd) {
                resume(258);
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

