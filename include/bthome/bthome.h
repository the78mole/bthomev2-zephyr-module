/*
 * Copyright (c) 2026
 * SPDX-License-Identifier: Apache-2.0
 *
 * BTHome V2 library for Zephyr
 *
 * Specification: https://bthome.io/format/
 */

#ifndef BTHOME_BTHOME_H_
#define BTHOME_BTHOME_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/bluetooth/bluetooth.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * BTHome V2 protocol constants
 * -------------------------------------------------------------------------*/

/** BTHome V2 service UUID (16-bit, little-endian in payload: 0xD2 0xFC) */
#define BTHOME_SERVICE_UUID      0xFCD2U
#define BTHOME_UUID_LE_B0        0xD2U
#define BTHOME_UUID_LE_B1        0xFCU

/** Device information byte flags */
#define BTHOME_DEV_INFO_ENCRYPT  0x01U /**< Bit 0: encrypted payload */
#define BTHOME_DEV_INFO_TRIGGER  0x04U /**< Bit 2: trigger-based device */
#define BTHOME_DEV_INFO_VERSION  0x40U /**< Bits 6-7: BTHome version = 2 */

/**
 * Maximum service data that fits in one non-extended advertising packet:
 *   31 (ADV total) - 3 (flags AD) - 2 (SvcData AD header) = 26 bytes.
 */
#define BTHOME_SVC_DATA_MAX_LEN  26U

/** Maximum measurements that can be queued before encoding. */
#define BTHOME_MAX_MEASUREMENTS  12U

/**
 * Maximum value bytes per fixed-width measurement (covers uint32/sint32).
 * Variable-length objects (text/raw) are limited to this size too; include
 * the 1-byte length prefix as the first data byte.
 */
#define BTHOME_MAX_VALUE_LEN     4U

/* ---------------------------------------------------------------------------
 * Object IDs — miscellaneous / packet control
 * -------------------------------------------------------------------------*/
/** Packet counter (uint8); use to filter duplicate advertisements. */
#define BTHOME_OBJ_PACKET_ID         0x00U

/* ---------------------------------------------------------------------------
 * Object IDs — numeric sensors (0x01 … 0x2F, then 0x3D … 0x65)
 * -------------------------------------------------------------------------*/
/** Battery level, uint8, unit: % */
#define BTHOME_OBJ_BATTERY           0x01U
/** Temperature, sint16, factor 0.01, unit: °C */
#define BTHOME_OBJ_TEMPERATURE       0x02U
/** Humidity, uint16, factor 0.01, unit: % */
#define BTHOME_OBJ_HUMIDITY          0x03U
/** Pressure, uint24, factor 0.01, unit: hPa */
#define BTHOME_OBJ_PRESSURE          0x04U
/** Illuminance, uint24, factor 0.01, unit: lx */
#define BTHOME_OBJ_ILLUMINANCE       0x05U
/** Mass (kg), uint16, factor 0.01, unit: kg */
#define BTHOME_OBJ_MASS_KG           0x06U
/** Mass (lb), uint16, factor 0.01, unit: lb */
#define BTHOME_OBJ_MASS_LB           0x07U
/** Dew point, sint16, factor 0.01, unit: °C */
#define BTHOME_OBJ_DEW_POINT         0x08U
/** Count, uint8 */
#define BTHOME_OBJ_COUNT_UI8         0x09U
/** Energy, uint24, factor 0.001, unit: kWh */
#define BTHOME_OBJ_ENERGY            0x0AU
/** Power, uint24, factor 0.01, unit: W */
#define BTHOME_OBJ_POWER             0x0BU
/** Voltage, uint16, factor 0.001, unit: V */
#define BTHOME_OBJ_VOLTAGE           0x0CU
/** PM2.5 concentration, uint16, unit: µg/m³ */
#define BTHOME_OBJ_PM2_5             0x0DU
/** PM10 concentration, uint16, unit: µg/m³ */
#define BTHOME_OBJ_PM10              0x0EU
/** Generic boolean, uint8, 0/1 */
#define BTHOME_OBJ_GENERIC_BOOL      0x0FU
/** Power (binary on/off), uint8, 0/1 */
#define BTHOME_OBJ_POWER_BIN         0x10U
/** Opening binary sensor, uint8, 0/1 */
#define BTHOME_OBJ_OPENING_BIN       0x11U
/** CO₂ concentration, uint16, unit: ppm */
#define BTHOME_OBJ_CO2               0x12U
/** TVOC concentration, uint16, unit: µg/m³ */
#define BTHOME_OBJ_TVOC              0x13U
/** Moisture, uint16, factor 0.01, unit: % */
#define BTHOME_OBJ_MOISTURE          0x14U

