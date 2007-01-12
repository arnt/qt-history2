#ifndef GRAMMAR_P_H
#define GRAMMAR_P_H

class grammar
{
public:
  enum {
    EOF_SYMBOL = 0,
    COLON = 7,
    DECL = 12,
    DECL_FILE = 4,
    ERROR = 21,
    EXPECT = 14,
    EXPECT_RR = 15,
    ID = 2,
    IMPL = 13,
    IMPL_FILE = 5,
    LEFT = 16,
    MERGED_OUTPUT = 1,
    NONASSOC = 18,
    OR = 20,
    PARSER = 3,
    PREC = 19,
    RIGHT = 17,
    SEMICOLON = 8,
    START = 9,
    STRING_LITERAL = 10,
    TOKEN = 6,
    TOKEN_PREFIX = 11,

    ACCEPT_STATE = 68,
    RULE_COUNT = 45,
    STATE_COUNT = 69,
    TERMINAL_COUNT = 22,
    NON_TERMINAL_COUNT = 24,

    GOTO_INDEX_OFFSET = 69,
    GOTO_INFO_OFFSET = 60,
    GOTO_CHECK_OFFSET = 60,
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


#endif // GRAMMAR_P_H

