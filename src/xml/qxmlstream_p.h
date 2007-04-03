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

    ACCEPT_STATE = 412,
    RULE_COUNT = 264,
    STATE_COUNT = 423,
    TERMINAL_COUNT = 55,
    NON_TERMINAL_COUNT = 83,

    GOTO_INDEX_OFFSET = 423,
    GOTO_INFO_OFFSET = 962,
    GOTO_CHECK_OFFSET = 962,
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
  "ENTITIES", "NMTOKEN", "NMTOKENS", "xml", "version"};

const int QXmlStreamReader_Table::lhs [] = {
  55, 55, 57, 57, 57, 57, 57, 57, 57, 57, 
  65, 66, 67, 68, 62, 72, 72, 72, 74, 64, 
  64, 64, 64, 78, 77, 79, 79, 79, 79, 79, 
  79, 80, 80, 80, 80, 80, 80, 80, 86, 82, 
  87, 87, 87, 87, 90, 91, 92, 92, 92, 92, 
  93, 93, 95, 95, 95, 96, 96, 97, 97, 98, 
  98, 99, 99, 88, 88, 94, 89, 100, 100, 102, 
  102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 
  103, 104, 104, 104, 104, 106, 108, 109, 109, 83, 
  83, 110, 110, 111, 111, 84, 84, 84, 63, 63, 
  75, 113, 61, 114, 115, 85, 85, 85, 116, 116, 
  116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 
  116, 116, 116, 116, 116, 116, 116, 116, 116, 117, 
  117, 117, 117, 70, 70, 70, 70, 118, 119, 118, 
  119, 118, 119, 118, 119, 121, 121, 121, 121, 121, 
  121, 121, 121, 121, 121, 121, 121, 121, 121, 121, 
  121, 121, 121, 121, 121, 121, 121, 120, 112, 112, 
  112, 112, 122, 123, 122, 123, 122, 123, 122, 123, 
  124, 124, 124, 124, 124, 124, 124, 124, 124, 124, 
  124, 124, 124, 124, 124, 124, 124, 124, 124, 124, 
  124, 124, 124, 124, 124, 105, 105, 105, 105, 127, 
  128, 127, 128, 127, 127, 128, 128, 129, 129, 129, 
  129, 132, 71, 71, 71, 133, 133, 134, 60, 58, 
  59, 135, 81, 126, 131, 130, 125, 136, 136, 136, 
  136, 56, 56, 56, 56, 56, 56, 56, 56, 56, 
  56, 56, 56, 73, 69, 69, 107, 76, 101, 101, 
  101, 101, 101, 137};

const int QXmlStreamReader_Table:: rhs[] = {
  2, 1, 4, 2, 2, 2, 2, 2, 2, 0, 
  1, 2, 1, 2, 9, 2, 4, 0, 4, 6, 
  4, 4, 6, 1, 3, 1, 1, 1, 2, 2, 
  2, 1, 1, 1, 1, 1, 1, 1, 4, 4, 
  1, 1, 1, 1, 1, 2, 1, 1, 1, 0, 
  2, 2, 2, 6, 6, 1, 5, 1, 5, 3, 
  5, 0, 1, 6, 8, 4, 2, 1, 5, 1, 
  0, 1, 1, 1, 1, 1, 1, 1, 6, 7, 
  1, 2, 2, 1, 4, 3, 3, 1, 2, 5, 
  6, 4, 6, 3, 5, 5, 3, 4, 3, 4, 
  2, 3, 2, 2, 4, 5, 5, 7, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 2, 2, 3, 3, 2, 2, 2, 
  2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 
  3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 2, 2, 3, 3, 2, 
  2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 
  1, 5, 0, 1, 3, 1, 3, 2, 4, 3, 
  5, 3, 3, 3, 3, 4, 4, 1, 1, 2, 
  2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 0, 1, 0, 1, 1, 1, 1, 1, 
  1, 1, 1, 2};