/* ---------------------------------------------------------------------------
 * Object IDs — binary sensors (all uint8; 0 = inactive, 1 = active)
 * -------------------------------------------------------------------------*/
#define BTHOME_OBJ_BATTERY_LOW       0x15U /**< Low battery */
#define BTHOME_OBJ_BATTERY_CHARGING  0x16U /**< Battery charging */
#define BTHOME_OBJ_CO                0x17U /**< Carbon monoxide detected */
#define BTHOME_OBJ_COLD              0x18U
#define BTHOME_OBJ_CONNECTIVITY      0x19U
#define BTHOME_OBJ_DOOR              0x1AU
#define BTHOME_OBJ_GARAGE_DOOR       0x1BU
#define BTHOME_OBJ_GAS               0x1CU
#define BTHOME_OBJ_HEAT              0x1DU
#define BTHOME_OBJ_LIGHT             0x1EU
#define BTHOME_OBJ_LOCK              0x1FU
#define BTHOME_OBJ_MOISTURE_BIN      0x20U
#define BTHOME_OBJ_MOTION            0x21U
#define BTHOME_OBJ_MOVING            0x22U
#define BTHOME_OBJ_OCCUPANCY         0x23U
#define BTHOME_OBJ_PLUG              0x24U
#define BTHOME_OBJ_PRESENCE          0x25U
#define BTHOME_OBJ_PROBLEM           0x26U
#define BTHOME_OBJ_RUNNING           0x27U
#define BTHOME_OBJ_SAFETY            0x28U
#define BTHOME_OBJ_SMOKE             0x29U
#define BTHOME_OBJ_SOUND             0x2AU
#define BTHOME_OBJ_TAMPER            0x2BU
#define BTHOME_OBJ_VIBRATION         0x2CU
#define BTHOME_OBJ_WINDOW            0x2DU

/* ---------------------------------------------------------------------------
 * Object IDs — numeric sensors (continued)
 * -------------------------------------------------------------------------*/
/** Humidity, uint8, unit: % (1 % resolution) */
#define BTHOME_OBJ_HUMIDITY_UI8      0x2EU
/** Moisture, uint8, unit: % (1 % resolution) */
#define BTHOME_OBJ_MOISTURE_UI8      0x2FU

/* ---------------------------------------------------------------------------
 * Object IDs — events
 * -------------------------------------------------------------------------*/
/** Button event, uint8 (see BTHOME_BTN_EVT_* constants) */
#define BTHOME_OBJ_BUTTON            0x3AU
/** Dimmer event: 2 bytes (direction, steps) */
#define BTHOME_OBJ_DIMMER            0x3CU
/** Count, uint16 */
#define BTHOME_OBJ_COUNT_UI16        0x3DU
/** Count, uint32 */
#define BTHOME_OBJ_COUNT_UI32        0x3EU
/** Rotation, sint16, factor 0.1, unit: ° */
#define BTHOME_OBJ_ROTATION          0x3FU

/* ---------------------------------------------------------------------------
 * Object IDs — extended numeric sensors (0x40 …)
 * -------------------------------------------------------------------------*/
