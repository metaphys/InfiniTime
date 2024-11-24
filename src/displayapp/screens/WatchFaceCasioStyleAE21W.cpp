#include "displayapp/screens/WatchFaceCasioStyleAE21W.h"
#include <lvgl/lvgl.h>
#include <cstdio>
#include <cmath>
#include "displayapp/Colors.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/DisplayApp.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/heartrate/HeartRateController.h"

using namespace Pinetime::Applications::Screens;

// Background lines points
static constexpr lv_point_t linesPointsBG[2] = {
    {0,120},
     {240,120}
};

// Time background lines points
static constexpr lv_point_t linesPointsTBG[6][2] = {
    {{13,181},{227,181}},
    {{3,181},{237,181}},
    {{8,140},{19,130}},
    {{221,130},{232,140}},
    {{232,223},{221,233}},
    {{19,233},{8,223}}
};

// Time background lines widths
static constexpr lv_style_int_t linesWidthsTBG[7] = {
    113, 93, 15, 15, 15, 15
};

// Time table lines points
static constexpr lv_point_t linePointsTT[4][2] = {
    {{2, 169},{237, 169}},
    {{132,124},{132, 169}},
    {{68, 124},{68, 169}},
    {{2, 147},{132, 147}}
};

// Graph frames lines points
static constexpr lv_point_t linePointsGF[2][9] = {
    {{15, 2}, {101, 2}, {113, 15}, {113, 101}, {101, 113}, {15, 113}, {2, 101}, {2, 15}, {15, 2}},
    {{139, 2}, {225, 2}, {237, 15}, {237, 101}, {225, 113}, {139, 113}, {126, 101}, {126, 15}, {139, 2}}
};

// Graph1 grid lines points
static constexpr lv_point_t linePointsG1G[10][2] = {
    // vertical line
    {{0, -90}, {0, 90}},
    // horizontal lines
    {{-90, 0}, {90, 0}}
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

static constexpr int16_t HourLength = 30;
static constexpr int16_t MinuteLength = 42;

bool to_be_initialized = true;

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
    lv_point_t CoordinateRelocateG1(int16_t radius, int16_t angle) {
        return lv_point_t {.x = CoordinateXRelocateG1(radius * static_cast<int32_t>(Sine(angle)) / LV_TRIG_SCALE),
                           .y = CoordinateYRelocateG1(radius * static_cast<int32_t>(Cosine(angle)) / LV_TRIG_SCALE)};
    }
}

WatchFaceCasioStyleAE21W::WatchFaceCasioStyleAE21W(Controllers::DateTime& dateTimeController,
                                                   const Controllers::Battery& batteryController,
                                                   const Controllers::Ble& bleController,
                                                   Controllers::NotificationManager& notificationManager,
                                                   Controllers::Settings& settingsController,
                                                   Controllers::HeartRateController& heartRateController,
                                                   Controllers::MotionController& motionController,
                                                   Controllers::FS& filesystem,
                                                   Controllers::SimpleWeatherService& weatherService)