const int QXmlStreamReader_Table::action_default [] = {
  10, 253, 0, 2, 1, 0, 127, 119, 121, 122, 
  129, 132, 125, 11, 116, 110, 0, 111, 131, 113, 
  117, 115, 123, 126, 128, 109, 112, 114, 120, 118, 
  130, 124, 246, 250, 245, 0, 133, 242, 249, 18, 
  244, 252, 251, 0, 0, 13, 248, 253, 223, 247, 
  0, 0, 258, 0, 239, 238, 0, 241, 240, 236, 
  232, 101, 257, 12, 228, 103, 0, 0, 255, 0, 
  0, 0, 166, 158, 160, 161, 135, 147, 164, 155, 
  149, 150, 146, 152, 156, 154, 162, 165, 145, 148, 
  151, 153, 159, 157, 167, 163, 143, 168, 0, 137, 
  141, 139, 144, 134, 142, 0, 140, 136, 138, 254, 
  0, 17, 16, 256, 0, 24, 21, 255, 0, 0, 
  20, 0, 0, 33, 38, 32, 0, 34, 255, 0, 
  35, 0, 0, 26, 0, 36, 0, 28, 37, 27, 
  0, 233, 42, 41, 255, 44, 50, 255, 43, 0, 
  45, 255, 50, 255, 0, 50, 255, 0, 0, 49, 
  47, 48, 52, 53, 255, 255, 0, 58, 255, 55, 
  255, 0, 59, 0, 56, 255, 54, 255, 0, 57, 
  66, 51, 0, 255, 62, 255, 0, 60, 63, 64, 
  0, 255, 62, 0, 61, 65, 46, 67, 0, 40, 
  0, 0, 255, 0, 96, 97, 0, 0, 0, 0, 
  255, 0, 202, 193, 195, 197, 170, 182, 200, 191, 
  185, 183, 186, 181, 188, 190, 198, 201, 180, 184, 
  187, 189, 194, 192, 196, 199, 203, 205, 204, 178, 
  0, 0, 0, 0, 237, 234, 172, 176, 174, 0, 
  0, 95, 179, 169, 177, 0, 175, 171, 173, 94, 
  0, 98, 0, 0, 0, 0, 0, 255, 88, 255, 
  71, 256, 0, 89, 0, 91, 70, 76, 75, 72, 
  73, 74, 255, 77, 78, 0, 0, 0, 263, 262, 
  260, 261, 259, 68, 255, 0, 255, 0, 0, 69, 
  79, 255, 0, 255, 0, 0, 80, 0, 81, 0, 
  84, 87, 0, 0, 207, 217, 216, 0, 219, 221, 
  220, 218, 0, 235, 209, 213, 211, 215, 206, 214, 
  0, 212, 208, 210, 0, 83, 82, 0, 85, 0, 
  86, 90, 102, 0, 39, 0, 0, 0, 0, 93, 
  92, 0, 105, 0, 0, 99, 100, 25, 29, 31, 
  30, 0, 0, 255, 256, 0, 255, 0, 108, 107, 
  255, 0, 106, 104, 0, 0, 22, 255, 19, 0, 
  23, 14, 0, 0, 243, 0, 255, 0, 231, 0, 
  224, 230, 0, 229, 226, 255, 255, 256, 225, 227, 
  0, 255, 0, 222, 255, 0, 255, 0, 223, 0, 
  0, 15, 264, 9, 5, 8, 4, 0, 7, 253, 
  6, 0, 3};

const int QXmlStreamReader_Table::goto_default [] = {
  2, 4, 3, 47, 384, 40, 34, 49, 46, 38, 
  237, 45, 44, 50, 114, 71, 389, 68, 113, 39, 
  43, 155, 117, 118, 134, 133, 137, 125, 123, 127, 
  135, 126, 147, 148, 145, 157, 156, 197, 153, 152, 
  154, 175, 168, 185, 189, 294, 293, 286, 312, 311, 
  310, 270, 64, 268, 269, 129, 128, 210, 35, 32, 
  136, 36, 37, 105, 98, 321, 97, 255, 240, 239, 
  236, 238, 330, 317, 316, 318, 320, 394, 395, 48, 
  42, 56, 0};

