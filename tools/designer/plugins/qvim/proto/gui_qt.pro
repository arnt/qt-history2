/* gui_kde.c */
void gui_mch_add_menu __ARGS((VimMenu *menu, VimMenu *parent, int idx));
void gui_mch_add_menu_item __ARGS((VimMenu *menu, VimMenu *parent, int idx));
void gui_mch_set_text_area_pos __ARGS((int x, int y, int w, int h));
void gui_mch_toggle_tearoffs __ARGS((int enable));
void gui_mch_destroy_menu __ARGS((VimMenu *menu));
void gui_mch_set_scrollbar_thumb __ARGS((GuiScrollbar *sb, int val, int size, int max));
void gui_mch_set_scrollbar_pos __ARGS((GuiScrollbar *sb, int x, int y, int w, int h));
void gui_mch_create_scrollbar __ARGS((GuiScrollbar *sb, int orient));
void gui_mch_destroy_scrollbar __ARGS((GuiScrollbar *sb));
char_u *gui_mch_browse __ARGS((int saving, char_u *title, char_u *dflt, char_u *ext, char_u *initdir, char_u *filter));
int gui_mch_dialog __ARGS((int type, char_u *title, char_u *message, char_u *buttons, int def_but));
void gui_mch_show_popupmenu __ARGS((VimMenu *menu));
void do_helpfind __ARGS((void));
