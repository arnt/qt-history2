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

    ACCEPT_STATE = 411,
    RULE_COUNT = 263,
    STATE_COUNT = 422,
    TERMINAL_COUNT = 55,
    NON_TERMINAL_COUNT = 82,

    GOTO_INDEX_OFFSET = 422,
    GOTO_INFO_OFFSET = 970,
    GOTO_CHECK_OFFSET = 970,
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
  65, 66, 67, 62, 71, 71, 71, 73, 64, 64, 
  64, 64, 77, 76, 78, 78, 78, 78, 78, 78, 
  79, 79, 79, 79, 79, 79, 79, 85, 81, 86, 
  86, 86, 86, 89, 90, 91, 91, 91, 91, 92, 
  92, 94, 94, 94, 95, 95, 96, 96, 97, 97, 
  98, 98, 87, 87, 93, 88, 99, 99, 101, 101, 
  101, 101, 101, 101, 101, 101, 101, 101, 101, 102, 
  103, 103, 103, 103, 105, 107, 108, 108, 82, 82, 
  109, 109, 110, 110, 83, 83, 83, 63, 63, 74, 
  112, 61, 113, 114, 84, 84, 84, 115, 115, 115, 
  115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 
  115, 115, 115, 115, 115, 115, 115, 115, 116, 116, 
  116, 116, 69, 69, 69, 69, 117, 118, 117, 118, 
  117, 118, 117, 118, 120, 120, 120, 120, 120, 120, 
  120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 
  120, 120, 120, 120, 120, 120, 119, 111, 111, 111, 
  111, 121, 122, 121, 122, 121, 122, 121, 122, 123, 
  123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 
  123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 
  123, 123, 123, 123, 104, 104, 104, 104, 126, 127, 
  126, 127, 126, 126, 127, 127, 128, 128, 128, 128, 
  131, 70, 70, 70, 132, 132, 133, 60, 58, 59, 
  134, 80, 125, 130, 129, 124, 135, 135, 135, 135, 
  56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 
  56, 56, 72, 68, 68, 106, 75, 100, 100, 100, 
  100, 100, 136};

const int QXmlStreamReader_Table:: rhs[] = {
  2, 1, 4, 2, 2, 2, 2, 2, 2, 0, 
  1, 2, 2, 9, 2, 4, 0, 4, 6, 4, 
  4, 6, 1, 3, 1, 1, 1, 2, 2, 2, 
  1, 1, 1, 1, 1, 1, 1, 4, 4, 1, 
  1, 1, 1, 1, 2, 1, 1, 1, 0, 2, 
  2, 2, 6, 6, 1, 5, 1, 5, 3, 5, 
  0, 1, 6, 8, 4, 2, 1, 5, 1, 0, 
  1, 1, 1, 1, 1, 1, 1, 6, 7, 1, 
  2, 2, 1, 4, 3, 3, 1, 2, 5, 6, 
  4, 6, 3, 5, 5, 3, 4, 3, 4, 2, 
  3, 2, 2, 4, 5, 5, 7, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 2, 2, 3, 3, 2, 2, 2, 2, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 
  3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 2, 2, 3, 3, 2, 2, 
  2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 
  5, 0, 1, 3, 1, 3, 2, 4, 3, 5, 
  3, 3, 3, 3, 4, 4, 1, 1, 2, 2, 
  2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 0, 1, 0, 1, 1, 1, 1, 1, 1, 
  1, 1, 2};