const int QXmlStreamReader_Table::action_index [] = {
  -27, -55, 37, 51, 907, 61, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, 88, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, 31, -55, -55, -55, 38, 
  -55, -55, -55, 103, 42, -55, -55, -55, 80, -55, 
  -14, 36, -55, 26, -55, -55, 101, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, 69, 63, 80, 324, 
  377, 80, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, 297, -55, 
  -55, -55, -55, -55, -55, 271, -55, -55, -55, -55, 
  57, -55, -55, -55, 52, -55, -55, 80, 162, 18, 
  -55, 35, -7, -55, -55, -55, 109, -55, 45, 155, 
  -55, 178, 4, -55, 218, -55, 49, -55, -55, -55, 
  15, -55, -55, -55, 80, -55, 123, 80, -55, 146, 
  -55, 80, 116, 80, 19, 123, 80, -19, 95, -55, 
  -55, -55, -55, 97, 80, 80, 96, -55, 80, 13, 
  80, 100, -55, 77, -55, 80, 23, 80, 83, -55, 
  -55, -55, 79, 80, -1, 27, -6, -55, -55, -55, 
  84, 27, -4, 2, -55, -55, -55, -55, 20, -55, 
  4, 22, 25, 21, -55, -55, 589, 128, 642, 128, 
  27, 74, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  801, 75, 30, 165, -55, -55, -55, -55, -55, 80, 
  128, -55, -55, -55, -55, 536, -55, -55, -55, -55, 
  41, -55, 32, 29, 8, 78, 24, 80, -55, 80, 
  190, 16, 39, -55, 34, -55, -55, -55, -55, -55, 
  -55, -55, 80, -55, -55, 28, 148, 176, -55, -55, 
  -55, -55, -55, -55, 27, 73, 27, 27, 158, -55, 
  -55, 27, 153, 27, 86, 27, -55, 748, -55, 483, 
  -55, -55, 104, 99, -55, -55, -55, 695, -55, -55, 
  -55, -55, -11, -55, -55, -55, -55, -55, -55, -55, 
  430, -55, -55, -55, 27, -55, -55, 82, -55, 27, 
  -55, -55, -55, 27, -55, 27, 27, -15, 27, -55, 
  -55, 27, -55, 106, 12, -55, -55, -55, -55, -55, 
  -55, 128, 128, 27, 128, 11, 27, 9, -55, -55, 
  27, 1, -55, -55, -20, 144, -55, 27, -55, -3, 
  -55, -55, 854, 135, -55, -23, 27, 0, -55, 53, 
  -26, -55, 10, -55, -55, 27, 27, -17, -55, -55, 
  -12, 27, 44, -55, 27, -2, 27, 71, 27, -16, 
  6, -55, -55, -55, -55, -55, -55, 46, -55, -55, 
  -55, 854, -55, 

  -83, -83, -83, 130, 77, -13, -83, -83, -83, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, -1, -83, -83, 4, -16, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, -83, -83, -83, 59, -83, -12, 
  52, 57, -83, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -19, -83, 
  -83, -83, -83, -83, -83, 61, -83, -83, -83, -83, 
  66, -83, -83, -83, -83, -83, -83, 64, 110, -83, 
  -83, -83, 16, -83, -83, -83, 15, -83, 25, -83, 
  -83, -83, 22, -83, 145, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, 27, -83, -83, 26, -83, -83, 
  -83, 18, 14, 20, -83, 29, 36, -83, -83, -83, 
  -83, -83, -83, -83, 43, 38, 24, -83, 28, -83, 
  33, 32, -83, 47, -83, 34, -83, 31, 30, -83, 
  -83, -83, -83, 35, -83, 42, 40, -83, -83, -83, 
  -83, 41, 39, 37, -83, -83, -83, -83, -83, -83, 
  23, -83, 19, -83, -83, -83, -83, 13, -44, 12, 
  21, 17, -83, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -83, -83, 
  -33, -55, -83, -83, -83, -83, -83, -83, -83, 68, 
  65, -83, -83, -83, -83, -10, -83, -83, -83, -83, 
  -83, -83, 60, -83, 67, 58, 70, 88, -83, 90, 
  -83, 69, -83, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, 51, -83, -83, -83, -83, -83, -83, -83, 
  -83, -83, -83, -83, 55, -83, 62, 49, -27, -83, 
  -83, 50, 44, 46, -83, 53, -83, 63, -83, 80, 
  -83, -83, -83, 56, -83, -83, -83, 152, -83, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -83, -83, 
  72, -83, -83, -83, 54, -83, -83, 48, -83, 45, 
  -83, -83, -83, -15, -83, 5, 6, 9, 11, -83, 
  -83, -4, -83, 0, -83, -83, -83, -83, -83, -83, 
  -83, -14, 2, 81, -5, -83, -2, -83, -83, -83, 
  8, -83, -83, -83, 10, 78, -83, -8, -83, -83, 
  -83, -83, 74, -83, -83, -36, 79, -83, -83, -83, 
  -52, -83, -83, -83, -83, 83, 1, 71, -83, -83, 
  -83, -3, -37, -83, 7, -83, -7, -6, 76, -83, 
  -83, -83, -83, -83, -83, -83, -83, -83, -83, 3, 
  -83, 99, -83};