/** Distance, uint16, unit: mm */
#define BTHOME_OBJ_DISTANCE_MM       0x40U
/** Distance, uint16, factor 0.1, unit: m */
#define BTHOME_OBJ_DISTANCE_M        0x41U
/** Duration, uint24, factor 0.001, unit: s */
#define BTHOME_OBJ_DURATION          0x42U
/** Current, uint16, factor 0.001, unit: A */
#define BTHOME_OBJ_CURRENT           0x43U
/** Speed, uint16, factor 0.01, unit: m/s */
#define BTHOME_OBJ_SPEED             0x44U
/** Temperature, sint16, factor 0.1, unit: °C (0.1 °C resolution) */
#define BTHOME_OBJ_TEMPERATURE_01    0x45U
/** UV index, uint8, factor 0.1 */
#define BTHOME_OBJ_UV_INDEX          0x46U
/** Volume, uint16, factor 0.1, unit: L */
#define BTHOME_OBJ_VOLUME_L_01       0x47U
/** Volume, uint16, unit: L (1 L resolution) */
#define BTHOME_OBJ_VOLUME_L          0x48U
/** Volume flow rate, uint16, factor 0.001, unit: m³/hr */
#define BTHOME_OBJ_VOLUME_FLOW_RATE  0x49U
/** Voltage, uint16, factor 0.1, unit: V */
#define BTHOME_OBJ_VOLTAGE_01        0x4AU
/** Gas, uint24, factor 0.001, unit: m³ */
#define BTHOME_OBJ_GAS_UI24          0x4BU
/** Gas, uint32, factor 0.001, unit: m³ */
#define BTHOME_OBJ_GAS_UI32          0x4CU
/** Energy, uint32, factor 0.001, unit: kWh */
#define BTHOME_OBJ_ENERGY_UI32       0x4DU
/** Volume, uint32, factor 0.001, unit: L */
#define BTHOME_OBJ_VOLUME_UI32       0x4EU
/** Water, uint32, factor 0.001, unit: L */
#define BTHOME_OBJ_WATER             0x4FU
/** Timestamp (Unix epoch), uint32, unit: s */
#define BTHOME_OBJ_TIMESTAMP         0x50U
/** Acceleration magnitude, uint16, factor 0.001, unit: m/s² */
#define BTHOME_OBJ_ACCELERATION      0x51U
/** Gyroscope, uint16, factor 0.001, unit: °/s */
#define BTHOME_OBJ_GYROSCOPE         0x52U
/** Text object (variable length, first data byte = length) */
#define BTHOME_OBJ_TEXT              0x53U
/** Raw bytes object (variable length, first data byte = length) */
#define BTHOME_OBJ_RAW               0x54U
/** Volume storage, uint32, factor 0.001, unit: L */
#define BTHOME_OBJ_VOLUME_STORAGE    0x55U
/** Conductivity, uint16, unit: µS/cm */
#define BTHOME_OBJ_CONDUCTIVITY      0x56U
/** Temperature, sint8, unit: °C (1 °C resolution) */
#define BTHOME_OBJ_TEMPERATURE_1     0x57U
/** Temperature, sint8, factor 0.35, unit: °C */
#define BTHOME_OBJ_TEMPERATURE_035   0x58U
/** Count, sint8 */
#define BTHOME_OBJ_COUNT_SI8         0x59U
/** Count, sint16 */
#define BTHOME_OBJ_COUNT_SI16        0x5AU
/** Count, sint32 */
#define BTHOME_OBJ_COUNT_SI32        0x5BU
/** Power, sint32, factor 0.01, unit: W (signed, for bidirectional metering) */
#define BTHOME_OBJ_POWER_SI32        0x5CU
/** Current, sint16, factor 0.001, unit: A (signed) */
#define BTHOME_OBJ_CURRENT_SI16      0x5DU
/** Direction, uint16, factor 0.01, unit: ° */
#define BTHOME_OBJ_DIRECTION         0x5EU
/** Precipitation, uint16, factor 0.1, unit: mm */
#define BTHOME_OBJ_PRECIPITATION     0x5FU
/** Channel number, uint8 */
#define BTHOME_OBJ_CHANNEL           0x60U
/** RPM, uint16, unit: rpm */
#define BTHOME_OBJ_RPM               0x61U
/** Speed, sint32, factor 0.000001, unit: m/s (signed) */
#define BTHOME_OBJ_SPEED_SI32        0x62U
/**
 * Acceleration axis, sint32, factor 0.000001, unit: m/s²
 * Use one entry per axis (X, Y, Z).
 */
