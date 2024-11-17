#include "displayapp/screens/WatchFaceCasioStyleAE21W.h"

#include <lvgl/lvgl.h>
//#include <cstdio>
//#include <cmath>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/Symbols.h"
//#include "components/battery/BatteryController.h"
//#include "components/ble/BleController.h"
#include "displayapp/screens/NotificationIcon.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
//#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;


// Time background lines points
static constexpr lv_point_t linePointsTBG[6][2] = {
    {{13,181},{227,181}},
    {{3,181},{237,181}},
    {{8,140},{19,130}},
    {{221,130},{232,140}},
    {{232,222},{221,232}},
    {{19,232},{8,222}}
};
// Time background lines widths
static constexpr lv_style_int_t lineWidthsTBG[7] = {
    112, 92, 15, 15, 15, 15
};
// Time table lines points
static constexpr lv_point_t linePointsTT[4][2] = {
    {{2, 169},{237, 169}},
    {{132,125},{132, 169}},
    {{68, 125},{68, 169}},
    {{2, 147},{132, 147}}
};

// Graph frames lines points
static constexpr lv_point_t linePointsGF[2][9] = {
    {{15, 2}, {101, 2}, {113, 15}, {113, 101}, {101, 113}, {15, 113}, {2, 101}, {2, 15}, {15, 2}},
    {{139, 2}, {225, 2}, {237, 15}, {237, 101}, {225, 113}, {139, 113}, {126, 101}, {126, 15}, {139, 2}}
};

// Graph1 grid lines points
static constexpr lv_point_t linePointsG1G[10][2] = {
    // vertical lines
    {{15, 0}, {15, 90}},
    {{30, 0}, {30, 90}},
    {{45, 0}, {45, 90}},
    {{60, 0}, {60, 90}},
    {{75, 0}, {75, 90}},
    // horizontal lines
    {{0, 15}, {90, 15}},
    {{0, 30}, {90, 30}},
    {{0, 45}, {90, 45}},
    {{0, 60}, {90, 60}},
    {{0, 75}, {90, 75}}
};
// Graphs1 scales top arrows lines points
static constexpr lv_point_t linePointsG1STA[4][2] = {
    {{55, 4}, {55, 12}},
    {{60, 4}, {60, 12}},
    {{55, 11}, {56, 13}},
    {{60, 11}, {59, 13}}
};

// Graphs1 scales top arrows lines widths
static constexpr lv_style_int_t lineWidthsG1STA[4] = {
    3, 3, 2, 2
};

// SEC label lines points
static constexpr lv_point_t linePointsSEC[12][2] = {
    // s
    {{0, 0}, {4, 0}},
    {{0, 0}, {0, 3}},
    {{0, 3}, {4, 3}},
    {{4, 3}, {4, 6}},
    {{0, 6}, {4, 6}},

    // e
    {{7, 0}, {11, 0}},
    {{7, 0}, {7, 6}},
    {{7, 3}, {11, 3}},
    {{7, 6}, {11, 6}},

    // c
    {{14, 0}, {18, 0}},
    {{14, 0}, {14, 6}},
    {{14, 6}, {18, 6}}
};

static constexpr int16_t HourLength = 27;
static constexpr int16_t MinuteLength = 42;
static constexpr int16_t SecondLength = 43;

namespace {
    // sin(90) = 1 so the value of _lv_trigo_sin(90) is the scaling factor
    const auto LV_TRIG_SCALE = _lv_trigo_sin(90);

    int16_t Cosine(int16_t angle) {
        return _lv_trigo_sin(angle + 90);
    }

    int16_t Sine(int16_t angle) {
        return _lv_trigo_sin(angle);
    }
    
    int16_t CoordinateXRelocateG1(int16_t x) {
        return (x - 63 + LV_HOR_RES / 2);
    }

    int16_t CoordinateYRelocateG1(int16_t y) {
        return std::abs(y + 64 - LV_HOR_RES / 2);
    }

    int16_t CoordinateXRelocateG2(int16_t x) {
        return (x + 61 + LV_HOR_RES / 2);
    }

