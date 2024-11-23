#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <displayapp/Controllers.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/BatteryIcon.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/ble/BleController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "utility/DirtyValue.h"
#include "components/battery/BatteryController.h"
#include "components/ble/NotificationManager.h"




namespace Pinetime {
    namespace Controllers {
        class Settings;
        class Battery;
        class Ble;
        class NotificationManager;
        class HeartRateController;
        class MotionController;
    }

    namespace Applications {
        namespace Screens {

            class WatchFaceCasioStyleAE21W : public Screen {
            public:
                WatchFaceCasioStyleAE21W(Controllers::DateTime& dateTimeController,
                                         const Controllers::Battery& batteryController,
                                         const Controllers::Ble& bleController,
                                         Controllers::NotificationManager& notificationManager,
                                         Controllers::Settings& settingsController,
                                         Controllers::HeartRateController& heartRateController,
                                         Controllers::MotionController& motionController,
                                         Controllers::FS& filesystem,
                                         Controllers::SimpleWeatherService& weather);
                ~WatchFaceCasioStyleAE21W() override;

                void Refresh() override;

                static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

            private:
                Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
                Utility::DirtyValue<bool> isCharging {};
                Utility::DirtyValue<bool> bleState {};
                Utility::DirtyValue<bool> bleRadioEnabled {};
                Utility::DirtyValue<uint32_t> stepCount {};
                Utility::DirtyValue<uint8_t> heartbeat {};
                Utility::DirtyValue<bool> heartbeatRunning {};
                Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime {};
                Utility::DirtyValue<bool> notificationState {};
                Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};
                Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;

                // colors
                lv_color_t color_bg = lv_color_hex(0x060606);
                lv_color_t color_lcd = lv_color_hex(0x000015);
                lv_color_t color_lcd_bg = lv_color_hex(0xd3d3c3);
                lv_color_t color_graph2_bg = lv_color_hex(0xADD8E6);

                // styles
                lv_style_t style_bg_lines;
                lv_style_t style_bg_lcd_lines;
                lv_style_t graph1_arrows_line_style;

                lv_obj_t* lv_obj;

                lv_point_t hour_point[2];
                lv_point_t minute_point[2];
                lv_point_t minute_point_trace[2];

                lv_obj_t* hour_body;
                lv_obj_t* minute_body;
                lv_obj_t* minute_body_trace;

                lv_obj_t* label_time;
                lv_obj_t* label_seconds;
                lv_obj_t* label_time_ampm;
                lv_obj_t* label_date;
                lv_obj_t* label_day_of_week;

                lv_obj_t* bleIcon;
                lv_obj_t* plugIcon;

                lv_obj_t* label_battery_value;
                lv_obj_t* heartbeatIcon;
                lv_obj_t* heartbeatValue;
                lv_obj_t* stepIcon;
                lv_obj_t* stepValue;
                lv_obj_t* notificationIcon;
                lv_obj_t* graph1MainDisc;
                lv_obj_t* graph2MainDisc;
                lv_obj_t* graph2SmallDisc;
                lv_obj_t* G2SecondMeter = nullptr;
                BatteryIcon batteryIcon;
                lv_obj_t* weatherIcon;
                lv_obj_t* temperature;


                Controllers::DateTime& dateTimeController;
                const Controllers::Battery& batteryController;
                const Controllers::Ble& bleController;
                Controllers::NotificationManager& notificationManager;
                Controllers::Settings& settingsController;
                Controllers::HeartRateController& heartRateController;
                Controllers::MotionController& motionController;
                Controllers::SimpleWeatherService& weatherService;

                void SetBatteryIcon();

                lv_task_t* taskRefresh;
                lv_font_t* font_segment20 = nullptr;
                lv_font_t* font_segment40 = nullptr;
                lv_font_t* font_segment50 = nullptr;
                lv_font_t* font_segment75 = nullptr;
            };
        }

        template <>
        struct WatchFaceTraits<WatchFace::CasioStyleAE21W> {
            static constexpr WatchFace watchFace = WatchFace::CasioStyleAE21W;
            static constexpr const char* name = "Casio AE21W";

            static Screens::Screen* Create(AppControllers& controllers) {
                return new Screens::WatchFaceCasioStyleAE21W(controllers.dateTimeController,
                                                             controllers.batteryController,
                                                             controllers.bleController,
                                                             controllers.notificationManager,
                                                             controllers.settingsController,
                                                             controllers.heartRateController,
                                                             controllers.motionController,
                                                             controllers.filesystem,
                                                             *controllers.weatherController);

            };

            static bool IsAvailable(Pinetime::Controllers::FS& filesystem) {
                return Screens::WatchFaceCasioStyleAE21W::IsAvailable(filesystem);
            }
        };
    }
}
