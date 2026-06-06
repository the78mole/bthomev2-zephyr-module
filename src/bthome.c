/*
 * Copyright (c) 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <bthome/bthome.h>

LOG_MODULE_REGISTER(bthome);

#define BTHOME_UUID_LSB ((uint8_t)(BTHOME_V2_SERVICE_UUID & 0xFFU))
#define BTHOME_UUID_MSB ((uint8_t)((BTHOME_V2_SERVICE_UUID >> 8U) & 0xFFU))
#define BTHOME_INFO_VERSION_2 0x40U
#define BTHOME_INFO_ENCRYPTED BIT(0)

static int reserve_bytes(struct bthome_builder *builder, size_t required)
{
if ((builder->capacity - builder->len) < required) {
return -ENOSPC;
}

return 0;
}

int bthome_builder_init(struct bthome_builder *builder, uint8_t *buffer, size_t capacity,
bool encrypted)
{
if ((builder == NULL) || (buffer == NULL)) {
return -EINVAL;
}

if (encrypted && !IS_ENABLED(CONFIG_BTHOME_ENCRYPTION)) {
return -ENOTSUP;
}

builder->buffer = buffer;
builder->capacity = capacity;
builder->len = 0U;

if (reserve_bytes(builder, 3U) != 0) {
return -ENOSPC;
}

builder->buffer[builder->len++] = BTHOME_UUID_LSB;
builder->buffer[builder->len++] = BTHOME_UUID_MSB;
builder->buffer[builder->len++] = BTHOME_INFO_VERSION_2 |
      (encrypted ? BTHOME_INFO_ENCRYPTED : 0U);

builder->ad.type = BT_DATA_SVC_DATA16;
builder->ad.data = builder->buffer;
builder->ad.data_len = builder->len;

return 0;
}

int bthome_builder_add_raw(struct bthome_builder *builder, uint8_t object_id, const uint8_t *value,
   size_t value_len)
{
if (builder == NULL) {
return -EINVAL;
}

if ((value_len > 0U) && (value == NULL)) {
return -EINVAL;
}

if (reserve_bytes(builder, value_len + 1U) != 0) {
return -ENOSPC;
}

builder->buffer[builder->len++] = object_id;
if (value_len > 0U) {
(void)memcpy(&builder->buffer[builder->len], value, value_len);
builder->len += value_len;
}

builder->ad.data_len = builder->len;

return 0;
}

int bthome_builder_add_u8(struct bthome_builder *builder, uint8_t object_id, uint8_t value)
{
return bthome_builder_add_raw(builder, object_id, &value, sizeof(value));
}

int bthome_builder_add_u16(struct bthome_builder *builder, uint8_t object_id, uint16_t value)
{
uint8_t encoded[sizeof(value)] = {
(uint8_t)(value & 0xFFU),
(uint8_t)(value >> 8U),
};

return bthome_builder_add_raw(builder, object_id, encoded, sizeof(encoded));
}

int bthome_builder_add_s16(struct bthome_builder *builder, uint8_t object_id, int16_t value)
{
uint16_t as_u16 = (uint16_t)value;

return bthome_builder_add_u16(builder, object_id, as_u16);
}

int bthome_builder_add_fixed_point_s16(struct bthome_builder *builder, uint8_t object_id,
			       int32_t value, int32_t factor)
{
int64_t scaled;

if (factor <= 0) {
	return -EINVAL;
}

scaled = (int64_t)value * (int64_t)factor;
if ((scaled > INT16_MAX) || (scaled < INT16_MIN)) {
	return -ERANGE;
}

return bthome_builder_add_s16(builder, object_id, (int16_t)scaled);
}

const struct bt_data *bthome_builder_get_ad(const struct bthome_builder *builder)
{
if (builder == NULL) {
return NULL;
}

return &builder->ad;
}