#define BTHOME_OBJ_ACCELERATION_AXIS 0x63U
/** Light level, uint8 (0=dark, 1=twilight, 2=bright) */
#define BTHOME_OBJ_LIGHT_LEVEL       0x64U
/** Settings revision, uint8 */
#define BTHOME_OBJ_SETTINGS_REV      0x65U

/* ---------------------------------------------------------------------------
 * Object IDs — device information
 * -------------------------------------------------------------------------*/
/** Device type ID, uint16 */
#define BTHOME_OBJ_DEVICE_TYPE_ID    0xF0U
/** Firmware version, uint32 (format: major.minor.patch.build) */
#define BTHOME_OBJ_FW_VERSION_UI32   0xF1U
/** Firmware version, uint24 (format: major.minor.patch) */
#define BTHOME_OBJ_FW_VERSION_UI24   0xF2U

/* ---------------------------------------------------------------------------
 * Button event values (used with BTHOME_OBJ_BUTTON)
 * -------------------------------------------------------------------------*/
#define BTHOME_BTN_EVT_NONE               0x00U
#define BTHOME_BTN_EVT_PRESS              0x01U
#define BTHOME_BTN_EVT_DOUBLE_PRESS       0x02U
#define BTHOME_BTN_EVT_TRIPLE_PRESS       0x03U
#define BTHOME_BTN_EVT_LONG_PRESS         0x04U
#define BTHOME_BTN_EVT_LONG_DOUBLE_PRESS  0x05U
#define BTHOME_BTN_EVT_LONG_TRIPLE_PRESS  0x06U
#define BTHOME_BTN_EVT_HOLD_PRESS         0x08U

/* ---------------------------------------------------------------------------
 * Dimmer direction values (low byte of BTHOME_OBJ_DIMMER)
 * -------------------------------------------------------------------------*/
#define BTHOME_DIMMER_NONE         0x00U
#define BTHOME_DIMMER_ROTATE_LEFT  0x01U
#define BTHOME_DIMMER_ROTATE_RIGHT 0x02U

/* ---------------------------------------------------------------------------
 * Context and measurement structures
 * -------------------------------------------------------------------------*/

/**
 * @brief Single BTHome V2 measurement entry (fixed-width value, ≤4 bytes).
 *
 * For variable-length objects (text/raw), the caller is responsible for
 * including the 1-byte length field as the first byte of @p data and
 * setting @p data_len accordingly (max BTHOME_MAX_VALUE_LEN).
 */
struct bthome_meas {
uint8_t obj_id;
uint8_t data_len;
uint8_t data[BTHOME_MAX_VALUE_LEN];
};

/**
 * @brief BTHome V2 encoder context.
 *
 * Holds queued measurements and the encoded service-data buffer.
 * All fields are managed by the library; treat them as opaque except for
 * reading @p svc_data / @p svc_data_len after calling bthome_encode().
 */
struct bthome_ctx {
/** Pending measurement queue */
struct bthome_meas meas[BTHOME_MAX_MEASUREMENTS];
/** Number of queued measurements */
uint8_t meas_count;
/** Encrypted payload flag (sets bit 0 of device-info byte) */
bool encrypted;
/** Trigger-based device flag (sets bit 2 of device-info byte) */
bool trigger_based;
/**
 * Encoded service-data buffer.
 * Layout: [UUID_LO][UUID_HI][device_info][obj_id][data]…
 * Valid after calling bthome_encode().
 */
uint8_t svc_data[BTHOME_SVC_DATA_MAX_LEN];
/** Valid bytes in @p svc_data */
uint8_t svc_data_len;
};

/* ---------------------------------------------------------------------------
 * Lifecycle API
 * -------------------------------------------------------------------------*/

/**
 * @brief Initialise a BTHome V2 encoder context.
 *
 * @param ctx           Context to initialise (must not be NULL).
 * @param encrypted     Set true to signal an encrypted payload in the device-
 *                      info byte.  Full AES-CCM encryption support requires
 *                      CONFIG_BTHOME_ENCRYPTION and a bind key.
 * @param trigger_based Set true for trigger-based (non-periodic) devices.
 */