    int16_t CoordinateYRelocateG2(int16_t y) {
        return std::abs(y + 64 - LV_HOR_RES / 2);
    }

    lv_point_t CoordinateRelocateG1(int16_t radius, int16_t angle) {
        return lv_point_t {.x = CoordinateXRelocateG1(radius * static_cast<int32_t>(Sine(angle)) / LV_TRIG_SCALE),
                           .y = CoordinateYRelocateG1(radius * static_cast<int32_t>(Cosine(angle)) / LV_TRIG_SCALE)};
    }
    lv_point_t CoordinateRelocateG2(int16_t radius, int16_t angle) {
        return lv_point_t {.x = CoordinateXRelocateG2(radius * static_cast<int32_t>(Sine(angle)) / LV_TRIG_SCALE),
                           .y = CoordinateYRelocateG2(radius * static_cast<int32_t>(Cosine(angle)) / LV_TRIG_SCALE)};
    }
}

WatchFaceCasioStyleAE21W::WatchFaceCasioStyleAE21W(Controllers::DateTime& dateTimeController,
                                                   const Controllers::Battery& batteryController,
                                                   const Controllers::Ble& bleController,
                                                   Controllers::NotificationManager& notificationManager,
                                                   Controllers::Settings& settingsController,
                                                   Controllers::HeartRateController& heartRateController,
                                                   Controllers::MotionController& motionController,
                                                   Controllers::FS& filesystem)