const int QXmlStreamReader_Table::action_default [] = {
  10, 252, 0, 2, 1, 0, 126, 118, 120, 121, 
  128, 131, 124, 11, 115, 109, 0, 110, 130, 112, 
  116, 114, 122, 125, 127, 108, 111, 113, 119, 117, 
  129, 123, 245, 249, 244, 0, 132, 241, 248, 17, 
  243, 251, 250, 0, 0, 247, 252, 222, 246, 0, 
  0, 257, 0, 238, 237, 0, 240, 239, 235, 231, 
  100, 256, 12, 227, 102, 0, 0, 254, 0, 0, 
  0, 165, 157, 159, 160, 134, 146, 163, 154, 148, 
  149, 145, 151, 155, 153, 161, 164, 144, 147, 150, 
  152, 158, 156, 166, 162, 142, 167, 0, 136, 140, 
  138, 143, 133, 141, 0, 139, 135, 137, 253, 0, 
  16, 15, 255, 0, 23, 20, 254, 0, 0, 19, 
  0, 0, 32, 37, 31, 0, 33, 254, 0, 34, 
  0, 0, 25, 0, 35, 0, 27, 36, 26, 0, 
  232, 41, 40, 254, 43, 49, 254, 42, 0, 44, 
  254, 49, 254, 0, 49, 254, 0, 0, 48, 46, 
  47, 51, 52, 254, 254, 0, 57, 254, 54, 254, 
  0, 58, 0, 55, 254, 53, 254, 0, 56, 65, 
  50, 0, 254, 61, 254, 0, 59, 62, 63, 0, 
  254, 61, 0, 60, 64, 45, 66, 0, 39, 0, 
  0, 254, 0, 95, 96, 0, 0, 0, 0, 254, 
  0, 201, 192, 194, 196, 169, 181, 199, 190, 184, 
  182, 185, 180, 187, 189, 197, 200, 179, 183, 186, 
  188, 193, 191, 195, 198, 202, 204, 203, 177, 0, 
  0, 0, 0, 236, 233, 171, 175, 173, 0, 0, 
  94, 178, 168, 176, 0, 174, 170, 172, 93, 0, 
  97, 0, 0, 0, 0, 0, 254, 87, 254, 70, 
  255, 0, 88, 0, 90, 69, 75, 74, 71, 72, 
  73, 254, 76, 77, 0, 0, 0, 262, 261, 259, 
  260, 258, 67, 254, 0, 254, 0, 0, 68, 78, 
  254, 0, 254, 0, 0, 79, 0, 80, 0, 83, 
  86, 0, 0, 206, 216, 215, 0, 218, 220, 219, 
  217, 0, 234, 208, 212, 210, 214, 205, 213, 0, 
  211, 207, 209, 0, 82, 81, 0, 84, 0, 85, 
  89, 101, 0, 38, 0, 0, 0, 0, 92, 91, 
  0, 104, 0, 0, 98, 99, 24, 28, 30, 29, 
  0, 0, 254, 255, 0, 254, 0, 107, 106, 254, 
  0, 105, 103, 0, 0, 21, 254, 18, 0, 22, 
  13, 0, 0, 242, 0, 254, 0, 230, 0, 223, 
  229, 0, 228, 225, 254, 254, 255, 224, 226, 0, 
  254, 0, 221, 254, 0, 254, 0, 222, 0, 0, 
  14, 263, 9, 5, 8, 4, 0, 7, 252, 6, 
  0, 3};

const int QXmlStreamReader_Table::goto_default [] = {
  2, 4, 3, 46, 383, 40, 34, 48, 45, 38, 
  236, 44, 49, 113, 70, 388, 67, 112, 39, 43, 
  154, 116, 117, 133, 132, 136, 124, 122, 126, 134, 
  125, 146, 147, 144, 156, 155, 196, 152, 151, 153, 
  174, 167, 184, 188, 293, 292, 285, 311, 310, 309, 
  269, 63, 267, 268, 128, 127, 209, 35, 32, 135, 
  36, 37, 104, 97, 320, 96, 254, 239, 238, 235, 
  237, 329, 316, 315, 317, 319, 393, 394, 47, 42, 
  55, 0};