void bthome_init(struct bthome_ctx *ctx, bool encrypted, bool trigger_based);

/**
 * @brief Clear all queued measurements (keeps encryption/trigger config).
 *
 * @param ctx  Initialised context.
 */
void bthome_clear(struct bthome_ctx *ctx);

/**
 * @brief Encode all queued measurements into the service-data buffer.
 *
 * Measurements are automatically sorted by object ID (ascending) as required
 * by the BTHome V2 specification.  After encoding, @p ctx->svc_data and
 * @p ctx->svc_data_len hold the result.
 *
 * @param ctx  Initialised context with at least one queued measurement.
 * @return     Number of bytes written to @p ctx->svc_data, or negative
 *             error code on failure.
 */
int bthome_encode(struct bthome_ctx *ctx);

/**
 * @brief Populate a @c bt_data entry pointing at the encoded service data.
 *
 * bthome_encode() must be called before this function.  The @p out_data
 * entry shares memory with @p ctx->svc_data, so the context must remain
 * valid for the lifetime of the advertisement.
 *
 * @code
 *   static struct bthome_ctx bthome;
 *   static struct bt_data ad[2];
 *
 *   bthome_init(&bthome, false, false);
 *   bthome_add_temperature(&bthome, 2350);   // 23.50 °C
 *   bthome_encode(&bthome);
 *
 *   ad[0] = (struct bt_data)BT_DATA_BYTES(BT_DATA_FLAGS,
 *                BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR);
 *   bthome_get_bt_data(&bthome, &ad[1]);
 *   bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
 * @endcode
 *
 * @param ctx       Encoded context.
 * @param out_data  bt_data struct to populate.
 * @return 0 on success, -EINVAL if the context has not been encoded yet.
 */
int bthome_get_bt_data(const struct bthome_ctx *ctx, struct bt_data *out_data);

/* ---------------------------------------------------------------------------
 * Measurement add API
 *
 * All functions return 0 on success or -ENOMEM when the measurement queue
 * is full.
 *
 * Unit conventions follow the BTHome V2 specification so no floating-point
 * arithmetic is needed on the caller side:
 *
 *   temperature   → value in 0.01 °C     (e.g. 2350 = 23.50 °C)
 *   humidity      → value in 0.01 %      (e.g. 5500 = 55.00 %)
 *   pressure      → value in 0.01 hPa    (e.g. 101325 = 1013.25 hPa)
 *   illuminance   → value in 0.01 lx
 *   acceleration  → value in 0.001 m/s²  (milli-m/s²)
 *   gyroscope     → value in 0.001 °/s   (milli-deg/s)
 * -------------------------------------------------------------------------*/

/** @brief Add packet ID (uint8). */
int bthome_add_packet_id(struct bthome_ctx *ctx, uint8_t id);

/** @brief Add battery level in percent (uint8, 0–100). */
int bthome_add_battery(struct bthome_ctx *ctx, uint8_t percent);

/**
 * @brief Add temperature with 0.01 °C resolution (object ID 0x02).
 * @param temp_cdegc Temperature in hundredths of a degree Celsius.
 *                   Example: 2350 = 23.50 °C, -530 = -5.30 °C.
 */
int bthome_add_temperature(struct bthome_ctx *ctx, int16_t temp_cdegc);

/**
 * @brief Add temperature with 0.1 °C resolution (object ID 0x45).
 * @param temp_ddegc Temperature in tenths of a degree Celsius.
 *                   Example: 235 = 23.5 °C, -53 = -5.3 °C.
 */
int bthome_add_temperature_01(struct bthome_ctx *ctx, int16_t temp_ddegc);

/**
 * @brief Add humidity with 0.01 % resolution (object ID 0x03).
 * @param humidity_cpct Humidity in hundredths of a percent (e.g. 5500 = 55.00 %).
 */
int bthome_add_humidity(struct bthome_ctx *ctx, uint16_t humidity_cpct);