const int QXmlStreamReader_Table::action_info [] = {
  62, 380, 401, 62, 388, 372, 52, 323, 410, 62, 
  411, 52, 406, 368, 393, 369, 356, 1, 188, 52, 
  52, 188, 120, 196, 199, 204, 205, 109, 52, 109, 
  52, 180, 170, 141, 52, 65, 177, 412, 275, 301, 
  404, 0, 62, 341, 60, 261, 0, 109, 245, 342, 
  52, 309, 307, 413, 16, 0, 116, 391, 62, 63, 
  61, 115, 55, 54, 70, 69, 263, 51, 52, 392, 
  70, 69, 67, 66, 0, 374, 70, 69, 70, 69, 
  241, 200, 109, 362, 361, 297, 296, 52, 151, 309, 
  307, 184, 183, 345, 151, 381, 192, 191, 305, 296, 
  52, 55, 54, 52, 52, 51, 151, 151, 109, 52, 
  165, 151, 63, 61, 62, 0, 164, 0, 0, 59, 
  144, 52, 52, 263, 0, 52, 52, 58, 57, 0, 
  354, 373, 374, 0, 0, 70, 69, 159, 161, 0, 
  160, 336, 335, 334, 159, 161, 109, 160, 376, 143, 
  142, 385, 150, 115, 308, 309, 307, 151, 0, 63, 
  61, 62, 208, 206, 109, 121, 0, 0, 0, 0, 
  288, 0, 52, 289, 0, 288, 291, 122, 289, 292, 
  290, 291, 0, 244, 292, 290, 0, 0, 0, 209, 
  207, 58, 57, 288, 0, 0, 289, 0, 263, 291, 
  0, 282, 292, 290, 0, 0, 0, 13, 264, 262, 
  265, 266, 0, 0, 0, 0, 0, 0, 0, 0, 
  109, 121, 278, 285, 0, 0, 0, 0, 357, 0, 
  0, 0, 0, 122, 0, 0, 276, 279, 280, 281, 
  277, 283, 284, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 94, 0, 89, 0, 80, 107, 106, 
  81, 90, 83, 91, 85, 79, 84, 93, 73, 92, 
  74, 75, 86, 95, 78, 87, 72, 82, 77, 94, 
  0, 89, 0, 80, 100, 99, 81, 90, 83, 91, 
  85, 79, 84, 93, 73, 92, 74, 75, 86, 95, 
  78, 87, 72, 82, 77, 0, 94, 0, 89, 0, 
  80, 88, 76, 81, 90, 83, 91, 85, 79, 84, 
  93, 73, 92, 74, 75, 86, 95, 78, 87, 72, 
  82, 77, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 
  0, 89, 0, 80, 103, 102, 81, 90, 83, 91, 
  85, 79, 84, 93, 73, 92, 74, 75, 86, 95, 
  78, 87, 72, 82, 77, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 94, 0, 89, 313, 80, 332, 331, 81, 
  90, 83, 91, 85, 79, 84, 93, 73, 92, 74, 
  75, 86, 95, 78, 87, 72, 82, 77, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 94, 0, 89, 313, 80, 
  328, 327, 81, 90, 83, 91, 85, 79, 84, 93, 
  73, 92, 74, 75, 86, 95, 78, 87, 72, 82, 
  77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 234, 221, 
  229, 211, 220, 257, 256, 222, 230, 224, 231, 225, 
  219, 0, 233, 213, 232, 214, 215, 226, 235, 218, 
  227, 212, 223, 217, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 234, 221, 229, 211, 220, 228, 216, 222, 230, 
  224, 231, 225, 219, 0, 233, 213, 232, 214, 215, 
  226, 235, 218, 227, 212, 223, 217, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 234, 221, 229, 211, 220, 253, 
  252, 222, 230, 224, 231, 225, 219, 0, 233, 213, 
  232, 214, 215, 226, 235, 218, 227, 212, 223, 217, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 94, 0, 89, 
  313, 80, 325, 324, 81, 90, 83, 91, 85, 79, 
  84, 93, 73, 92, 74, 75, 86, 95, 78, 87, 
  72, 82, 77, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  94, 0, 89, 313, 80, 315, 314, 81, 90, 83, 
  91, 85, 79, 84, 93, 73, 92, 74, 75, 86, 
  95, 78, 87, 72, 82, 77, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 234, 221, 229, 211, 220, 247, 246, 
  222, 230, 224, 231, 225, 219, 0, 233, 213, 232, 
  214, 215, 226, 235, 218, 227, 212, 223, 217, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 30, 383, 25, 5, 
  15, 24, 10, 17, 26, 19, 27, 21, 14, 20, 
  29, 7, 28, 8, 9, 22, 31, 12, 23, 6, 
  18, 11, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 
  16, 25, 5, 15, 24, 10, 17, 26, 19, 27, 
  21, 14, 20, 29, 7, 28, 8, 9, 22, 31, 
  12, 23, 6, 18, 11, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 

  396, 363, 390, 344, 421, 382, 379, 407, 53, 408, 
  366, 402, 367, 403, 352, 400, 386, 370, 355, 299, 
  353, 405, 371, 347, 350, 254, 243, 259, 249, 349, 
  348, 375, 158, 203, 163, 260, 248, 140, 242, 201, 
  198, 149, 169, 353, 202, 178, 101, 171, 176, 186, 
  182, 162, 173, 96, 146, 193, 190, 166, 194, 258, 
  304, 187, 167, 340, 302, 287, 181, 300, 179, 295, 
  172, 306, 337, 319, 112, 110, 298, 322, 119, 346, 
  251, 111, 319, 195, 41, 174, 250, 41, 343, 303, 
  319, 351, 409, 387, 390, 365, 378, 398, 338, 364, 
  377, 397, 272, 422, 274, 0, 271, 0, 271, 41, 
  0, 0, 267, 0, 0, 0, 124, 104, 138, 0, 
  130, 339, 132, 396, 0, 0, 108, 0, 139, 0, 
  131, 0, 0, 419, 0, 416, 414, 420, 418, 415, 
  0, 0, 0, 273, 0, 0, 333, 0, 399, 33, 
  417, 124, 33, 138, 329, 130, 0, 132, 0, 0, 
  0, 0, 319, 360, 0, 131, 0, 0, 0, 0, 
  358, 359, 0, 0, 33, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 326, 0, 0, 0, 
  0, 0, 0, 0, 0};