const int QXmlStreamReader_Table::action_index [] = {
  -14, -55, 14, 125, 915, 67, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, 152, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, 20, -55, -55, -55, 79, 
  -55, -55, -55, 139, 82, -55, -55, 1, -55, -50, 
  107, -55, -18, -55, -55, 126, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, 62, 87, 72, 279, 332, 
  72, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, 385, -55, -55, 
  -55, -55, -55, -55, 305, -55, -55, -55, -55, 95, 
  -55, -55, -55, 117, -55, -55, 72, 226, 43, -55, 
  113, 22, -55, -55, -55, 131, -55, 51, 175, -55, 
  186, 40, -55, 210, -55, 48, -55, -55, -55, 39, 
  -55, -55, -55, 72, -55, 119, 72, -55, 159, -55, 
  72, 127, 72, 27, 136, 72, -11, 85, -55, -55, 
  -55, -55, 78, 72, 72, 89, -55, 72, 35, 72, 
  54, -55, 50, -55, 72, 33, 72, 66, -55, -55, 
  -55, 112, 72, 38, 72, 41, -55, -55, -55, 73, 
  72, 37, 36, -55, -55, -55, -55, 34, -55, 32, 
  28, 72, 29, -55, -55, 597, 173, 756, 173, 72, 
  130, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, 703, 
  80, 31, 128, -55, -55, -55, -55, -55, 72, 109, 
  -55, -55, -55, -55, 650, -55, -55, -55, -55, 30, 
  -55, 26, 15, 45, 94, 49, 72, -55, 72, 211, 
  42, 46, -55, 47, -55, -55, -55, -55, -55, -55, 
  -55, 72, -55, -55, 44, 167, 188, -55, -55, -55, 
  -55, -55, -55, 72, 110, 72, 72, 174, -55, -55, 
  0, 176, -1, 77, 3, -55, 438, -55, 491, -55, 
  -55, 4, 75, -55, -55, -55, 544, -55, -55, -55, 
  -55, 19, -55, -55, -55, -55, -55, -55, -55, 809, 
  -55, -55, -55, 7, -55, -55, 111, -55, 72, -55, 
  -55, -55, 72, -55, 72, 72, -20, 72, -55, -55, 
  72, -55, 137, 21, -55, -55, -55, -55, -55, -55, 
  124, 122, 72, 91, 25, 72, 24, -55, -55, 72, 
  23, -55, -55, -3, 160, -55, 72, -55, 17, -55, 
  -55, 862, 163, -55, -4, 10, 11, -55, 68, -15, 
  -55, 9, -55, -55, 8, 6, -19, -55, -55, 2, 
  16, 56, -55, 72, 12, 72, 97, 18, -5, 13, 
  -55, -55, -55, -55, -55, -55, 59, -55, -55, -55, 
  862, -55, 

  -82, -82, -82, 196, 80, -15, -82, -82, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, -82, -5, -82, 22, -9, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, -82, -82, -82, 59, -82, 25, 58, 
  61, -82, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -8, -82, -82, 
  -82, -82, -82, -82, 63, -82, -82, -82, -82, 65, 
  -82, -82, -82, -82, -82, -82, 69, 101, -82, -82, 
  -82, 12, -82, -82, -82, 14, -82, 27, -82, -82, 
  -82, 24, -82, 118, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, 29, -82, -82, 28, -82, -82, -82, 
  18, 2, 21, -82, 17, 38, -82, -82, -82, -82, 
  -82, -82, -82, 44, 45, 37, -82, 32, -82, 34, 
  33, -82, 49, -82, 35, -82, 36, 31, -82, -82, 
  -82, -82, 30, -82, 41, 43, -82, -82, -82, -82, 
  42, 40, 39, -82, -82, -82, -82, -82, -82, 26, 
  -82, 20, -82, -82, -82, -82, 15, -43, 13, 23, 
  19, -82, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -82, -82, -31, 
  -45, -82, -82, -82, -82, -82, -82, -82, 70, 67, 
  -82, -82, -82, -82, -18, -82, -82, -82, -82, -82, 
  -82, 64, -82, 68, 60, 71, 91, -82, 93, -82, 
  66, -82, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, 53, -82, -82, -82, -82, -82, -82, -82, -82, 
  -82, -82, -82, 56, -82, 62, 50, -19, -82, -82, 
  52, 16, 117, -82, 54, -82, 51, -82, 84, -82, 
  -82, -82, 57, -82, -82, -82, 82, -82, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -82, -82, 74, 
  -82, -82, -82, 55, -82, -82, 46, -82, 47, -82, 
  -82, -82, -14, -82, 3, 7, 10, 11, -82, -82, 
  -4, -82, 1, -82, -82, -82, -82, -82, -82, -82, 
  -10, 48, 83, 0, -82, -2, -82, -82, -82, 9, 
  -82, -82, -82, -20, 76, -82, -11, -82, -82, -82, 
  -82, 75, -82, -82, -35, -3, -82, -82, -82, -50, 
  -82, -82, -82, -82, 85, 4, 72, -82, -82, -82, 
  6, -40, -82, 8, -82, -1, -7, 86, -82, -82, 
  -82, -82, -82, -82, -82, -82, -82, -82, 5, -82, 
  95, -82};

