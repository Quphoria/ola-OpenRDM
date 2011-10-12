/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * RDMENUMS.h
 * Provide a generic RDM ENUMS that can use different implementations.
 * Copyright (C) 2010 Simon Newton
 */

#ifndef INCLUDE_OLA_RDM_RDMENUMS_H_
#define INCLUDE_OLA_RDM_RDMENUMS_H_

#include <stdint.h>

namespace ola {
namespace rdm {

static const uint16_t ROOT_RDM_DEVICE = 0;
static const uint16_t ALL_RDM_SUBDEVICES = 0xffff;
static const uint16_t MAX_SUBDEVICE_NUMBER = 0x0200;
// This is separated out because we never propagate OVERFLOWs up to the
// clients.
static const uint8_t ACK_OVERFLOW = 3;


typedef enum {
  // discovery
  PID_DISC_UNIQUE_BRANCH = 0x0001,
  PID_DISC_MUTE = 0x0002,
  PID_DISC_UN_MUTE = 0x0003,
  // network managment
  PID_PROXIED_DEVICES = 0x0010,
  PID_PROXIED_DEVICE_COUNT = 0x0011,
  PID_COMMS_STATUS = 0x0015,
  // status collection
  PID_QUEUED_MESSAGE = 0x0020,
  PID_STATUS_MESSAGES = 0x0030,
  PID_STATUS_ID_DESCRIPTION = 0x0031,
  PID_CLEAR_STATUS_ID = 0x0032,
  PID_SUB_DEVICE_STATUS_REPORT_THRESHOLD = 0x0033,
  // RDM information
  PID_SUPPORTED_PARAMETERS = 0x0050,
  PID_PARAMETER_DESCRIPTION = 0x0051,
  // production information
  PID_DEVICE_INFO = 0x0060,
  PID_PRODUCT_DETAIL_ID_LIST = 0x0070,
  PID_DEVICE_MODEL_DESCRIPTION = 0x0080,
  PID_MANUFACTURER_LABEL = 0x0081,
  PID_DEVICE_LABEL = 0x0082,
  PID_FACTORY_DEFAULTS = 0x0090,
  PID_LANGUAGE_CAPABILITIES = 0x00A0,
  PID_LANGUAGE = 0x00B0,
  PID_SOFTWARE_VERSION_LABEL = 0x00C0,
  PID_BOOT_SOFTWARE_VERSION_ID = 0x00C1,
  PID_BOOT_SOFTWARE_VERSION_LABEL = 0x00C2,
  // dmx512
  PID_DMX_PERSONALITY = 0x00E0,
  PID_DMX_PERSONALITY_DESCRIPTION = 0x00E1,
  PID_DMX_START_ADDRESS = 0x00F0,
  PID_SLOT_INFO = 0x0120,
  PID_SLOT_DESCRIPTION = 0x0121,
  PID_DEFAULT_SLOT_VALUE = 0x0122,
  // sensors
  PID_SENSOR_DEFINITION = 0x0200,
  PID_SENSOR_VALUE = 0x0201,
  PID_RECORD_SENSORS = 0x0202,
  // power/lamp settings
  PID_DEVICE_HOURS = 0x0400,
  PID_LAMP_HOURS = 0x0401,
  PID_LAMP_STRIKES = 0x0402,
  PID_LAMP_STATE =  0x0403,
  PID_LAMP_ON_MODE = 0x0404,
  PID_DEVICE_POWER_CYCLES = 0x0405,
  // display settings
  PID_DISPLAY_INVERT = 0x0500,
  PID_DISPLAY_LEVEL = 0x0501,
  // configuration
  PID_PAN_INVERT = 0x0600,
  PID_TILT_INVERT = 0x0601,
  PID_PAN_TILT_SWAP = 0x0602,
  PID_REAL_TIME_CLOCK = 0x0603,
  // control
  PID_IDENTIFY_DEVICE = 0x1000,
  PID_RESET_DEVICE = 0x1001,
  PID_POWER_STATE = 0x1010,
  PID_PERFORM_SELFTEST = 0x1020,
  PID_SELF_TEST_DESCRIPTION = 0x1021,
  PID_CAPTURE_PRESET = 0x1030,
  PID_PRESET_PLAYBACK = 0x1031,
} rdm_pid;


typedef enum {
  STATUS_NONE = 0x0,
  STATUS_GET_LAST_MESSAGE = 0x1,
  STATUS_ADVISORY = 0x2,
  STATUS_WARNING = 0x3,
  STATUS_ERROR = 0x4,
  STATUS_ADVISORY_CLEARED = 0x12,
  STATUS_WARNING_CLEARED = 0x13,
  STATUS_ERROR_CLEARED = 0x14,
} rdm_status_type;


typedef enum {
  SENSOR_TEMPERATURE = 0x00,
  SENSOR_VOLTAGE = 0x01,
  SENSOR_CURRENT = 0x02,
  SENSOR_FREQUENCY = 0x03,
  SENSOR_RESISTANCE = 0x04,
  SENSOR_POWER = 0x05,
  SENSOR_MASS = 0x06,
  SENSOR_LENGTH = 0x07,
  SENSOR_AREA = 0x08,
  SENSOR_VOLUME = 0x09,
  SENSOR_DENSITY = 0x0A,
  SENSOR_VELOCITY = 0x0B,
  SENSOR_ACCELERATION = 0x0C,
  SENSOR_FORCE = 0x0D,
  SENSOR_ENERGY = 0x0E,
  SENSOR_PRESSURE = 0x0F,
  SENSOR_TIME = 0x10,
  SENSOR_ANGLE = 0x11,
  SENSOR_POSITION_X = 0x12,
  SENSOR_POSITION_Y = 0x13,
  SENSOR_POSITION_Z = 0x14,
  SENSOR_ANGULAR_VELOCITY = 0x15,
  SENSOR_LUMINOUS_INTENSITY = 0x16,
  SENSOR_LUMINOUS_FLUX = 0x17,
  SENSOR_ILLUMINANCE = 0x18,
  SENSOR_CHROMINANCE_RED = 0x19,
  SENSOR_CHROMINANCE_GREEN = 0x1A,
  SENSOR_CHROMINANCE_BLUE = 0x1B,
  SENSOR_CONTACTS = 0x1C,
  SENSOR_MEMORY = 0x1D,
  SENSOR_ITEMS = 0x1E,
  SENSOR_HUMIDITY = 0x1F,
  SENSOR_COUNTER_16BIT = 0x20,
  SENSOR_OTHER = 0x7F,
} rdm_sensor_type;

typedef enum {
  UNITS_NONE = 0x00,
  UNITS_CENTIGRADE = 0x01,
  UNITS_VOLTS_DC = 0x02,
  UNITS_VOLTS_AC_PEAK = 0x03,
  UNITS_VOLTS_AC_RMS = 0x04,
  UNITS_AMPERE_DC = 0x05,
  UNITS_AMPERE_AC_PEAK = 0x06,
  UNITS_AMPERE_AC_RMS = 0x07,
  UNITS_HERTZ = 0x08,
  UNITS_OHM = 0x09,
  UNITS_WATT = 0x0A,
  UNITS_KILOGRAM = 0x0B,
  UNITS_METERS = 0x0C,
  UNITS_METERS_SQUARED = 0x0D,
  UNITS_METERS_CUBED = 0x0E,
  UNITS_KILOGRAMMES_PER_METER_CUBED = 0x0F,
  UNITS_METERS_PER_SECOND = 0x10,
  UNITS_METERS_PER_SECOND_SQUARED = 0x11,
  UNITS_NEWTON = 0x12,
  UNITS_JOULE = 0x13,
  UNITS_PASCAL = 0x14,
  UNITS_SECOND = 0x15,
  UNITS_DEGREE = 0x16,
  UNITS_STERADIAN = 0x17,
  UNITS_CANDELA = 0x18,
  UNITS_LUMEN = 0x19,
  UNITS_LUX = 0x1A,
  UNITS_IRE = 0x1B,
  UNITS_BYTE = 0x1C,
} rdm_pid_unit;


typedef enum {
  PREFIX_NONE = 0x00,
  PREFIX_DECI = 0x01,
  PREFIX_CENTI = 0x02,
  PREFIX_MILLI = 0x03,
  PREFIX_MICRO = 0x04,
  PREFIX_NANO = 0x05,
  PREFIX_PICO = 0x06,
  PREFIX_FEMPTO = 0x07,
  PREFIX_ATTO = 0x08,
  PREFIX_ZEPTO = 0x09,
  PREFIX_YOCTO = 0x0A,
  PREFIX_DECA = 0x11,
  PREFIX_HECTO = 0x12,
  PREFIX_KILO = 0x13,
  PREFIX_MEGA = 0x14,
  PREFIX_GIGA = 0x15,
  PREFIX_TERRA = 0x16,
  PREFIX_PETA = 0x17,
  PREFIX_EXA = 0x18,
  PREFIX_ZETTA = 0x19,
  PREFIX_YOTTA = 0x1A,
} rdm_pid_prefix;


typedef enum {
  CC_GET = 0x01,
  CC_SET = 0x02,
  CC_GET_SET = 0x03,
} rdm_command_class;


typedef enum {
  DS_NOT_DEFINED = 0x0,
  DS_BIT_FIELD = 0x01,
  DS_ASCII = 0x02,
  DS_UNSIGNED_BYTE = 0x03,
  DS_SIGNED_BYTE = 0x04,
  DS_UNSIGNED_WORD = 0x05,
  DS_SIGNED_WORD = 0x06,
  DS_UNSIGNED_DWORD = 0x07,
  DS_SIGNED_DWORD = 0x08,
} rdm_data_type;


typedef enum {
  NR_UNKNOWN_PID = 0x0000,
  NR_FORMAT_ERROR = 0x0001,
  NR_HARDWARE_FAULT = 0x0002,
  NR_PROXY_REJECT = 0x0003,
  NR_WRITE_PROTECT = 0x0004,
  NR_UNSUPPORTED_COMMAND_CLASS = 0x0005,
  NR_DATA_OUT_OF_RANGE = 0x0006,
  NR_BUFFER_FULL = 0x0007,
  NR_PACKET_SIZE_UNSUPPORTED = 0x0008,
  NR_SUB_DEVICE_OUT_OF_RANGE = 0x0009,
  NR_PROXY_BUFFER_FULL = 0x000A,
  NR_PROXY_BUSY = 0x000B,
  NR_PROXY_CACHE_EXPIRED = 0x000C,
  NR_PORT_NUMBER_INVALID = 0x000D,
  NR_UNIVERSE_TYPE_NOT_SUPPORTED = 0x000E,
} rdm_nack_reason;


typedef enum {
  PRODUCT_CATEGORY_NOT_DECLARED = 0x0000,
  PRODUCT_CATEGORY_FIXTURE = 0x0100,
  PRODUCT_CATEGORY_FIXTURE_FIXED = 0x0101,
  PRODUCT_CATEGORY_FIXTURE_MOVING_YOKE = 0x0102,
  PRODUCT_CATEGORY_FIXTURE_MOVING_MIRROR = 0x0103,
  PRODUCT_CATEGORY_FIXTURE_OTHER = 0x01FF,
  PRODUCT_CATEGORY_FIXTURE_ACCESSORY = 0x0200,
  PRODUCT_CATEGORY_FIXTURE_ACCESSORY_COLOR = 0x0201,
  PRODUCT_CATEGORY_FIXTURE_ACCESSORY_YOKE = 0x0202,
  PRODUCT_CATEGORY_FIXTURE_ACCESSORY_MIRROR = 0x0203,
  PRODUCT_CATEGORY_FIXTURE_ACCESSORY_EFFECT = 0x0204,
  PRODUCT_CATEGORY_FIXTURE_ACCESSORY_BEAM = 0x0205,
  PRODUCT_CATEGORY_FIXTURE_ACCESSORY_OTHER = 0x02FF,
  PRODUCT_CATEGORY_PROJECTOR = 0x0300,
  PRODUCT_CATEGORY_PROJECTOR_FIXED = 0x0301,
  PRODUCT_CATEGORY_PROJECTOR_MOVING_YOKE = 0x0302,
  PRODUCT_CATEGORY_PROJECTOR_MOVING_MIRROR = 0x0303,
  PRODUCT_CATEGORY_PROJECTOR_OTHER = 0x03FF,
  PRODUCT_CATEGORY_ATMOSPHERIC = 0x0400,
  PRODUCT_CATEGORY_ATMOSPHERIC_EFFECT = 0x0401,
  PRODUCT_CATEGORY_ATMOSPHERIC_PYRO = 0x0402,
  PRODUCT_CATEGORY_ATMOSPHERIC_OTHER = 0x04FF,
  PRODUCT_CATEGORY_DIMMER = 0x0500,
  PRODUCT_CATEGORY_DIMMER_AC_INCANDESCENT = 0x0501,
  PRODUCT_CATEGORY_DIMMER_AC_FLUORESCENT = 0x0502,
  PRODUCT_CATEGORY_DIMMER_AC_COLDCATHODE = 0x0503,
  PRODUCT_CATEGORY_DIMMER_AC_NONDIM = 0x0504,
  PRODUCT_CATEGORY_DIMMER_AC_ELV = 0x0505,
  PRODUCT_CATEGORY_DIMMER_AC_OTHER = 0x0506,
  PRODUCT_CATEGORY_DIMMER_DC_LEVEL = 0x0507,
  PRODUCT_CATEGORY_DIMMER_DC_PWM = 0x0508,
  PRODUCT_CATEGORY_DIMMER_CS_LED = 0x0509,
  PRODUCT_CATEGORY_DIMMER_OTHER = 0x05FF,
  PRODUCT_CATEGORY_POWER = 0x0600,
  PRODUCT_CATEGORY_POWER_CONTROL = 0x0601,
  PRODUCT_CATEGORY_POWER_SOURCE = 0x0602,
  PRODUCT_CATEGORY_POWER_OTHER = 0x06FF,
  PRODUCT_CATEGORY_SCENIC = 0x0700,
  PRODUCT_CATEGORY_SCENIC_DRIVE = 0x0701,
  PRODUCT_CATEGORY_SCENIC_OTHER = 0x07FF,
  PRODUCT_CATEGORY_DATA = 0x0800,
  PRODUCT_CATEGORY_DATA_DISTRIBUTION = 0x0801,
  PRODUCT_CATEGORY_DATA_CONVERSION = 0x0802,
  PRODUCT_CATEGORY_DATA_OTHER = 0x08FF,
  PRODUCT_CATEGORY_AV = 0x0900,
  PRODUCT_CATEGORY_AV_AUDIO = 0x0901,
  PRODUCT_CATEGORY_AV_VIDEO = 0x0902,
  PRODUCT_CATEGORY_AV_OTHER = 0x09FF,
  PRODUCT_CATEGORY_MONITOR = 0x0A00,
  PRODUCT_CATEGORY_MONITOR_ACLINEPOWER = 0x0A01,
  PRODUCT_CATEGORY_MONITOR_DCPOWER = 0x0A02,
  PRODUCT_CATEGORY_MONITOR_ENVIRONMENTAL = 0x0A03,
  PRODUCT_CATEGORY_MONITOR_OTHER = 0x0AFF,
  PRODUCT_CATEGORY_CONTROL = 0x7000,
  PRODUCT_CATEGORY_CONTROL_CONTROLLER = 0x7001,
  PRODUCT_CATEGORY_CONTROL_BACKUPDEVICE = 0x7002,
  PRODUCT_CATEGORY_CONTROL_OTHER = 0x70FF,
  PRODUCT_CATEGORY_TEST = 0x7100,
  PRODUCT_CATEGORY_TEST_EQUIPMENT = 0x7101,
  PRODUCT_CATEGORY_TEST_EQUIPMENT_OTHER = 0x71FF,
  PRODUCT_CATEGORY_OTHER = 0x7FFF,
} rdm_product_category;


// product details
typedef enum {
  PRODUCT_DETAIL_NOT_DECLARED = 0x0000,
  PRODUCT_DETAIL_ARC = 0x0001,
  PRODUCT_DETAIL_METAL_HALIDE = 0x0002,
  PRODUCT_DETAIL_INCANDESCENT = 0x0003,
  PRODUCT_DETAIL_LED = 0x0004,
  PRODUCT_DETAIL_FLUROESCENT = 0x0005,
  PRODUCT_DETAIL_COLDCATHODE = 0x0006,
  PRODUCT_DETAIL_ELECTROLUMINESCENT = 0x0007,
  PRODUCT_DETAIL_LASER = 0x0008,
  PRODUCT_DETAIL_FLASHTUBE = 0x0009,
  PRODUCT_DETAIL_COLORSCROLLER = 0x0100,
  PRODUCT_DETAIL_COLORWHEEL = 0x0101,
  PRODUCT_DETAIL_COLORCHANGE = 0x0102,
  PRODUCT_DETAIL_IRIS_DOUSER = 0x0103,
  PRODUCT_DETAIL_DIMMING_SHUTTER = 0x0104,
  PRODUCT_DETAIL_PROFILE_SHUTTER = 0x0105,
  PRODUCT_DETAIL_BARNDOOR_SHUTTER = 0x0106,
  PRODUCT_DETAIL_EFFECTS_DISC = 0x0107,
  PRODUCT_DETAIL_GOBO_ROTATOR = 0x0108,
  PRODUCT_DETAIL_VIDEO = 0x0200,
  PRODUCT_DETAIL_SLIDE = 0x0201,
  PRODUCT_DETAIL_FILM = 0x0202,
  PRODUCT_DETAIL_OILWHEEL = 0x0203,
  PRODUCT_DETAIL_LCDGATE = 0x0204,
  PRODUCT_DETAIL_FOGGER_GLYCOL = 0x0300,
  PRODUCT_DETAIL_FOGGER_MINERALOIL = 0x0301,
  PRODUCT_DETAIL_FOGGER_WATER = 0x0302,
  PRODUCT_DETAIL_CO2 = 0x0303,
  PRODUCT_DETAIL_LN2 = 0x0304,
  PRODUCT_DETAIL_BUBBLE = 0x0305,
  PRODUCT_DETAIL_FLAME_PROPANE = 0x0306,
  PRODUCT_DETAIL_FLAME_OTHER = 0x0307,
  PRODUCT_DETAIL_OLEFACTORY_STIMULATOR = 0x0308,
  PRODUCT_DETAIL_SNOW = 0x0309,
  PRODUCT_DETAIL_WATER_JET = 0x030A,
  PRODUCT_DETAIL_WIND = 0x030B,
  PRODUCT_DETAIL_CONFETTI = 0x030C,
  PRODUCT_DETAIL_HAZARD = 0x030D,
  PRODUCT_DETAIL_PHASE_CONTROL = 0x0400,
  PRODUCT_DETAIL_REVERSE_PHASE_CONTROL = 0x0401,
  PRODUCT_DETAIL_SINE = 0x0402,
  PRODUCT_DETAIL_PWM = 0x0403,
  PRODUCT_DETAIL_DC = 0x0404,
  PRODUCT_DETAIL_HFBALLAST = 0x0405,
  PRODUCT_DETAIL_HFHV_NEONBALLAST = 0x0406,
  PRODUCT_DETAIL_HFHV_EL = 0x0407,
  PRODUCT_DETAIL_MHR_BALLAST = 0x0408,
  PRODUCT_DETAIL_BITANGLE_MODULATION = 0x0409,
  PRODUCT_DETAIL_FREQUENCY_MODULATION = 0x040A,
  PRODUCT_DETAIL_HIGHFREQUENCY_12V = 0x040B,
  PRODUCT_DETAIL_RELAY_MECHANICAL = 0x040C,
  PRODUCT_DETAIL_RELAY_ELECTRONIC = 0x040D,
  PRODUCT_DETAIL_SWITCH_ELECTRONIC = 0x040E,
  PRODUCT_DETAIL_CONTACTOR = 0x040F,
  PRODUCT_DETAIL_MIRRORBALL_ROTATOR = 0x0500,
  PRODUCT_DETAIL_OTHER_ROTATOR = 0x0501,
  PRODUCT_DETAIL_KABUKI_DROP = 0x0502,
  PRODUCT_DETAIL_CURTAIN = 0x0503,
  PRODUCT_DETAIL_LINESET = 0x0504,
  PRODUCT_DETAIL_MOTOR_CONTROL = 0x0505,
  PRODUCT_DETAIL_DAMPER_CONTROL = 0x0506,
  PRODUCT_DETAIL_SPLITTER = 0x0600,
  PRODUCT_DETAIL_ETHERNET_NODE = 0x0601,
  PRODUCT_DETAIL_MERGE = 0x0602,
  PRODUCT_DETAIL_DATAPATCH = 0x0603,
  PRODUCT_DETAIL_WIRELESS_LINK = 0x0604,
  PRODUCT_DETAIL_PROTOCOL_CONVERTOR = 0x0701,
  PRODUCT_DETAIL_ANALOG_DEMULTIPLEX = 0x0702,
  PRODUCT_DETAIL_ANALOG_MULTIPLEX = 0x0703,
  PRODUCT_DETAIL_SWITCH_PANEL = 0x0704,
  PRODUCT_DETAIL_ROUTER = 0x0800,
  PRODUCT_DETAIL_FADER = 0x0801,
  PRODUCT_DETAIL_MIXER = 0x0802,
  PRODUCT_DETAIL_CHANGEOVER_MANUAL = 0x0900,
  PRODUCT_DETAIL_CHANGEOVER_AUTO = 0x0901,
  PRODUCT_DETAIL_TEST = 0x0902,
  PRODUCT_DETAIL_GFI_RCD = 0x0A00,
  PRODUCT_DETAIL_BATTERY = 0x0A01,
  PRODUCT_DETAIL_CONTROLLABLE_BREAKER = 0x0A02,
  PRODUCT_DETAIL_OTHER = 0x7FFF,
} rdm_product_detail;


// slot types
typedef enum {
  ST_PRIMARY = 0x00,
  ST_SEC_FINE = 0x01,
  ST_SEC_TIMING = 0x02,
  ST_SEC_SPEED = 0x03,
  ST_SEC_CONTROL = 0x04,
  ST_SEC_INDEX = 0x05,
  ST_SEC_ROTATION = 0x06,
  ST_SEC_INDEX_ROTATE = 0x07,
  ST_SEC_UNDEFINED = 0xFF,
} rdm_slot_type;


// slot definitions
typedef enum {
  SD_INTENSITY = 0x0001,
  SD_INTENSITY_MASTER = 0x0002,
  SD_PAN = 0x0101,
  SD_TILT = 0x0102,
  SD_COLOR_WHEEL = 0x0201,
  SD_COLOR_SUB_CYAN = 0x0202,
  SD_COLOR_SUB_YELLOW = 0x0203,
  SD_COLOR_SUB_MAGENTA = 0x0204,
  SD_COLOR_ADD_RED = 0x0205,
  SD_COLOR_ADD_GREEN = 0x0206,
  SD_COLOR_ADD_BLUE = 0x0207,
  SD_COLOR_CORRECTION = 0x0208,
  SD_COLOR_SCROLL = 0x0209,
  SD_COLOR_SEMAPHORE = 0x0210,
  SD_STATIC_GOBO_WHEEL = 0x0301,
  SD_ROTO_GOBO_WHEEL = 0x0302,
  SD_PRISM_WHEEL = 0x0303,
  SD_EFFECTS_WHEEL = 0x0304,
  SD_BEAM_SIZE_IRIS = 0x0401,
  SD_EDGE = 0x0402,
  SD_FROST = 0x0403,
  SD_STROBE = 0x0404,
  SD_ZOOM = 0x0405,
  SD_FRAMING_SHUTTER = 0x0406,
  SD_SHUTTER_ROTATE = 0x0407,
  SD_DOUSER = 0x0408,
  SD_BARN_DOOR = 0x0409,
  SD_LAMP_CONTROL = 0x0501,
  SD_FIXTURE_CONTROL = 0x0502,
  SD_FIXTURE_SPEED = 0x0503,
  SD_MACRO = 0x0504,
  SD_UNDEFINED = 0xFFFF,
} rdm_slot_definition;


typedef enum {
  STS_CAL_FAIL = 0x0001,
  STS_SENS_NOT_FOUND = 0x0002,
  STS_SENS_ALWAYS_ON = 0x0003,
  STS_LAMP_DOUSED = 0x0011,
  STS_LAMP_STRIKE = 0x0012,
  STS_OVERTEMP = 0x0021,
  STS_UNDERTEMP = 0x0022,
  STS_SENS_OUT_RANGE = 0x0023,
  STS_OVERVOLTAGE_PHASE = 0x0031,
  STS_UNDERVOLTAGE_PHASE = 0x0032,
  STS_OVERCURRENT = 0x0033,
  STS_UNDERCURRENT = 0x0034,
  STS_PHASE = 0x0035,
  STS_PHASE_ERROR = 0x0036,
  STS_AMPS = 0x0037,
  STS_VOLTS = 0x0038,
  STS_DIMSLOT_OCCUPIED = 0x0041,
  STS_BREAKER_TRIP = 0x0042,
  STS_WATTS = 0x0043,
  STS_DIM_FAILURE = 0x0044,
  STS_DIM_PANIC = 0x0045,
  STS_READY = 0x0050,
  STS_NOT_READY = 0x0051,
  STS_LOW_FLUID = 0x0052,
} rdm_status_message_id;


typedef enum {
  LAMP_OFF = 0x00,
  LAMP_ON = 0x01,
  LAMP_STRIKE = 0x02,
  LAMP_STANDBY = 0x03,
  LAMP_NOT_PRESENT = 0x04,
  LAMP_ERROR = 0x7F,
} rdm_lamp_state;

typedef enum {
  LAMP_ON_MODE_OFF = 0x00,
  LAMP_ON_MODE_DMX = 0x01,
  LAMP_ON_MODE_ON = 0x02,
  LAMP_ON_MODE_AFTER_CAL = 0x03,
} rdm_lamp_mode;


typedef enum {
  POWER_STATE_FULL_OFF = 0x00,
  POWER_STATE_SHUTDOWN = 0x01,
  POWER_STATE_STANDBY = 0x02,
  POWER_STATE_NORMAL = 0xFF,
} rdm_power_state;

typedef enum {
  DISPLAY_INVERT_OFF = 0x00,
  DISPLAY_INVERT_ON = 0x01,
  DISPLAY_INVERT_AUTO = 0x02,
} rdm_display_invert;

// the two special presets
static const uint16_t PRESET_PLAYBACK_OFF = 0x0000;
static const uint16_t PRESET_PLAYBACK_ALL = 0xffff;

// bit masks for sensor values
static const uint8_t SENSOR_RECORDED_VALUE = 0x01;
static const uint8_t SENSOR_RECORDED_RANGE_VALUES = 0x02;
}  // rdm
}  // ola
#endif  // INCLUDE_OLA_RDM_RDMENUMS_H_