const int QXmlStreamReader_Table::action_check [] = {
  26, 4, 14, 26, 4, 4, 26, 18, 24, 26, 
  4, 26, 14, 4, 4, 4, 4, 44, 22, 26, 
  26, 22, 4, 42, 4, 4, 4, 2, 26, 2, 
  26, 12, 19, 18, 26, 4, 13, 0, 4, 11, 
  54, -1, 26, 4, 18, 4, -1, 2, 18, 20, 
  26, 7, 8, 2, 3, -1, 4, 4, 26, 24, 
  25, 9, 26, 27, 7, 8, 20, 6, 26, 16, 
  7, 8, 34, 35, -1, 29, 7, 8, 7, 8, 
  6, 36, 2, 34, 35, 12, 13, 26, 11, 7, 
  8, 12, 13, 15, 11, 53, 12, 13, 12, 13, 
  26, 26, 27, 26, 26, 6, 11, 11, 2, 26, 
  13, 11, 24, 25, 26, -1, 19, -1, -1, 18, 
  11, 26, 26, 20, -1, 26, 26, 26, 27, -1, 
  24, 28, 29, -1, -1, 7, 8, 21, 22, -1, 
  24, 37, 38, 39, 21, 22, 2, 24, 4, 40, 
  41, 16, 6, 9, 6, 7, 8, 11, -1, 24, 
  25, 26, 7, 8, 2, 3, -1, -1, -1, -1, 
  17, -1, 26, 20, -1, 17, 23, 15, 20, 26, 
  27, 23, -1, 18, 26, 27, -1, -1, -1, 34, 
  35, 26, 27, 17, -1, -1, 20, -1, 20, 23, 
  -1, 11, 26, 27, -1, -1, -1, 45, 30, 31, 
  32, 33, -1, -1, -1, -1, -1, -1, -1, -1, 
  2, 3, 32, 33, -1, -1, -1, -1, 10, -1, 
  -1, -1, -1, 15, -1, -1, 46, 47, 48, 49, 
  50, 51, 52, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, 45, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, 2, -1, 4, -1, 6, 7, 8, 
  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 
  19, 20, 21, 22, 23, 24, 25, 26, 27, 2, 
  -1, 4, -1, 6, 7, 8, 9, 10, 11, 12, 
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 
  23, 24, 25, 26, 27, -1, 2, -1, 4, -1, 
  6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 
  26, 27, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 
  -1, 4, -1, 6, 7, 8, 9, 10, 11, 12, 
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 
  23, 24, 25, 26, 27, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
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
  8, 9, 10, 11, 12, 13, 14, -1, 16, 17, 
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 2, -1, 4, 
  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  45, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
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
  -1, -1, -1, -1, -1, -1, 2, 3, 4, 5, 
  6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 
  26, 27, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 45, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 
  3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 
  23, 24, 25, 26, 27, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, 45, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, 

  52, 15, 18, 18, 1, 1, 14, 14, 21, 15, 
  15, 14, 14, 50, 18, 14, 52, 15, 18, 46, 
  21, 14, 14, 18, 18, 69, 81, 15, 15, 18, 
  21, 21, 14, 14, 14, 14, 69, 21, 21, 14, 
  14, 14, 14, 21, 21, 14, 65, 14, 14, 14, 
  14, 37, 14, 65, 39, 14, 14, 14, 21, 69, 
  14, 21, 38, 18, 14, 14, 37, 18, 38, 14, 
  38, 18, 18, 10, 15, 18, 14, 21, 14, 21, 
  15, 15, 10, 44, 10, 38, 18, 10, 21, 45, 
  10, 21, 16, 14, 18, 14, 18, 14, 50, 18, 
  22, 18, 14, 4, 14, -1, 18, -1, 18, 10, 
  -1, -1, 52, -1, -1, -1, 6, 65, 8, -1, 
  10, 52, 12, 52, -1, -1, 65, -1, 18, -1, 
  20, -1, -1, 3, -1, 5, 6, 7, 8, 9, 
  -1, -1, -1, 53, -1, -1, 74, -1, 77, 75, 
  20, 6, 75, 8, 74, 10, -1, 12, -1, -1, 
  -1, -1, 10, 18, -1, 20, -1, -1, -1, -1, 
  25, 26, -1, -1, 75, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, 74, -1, -1, -1, 
  -1, -1, -1, -1, -1};


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
    uint xmlDeclOK : 1;
    uint standalone : 1;
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

    bool validateName(const QStringRef &name);

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
            xmlDeclOK = (characterOffset + readBufferPos - putStack.size() == 2);
        break;

        case 12:
            if (!scanString(spell[XML], XML) && atEnd) {
                resume(12);
                return false;
            }
        break;

        case 13:
            if (!scanString(spell[VERSION], VERSION, false) && atEnd) {
                resume(13);
                return false;
            }
        break;

        case 14:
            type = QXmlStreamReader::StartDocument;
            if (xmlDeclOK)
                startDocument(symString(6));
            else
                raiseWellFormedError(QXmlStream::tr("XML declaration not at start of document."));
        break;

        case 15:
            hasExternalDtdSubset = true;
        break;

        case 16:
            checkPublicLiteral(symString(2));
            hasExternalDtdSubset = true;
        break;

        case 18:
            if (!scanPublicOrSystem() && atEnd) {
                resume(18);
                return false;
            }
        break;

        case 19:
        case 20:
        case 21:
        case 22:
            type = QXmlStreamReader::DTD;
            text = &textBuffer;
        break;

        case 23:
            scanDtd = true;
        break;

        case 24:
            scanDtd = false;
        break;

        case 38:
            if (!scanString(spell[EMPTY], EMPTY, false)
                && !scanString(spell[ANY], ANY, false)
                && atEnd) {
                resume(38);
                return false;
            }
        break;

        case 44:
            if (!scanString(spell[PCDATA], PCDATA, false) && atEnd) {
                resume(44);
                return false;
            }
        break;

        case 69: {
            lastAttributeIsCData = true;
        } break;

        case 80:
            if (!scanAfterDefaultDecl() && atEnd) {
                resume(80);
                return false;
            }
        break;

        case 85:
                sym(1) = sym(2);
                lastAttributeValue.clear();
                lastAttributeIsCData = false;
                if (!scanAttType() && atEnd) {
                    resume(85);
                    return false;
                }
        break;

        case 86: {
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

        case 90: {
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

        case 91: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(91);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(3);
        } break;

        case 92: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(92);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(5);
            entityDeclaration.parameter = true;
        } break;

        case 93: {
            if (!scanNData() && atEnd) {
                resume(93);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.systemId = symString(3);
            entityDeclaration.external = true;
        } break;

        case 94: {
            if (!scanNData() && atEnd) {
                resume(94);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            checkPublicLiteral((entityDeclaration.publicId = symString(3)));
            entityDeclaration.systemId = symString(5);
            entityDeclaration.external = true;
        } break;

        case 95: {
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.notationName = symString(3);
            if (entityDeclaration.parameter)
                raiseWellFormedError(QXmlStream::tr("NDATA in parameter entity declaration."));
        }
        //fall through

        case 96:
        case 97: {
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

        case 98: {
            type = QXmlStreamReader::ProcessingInstruction;
            int pos = sym(3).pos + sym(3).len;
            processingInstructionTarget = symString(2);
            if (scanUntil("?>")) {
                processingInstructionData = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 2);
                const QString piTarget(processingInstructionTarget.toString());
                if (!piTarget.compare(QLatin1String("xml"), Qt::CaseInsensitive)) {
                    raiseWellFormedError(QXmlStream::tr("xml is an invalid processing instruction name."));
                }
                else if(!QXmlUtils::isNCName(piTarget)) 
                    raiseWellFormedError(QXmlStream::tr("%1 is an invalid processing instruction name.").arg(piTarget));
            } else {
                resume(98);
                return false;
            }
        } break;

        case 99:
            type = QXmlStreamReader::ProcessingInstruction;
            processingInstructionTarget = symString(2);
            if (!processingInstructionTarget.toString().compare(QLatin1String("xml"), Qt::CaseInsensitive))
                raiseWellFormedError(QXmlStream::tr("Invalid processing instruction name."));
        break;

        case 100:
            if (!scanAfterLangleBang() && atEnd) {
                resume(100);
                return false;
            }
        break;

        case 101: 
            if (!scanUntil("--")) {
                resume(101);
                return false;
            }
        break;

        case 102: {
            type = QXmlStreamReader::Comment;
            int pos = sym(1).pos + 4;
            text = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 3);
        } break;

        case 103: {
            type = QXmlStreamReader::Characters;
            isCDATA = true;
            int pos = sym(2).pos;
            if (scanUntil("]]>", -1)) {
                text = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 3);
            } else {
                resume(103);
                return false;
            }
        } break;

        case 104: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(104);
                return false;
            }
            NotationDeclaration &notationDeclaration = notationDeclarations.push();
            notationDeclaration.name = symString(3);
        } break;

        case 105: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId = symString(3);
            notationDeclaration.publicId.clear();
        } break;

        case 106: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId.clear();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
        } break;

        case 107: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
            notationDeclaration.systemId = symString(5);
        } break;

        case 129:
            isWhitespace = true;
            // fall through

        case 130:
        case 131:
        case 132:
            type = QXmlStreamReader::Characters;
            sym(1).len += fastScanContentCharList();
            if (atEnd && !inParseEntity) {
                resume(132);
                return false;
            }
            text = &textBuffer;
        break;

        case 133:
        case 134:
            clearSym();
        break;

        case 135:
        case 136:
            sym(1) = sym(2);
        break;

        case 137:
        case 138:
        case 139:
        case 140:
            sym(1).len += sym(2).len;
        break;

        case 166:
            textBuffer.data()[textBuffer.size()-1] = QLatin1Char(' ');
        break;

        case 167:
            sym(1).len += fastScanLiteralContent();
            if (atEnd) {
                resume(167);
                return false;
            }
        break;

        case 168:
        case 169:
            clearSym();
        break;

        case 170:
        case 171:
	    sym(1) = sym(2);
        break;

        case 172:
        case 173:
        case 174:
        case 175:
            sym(1).len += sym(2).len;
        break;

        case 205:
        case 206:
            clearSym();
        break;

        case 207:
        case 208:
            sym(1) = sym(2);
            lastAttributeValue = symString(1);
        break;

        case 209:
        case 210:
        case 211:
        case 212:
            sym(1).len += sym(2).len;
        break;

        case 221: {
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

        case 227: {
            Tag &tag = tagStack_push();
            prefix = tag.namespaceDeclaration.prefix  = addToStringStorage(symPrefix(2));
            name = tag.name = addToStringStorage(symString(2));
            qualifiedName = tag.qualifiedName = addToStringStorage(symName(2));
            if (!validateName(qualifiedName))
                raiseWellFormedError(QXmlStream::tr("Invalid XML name."));
        } break;

        case 228:
            isEmptyElement = true;
        // fall through

        case 229:
            type = QXmlStreamReader::StartElement;
            resolveTag();
            if (tagStack.size() == 1 && hasSeenTag && !inParseEntity)
                raiseWellFormedError(QXmlStream::tr("Extra content at end of document."));
            hasSeenTag = true;
        break;

        case 230: {
            type = QXmlStreamReader::EndElement;
            Tag &tag = tagStack_pop();

            namespaceUri = tag.namespaceDeclaration.namespaceUri;
            name = tag.name;
            qualifiedName = tag.qualifiedName;
            if (qualifiedName != symName(3))
                raiseWellFormedError(QXmlStream::tr("Opening and ending tag mismatch."));
        } break;

        case 231: {
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

        case 232: {
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

        case 233:
            sym(1).len += sym(2).len + 1;
        break;

        case 234: {
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

        case 235: {
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

        case 236: {
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

        case 239:
        case 240:
            sym(1).len += sym(2).len;
        break;

        case 253:
            sym(1).len += fastScanSpace();
            if (atEnd) {
                resume(253);
                return false;
            }
        break;

        case 256: {
            sym(1).len += fastScanName(&sym(1).prefix);
            if (atEnd) {
                resume(256);
                return false;
            }
        } break;

        case 257:
            sym(1).len += fastScanName();
            if (atEnd) {
                resume(257);
                return false;
            }
        break;

        case 258:
        case 259:
        case 260:
        case 261:
        case 262:
            sym(1).len += fastScanName();
            if (atEnd) {
                resume(262);
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