const int QXmlStreamReader_Table::action_info [] = {
  59, 108, 108, 108, 403, 108, 51, 61, 108, 108, 
  108, 61, 108, 392, 411, 387, 400, 410, 108, 409, 
  108, 379, 61, 51, 64, 355, 405, 371, 367, 368, 
  1, 195, 204, 203, 260, 341, 0, 322, 198, 179, 
  0, 335, 334, 333, 0, 0, 176, 119, 51, 244, 
  340, 274, 61, 108, 169, 300, 0, 140, 51, 187, 
  187, 150, 51, 308, 306, 150, 51, 51, 61, 69, 
  68, 51, 390, 50, 108, 51, 51, 150, 0, 262, 
  51, 50, 361, 360, 391, 191, 190, 199, 373, 304, 
  295, 164, 51, 51, 69, 68, 150, 163, 69, 68, 
  150, 51, 69, 68, 69, 68, 54, 53, 51, 344, 
  0, 51, 0, 66, 65, 51, 69, 68, 308, 306, 
  51, 115, 296, 295, 183, 182, 114, 412, 16, 69, 
  68, 69, 68, 54, 53, 380, 240, 62, 60, 108, 
  158, 160, 143, 159, 58, 0, 243, 0, 158, 160, 
  0, 159, 57, 56, 57, 56, 51, 158, 160, 262, 
  159, 353, 108, 0, 375, 149, 0, 372, 373, 114, 
  150, 142, 141, 307, 308, 306, 62, 60, 61, 384, 
  69, 68, 207, 205, 0, 51, 0, 62, 60, 61, 
  0, 287, 0, 287, 288, 0, 288, 290, 0, 290, 
  291, 289, 291, 289, 0, 287, 262, 0, 288, 208, 
  206, 290, 108, 120, 291, 289, 263, 261, 264, 265, 
  356, 0, 281, 0, 0, 121, 0, 0, 108, 120, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 121, 0, 277, 284, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 13, 0, 275, 278, 279, 
  280, 276, 282, 283, 0, 0, 0, 0, 0, 0, 
  0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 93, 0, 88, 0, 79, 87, 75, 80, 89, 
  82, 90, 84, 78, 83, 92, 72, 91, 73, 74, 
  85, 94, 77, 86, 71, 81, 76, 93, 0, 88, 
  0, 79, 106, 105, 80, 89, 82, 90, 84, 78, 
  83, 92, 72, 91, 73, 74, 85, 94, 77, 86, 
  71, 81, 76, 0, 93, 0, 88, 0, 79, 102, 
  101, 80, 89, 82, 90, 84, 78, 83, 92, 72, 
  91, 73, 74, 85, 94, 77, 86, 71, 81, 76, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 93, 0, 88, 
  0, 79, 99, 98, 80, 89, 82, 90, 84, 78, 
  83, 92, 72, 91, 73, 74, 85, 94, 77, 86, 
  71, 81, 76, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  93, 0, 88, 312, 79, 314, 313, 80, 89, 82, 
  90, 84, 78, 83, 92, 72, 91, 73, 74, 85, 
  94, 77, 86, 71, 81, 76, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 93, 0, 88, 312, 79, 327, 326, 
  80, 89, 82, 90, 84, 78, 83, 92, 72, 91, 
  73, 74, 85, 94, 77, 86, 71, 81, 76, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 93, 0, 88, 312, 
  79, 324, 323, 80, 89, 82, 90, 84, 78, 83, 
  92, 72, 91, 73, 74, 85, 94, 77, 86, 71, 
  81, 76, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 233, 
  220, 228, 210, 219, 227, 215, 221, 229, 223, 230, 
  224, 218, 0, 232, 212, 231, 213, 214, 225, 234, 
  217, 226, 211, 222, 216, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 233, 220, 228, 210, 219, 256, 255, 221, 
  229, 223, 230, 224, 218, 0, 232, 212, 231, 213, 
  214, 225, 234, 217, 226, 211, 222, 216, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 233, 220, 228, 210, 219, 
  246, 245, 221, 229, 223, 230, 224, 218, 0, 232, 
  212, 231, 213, 214, 225, 234, 217, 226, 211, 222, 
  216, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 233, 220, 
  228, 210, 219, 252, 251, 221, 229, 223, 230, 224, 
  218, 0, 232, 212, 231, 213, 214, 225, 234, 217, 
  226, 211, 222, 216, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 93, 0, 88, 312, 79, 331, 330, 80, 89, 
  82, 90, 84, 78, 83, 92, 72, 91, 73, 74, 
  85, 94, 77, 86, 71, 81, 76, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 30, 382, 25, 5, 15, 24, 
  10, 17, 26, 19, 27, 21, 14, 20, 29, 7, 
  28, 8, 9, 22, 31, 12, 23, 6, 18, 11, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 30, 16, 25, 
  5, 15, 24, 10, 17, 26, 19, 27, 21, 14, 
  20, 29, 7, 28, 8, 9, 22, 31, 12, 23, 
  6, 18, 11, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

  374, 395, 378, 343, 362, 52, 420, 407, 389, 402, 
  386, 366, 406, 351, 365, 352, 385, 399, 354, 401, 
  346, 404, 370, 381, 349, 253, 298, 258, 348, 248, 
  347, 157, 139, 202, 162, 242, 259, 247, 161, 241, 
  200, 197, 148, 185, 352, 168, 201, 170, 175, 177, 
  257, 181, 145, 180, 189, 192, 100, 165, 172, 193, 
  302, 318, 369, 186, 339, 301, 286, 299, 178, 294, 
  171, 305, 336, 111, 166, 297, 0, 321, 109, 110, 
  345, 250, 118, 194, 318, 41, 173, 249, 342, 95, 
  41, 350, 318, 377, 318, 337, 364, 376, 397, 421, 
  363, 408, 396, 389, 271, 41, 273, 123, 270, 137, 
  270, 129, 131, 0, 0, 266, 0, 338, 138, 0, 
  130, 0, 103, 395, 123, 0, 137, 107, 129, 131, 
  303, 0, 0, 0, 0, 359, 0, 130, 0, 0, 
  0, 0, 357, 358, 0, 272, 0, 332, 398, 33, 
  0, 0, 0, 0, 33, 325, 0, 328, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 33, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 418, 
  0, 415, 413, 419, 417, 414, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 416, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0};