/**
 * @brief Add atmospheric pressure with 0.01 hPa resolution (object ID 0x04).
 * @param pressure_chpa Pressure in hundredths of hPa (e.g. 101325 = 1013.25 hPa).
 */
int bthome_add_pressure(struct bthome_ctx *ctx, uint32_t pressure_chpa);

/**
 * @brief Add illuminance with 0.01 lx resolution (object ID 0x05).
 * @param illuminance_clx Illuminance in hundredths of a lux.
 */
int bthome_add_illuminance(struct bthome_ctx *ctx, uint32_t illuminance_clx);

/** @brief Add CO₂ concentration in ppm (uint16, object ID 0x12). */
int bthome_add_co2(struct bthome_ctx *ctx, uint16_t ppm);

/** @brief Add TVOC concentration in µg/m³ (uint16, object ID 0x13). */
int bthome_add_tvoc(struct bthome_ctx *ctx, uint16_t ugm3);

/** @brief Add voltage with 0.001 V resolution (uint16, object ID 0x0C). */
int bthome_add_voltage(struct bthome_ctx *ctx, uint16_t millivolts);

/** @brief Add dew point with 0.01 °C resolution (sint16, object ID 0x08). */
int bthome_add_dew_point(struct bthome_ctx *ctx, int16_t temp_cdegc);

/**
 * @brief Add acceleration magnitude (uint16, factor 0.001 m/s², object ID 0x51).
 * @param milli_ms2 Acceleration in milli-m/s² (e.g. 9810 = 9.810 m/s²).
 */
int bthome_add_acceleration(struct bthome_ctx *ctx, uint16_t milli_ms2);

/**
 * @brief Add a single acceleration axis value (sint32, factor 0.000001 m/s²,
 *        object ID 0x63).
 *
 * Call once per axis (X, Y, Z).  Three consecutive calls produce three
 * independent 0x63 objects.
 *
 * @param micro_ms2  Acceleration in micro-m/s² (e.g. -3 123 456 = -3.123456 m/s²).
 */
int bthome_add_acceleration_axis(struct bthome_ctx *ctx, int32_t micro_ms2);

/**
 * @brief Add gyroscope reading (uint16, factor 0.001 °/s, object ID 0x52).
 * @param milli_degs Rotation rate in milli-degrees per second.
 */
int bthome_add_gyroscope(struct bthome_ctx *ctx, uint16_t milli_degs);

/** @brief Add Unix timestamp (uint32, object ID 0x50). */
int bthome_add_timestamp(struct bthome_ctx *ctx, uint32_t unix_s);

/**
 * @brief Add a binary sensor state.
 * @param obj_id  One of the BTHOME_OBJ_* binary sensor constants.
 * @param active  true = sensor active / detected / open; false = inactive.
 */
int bthome_add_binary(struct bthome_ctx *ctx, uint8_t obj_id, bool active);

/**
 * @brief Add a button event.
 * @param event  One of the BTHOME_BTN_EVT_* constants.
 */
int bthome_add_button(struct bthome_ctx *ctx, uint8_t event);

/**
 * @brief Add a dimmer event.
 * @param direction  BTHOME_DIMMER_ROTATE_LEFT or BTHOME_DIMMER_ROTATE_RIGHT.
 * @param steps      Number of rotation steps.
 */
int bthome_add_dimmer(struct bthome_ctx *ctx, uint8_t direction, uint8_t steps);

/**
 * @brief Add a raw or variable-length measurement.
 *
 * For text (0x53) and raw (0x54) objects, the first byte of @p data must be
 * the payload length as required by the BTHome V2 specification, and
 * @p data_len must include that length prefix byte.
 *
 * @param obj_id    Object ID.
 * @param data      Pointer to data bytes (max BTHOME_MAX_VALUE_LEN).
 * @param data_len  Number of bytes in @p data.
 * @return 0 on success, -EINVAL if data_len exceeds BTHOME_MAX_VALUE_LEN,
 *         -ENOMEM if the measurement queue is full.
 */
int bthome_add_raw(struct bthome_ctx *ctx, uint8_t obj_id,
   const uint8_t *data, uint8_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* BTHOME_BTHOME_H_ */