: currentDateTime {{}},
  batteryIcon(true),
  dateTimeController {dateTimeController},
  batteryController {batteryController},
  bleController {bleController},
  notificationManager {notificationManager},
  settingsController {settingsController},
  heartRateController {heartRateController},
  motionController {motionController} {

    lfs_file f = {};
    if (filesystem.FileOpen(&f, "/fonts/7segments_20.bin", LFS_O_RDONLY) >= 0) {
        filesystem.FileClose(&f);
        font_segment20 = lv_font_load("F:/fonts/7segments_20.bin");
    }
    if (filesystem.FileOpen(&f, "/fonts/7segments_40.bin", LFS_O_RDONLY) >= 0) {
        filesystem.FileClose(&f);
        font_segment40 = lv_font_load("F:/fonts/7segments_40.bin");
    }
  
    if (filesystem.FileOpen(&f, "/fonts/7segments_50.bin", LFS_O_RDONLY) >= 0) {
        filesystem.FileClose(&f);
        font_segment50 = lv_font_load("F:/fonts/7segments_50.bin");
    }

    if (filesystem.FileOpen(&f, "/fonts/7segments_75.bin", LFS_O_RDONLY) >= 0) {
        filesystem.FileClose(&f);
        font_segment75 = lv_font_load("F:/fonts/7segments_75.bin");
    }

    // set lines styles
    lv_style_init(&style_gray_lines);
    lv_style_set_line_width(&style_gray_lines, LV_STATE_DEFAULT, 2);
    lv_style_set_line_color(&style_gray_lines, LV_STATE_DEFAULT, color_gray);
    lv_style_set_bg_color(&style_gray_lines, LV_STATE_DEFAULT, color_gray);
    lv_style_set_line_rounded(&style_gray_lines, LV_STATE_DEFAULT, true);
    
    lv_style_init(&style_black_lines);    
    lv_style_set_line_width(&style_black_lines, LV_STATE_DEFAULT, 1);
    lv_style_set_line_color(&style_black_lines, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_line_rounded(&style_black_lines, LV_STATE_DEFAULT, true);

    // Draw time background
    for (int i = 0; i < 7; i++) {
        lines[i] = lv_line_create(lv_scr_act(), nullptr);
        lv_obj_set_style_local_line_width(lines[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lineWidthsTBG[i]);
        lv_obj_set_style_local_line_color(lines[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, color_gray);
        lv_line_set_points(lines[i], linePointsTBG[i], 2);
    }

    // Draw graphs discs
    graphDisc = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(graphDisc, 85, 85);
    lv_obj_set_style_local_radius(graphDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 43);
    lv_obj_set_style_local_bg_color(graphDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_gray);
    lv_obj_align(graphDisc, nullptr, LV_ALIGN_IN_TOP_MID, -63, 14);

    graphDisc = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(graphDisc, 85, 85);
    lv_obj_set_style_local_radius(graphDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 43);
    lv_obj_set_style_local_bg_color(graphDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_disc2);
    lv_obj_align(graphDisc, nullptr, LV_ALIGN_IN_TOP_MID, 61, 14);

    // small graph1 disc
    graphDisc = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(graphDisc, 15, 15);
    lv_obj_set_style_local_radius(graphDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_bg_color(graphDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_align(graphDisc, nullptr, LV_ALIGN_IN_TOP_MID, -63, 49);

    // Draw Graphs frames
    for (int i = 0; i < 2; i++) {
        lines[i] = lv_line_create(lv_scr_act(), nullptr);
        lv_line_set_points(lines[i], linePointsGF[i], 9);
        lv_obj_add_style(lines[i], LV_LINE_PART_MAIN, &style_gray_lines);
    }

    // graph1 DRAW grid        
    for (int i = 0; i < 10; i++) {
        lines[i] = lv_line_create(lv_scr_act(), nullptr);
        lv_obj_add_style(lines[i], LV_OBJ_PART_MAIN, &style_black_lines);
        lv_obj_align(lines[i], nullptr, LV_ALIGN_IN_TOP_MID, -58, 11);
        lv_line_set_points(lines[i], linePointsG1G[i], 2);
    }
    
    // Draw Background time table
    for (int i = 0; i < 4; i++) {
        lines[i] = lv_line_create(lv_scr_act(), nullptr);
        lv_line_set_points(lines[i], linePointsTT[i], 2);
        lv_obj_add_style(lines[i], LV_LINE_PART_MAIN, &style_black_lines);
    }

    // Draw graph1 scales
    lines[0] = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lines[0], 129, 129);
    lv_linemeter_set_scale(lines[0], 336, 57);
    lv_linemeter_set_angle_offset(lines[0], 180);
    lv_obj_set_style_local_bg_opa(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_scale_end_line_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_scale_end_color(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, color_gray);
    lv_obj_align(lines[0], nullptr, LV_ALIGN_IN_TOP_MID, -62, -6);

    lines[0] = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lines[0], 131, 131);
    lv_linemeter_set_scale(lines[0], 300, 11);
    lv_linemeter_set_angle_offset(lines[0], 180);
    lv_obj_set_style_local_bg_opa(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 6);
    lv_obj_set_style_local_scale_end_line_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_scale_end_color(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, color_gray);
    lv_obj_align(lines[0], nullptr, LV_ALIGN_IN_TOP_MID, -62, -8);

    lines[0] = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lines[0], 131, 131);
    lv_linemeter_set_scale(lines[0], 180, 3);
    lv_linemeter_set_angle_offset(lines[0], 180);
    lv_obj_set_style_local_bg_opa(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 7);
    lv_obj_set_style_local_scale_end_line_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_scale_end_color(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, color_gray);
    lv_obj_align(lines[0], nullptr, LV_ALIGN_IN_TOP_MID, -62, -8);

    // Draw graph2 scales
    lines[0] = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lines[0], 129, 129);
    lv_linemeter_set_scale(lines[0], 354, 60);
    lv_linemeter_set_angle_offset(lines[0], 3);
    lv_obj_set_style_local_bg_opa(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_scale_end_line_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_scale_end_color(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, color_gray);
    lv_obj_align(lines[0], nullptr, LV_ALIGN_IN_TOP_MID, 62, -6);
    
    lines[0] = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lines[0], 131, 131);
    lv_linemeter_set_scale(lines[0], 330, 12);
    lv_linemeter_set_angle_offset(lines[0], 15);
    lv_obj_set_style_local_bg_opa(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_scale_end_line_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_scale_end_color(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, color_gray);
    lv_obj_align(lines[0], nullptr, LV_ALIGN_IN_TOP_MID, 62, -8);

    lines[0] = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lines[0], 131, 131);
    lv_linemeter_set_scale(lines[0], 270, 4);
    lv_linemeter_set_angle_offset(lines[0], 45);
    lv_obj_set_style_local_bg_opa(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 7);
    lv_obj_set_style_local_scale_end_line_width(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_scale_end_color(lines[0], LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, color_gray);
    lv_obj_align(lines[0], nullptr, LV_ALIGN_IN_TOP_MID, 62, -8);

    // Draw graph1 scales top arrows
    for (int i = 0; i < 4; i++) {
        lines[i] = lv_line_create(lv_scr_act(), nullptr);
        lv_obj_set_style_local_line_width(lines[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lineWidthsG1STA[i]);
        lv_obj_set_style_local_line_color(lines[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, color_gray);
        lv_line_set_points(lines[i], linePointsG1STA[i], 2);
    }


    // G1 clock
    minute_body = lv_line_create(lv_scr_act(), nullptr);
    minute_body_trace = lv_line_create(lv_scr_act(), nullptr);
    hour_body = lv_line_create(lv_scr_act(), nullptr);
    
    lv_style_init(&second_line_style);
    lv_style_set_line_width(&second_line_style, LV_STATE_DEFAULT, 3);
    lv_style_set_line_color(&second_line_style, LV_STATE_DEFAULT, color_text);
    lv_style_set_line_rounded(&second_line_style, LV_STATE_DEFAULT, true);

    lv_style_init(&minute_line_style);
    lv_style_set_line_width(&minute_line_style, LV_STATE_DEFAULT, 3);
    lv_style_set_line_color(&minute_line_style, LV_STATE_DEFAULT, color_text);
    lv_style_set_line_rounded(&minute_line_style, LV_STATE_DEFAULT, true);
    lv_obj_add_style(minute_body, LV_LINE_PART_MAIN, &minute_line_style);

    lv_style_init(&minute_line_style_trace);
    lv_style_set_line_width(&minute_line_style_trace, LV_STATE_DEFAULT, 3);
    lv_style_set_line_color(&minute_line_style_trace, LV_STATE_DEFAULT, color_text);
    lv_style_set_line_rounded(&minute_line_style_trace, LV_STATE_DEFAULT, false);
    lv_obj_add_style(minute_body_trace, LV_LINE_PART_MAIN, &minute_line_style_trace);

    lv_style_init(&hour_line_style);
    lv_style_set_line_width(&hour_line_style, LV_STATE_DEFAULT, 3);
    lv_style_set_line_color(&hour_line_style, LV_STATE_DEFAULT, color_text);
    lv_style_set_line_rounded(&hour_line_style, LV_STATE_DEFAULT, true);
    lv_obj_add_style(hour_body, LV_LINE_PART_MAIN, &hour_line_style);


    // Draw invisible seconds lines
     for (int i = 0; i < 60; i++) {
         second_body[i] = lv_line_create(lv_scr_act(), nullptr);
         second_points[i][0] = CoordinateRelocateG2(15, i * 6);
         second_points[i][1] = CoordinateRelocateG2(SecondLength, i * 6);
         lv_line_set_points(second_body[i], second_points[i], 2);
         lv_obj_set_hidden(second_body[i], true);
         lv_obj_add_style(second_body[i], LV_LINE_PART_MAIN, &second_line_style);
     }

    // small graph2 disc
    graphDisc = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(graphDisc, 29, 29);
    lv_obj_set_style_local_radius(graphDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 14);
    lv_obj_set_style_local_bg_color(graphDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_align(graphDisc, nullptr, LV_ALIGN_IN_TOP_MID, 61, 42);

    // Draw Graphs2 SEC label
    for (int i = 0; i < 12; i++) {
        lines[i] = lv_line_create(lv_scr_act(), nullptr);
        lv_obj_align(lines[i], nullptr, LV_ALIGN_IN_TOP_MID, 103, 53);
        lv_line_set_points(lines[i], linePointsSEC[i], 2);
        lv_obj_add_style(lines[i], LV_LINE_PART_MAIN, &style_gray_lines);
    }

    // Icons and Labels
    label_day_of_week = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_align(label_day_of_week, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 3, -46);
    lv_obj_set_style_local_text_color(label_day_of_week, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_label_set_text_static(label_day_of_week, "SUN");

    label_date = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 135, 132);
    lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_obj_set_style_local_text_font(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment40);
    lv_label_set_text_static(label_date, "6-30");
    
    label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(label_time_ampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_label_set_text_static(label_time_ampm, "");
    lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 10, -4);

    label_time = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment75);
    lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 30, 175);

    label_seconds = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(label_seconds, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_obj_set_style_local_text_font(label_seconds, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment50);
    lv_obj_align(label_seconds, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -55, -9);
    
    batteryIcon.Create(lv_scr_act());
    lv_obj_align(batteryIcon.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 14, 126);

    plugIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(plugIcon, Symbols::plug);
    lv_obj_set_style_local_text_color(plugIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_obj_align(plugIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 14, 125);

    bleIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_label_set_text_static(bleIcon, Symbols::bluetooth);
    lv_obj_align(bleIcon, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 34, 125);

    notificationIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
    lv_obj_align(notificationIcon, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 54, 126);

    stepIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_label_set_text_static(stepIcon, Symbols::shoe);
    lv_obj_align(stepIcon, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 73, 125);
    
    stepValue = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_obj_set_style_local_text_font(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment20);
    lv_label_set_text_static(stepValue, "0K");
    lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 103, 129);

    heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
    lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 75, 147);
    
    heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
    lv_obj_set_style_local_text_font(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment20);
    lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    lv_label_set_text_static(heartbeatValue, "");
    lv_obj_align(heartbeatValue, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 103, 152);
    taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
    Refresh();
}

WatchFaceCasioStyleAE21W::~WatchFaceCasioStyleAE21W() {
    lv_task_del(taskRefresh);

    lv_style_reset(&style_black_lines);
    lv_style_reset(&style_gray_lines);

    lv_style_reset(&hour_line_style);
    lv_style_reset(&minute_line_style);
    lv_style_reset(&minute_line_style_trace);
    lv_style_reset(&second_line_style);
    
    if (font_segment20 != nullptr) {
        lv_font_free(font_segment20);
    }

    if (font_segment40 != nullptr) {
        lv_font_free(font_segment40);
    }

    if (font_segment50 != nullptr) {
        lv_font_free(font_segment50);
    }

    if (font_segment75 != nullptr) {
        lv_font_free(font_segment75);
    }
    
    lv_obj_clean(lv_scr_act());
}

void WatchFaceCasioStyleAE21W::UpdateG1Clock(uint8_t hour, uint8_t minute) {
    auto const angle_min = minute * 6;
    minute_point[0] = CoordinateRelocateG1(12, angle_min);
    minute_point[1] = CoordinateRelocateG1(HourLength, angle_min);

    minute_point_trace[0] = CoordinateRelocateG1(HourLength + 3, angle_min);
    minute_point_trace[1] = CoordinateRelocateG1(MinuteLength, angle_min);

    lv_line_set_points(minute_body, minute_point, 2);
    lv_line_set_points(minute_body_trace, minute_point_trace, 2);

    auto const angle_hour = (hour * 30 + minute / 2);
    hour_point[0] = CoordinateRelocateG1(12, angle_hour);
    hour_point[1] = CoordinateRelocateG1(HourLength, angle_hour);
    
    lv_line_set_points(hour_body, hour_point, 2);
}

void WatchFaceCasioStyleAE21W::UpdateG2ClockSeconds(uint8_t second) {
    if (second >= 60) return;
    if(second > 0 && lv_obj_get_hidden(second_body[second - 1])) {
        // Initialisation of seconds graph
        for (int i = 0; i < second; i++) {
            lv_obj_set_hidden(second_body[i], false);
        }
    }
    
    // Clear seconds graph
    if (second == 0) {
        for (int i = 0; i < 60; i++) {
            lv_obj_set_hidden(second_body[i], true);
        }
    }
    lv_obj_set_hidden(second_body[second], false);
}


void WatchFaceCasioStyleAE21W::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
  batteryIcon.SetColor(color_text);
}

void WatchFaceCasioStyleAE21W::Refresh() {
    isCharging = batteryController.IsCharging();
    if (isCharging.IsUpdated()) {
        if (isCharging.Get()) {
            lv_obj_set_hidden(batteryIcon.GetObject(), true);
            lv_obj_set_hidden(plugIcon, false);
        } else {
            lv_obj_set_hidden(batteryIcon.GetObject(), false);
            lv_obj_set_hidden(plugIcon, true);
            SetBatteryIcon();
        }
    }
    if (!isCharging.Get()) {
        batteryPercentRemaining = batteryController.PercentRemaining();
        if (batteryPercentRemaining.IsUpdated()) {
            SetBatteryIcon();
        }
    }

    bleState = bleController.IsConnected();
    bleRadioEnabled = bleController.IsRadioEnabled();
    if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
        lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
    }
        
    notificationState = notificationManager.AreNewNotificationsAvailable();
    
    if (notificationState.IsUpdated()) {
        lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
    }

    // seconds
    currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());
    if (currentDateTime.IsUpdated()) {
        uint8_t second = dateTimeController.Seconds();
        lv_label_set_text_fmt(label_seconds, "%02d", second);
        UpdateG2ClockSeconds(second);

    }

    // minutes / hours
    currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
    if (currentDateTime.IsUpdated()) {
        uint8_t hour = dateTimeController.Hours();
        uint8_t minute = dateTimeController.Minutes();
        UpdateG1Clock(hour, minute);
        
        if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
            char ampmChar[3] = "AM";
            if (hour == 0) {
                hour = 12;
            } else if (hour == 12) {
                ampmChar[0] = 'P';
            } else if (hour > 12) {
                hour = hour - 12;
                ampmChar[0] = 'P';
            }
            lv_label_set_text(label_time_ampm, ampmChar);
            lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
        } else {
            lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
        }
    
        currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
        if (currentDate.IsUpdated()) {
            Controllers::DateTime::Months month = dateTimeController.Month();
            uint8_t day = dateTimeController.Day();
            if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
                // 24h mode: ddmmyyyy, first DOW=Monday;
                lv_label_set_text_fmt(label_date, "%2d-%2d", day, month);
            } else {
                // 12h mode: mmddyyyy, first DOW=Sunday;
                lv_label_set_text_fmt(label_date, "%2d-%2d", month, day);
            }

            lv_label_set_text_fmt(label_day_of_week, "%s", dateTimeController.DayOfWeekShortToString());
        }    
    }


    heartbeat = heartRateController.HeartRate();
    heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
    if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
        if (heartbeatRunning.Get()) {
            lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
            lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
        } else {
            lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
            lv_label_set_text_static(heartbeatValue, "OFF");
        }
    }

    stepCount = motionController.NbSteps();
    if (stepCount.IsUpdated()) {
        lv_label_set_text_fmt(stepValue, "%lu", (stepCount.Get() / 1000));
        lv_obj_realign(stepValue);
        lv_obj_realign(stepIcon);
    }
}

bool WatchFaceCasioStyleAE21W::IsAvailable(Pinetime::Controllers::FS& filesystem) {

    lfs_file file = {};

    if (filesystem.FileOpen(&file, "/fonts/7segments_20.bin", LFS_O_RDONLY) < 0) {
        return false;
    }
    
    filesystem.FileClose(&file);
    if (filesystem.FileOpen(&file, "/fonts/7segments_40.bin", LFS_O_RDONLY) < 0) {
        return false;
    }

    filesystem.FileClose(&file);
    if (filesystem.FileOpen(&file, "/fonts/7segments_50.bin", LFS_O_RDONLY) < 0) {
        return false;
    }

    filesystem.FileClose(&file);
    if (filesystem.FileOpen(&file, "/fonts/7segments_75.bin", LFS_O_RDONLY) < 0) {
        return false;
    }

    filesystem.FileClose(&file);
    return true;
}