const int QXmlStreamReader_Table::action_check [] = {
  18, 2, 2, 2, 54, 2, 26, 26, 2, 2, 
  2, 26, 2, 4, 0, 4, 14, 4, 2, 24, 
  2, 4, 26, 26, 4, 4, 14, 4, 4, 4, 
  44, 42, 4, 4, 4, 20, -1, 18, 4, 12, 
  -1, 37, 38, 39, -1, -1, 13, 4, 26, 18, 
  4, 4, 26, 2, 19, 11, -1, 18, 26, 22, 
  22, 11, 26, 7, 8, 11, 26, 26, 26, 7, 
  8, 26, 4, 6, 2, 26, 26, 11, -1, 20, 
  26, 6, 34, 35, 16, 12, 13, 36, 29, 12, 
  13, 13, 26, 26, 7, 8, 11, 19, 7, 8, 
  11, 26, 7, 8, 7, 8, 26, 27, 26, 15, 
  -1, 26, -1, 34, 35, 26, 7, 8, 7, 8, 
  26, 4, 12, 13, 12, 13, 9, 2, 3, 7, 
  8, 7, 8, 26, 27, 53, 6, 24, 25, 2, 
  21, 22, 11, 24, 18, -1, 18, -1, 21, 22, 
  -1, 24, 26, 27, 26, 27, 26, 21, 22, 20, 
  24, 24, 2, -1, 4, 6, -1, 28, 29, 9, 
  11, 40, 41, 6, 7, 8, 24, 25, 26, 16, 
  7, 8, 7, 8, -1, 26, -1, 24, 25, 26, 
  -1, 17, -1, 17, 20, -1, 20, 23, -1, 23, 
  26, 27, 26, 27, -1, 17, 20, -1, 20, 34, 
  35, 23, 2, 3, 26, 27, 30, 31, 32, 33, 
  10, -1, 11, -1, -1, 15, -1, -1, 2, 3, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 15, -1, 32, 33, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 45, -1, 46, 47, 48, 
  49, 50, 51, 52, -1, -1, -1, -1, -1, -1, 
  -1, 45, -1, -1, -1, -1, -1, -1, -1, -1, 
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
  -1, -1, -1, 2, -1, 4, 5, 6, 7, 8, 
  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 
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
  -1, -1, 2, 3, 4, 5, 6, 7, 8, 9, 
  10, 11, 12, 13, 14, -1, 16, 17, 18, 19, 
  20, 21, 22, 23, 24, 25, 26, 27, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 45, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 2, 3, 4, 5, 6, 
  7, 8, 9, 10, 11, 12, 13, 14, -1, 16, 
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 
  27, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, 45, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, 2, 3, 
  4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 
  14, -1, 16, 17, 18, 19, 20, 21, 22, 23, 
  24, 25, 26, 27, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 45, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 2, -1, 4, 5, 6, 7, 8, 9, 10, 
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
  21, 22, 23, 24, 25, 26, 27, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, 45, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, 2, 3, 4, 5, 6, 7, 
  8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 2, 3, 4, 
  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  45, -1, -1, -1, -1, -1, -1, -1, -1, -1, 

  20, 51, 13, 17, 14, 20, 1, 14, 17, 49, 
  13, 13, 13, 17, 14, 20, 51, 13, 17, 13, 
  17, 13, 13, 1, 17, 68, 45, 14, 17, 14, 
  20, 13, 20, 13, 13, 80, 13, 68, 36, 20, 
  13, 13, 13, 13, 20, 13, 20, 13, 13, 13, 
  68, 13, 38, 36, 13, 13, 64, 13, 13, 20, 
  44, 10, 14, 20, 17, 13, 13, 17, 37, 13, 
  37, 17, 17, 14, 37, 13, -1, 20, 17, 14, 
  20, 14, 13, 43, 10, 10, 37, 17, 20, 64, 
  10, 20, 10, 17, 10, 49, 13, 21, 13, 4, 
  17, 15, 17, 17, 13, 10, 13, 6, 17, 8, 
  17, 10, 11, -1, -1, 51, -1, 51, 17, -1, 
  19, -1, 64, 51, 6, -1, 8, 64, 10, 11, 
  13, -1, -1, -1, -1, 17, -1, 19, -1, -1, 
  -1, -1, 24, 25, -1, 52, -1, 73, 76, 74, 
  -1, -1, -1, -1, 74, 73, -1, 73, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 74, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 3, 
  -1, 5, 6, 7, 8, 9, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 19, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1};


