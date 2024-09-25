#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <displayapp/screens/BatteryIcon.h>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "utility/DirtyValue.h"

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
                                         Controllers::FS& filesystem);
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
                Utility::DirtyValue<bool> notificationState {false};
                Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;
                
                lv_color_t color_text = lv_color_hex(0x1A1A1A);
                lv_color_t color_gray = lv_color_hex(0xD3D3D3);
                lv_color_t color_disc2 = lv_color_hex(0xADD8E6);
                
                lv_obj_t* line_object;
                lv_style_t style_black_lines;
                lv_style_t style_gray_lines;
                
                lv_obj_t* lines[12];
                
                lv_point_t hour_point[2];
                lv_point_t minute_point[2];
                lv_point_t minute_point_trace[2];
                lv_point_t second_points[60][2];

                lv_obj_t* hour_body;
                lv_obj_t* minute_body;
                lv_obj_t* minute_body_trace;
                lv_obj_t* second_body[60];

                lv_style_t hour_line_style;
                lv_style_t minute_line_style;
                lv_style_t minute_line_style_trace;
                lv_style_t second_line_style;

                lv_obj_t* label_time;
                lv_obj_t* label_seconds;
                lv_obj_t* label_time_ampm;
                lv_obj_t* label_date;
                lv_obj_t* label_day_of_week;
                lv_obj_t* label_day_of_year;

                lv_obj_t* bleIcon;
                lv_obj_t* plugIcon;

                lv_obj_t* label_battery_value;
                lv_obj_t* heartbeatIcon;
                lv_obj_t* heartbeatValue;
                lv_obj_t* stepIcon;
                lv_obj_t* stepValue;
                lv_obj_t* notificationIcon;
                lv_obj_t* graphDisc;
                
                BatteryIcon batteryIcon;

                Controllers::DateTime& dateTimeController;
                const Controllers::Battery& batteryController;
                const Controllers::Ble& bleController;
                Controllers::NotificationManager& notificationManager;
                Controllers::Settings& settingsController;
                Controllers::HeartRateController& heartRateController;
                Controllers::MotionController& motionController;

                void UpdateG1Clock(uint8_t hour, uint8_t minute);
                void UpdateG2ClockSeconds(uint8_t second);
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
                                                             controllers.filesystem);
            };

            static bool IsAvailable(Pinetime::Controllers::FS& filesystem) {
                return Screens::WatchFaceCasioStyleAE21W::IsAvailable(filesystem);
            }
        };
    }
}
