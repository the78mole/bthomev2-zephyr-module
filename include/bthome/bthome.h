/*
 * Copyright (c) 2026
 * SPDX-License-Identifier: Apache-2.0
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

#define BTHOME_V2_SERVICE_UUID 0xFCD2U

struct bthome_builder {
	uint8_t *buffer;
	size_t capacity;
	size_t len;
	struct bt_data ad;
};

int bthome_builder_init(struct bthome_builder *builder, uint8_t *buffer, size_t capacity,
			bool encrypted);

int bthome_builder_add_raw(struct bthome_builder *builder, uint8_t object_id, const uint8_t *value,
			   size_t value_len);

int bthome_builder_add_u8(struct bthome_builder *builder, uint8_t object_id, uint8_t value);

int bthome_builder_add_u16(struct bthome_builder *builder, uint8_t object_id, uint16_t value);

int bthome_builder_add_s16(struct bthome_builder *builder, uint8_t object_id, int16_t value);

int bthome_builder_add_fixed_point_s16(struct bthome_builder *builder, uint8_t object_id,
				       int32_t value, int32_t scale);

const struct bt_data *bthome_builder_get_ad(const struct bthome_builder *builder);

#ifdef __cplusplus
}
#endif

#endif /* BTHOME_BTHOME_H_ */