#line 83 "qxmlstream.g"

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
            raiseWellFormedError(QObject::tr("recursive entity detected."));
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
    uint lockEncoding : 1;

    int resumeReduction;
    void resume(int rule);

    inline bool entitiesAndNamespacePrefixesMustBeDeclared() const {
        return (!inParseEntity && (standalone || !referenceToUnparsedEntityDetected));
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
    inline Value &sym(int index)
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

#line 664 "qxmlstream.g"

        case 0:
            type = QXmlStreamReader::EndDocument;
        break;

#line 671 "qxmlstream.g"

        case 1:
            if (type != QXmlStreamReader::Invalid) {
                if (hasSeenTag || inParseEntity) {
                    type = QXmlStreamReader::EndDocument;
                } else {
                    raiseError(QXmlStreamReader::PrematureEndOfDocumentError, QObject::tr("Start tag expected."));
                    // reset the parser
                    tos = 0;
                    state_stack[tos++] = 0;
                    state_stack[tos] = 0;
                    return false;
                }
            }
        break;

#line 699 "qxmlstream.g"

        case 10:
            entityReferenceStack.pop()->isCurrentlyReferenced = false;
            clearSym();
        break;

#line 708 "qxmlstream.g"

        case 11:
            xmlDeclOK = (characterOffset + readBufferPos - putStack.size() == 2);
            if (!scanString(spell[XML], XML) && atEnd) {
                resume(11);
                return false;
            }
        break;

#line 719 "qxmlstream.g"

        case 12:
            if (!scanString(spell[VERSION], VERSION, false) && atEnd) {
                resume(12);
                return false;
            }
        break;

#line 729 "qxmlstream.g"

        case 13:
            type = QXmlStreamReader::StartDocument;
            if (xmlDeclOK)
                startDocument(symString(6));
            else
                raiseWellFormedError(QObject::tr("XML declaration not at start of document."));
        break;

#line 740 "qxmlstream.g"

        case 14:
            referenceToUnparsedEntityDetected = true;
        break;

#line 746 "qxmlstream.g"

        case 15:
            checkPublicLiteral(symString(2));
            referenceToUnparsedEntityDetected = true;
        break;

#line 755 "qxmlstream.g"

        case 17:
            if (!scanPublicOrSystem() && atEnd) {
                resume(17);
                return false;
            }
        break;

#line 765 "qxmlstream.g"

        case 18:
#line 768 "qxmlstream.g"

        case 19:
#line 771 "qxmlstream.g"

        case 20:
#line 774 "qxmlstream.g"

        case 21:
            type = QXmlStreamReader::DTD;
            text = &textBuffer;
        break;

#line 782 "qxmlstream.g"

        case 22:
            scanDtd = true;
        break;

#line 789 "qxmlstream.g"

        case 23:
            scanDtd = false;
        break;

#line 803 "qxmlstream.g"

        case 37:
            if (!scanString(spell[EMPTY], EMPTY, false)
                && !scanString(spell[ANY], ANY, false)
                && atEnd) {
                resume(37);
                return false;
            }
        break;

#line 820 "qxmlstream.g"

        case 43:
            if (!scanString(spell[PCDATA], PCDATA, false) && atEnd) {
                resume(43);
                return false;
            }
        break;

#line 861 "qxmlstream.g"

        case 68: {
            lastAttributeIsCData = true;
        } break;

#line 872 "qxmlstream.g"

        case 79:
            if (!scanAfterDefaultDecl() && atEnd) {
                resume(79);
                return false;
            }
        break;

#line 886 "qxmlstream.g"

        case 84:
                sym(1) = sym(2);
                lastAttributeValue.clear();
                lastAttributeIsCData = false;
                if (!scanAttType() && atEnd) {
                    resume(84);
                    return false;
                }
        break;

#line 899 "qxmlstream.g"

        case 85: {
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

#line 924 "qxmlstream.g"

        case 89: {
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

#line 947 "qxmlstream.g"

        case 90: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(90);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(3);
        } break;

#line 960 "qxmlstream.g"

        case 91: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(91);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(5);
            entityDeclaration.parameter = true;
        } break;

#line 974 "qxmlstream.g"

        case 92: {
            if (!scanNData() && atEnd) {
                resume(92);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.systemId = symString(3);
            entityDeclaration.external = true;
        } break;

#line 987 "qxmlstream.g"

        case 93: {
            if (!scanNData() && atEnd) {
                resume(93);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            checkPublicLiteral((entityDeclaration.publicId = symString(3)));
            entityDeclaration.systemId = symString(5);
            entityDeclaration.external = true;
        } break;

#line 1001 "qxmlstream.g"

        case 94: {
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.notationName = symString(3);
            if (entityDeclaration.parameter)
                raiseWellFormedError(QObject::tr("NDATA in parameter entity declaration."));
        }
        //fall through

#line 1012 "qxmlstream.g"

        case 95:
#line 1016 "qxmlstream.g"

        case 96: {
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

#line 1038 "qxmlstream.g"

        case 97: {
            type = QXmlStreamReader::ProcessingInstruction;
            int pos = sym(3).pos + sym(3).len;
            processingInstructionTarget = symString(2);
            if (scanUntil("?>")) {
                processingInstructionData = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 2);
                if (!processingInstructionTarget.toString().compare(QLatin1String("xml"), Qt::CaseInsensitive)) {
                    raiseWellFormedError(QObject::tr("Invalid processing instruction name."));
                }
            } else {
                resume(97);
                return false;
            }
        } break;

#line 1056 "qxmlstream.g"

        case 98:
            type = QXmlStreamReader::ProcessingInstruction;
            processingInstructionTarget = symString(2);
            if (!processingInstructionTarget.toString().compare(QLatin1String("xml"), Qt::CaseInsensitive))
                raiseWellFormedError(QObject::tr("Invalid processing instruction name."));
        break;

#line 1067 "qxmlstream.g"

        case 99:
            if (!scanAfterLangleBang() && atEnd) {
                resume(99);
                return false;
            }
        break;

#line 1077 "qxmlstream.g"

        case 100: {
            type = QXmlStreamReader::Comment;
            int pos = sym(3).pos + 1;
            if (scanUntil("--")) {
                text = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 2);
            } else {
                resume(100);
                return false;
            }
        } break;

#line 1094 "qxmlstream.g"

        case 102:
            type = QXmlStreamReader::Characters;
            isCDATA = true;
            clearTextBuffer();
            if (scanUntil("]]>", -1)) {
                textBuffer.chop(3);
                text = &textBuffer;
            } else {
                resume(102);
                return false;
            }
        break;

#line 1110 "qxmlstream.g"

        case 103: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(103);
                return false;
            }
            NotationDeclaration &notationDeclaration = notationDeclarations.push();
            notationDeclaration.name = symString(3);
        } break;

