#pragma once

#include "NGS/NGS.h"
#include "ESP32/Defined.h"
#include "ESP32/Config.h"
#include "ESP32/Register.h"

OPT_BEGIN

constexpr ngs::pin_t
SLAVE_ADDRESS = EOD_ADDRESS,
SLAVE_SDA = 4,
SLAVE_SCLK = 5,

LED_TEST = 6,

MOTOR_L_GPIO0 = 35,
MOTOR_L_GPIO1 = 36,
MOTOR_L_IMPULSE0 = 37,
MOTOR_L_IMPULSE1 = 38,
MOTOR_L_PWM = 39,

MOTOR_R_GPIO0 = 9,
MOTOR_R_GPIO1 = 10,
MOTOR_R_IMPULSE0 = 11,
MOTOR_R_IMPULSE1 = 12,
MOTOR_R_PWM = 13,

GYRO_SDA = 15,
GYRO_SCLK = 7
;

constexpr std::array<ngs::pin_t, 8> GRAY = { 21,47,48,45,16,17,18,8 };

constexpr ngs::float32
WHEEL_CIRCUMFERENCE = 2 * 0.04 * std::numbers::pi,
PLUSE_COUNT_PER_RAD = 330,

WHEEL_SPEED_MAX_3_3 = 0.02f,
WHEEL_SPEED_MAX_5_0 = 0.048f,
WHEEL_SPEED_MAX_12_0 = 0.10f
;

constexpr ngs::Point3f PID = { 0.1,0.01,0 };

OPT_END
