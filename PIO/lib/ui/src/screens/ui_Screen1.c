// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.4
// LVGL version: 8.3.6
// Project name: td world clock

#include "../ui.h"

void ui_Screen1_screen_init(void)
{
ui_Screen1 = lv_obj_create(NULL);
lv_obj_clear_flag( ui_Screen1, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(ui_Screen1, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Panel_map = lv_obj_create(ui_Screen1);
lv_obj_set_width( ui_Panel_map, 162);
lv_obj_set_height( ui_Panel_map, 78);
lv_obj_set_x( ui_Panel_map, 6 );
lv_obj_set_y( ui_Panel_map, 6 );
lv_obj_clear_flag( ui_Panel_map, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_radius(ui_Panel_map, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_bg_color(ui_Panel_map, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(ui_Panel_map, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_border_color(ui_Panel_map, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_border_opa(ui_Panel_map, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_outline_color(ui_Panel_map, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_outline_opa(ui_Panel_map, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_img_map_bg = lv_img_create(ui_Panel_map);
lv_img_set_src(ui_img_map_bg, &ui_img_map_image_s_png);
lv_obj_set_width( ui_img_map_bg, LV_SIZE_CONTENT);  /// 162
lv_obj_set_height( ui_img_map_bg, LV_SIZE_CONTENT);   /// 78
lv_obj_set_align( ui_img_map_bg, LV_ALIGN_CENTER );
lv_obj_add_flag( ui_img_map_bg, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( ui_img_map_bg, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

ui_img_map_bg_blue = lv_img_create(ui_Panel_map);
lv_img_set_src(ui_img_map_bg_blue, &ui_img_map_image_blue_png);
lv_obj_set_width( ui_img_map_bg_blue, LV_SIZE_CONTENT);  /// 162
lv_obj_set_height( ui_img_map_bg_blue, LV_SIZE_CONTENT);   /// 78
lv_obj_set_align( ui_img_map_bg_blue, LV_ALIGN_CENTER );
lv_obj_add_flag( ui_img_map_bg_blue, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( ui_img_map_bg_blue, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

ui_img_world_map = lv_img_create(ui_Panel_map);
lv_img_set_src(ui_img_world_map, &ui_img_world_map_png);
lv_obj_set_width( ui_img_world_map, LV_SIZE_CONTENT);  /// 143
lv_obj_set_height( ui_img_world_map, LV_SIZE_CONTENT);   /// 70
lv_obj_set_x( ui_img_world_map, -1 );
lv_obj_set_y( ui_img_world_map, 1 );
lv_obj_set_align( ui_img_world_map, LV_ALIGN_CENTER );
lv_obj_add_flag( ui_img_world_map, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( ui_img_world_map, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_blend_mode(ui_img_world_map, LV_BLEND_MODE_MULTIPLY, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_img_recolor(ui_img_world_map, lv_color_hex(0x000000), LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_img_recolor_opa(ui_img_world_map, 100, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label_cc_a = lv_label_create(ui_Panel_map);
lv_obj_set_width( ui_Label_cc_a, 34);
lv_obj_set_height( ui_Label_cc_a, 17);
lv_obj_set_x( ui_Label_cc_a, -7 );
lv_obj_set_y( ui_Label_cc_a, 44 );
lv_label_set_long_mode(ui_Label_cc_a,LV_LABEL_LONG_CLIP);
lv_label_set_text(ui_Label_cc_a,"xxx");
lv_obj_set_style_text_color(ui_Label_cc_a, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_cc_a, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_cc_a, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_cc_a, &ui_font_ds12, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Panel_local_time = lv_obj_create(ui_Screen1);
lv_obj_set_width( ui_Panel_local_time, 141);
lv_obj_set_height( ui_Panel_local_time, 44);
lv_obj_set_x( ui_Panel_local_time, 173 );
lv_obj_set_y( ui_Panel_local_time, 6 );
lv_obj_clear_flag( ui_Panel_local_time, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_radius(ui_Panel_local_time, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_bg_color(ui_Panel_local_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(ui_Panel_local_time, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_border_color(ui_Panel_local_time, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_border_opa(ui_Panel_local_time, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_img_local_time = lv_img_create(ui_Panel_local_time);
lv_img_set_src(ui_img_local_time, &ui_img_current_time_s_png);
lv_obj_set_width( ui_img_local_time, LV_SIZE_CONTENT);  /// 141
lv_obj_set_height( ui_img_local_time, LV_SIZE_CONTENT);   /// 44
lv_obj_set_align( ui_img_local_time, LV_ALIGN_CENTER );
lv_obj_add_flag( ui_img_local_time, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( ui_img_local_time, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

ui_img_local_time_blue = lv_img_create(ui_Panel_local_time);
lv_img_set_src(ui_img_local_time_blue, &ui_img_current_time_blue_png);
lv_obj_set_width( ui_img_local_time_blue, LV_SIZE_CONTENT);  /// 141
lv_obj_set_height( ui_img_local_time_blue, LV_SIZE_CONTENT);   /// 44
lv_obj_set_align( ui_img_local_time_blue, LV_ALIGN_CENTER );
lv_obj_add_flag( ui_img_local_time_blue, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( ui_img_local_time_blue, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

ui_Label_lt_b = lv_label_create(ui_Panel_local_time);
lv_obj_set_width( ui_Label_lt_b, 106);
lv_obj_set_height( ui_Label_lt_b, 32);
lv_obj_set_x( ui_Label_lt_b, -14 );
lv_obj_set_y( ui_Label_lt_b, 1 );
lv_obj_set_align( ui_Label_lt_b, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label_lt_b,"00:00");
lv_obj_set_style_text_color(ui_Label_lt_b, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_lt_b, 70, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_lt_b, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_lt_b, &ui_font_ds29, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label_lt_a = lv_label_create(ui_Panel_local_time);
lv_obj_set_width( ui_Label_lt_a, 106);
lv_obj_set_height( ui_Label_lt_a, 32);
lv_obj_set_x( ui_Label_lt_a, -14 );
lv_obj_set_y( ui_Label_lt_a, 1 );
lv_obj_set_align( ui_Label_lt_a, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label_lt_a,"00:00");
lv_obj_set_style_text_color(ui_Label_lt_a, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_lt_a, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_lt_a, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_lt_a, &ui_font_ds29, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label_lt_ampm = lv_label_create(ui_Panel_local_time);
lv_obj_set_width( ui_Label_lt_ampm, 30);
lv_obj_set_height( ui_Label_lt_ampm, 22);
lv_obj_set_x( ui_Label_lt_ampm, 51 );
lv_obj_set_y( ui_Label_lt_ampm, -5 );
lv_obj_set_align( ui_Label_lt_ampm, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label_lt_ampm,"xx");
lv_obj_set_style_text_color(ui_Label_lt_ampm, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_lt_ampm, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_lt_ampm, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_lt_ampm, &ui_font_ds16, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Panel_world_time = lv_obj_create(ui_Screen1);
lv_obj_set_width( ui_Panel_world_time, 308);
lv_obj_set_height( ui_Panel_world_time, 108);
lv_obj_set_x( ui_Panel_world_time, 6 );
lv_obj_set_y( ui_Panel_world_time, 56 );
lv_obj_clear_flag( ui_Panel_world_time, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_radius(ui_Panel_world_time, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_bg_color(ui_Panel_world_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(ui_Panel_world_time, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_border_color(ui_Panel_world_time, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_border_opa(ui_Panel_world_time, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_img_world_time = lv_img_create(ui_Panel_world_time);
lv_img_set_src(ui_img_world_time, &ui_img_world_time_s_png);
lv_obj_set_width( ui_img_world_time, LV_SIZE_CONTENT);  /// 147
lv_obj_set_height( ui_img_world_time, LV_SIZE_CONTENT);   /// 44
lv_obj_set_align( ui_img_world_time, LV_ALIGN_CENTER );
lv_obj_add_flag( ui_img_world_time, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( ui_img_world_time, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

ui_img_world_time_blue = lv_img_create(ui_Panel_world_time);
lv_img_set_src(ui_img_world_time_blue, &ui_img_world_time_blue_png);
lv_obj_set_width( ui_img_world_time_blue, LV_SIZE_CONTENT);  /// 308
lv_obj_set_height( ui_img_world_time_blue, LV_SIZE_CONTENT);   /// 108
lv_obj_set_align( ui_img_world_time_blue, LV_ALIGN_CENTER );
lv_obj_add_flag( ui_img_world_time_blue, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( ui_img_world_time_blue, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

ui_Label_wt_b = lv_label_create(ui_Panel_world_time);
lv_obj_set_width( ui_Label_wt_b, 212);
lv_obj_set_height( ui_Label_wt_b, 66);
lv_obj_set_x( ui_Label_wt_b, -5 );
lv_obj_set_y( ui_Label_wt_b, 25 );
lv_label_set_text(ui_Label_wt_b,"00:00");
lv_obj_set_style_text_color(ui_Label_wt_b, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_wt_b, 70, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_wt_b, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_wt_b, &ui_font_ds60, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label_wt_a = lv_label_create(ui_Panel_world_time);
lv_obj_set_width( ui_Label_wt_a, 212);
lv_obj_set_height( ui_Label_wt_a, 66);
lv_obj_set_x( ui_Label_wt_a, -5 );
lv_obj_set_y( ui_Label_wt_a, 25 );
lv_label_set_text(ui_Label_wt_a,"00:00");
lv_obj_set_style_text_color(ui_Label_wt_a, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_wt_a, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_wt_a, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_wt_a, &ui_font_ds60, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label_cc = lv_label_create(ui_Panel_world_time);
lv_obj_set_width( ui_Label_cc, 48);
lv_obj_set_height( ui_Label_cc, 17);
lv_obj_set_x( ui_Label_cc, 164 );
lv_obj_set_y( ui_Label_cc, -6 );
lv_label_set_text(ui_Label_cc,"xxx");
lv_obj_set_style_text_color(ui_Label_cc, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_cc, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_cc, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_cc, &ui_font_ds16, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label_gmt = lv_label_create(ui_Panel_world_time);
lv_obj_set_width( ui_Label_gmt, 64);
lv_obj_set_height( ui_Label_gmt, 17);
lv_obj_set_x( ui_Label_gmt, 215 );
lv_obj_set_y( ui_Label_gmt, -6 );
lv_label_set_text(ui_Label_gmt,"XX:XX");
lv_obj_set_style_text_color(ui_Label_gmt, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_gmt, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_gmt, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_gmt, &ui_font_ds16, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label_wt_ampm = lv_label_create(ui_Panel_world_time);
lv_obj_set_width( ui_Label_wt_ampm, 38);
lv_obj_set_height( ui_Label_wt_ampm, 17);
lv_obj_set_x( ui_Label_wt_ampm, 220 );
lv_obj_set_y( ui_Label_wt_ampm, 25 );
lv_label_set_text(ui_Label_wt_ampm,"XX");
lv_obj_set_style_text_color(ui_Label_wt_ampm, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_wt_ampm, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_wt_ampm, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_wt_ampm, &ui_font_ds20, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label_wt_sec_b = lv_label_create(ui_Panel_world_time);
lv_obj_set_width( ui_Label_wt_sec_b, 54);
lv_obj_set_height( ui_Label_wt_sec_b, 28);
lv_obj_set_x( ui_Label_wt_sec_b, 215 );
lv_obj_set_y( ui_Label_wt_sec_b, 55 );
lv_label_set_text(ui_Label_wt_sec_b,"00");
lv_obj_set_style_text_color(ui_Label_wt_sec_b, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_wt_sec_b, 70, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_wt_sec_b, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_wt_sec_b, &ui_font_ds29, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label_wt_sec_a = lv_label_create(ui_Panel_world_time);
lv_obj_set_width( ui_Label_wt_sec_a, 54);
lv_obj_set_height( ui_Label_wt_sec_a, 28);
lv_obj_set_x( ui_Label_wt_sec_a, 215 );
lv_obj_set_y( ui_Label_wt_sec_a, 55 );
lv_label_set_text(ui_Label_wt_sec_a,"00");
lv_obj_set_style_text_color(ui_Label_wt_sec_a, lv_color_hex(0x161616), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_Label_wt_sec_a, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(ui_Label_wt_sec_a, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label_wt_sec_a, &ui_font_ds29, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_img_loc = lv_img_create(ui_Screen1);
lv_img_set_src(ui_img_loc, &ui_img_loc_png);
lv_obj_set_width( ui_img_loc, 27);
lv_obj_set_height( ui_img_loc, 27);
lv_obj_set_x( ui_img_loc, 11 );
lv_obj_set_y( ui_img_loc, 12 );
lv_obj_add_flag( ui_img_loc, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( ui_img_loc, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_img_set_zoom(ui_img_loc,200);

ui_action_1 = lv_label_create(ui_Screen1);
lv_obj_set_width( ui_action_1, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_action_1, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_action_1, 20 );
lv_obj_set_y( ui_action_1, -52 );
lv_obj_set_align( ui_action_1, LV_ALIGN_CENTER );
lv_label_set_text(ui_action_1,"]");
lv_obj_add_flag( ui_action_1, LV_OBJ_FLAG_HIDDEN );   /// Flags
lv_obj_set_style_text_color(ui_action_1, lv_color_hex(0x101410), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_action_1, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_action_1, &ui_font_ds29, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_action_2 = lv_label_create(ui_Screen1);
lv_obj_set_width( ui_action_2, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_action_2, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_action_2, -147 );
lv_obj_set_y( ui_action_2, 49 );
lv_obj_set_align( ui_action_2, LV_ALIGN_CENTER );
lv_label_set_text(ui_action_2,"]");
lv_obj_add_flag( ui_action_2, LV_OBJ_FLAG_HIDDEN );   /// Flags
lv_obj_set_style_text_color(ui_action_2, lv_color_hex(0x101410), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_action_2, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_action_2, &ui_font_ds29, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_action_3 = lv_label_create(ui_Screen1);
lv_obj_set_width( ui_action_3, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_action_3, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_action_3, -147 );
lv_obj_set_y( ui_action_3, -33 );
lv_obj_set_align( ui_action_3, LV_ALIGN_CENTER );
lv_label_set_text(ui_action_3,"]");
lv_obj_add_flag( ui_action_3, LV_OBJ_FLAG_HIDDEN );   /// Flags
lv_obj_set_style_text_color(ui_action_3, lv_color_hex(0x101410), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(ui_action_3, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_action_3, &ui_font_ds29, LV_PART_MAIN| LV_STATE_DEFAULT);

}
