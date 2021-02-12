#pragma once

#include "../config.h"

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

enum lang_value_masks {
	lang_value_mask_sign           = 0x8000000000000000,
	lang_value_mask_sign_shift     = 63,
	lang_value_mask_exponent       = 0x7FF0000000000000,
	lang_value_mask_exponent_shift = 52,
	lang_value_mask_fraction       = 0x000FFFFFFFFFFFFF,
	lang_value_mask_object         = 0x7FF5A00000000000,

	lang_value_max_object = lang_value_mask_fraction >> 8,
};

LANG_VM_API inline
int lang_is_object(uint64_t v) {
	return (v & lang_value_mask_object) == lang_value_mask_object;
}

LANG_VM_API inline
uint64_t lang_object2val(uint64_t objectIndex) {
	assert(objectIndex <= lang_value_max_object && "Can't encode values this big");
	return lang_value_mask_object | objectIndex;
}

LANG_VM_API inline
uint64_t lang_val2object(uint64_t value) {
	assert(lang_is_object(value)&&"That's not an object");
	return value & lang_value_mask_fraction;
}