#line 1122 "qxmlstream.g"

        case 104: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId = symString(3);
            notationDeclaration.publicId.clear();
        } break;

#line 1131 "qxmlstream.g"

        case 105: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId.clear();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
        } break;

#line 1140 "qxmlstream.g"

        case 106: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
            notationDeclaration.systemId = symString(5);
        } break;

#line 1153 "qxmlstream.g"

        case 128:
            isWhitespace = true;
            // fall through

#line 1159 "qxmlstream.g"

        case 129:
#line 1162 "qxmlstream.g"

        case 130:
#line 1165 "qxmlstream.g"

        case 131:
            type = QXmlStreamReader::Characters;
            sym(1).len += fastScanContentCharList();
            if (atEnd && !inParseEntity) {
                resume(131);
                return false;
            }
            text = &textBuffer;
        break;

#line 1180 "qxmlstream.g"

        case 132:
#line 1183 "qxmlstream.g"

        case 133:
            clearSym();
        break;

#line 1189 "qxmlstream.g"

        case 134:
#line 1192 "qxmlstream.g"

        case 135:
            sym(1) = sym(2);
        break;

#line 1199 "qxmlstream.g"

        case 136:
#line 1202 "qxmlstream.g"

        case 137:
#line 1205 "qxmlstream.g"

        case 138:
#line 1208 "qxmlstream.g"

        case 139:
            sym(1).len += sym(2).len;
        break;

#line 1221 "qxmlstream.g"

        case 165:
            textBuffer.data()[textBuffer.size()-1] = QLatin1Char(' ');
        break;

#line 1228 "qxmlstream.g"

        case 166:
            sym(1).len += fastScanLiteralContent();
            if (atEnd) {
                resume(166);
                return false;
            }
        break;

#line 1240 "qxmlstream.g"

        case 168:
            clearSym();
        break;

#line 1247 "qxmlstream.g"

        case 169:
#line 1250 "qxmlstream.g"

        case 170:
	    sym(1) = sym(2);
        break;

#line 1257 "qxmlstream.g"

        case 171:
#line 1260 "qxmlstream.g"

        case 172:
#line 1263 "qxmlstream.g"

        case 173:
#line 1266 "qxmlstream.g"

        case 174:
            sym(1).len += sym(2).len;
        break;

#line 1281 "qxmlstream.g"

        case 205:
            clearSym();
        break;

#line 1287 "qxmlstream.g"

        case 206:
#line 1290 "qxmlstream.g"

        case 207:
            sym(1) = sym(2);
            lastAttributeValue = symString(1);
        break;

#line 1298 "qxmlstream.g"

        case 208:
#line 1301 "qxmlstream.g"

        case 209:
#line 1304 "qxmlstream.g"

        case 210:
#line 1307 "qxmlstream.g"

        case 211:
            sym(1).len += sym(2).len;
        break;

#line 1318 "qxmlstream.g"

        case 220: {
            QStringRef prefix = symPrefix(1);
            if (prefix.isEmpty() && symString(1) == QLatin1String("xmlns")) {
                NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.push();
                namespaceDeclaration.prefix.clear();
                namespaceDeclaration.namespaceUri = addToStringStorage(symString(5));
            } else if (prefix == QLatin1String("xmlns")) {
                NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.push();
                QStringRef namespacePrefix = symString(1);
                QStringRef namespaceUri = symString(5);
                if (namespacePrefix == QLatin1String("xml")
                    && namespaceForPrefix(namespacePrefix) != namespaceUri)
                    raiseWellFormedError(QObject::tr("Namespace \'xml\' redefined."));

                namespaceDeclaration.prefix = addToStringStorage(namespacePrefix);
                namespaceDeclaration.namespaceUri = addToStringStorage(namespaceUri);
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
            }
        } break;