: currentDateTime {{}},
  batteryIcon(true),
  dateTimeController {dateTimeController},
  batteryController {batteryController},
  bleController {bleController},
  notificationManager {notificationManager},
  settingsController {settingsController},
  heartRateController {heartRateController},
  motionController {motionController},
  weatherService {weatherService} {


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
    lv_style_init(&style_bg_lcd_lines);
    lv_style_set_line_width(&style_bg_lcd_lines, LV_STATE_DEFAULT, 2);
    lv_style_set_line_color(&style_bg_lcd_lines, LV_STATE_DEFAULT, color_lcd_bg);
    lv_style_set_bg_color(&style_bg_lcd_lines, LV_STATE_DEFAULT, color_lcd_bg);
    lv_style_set_line_rounded(&style_bg_lcd_lines, LV_STATE_DEFAULT, false);

    lv_style_init(&style_bg_lines);
    lv_style_set_line_width(&style_bg_lines, LV_STATE_DEFAULT, 1);
    lv_style_set_line_color(&style_bg_lines, LV_STATE_DEFAULT, color_bg);
    lv_style_set_line_rounded(&style_bg_lines, LV_STATE_DEFAULT, false);

    lv_style_init(&graph1_arrows_line_style);
    lv_style_set_line_width(&graph1_arrows_line_style, LV_STATE_DEFAULT, 3);
    lv_style_set_line_color(&graph1_arrows_line_style, LV_STATE_DEFAULT, color_lcd);
    lv_style_set_line_rounded(&graph1_arrows_line_style, LV_STATE_DEFAULT, false);

    // Draw backgroud
    lv_obj = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_line_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 240);
    lv_obj_set_style_local_line_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_bg);
    lv_line_set_points(lv_obj, linesPointsBG, 2);

    // Draw time background
    for (int i = 0; i < 6; i++) {
        lv_obj = lv_line_create(lv_scr_act(), nullptr);
        lv_obj_set_style_local_line_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, linesWidthsTBG[i]);
        lv_obj_set_style_local_line_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd_bg);
        lv_line_set_points(lv_obj, linesPointsTBG[i], 2);
    }

    // Draw Background time table
    for (int i = 0; i < 4; i++) {
        lv_obj = lv_line_create(lv_scr_act(), nullptr);
        lv_line_set_points(lv_obj, linePointsTT[i], 2);
        lv_obj_add_style(lv_obj, LV_OBJ_PART_MAIN, &style_bg_lines);
    }

    // Draw graph1 main disc
    graph1MainDisc = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(graph1MainDisc, 85, 85);
    lv_obj_set_style_local_radius(graph1MainDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 43);
    lv_obj_set_style_local_bg_color(graph1MainDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd_bg);
    lv_obj_align(graph1MainDisc, nullptr, LV_ALIGN_IN_TOP_MID, -63, 14);

    // small graph1 disc
    lv_obj = lv_obj_create(graph1MainDisc, NULL);
    lv_obj_set_size(lv_obj, 15, 15);
    lv_obj_set_style_local_radius(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_bg_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_bg);
    lv_obj_align(lv_obj, graph1MainDisc, LV_ALIGN_CENTER, 0, 0);

    // Draw Graph1 frame
    lv_obj = lv_line_create(lv_scr_act(), nullptr);
    lv_line_set_points(lv_obj, linePointsGF[0], 9);
    lv_obj_add_style(lv_obj, LV_OBJ_PART_MAIN, &style_bg_lcd_lines);

    // Draw graph1 scales
    // minutes
    lv_obj = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lv_obj, 131, 131);
    lv_linemeter_set_scale(lv_obj, 336, 57);
    lv_linemeter_set_angle_offset(lv_obj, 180);
    lv_obj_set_style_local_bg_opa(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_scale_end_line_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_scale_end_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd_bg);
    lv_obj_align(lv_obj, graph1MainDisc, LV_ALIGN_CENTER, 1, 1);

    // 15 mins
    lv_obj = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lv_obj, 131, 131);
    lv_linemeter_set_scale(lv_obj, 180, 3);
    lv_linemeter_set_angle_offset(lv_obj, 180);
    lv_obj_set_style_local_bg_opa(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 7);
    lv_obj_set_style_local_scale_end_line_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_scale_end_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd_bg);
    lv_obj_align(lv_obj, graph1MainDisc, LV_ALIGN_CENTER, 1, 1);

    // hours
    lv_obj = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lv_obj, 131, 131);
    lv_linemeter_set_scale(lv_obj, 300, 11);
    lv_linemeter_set_angle_offset(lv_obj, 180);
    lv_obj_set_style_local_bg_opa(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 6);
    lv_obj_set_style_local_scale_end_line_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_scale_end_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd_bg);
    lv_obj_align(lv_obj, graph1MainDisc, LV_ALIGN_CENTER, 1, 1);

    // top arrows
    lv_obj = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lv_obj, 131, 131);
    lv_linemeter_set_scale(lv_obj, 6, 2);
    lv_linemeter_set_angle_offset(lv_obj, 0);
    lv_obj_set_style_local_bg_opa(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 9);
    lv_obj_set_style_local_scale_end_line_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_scale_end_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd_bg);
    lv_obj_align(lv_obj, graph1MainDisc, LV_ALIGN_CENTER, 1, 1);

    // Draw graph1 grid
    for (int i = 0; i < 5; i++) {
        // vertical lines
        lv_obj = lv_line_create(lv_scr_act(), nullptr);
        lv_obj_add_style(lv_obj, LV_OBJ_PART_MAIN, &style_bg_lines);
        lv_line_set_points(lv_obj, linePointsG1G[0], 2);
        lv_obj_align(lv_obj, graph1MainDisc, LV_ALIGN_CENTER, -30 + i * 15, 0);
        // horizontal lines
        lv_obj = lv_line_create(lv_scr_act(), nullptr);
        lv_obj_add_style(lv_obj, LV_OBJ_PART_MAIN, &style_bg_lines);
        lv_line_set_points(lv_obj, linePointsG1G[1], 2);
        lv_obj_align(lv_obj, graph1MainDisc, LV_ALIGN_CENTER, 0, -30 + i * 15);
    }


    // G1 clock
    minute_body = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_add_style(minute_body, LV_OBJ_PART_MAIN, &graph1_arrows_line_style);

    minute_body_trace = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_add_style(minute_body_trace, LV_OBJ_PART_MAIN, &graph1_arrows_line_style);

    hour_body = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_add_style(hour_body, LV_OBJ_PART_MAIN, &graph1_arrows_line_style);

    // Draw Graph2 frame
    lv_obj = lv_line_create(lv_scr_act(), nullptr);
    lv_line_set_points(lv_obj, linePointsGF[1], 9);
    lv_obj_add_style(lv_obj, LV_OBJ_PART_MAIN, &style_bg_lcd_lines);

    // Draw graph2 main disc
    graph2MainDisc = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(graph2MainDisc, 85, 85);
    lv_obj_set_style_local_radius(graph2MainDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 43);
    lv_obj_set_style_local_bg_color(graph2MainDisc, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_graph2_bg);
    lv_obj_align(graph2MainDisc, nullptr, LV_ALIGN_IN_TOP_MID, 61, 14);

    // Draw graph2 scales
    // hours
    lv_obj = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lv_obj, 131, 131);
    lv_linemeter_set_scale(lv_obj, 330, 12);
    lv_linemeter_set_angle_offset(lv_obj, 15);
    lv_obj_set_style_local_bg_opa(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 6);
    lv_obj_set_style_local_scale_end_line_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_scale_end_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd_bg);
    lv_obj_align(lv_obj, graph2MainDisc, LV_ALIGN_CENTER, 1, 1);

    // 15 minutes
    lv_obj = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lv_obj, 131, 131);
    lv_linemeter_set_scale(lv_obj, 180, 3);
    lv_linemeter_set_angle_offset(lv_obj, 180);
    lv_obj_set_style_local_bg_opa(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 7);
    lv_obj_set_style_local_scale_end_line_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_scale_end_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd_bg);
    lv_obj_align(lv_obj, graph2MainDisc, LV_ALIGN_CENTER, 1, 1);

    // minutes
    lv_obj = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(lv_obj, 131, 131);
    lv_linemeter_set_scale(lv_obj, 354, 60);
    lv_linemeter_set_angle_offset(lv_obj, 3);
    lv_obj_set_style_local_bg_opa(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_scale_end_line_width(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_scale_end_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd_bg);
    lv_obj_align(lv_obj, graph2MainDisc, LV_ALIGN_CENTER, 1, 1);

    G2SecondMeter = lv_linemeter_create(lv_scr_act(), nullptr);
    lv_obj_set_size(G2SecondMeter, 110, 110);
    lv_obj_set_style_local_bg_opa(G2SecondMeter, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_scale_width(G2SecondMeter, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 30);
    lv_obj_set_style_local_scale_end_line_width(G2SecondMeter, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_scale_end_border_width(G2SecondMeter, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_scale_end_color(G2SecondMeter, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_obj_align(G2SecondMeter, graph2MainDisc, LV_ALIGN_CENTER, 1, 1);

    // small graph2 disc
    lv_obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(lv_obj, 29, 29);
    lv_obj_set_style_local_radius(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 14);
    lv_obj_set_style_local_bg_color(lv_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_bg);
    lv_obj_align(lv_obj, graph2MainDisc, LV_ALIGN_CENTER, 0, 0);

    // Draw Graph2 SEC label
    for (int i = 0; i < 12; i++) {
        lv_obj = lv_line_create(lv_scr_act(), nullptr);
        lv_obj_align(lv_obj, nullptr, LV_ALIGN_IN_TOP_MID, 103, 53);
        lv_line_set_points(lv_obj, linePointsSEC[i], 2);
        lv_obj_add_style(lv_obj, LV_OBJ_PART_MAIN, &style_bg_lcd_lines);
    }

    // Icons and Labels
    label_day_of_week = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_align(label_day_of_week, nullptr, LV_ALIGN_IN_RIGHT_MID, 3, 61);
    lv_obj_set_style_local_text_color(label_day_of_week, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_label_set_text_static(label_day_of_week, "SUN");

    label_date = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 135, 132);
    lv_obj_set_style_local_text_color(label_date, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_obj_set_style_local_text_font(label_date, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font_segment40);
    lv_label_set_text_static(label_date, "6-30");

    label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(label_time_ampm, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_label_set_text_static(label_time_ampm, "");
    lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 10, -4);

    label_time = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(label_time, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_obj_set_style_local_text_font(label_time, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font_segment75);
    lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 33, 175);

    label_seconds = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(label_seconds, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_obj_set_style_local_text_font(label_seconds, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font_segment50);
    lv_obj_align(label_seconds, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -55, -9);

    batteryIcon.Create(lv_scr_act());
    lv_obj_align(batteryIcon.GetObject(), nullptr, LV_ALIGN_IN_LEFT_MID, 14, 16);

    plugIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(plugIcon, Symbols::plug);
    lv_obj_set_style_local_text_color(plugIcon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_obj_align(plugIcon, nullptr, LV_ALIGN_IN_LEFT_MID, 14, 16);

    bleIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(bleIcon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_label_set_text_static(bleIcon, Symbols::bluetooth);
    lv_obj_align(bleIcon, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 34, 16);

    notificationIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(notificationIcon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_obj_align(notificationIcon, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 54, 16);

    stepIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(stepIcon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_label_set_text_static(stepIcon, Symbols::shoe);
    lv_obj_align(stepIcon, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 73, 16);

    stepValue = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(stepValue, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_obj_set_style_local_text_font(stepValue, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font_segment20);
    lv_label_set_text_static(stepValue, "0K");
    lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 103, 16);

    heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(heartbeatIcon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
    lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 75, 38);

    heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(heartbeatValue, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_obj_set_style_local_text_font(heartbeatValue, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font_segment20);
    lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    lv_label_set_text_static(heartbeatValue, "");
    lv_obj_align(heartbeatValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 103, 38);

    weatherIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_obj_set_style_local_text_font(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_awesome17);
    lv_label_set_text(weatherIcon, Symbols::ban);
    lv_obj_align(weatherIcon, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 5, 70);
    lv_obj_set_auto_realign(weatherIcon, true);

    temperature = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_lcd);
    lv_label_set_text(temperature, "--");
    lv_obj_align(temperature, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 5, 95);

    taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
    Refresh();
}

WatchFaceCasioStyleAE21W::~WatchFaceCasioStyleAE21W() {
    lv_task_del(taskRefresh);

    lv_style_reset(&style_bg_lines);
    lv_style_reset(&style_bg_lcd_lines);
    lv_style_reset(&graph1_arrows_line_style);

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

void WatchFaceCasioStyleAE21W::SetBatteryIcon() {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);
    if (batteryPercent < 10) {
        batteryIcon.SetColor(LV_COLOR_RED);
    } else {
        batteryIcon.SetColor(color_lcd);
    }
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
        if (second == 1) {
            lv_linemeter_set_scale(G2SecondMeter, 0, 60);
            lv_linemeter_set_angle_offset(G2SecondMeter, 0);
        } else {
            lv_linemeter_set_scale(G2SecondMeter,(second - 1) * 6, second);
            lv_linemeter_set_angle_offset(G2SecondMeter, second * 3 - 1);
        }

        currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
        if (currentDateTime.IsUpdated() || second == 0) {
            uint8_t hour = dateTimeController.Hours();
            uint8_t minute = dateTimeController.Minutes();
            if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
                char ampmChar[2] = "A";
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
            // Update G1Clock
            auto const angle_min = minute * 6;
            minute_point[0] = CoordinateRelocateG1(10, angle_min);
            minute_point[1] = CoordinateRelocateG1(HourLength, angle_min);
            lv_line_set_points(minute_body, minute_point, 2);

            minute_point_trace[0] = CoordinateRelocateG1(HourLength + 2, angle_min);
            minute_point_trace[1] = CoordinateRelocateG1(MinuteLength, angle_min);
            lv_line_set_points(minute_body_trace, minute_point_trace, 2);

            auto const angle_hour = (hour * 30 + minute / 2);
            hour_point[0] = CoordinateRelocateG1(10, angle_hour);
            hour_point[1] = CoordinateRelocateG1(HourLength, angle_hour);
            lv_line_set_points(hour_body, hour_point, 2);
        }
    }

    currentDateTime = std::chrono::time_point_cast<std::chrono::days>(dateTimeController.CurrentDateTime());
    if (currentDateTime.IsUpdated()) {
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
            lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
        } else {
            lv_label_set_text_static(heartbeatValue, "OFF");
        }
    }

    stepCount = motionController.NbSteps();
    if (stepCount.IsUpdated()) {
        lv_label_set_text_fmt(stepValue, "%lu", (stepCount.Get() / 1000));
        lv_obj_realign(stepValue);
        lv_obj_realign(stepIcon);
    }

    currentWeather = weatherService.Current();
    if (currentWeather.IsUpdated()) {
        auto optCurrentWeather = currentWeather.Get();
        if (optCurrentWeather) {
            int16_t temp = optCurrentWeather->temperature.Celsius();
            if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
                temp = optCurrentWeather->temperature.Fahrenheit();
            }
            lv_label_set_text_fmt(temperature, "%d°", temp);
            lv_label_set_text(weatherIcon, Symbols::GetSymbol(optCurrentWeather->iconId));
        } else {
            lv_label_set_text(temperature, "--");
            lv_label_set_text(weatherIcon, Symbols::ban);
        }
        lv_obj_realign(temperature);
        lv_obj_realign(weatherIcon);
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
