/*
 * Copyright (c) 2026
 * SPDX-License-Identifier: Apache-2.0
 *
 * BTHome V2 library for Zephyr
 *
 * Specification: https://bthome.io/format/
 */

#include <string.h>

#include <zephyr/logging/log.h>

#include <bthome/bthome.h>

LOG_MODULE_REGISTER(bthome);

/* ---------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------*/

/**
 * @brief Claim a free measurement slot from the context queue.
 * @return Pointer to the free slot, or NULL when the queue is full.
 */
static struct bthome_meas *alloc_meas(struct bthome_ctx *ctx)
{
if (ctx->meas_count >= BTHOME_MAX_MEASUREMENTS) {
return NULL;
}

return &ctx->meas[ctx->meas_count++];
}

static int add_u8(struct bthome_ctx *ctx, uint8_t obj_id, uint8_t val)
{
struct bthome_meas *m = alloc_meas(ctx);

if (m == NULL) {
return -ENOMEM;
}

m->obj_id   = obj_id;
m->data_len = 1U;
m->data[0]  = val;
return 0;
}

/* Used by future sint8 measurement types (e.g. BTHOME_OBJ_TEMPERATURE_1) */
static int add_s8(struct bthome_ctx *ctx, uint8_t obj_id,
	  int8_t val) __attribute__((unused));
static int add_s8(struct bthome_ctx *ctx, uint8_t obj_id, int8_t val)
{
return add_u8(ctx, obj_id, (uint8_t)val);
}

static int add_u16(struct bthome_ctx *ctx, uint8_t obj_id, uint16_t val)
{
struct bthome_meas *m = alloc_meas(ctx);

if (m == NULL) {
return -ENOMEM;
}

m->obj_id   = obj_id;
m->data_len = 2U;
m->data[0]  = (uint8_t)(val & 0xFFU);
m->data[1]  = (uint8_t)((val >> 8U) & 0xFFU);
return 0;
}

static int add_s16(struct bthome_ctx *ctx, uint8_t obj_id, int16_t val)
{
return add_u16(ctx, obj_id, (uint16_t)val);
}

static int add_u24(struct bthome_ctx *ctx, uint8_t obj_id, uint32_t val)
{
struct bthome_meas *m = alloc_meas(ctx);

if (m == NULL) {
return -ENOMEM;
}

m->obj_id   = obj_id;
m->data_len = 3U;
m->data[0]  = (uint8_t)(val & 0xFFU);
m->data[1]  = (uint8_t)((val >> 8U) & 0xFFU);
m->data[2]  = (uint8_t)((val >> 16U) & 0xFFU);
return 0;
}

static int add_u32(struct bthome_ctx *ctx, uint8_t obj_id, uint32_t val)
{
struct bthome_meas *m = alloc_meas(ctx);

if (m == NULL) {
return -ENOMEM;
}

m->obj_id   = obj_id;
m->data_len = 4U;
m->data[0]  = (uint8_t)(val & 0xFFU);
m->data[1]  = (uint8_t)((val >> 8U) & 0xFFU);
m->data[2]  = (uint8_t)((val >> 16U) & 0xFFU);
m->data[3]  = (uint8_t)((val >> 24U) & 0xFFU);
return 0;
}

static int add_s32(struct bthome_ctx *ctx, uint8_t obj_id, int32_t val)
{
return add_u32(ctx, obj_id, (uint32_t)val);
}

/* ---------------------------------------------------------------------------
 * Sorting helper (insertion sort, stable, O(n²), adequate for ≤12 entries)
 * -------------------------------------------------------------------------*/

static void sort_measurements(struct bthome_ctx *ctx)
{
for (uint8_t i = 1U; i < ctx->meas_count; i++) {
struct bthome_meas key = ctx->meas[i];
int j = (int)i - 1;

while (j >= 0 && ctx->meas[j].obj_id > key.obj_id) {
ctx->meas[j + 1] = ctx->meas[j];
j--;
}
ctx->meas[j + 1] = key;
}
}

/* ---------------------------------------------------------------------------
 * Lifecycle
 * -------------------------------------------------------------------------*/

void bthome_init(struct bthome_ctx *ctx, bool encrypted, bool trigger_based)
{
(void)memset(ctx, 0, sizeof(*ctx));
ctx->encrypted     = encrypted;
ctx->trigger_based = trigger_based;
}

void bthome_clear(struct bthome_ctx *ctx)
{
ctx->meas_count   = 0U;
ctx->svc_data_len = 0U;
}

/* ---------------------------------------------------------------------------
 * Encode
 * -------------------------------------------------------------------------*/

int bthome_encode(struct bthome_ctx *ctx)
{
uint8_t *buf;
uint8_t  pos = 0U;
uint8_t  dev_info;

if (ctx == NULL) {
return -EINVAL;
}

buf = ctx->svc_data;

/* UUID (little-endian) */
buf[pos++] = BTHOME_UUID_LE_B0;
buf[pos++] = BTHOME_UUID_LE_B1;

/* Device information byte */
dev_info = BTHOME_DEV_INFO_VERSION;

if (ctx->encrypted) {
dev_info |= BTHOME_DEV_INFO_ENCRYPT;
}
if (ctx->trigger_based) {
dev_info |= BTHOME_DEV_INFO_TRIGGER;
}
buf[pos++] = dev_info;

/* Sort measurements ascending by object ID (spec requirement) */
sort_measurements(ctx);

/* Serialise measurements */
for (uint8_t i = 0U; i < ctx->meas_count; i++) {
const struct bthome_meas *m = &ctx->meas[i];

if ((pos + 1U + m->data_len) > BTHOME_SVC_DATA_MAX_LEN) {
LOG_WRN("payload full, dropping measurement 0x%02X",
m->obj_id);
break;
}

buf[pos++] = m->obj_id;
(void)memcpy(&buf[pos], m->data, m->data_len);
pos += m->data_len;
}

ctx->svc_data_len = pos;
return (int)pos;
}