#line 1385 "qxmlstream.g"

        case 226: {
            Tag &tag = tagStack_push();
            prefix = tag.namespaceDeclaration.prefix  = addToStringStorage(symPrefix(2));
            name = tag.name = addToStringStorage(symString(2));
            qualifiedName = tag.qualifiedName = addToStringStorage(symName(2));
            if (!validateName(qualifiedName))
                raiseWellFormedError(QObject::tr("Invalid XML name."));
        } break;

#line 1398 "qxmlstream.g"

        case 227:
            isEmptyElement = true;
        // fall through

#line 1406 "qxmlstream.g"

        case 228:
            type = QXmlStreamReader::StartElement;
            resolveTag();
            if (tagStack.size() == 1 && hasSeenTag && !inParseEntity)
                raiseWellFormedError(QObject::tr("Extra content at end of document."));
            hasSeenTag = true;
        break;

#line 1418 "qxmlstream.g"

        case 229: {
            type = QXmlStreamReader::EndElement;
            Tag &tag = tagStack_pop();

            namespaceUri = tag.namespaceDeclaration.namespaceUri;
            name = tag.name;
            qualifiedName = tag.qualifiedName;
            if (qualifiedName != symName(3))
                raiseWellFormedError(QObject::tr("Opening and ending tag mismatch."));
        } break;

#line 1432 "qxmlstream.g"

        case 230: {
            sym(1).len += sym(2).len + 1;
            QString reference = symString(2).toString();
            if (entityHash.contains(reference)) {
                Entity &entity = entityHash[reference];
                if (entity.unparsed) {
                    raiseWellFormedError(QObject::tr("reference to unparsed entity '%1'.").arg(reference));
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
            } else if (entitiesAndNamespacePrefixesMustBeDeclared()) {
                raiseWellFormedError(QObject::tr("entity '%1' not declared.").arg(reference));
                break;
            }
            type = QXmlStreamReader::EntityReference;
            name = symString(2);

        } break;

#line 1464 "qxmlstream.g"

        case 231: {
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
            } else if (entitiesAndNamespacePrefixesMustBeDeclared()) {
                raiseWellFormedError(QObject::tr("entity '%1' not declared.").arg(symString(2).toString()));
            }
        } break;

#line 1487 "qxmlstream.g"

        case 232:
            sym(1).len += sym(2).len + 1;
        break;

#line 1494 "qxmlstream.g"

        case 233: {
            sym(1).len += sym(2).len + 1;
            QString reference = symString(2).toString();
            if (entityHash.contains(reference)) {
                Entity &entity = entityHash[reference];
                if (entity.unparsed || entity.value.isNull()) {
                    raiseWellFormedError(QObject::tr("reference to external entity '%1' in attribute value.").arg(reference));
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
            } else if (entitiesAndNamespacePrefixesMustBeDeclared()) {
                raiseWellFormedError(QObject::tr("entity '%1' not declared.").arg(reference));
            }
        } break;

#line 1521 "qxmlstream.g"

        case 234: {
            if (uint s = resolveCharRef(3)) {
                if (s >= 0xffff)
                    putStringLiteral(QString::fromUcs4(&s, 1));
                else
                    putChar((LETTER << 16) | s);

                textBuffer.chop(3 + sym(3).len);
                clearSym();
            } else {
                raiseWellFormedError(QObject::tr("Invalid character reference."));
            }
        } break;

#line 1538 "qxmlstream.g"

        case 235: {
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
                raiseWellFormedError(QObject::tr("Invalid character reference."));
            }
        } break;

#line 1561 "qxmlstream.g"

        case 238:
#line 1564 "qxmlstream.g"

        case 239:
            sym(1).len += sym(2).len;
        break;

#line 1586 "qxmlstream.g"

        case 252:
            sym(1).len += fastScanSpace();
            if (atEnd) {
                resume(252);
                return false;
            }
        break;

#line 1606 "qxmlstream.g"

        case 255: {
            sym(1).len += fastScanName(&sym(1).prefix);
            if (atEnd) {
                resume(255);
                return false;
            }
        } break;

#line 1622 "qxmlstream.g"

        case 256:
            sym(1).len += fastScanName();
            if (atEnd) {
                resume(256);
                return false;
            }
        break;

#line 1633 "qxmlstream.g"

        case 257:
#line 1636 "qxmlstream.g"

        case 258:
#line 1639 "qxmlstream.g"

        case 259:
#line 1642 "qxmlstream.g"

        case 260:
#line 1645 "qxmlstream.g"

        case 261:
            sym(1).len += fastScanName();
            if (atEnd) {
                resume(261);
                return false;
            }
        break;

#line 1656 "qxmlstream.g"

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