/* ---------------------------------------------------------------------------
 * bt_data helper
 * -------------------------------------------------------------------------*/

int bthome_get_bt_data(const struct bthome_ctx *ctx, struct bt_data *out_data)
{
if ((ctx == NULL) || (out_data == NULL) || (ctx->svc_data_len == 0U)) {
return -EINVAL;
}

out_data->type     = BT_DATA_SVC_DATA16;
out_data->data_len = ctx->svc_data_len;
out_data->data     = ctx->svc_data;
return 0;
}

/* ---------------------------------------------------------------------------
 * Measurement add API
 * -------------------------------------------------------------------------*/

int bthome_add_packet_id(struct bthome_ctx *ctx, uint8_t id)
{
return add_u8(ctx, BTHOME_OBJ_PACKET_ID, id);
}

int bthome_add_battery(struct bthome_ctx *ctx, uint8_t percent)
{
return add_u8(ctx, BTHOME_OBJ_BATTERY, percent);
}

int bthome_add_temperature(struct bthome_ctx *ctx, int16_t temp_cdegc)
{
return add_s16(ctx, BTHOME_OBJ_TEMPERATURE, temp_cdegc);
}

int bthome_add_temperature_01(struct bthome_ctx *ctx, int16_t temp_ddegc)
{
return add_s16(ctx, BTHOME_OBJ_TEMPERATURE_01, temp_ddegc);
}

int bthome_add_humidity(struct bthome_ctx *ctx, uint16_t humidity_cpct)
{
return add_u16(ctx, BTHOME_OBJ_HUMIDITY, humidity_cpct);
}

int bthome_add_pressure(struct bthome_ctx *ctx, uint32_t pressure_chpa)
{
return add_u24(ctx, BTHOME_OBJ_PRESSURE, pressure_chpa);
}

int bthome_add_illuminance(struct bthome_ctx *ctx, uint32_t illuminance_clx)
{
return add_u24(ctx, BTHOME_OBJ_ILLUMINANCE, illuminance_clx);
}

int bthome_add_co2(struct bthome_ctx *ctx, uint16_t ppm)
{
return add_u16(ctx, BTHOME_OBJ_CO2, ppm);
}

int bthome_add_tvoc(struct bthome_ctx *ctx, uint16_t ugm3)
{
return add_u16(ctx, BTHOME_OBJ_TVOC, ugm3);
}

int bthome_add_voltage(struct bthome_ctx *ctx, uint16_t millivolts)
{
return add_u16(ctx, BTHOME_OBJ_VOLTAGE, millivolts);
}

int bthome_add_dew_point(struct bthome_ctx *ctx, int16_t temp_cdegc)
{
return add_s16(ctx, BTHOME_OBJ_DEW_POINT, temp_cdegc);
}

int bthome_add_acceleration(struct bthome_ctx *ctx, uint16_t milli_ms2)
{
return add_u16(ctx, BTHOME_OBJ_ACCELERATION, milli_ms2);
}

int bthome_add_acceleration_axis(struct bthome_ctx *ctx, int32_t micro_ms2)
{
return add_s32(ctx, BTHOME_OBJ_ACCELERATION_AXIS, micro_ms2);
}

int bthome_add_gyroscope(struct bthome_ctx *ctx, uint16_t milli_degs)
{
return add_u16(ctx, BTHOME_OBJ_GYROSCOPE, milli_degs);
}

int bthome_add_timestamp(struct bthome_ctx *ctx, uint32_t unix_s)
{
return add_u32(ctx, BTHOME_OBJ_TIMESTAMP, unix_s);
}

int bthome_add_binary(struct bthome_ctx *ctx, uint8_t obj_id, bool active)
{
return add_u8(ctx, obj_id, active ? 1U : 0U);
}

int bthome_add_button(struct bthome_ctx *ctx, uint8_t event)
{
return add_u8(ctx, BTHOME_OBJ_BUTTON, event);
}

int bthome_add_dimmer(struct bthome_ctx *ctx, uint8_t direction, uint8_t steps)
{
struct bthome_meas *m = alloc_meas(ctx);

if (m == NULL) {
return -ENOMEM;
}

m->obj_id   = BTHOME_OBJ_DIMMER;
m->data_len = 2U;
m->data[0]  = direction;
m->data[1]  = steps;
return 0;
}

int bthome_add_raw(struct bthome_ctx *ctx, uint8_t obj_id,
   const uint8_t *data, uint8_t data_len)
{
struct bthome_meas *m;

if ((data_len > BTHOME_MAX_VALUE_LEN) || (data == NULL)) {
return -EINVAL;
}

m = alloc_meas(ctx);
if (m == NULL) {
return -ENOMEM;
}

m->obj_id   = obj_id;
m->data_len = data_len;
(void)memcpy(m->data, data, data_len);
return 0;
}
