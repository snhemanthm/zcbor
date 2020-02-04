/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CDDL_CBOR_H__
#define CDDL_CBOR_H__
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/** The cbor_decode library provides functions for decoding CBOR data elements.
 *
 * This library is primarily meant to be called from code generated by
 * $CDDL_GEN_BASE/scripts/cddl_gen.py
 *
 * Some details to notice about this library:
 *  - Integers are all 32 bits (uint32_t and size_t). This means that CBOR's
 *    64 bit values are not supported. This applies to integer types, as well as
 *    lengths for other types.
 *  - Strings are kept in the container type cbor_string_type_t, which is a
 *    pointer and a length.
 *  - When a function returns false, it only means that decoding that particular
 *    value failed. If a value is allowed to take multiple different values,
 *    another decoding function can be called if the first fails. All functions
 *    are designed to reset pp_payload and p_elem_count to their original values
 *    if they return false.
 *  - There is some type casting going on under the hood to make the code
 *    generator friendly. See especially the decoder_t type which is compatible
 *    with all functions except multi_decode, but the compiler doesn't "know"
 *    this because they are defined with different pointer types. It also means
 *    any usage of multi_decode must be made with care for function types.
 *  - This library has no function for semantic tags.
 *  - This library doesn't distinguish lists from maps.
 *
 *
 * CBOR's format is described well on Wikipedia
 *  - https://en.wikipedia.org/wiki/CBOR
 * but here's a synopsis:
 *
 * Encoded CBOR data elements look like this.
 *
 * | Header             | Value                  | Payload                   |
 * | 1 byte             | 0, 1, 2, 4, or 8 bytes | 0 - 2^64-1 bytes/elements |
 * | 3 bits | 5 bits    |
 * | Major  | Additional|
 * | Type   | info      |
 *
 * The available major types can be seen in @ref cbor_major_type_t.
 *
 * PINT, NINT, TAG, and PRIM elements have no payload, only Value.
 * PINT: Interpret the Value as a positive integer.
 * NINT: Interpret the Value as a positive integer, then multiply by -1 and
 *       subtract 1.
 * TAG: The Value says something about the next non-tag element.
 *      See https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml
 * PRIM: Different Values mean different things:
 *       20: "false"
 *       21: "true"
 *       22: "null"
 *       23: "undefined"
 *       >256: Interpret as IEEE 754 float with given precision
 *
 * For BSTR, TSTR, LIST, and MAP, the Value describes the length of the payload.
 * For BSTR and TSTR, the length is in bytes, for LIST, the length is in number
 * of elements, and for MAP, the length is in number of key/value element pairs.
 *
 * For LIST and MAP, sub elements are regular CBOR elements with their own
 * Header, Value and Payload. LISTs and MAPs can be recursively encoded.
 *
 * For all types, Values 0-23 are encoded directly in the "Additional info",
 * meaning that the "Value" field is 0 bytes long. If "Additional info" is 24,
 * 25, 26, or 27, the "Value" field is 1, 2, 4, or 8 bytes long, respectively.
 *
 * The additional info means slightly different things for different major
 * types.
 */


/** Convenience type that allows pointing to strings directly inside the payload
 *  without the need to copy out.
 */
typedef struct
{
	const uint8_t *value;
	size_t len;
} cbor_string_type_t;

#ifdef CDDL_CBOR_VERBOSE
#include <sys/printk.h>
#define cbor_decode_trace() (printk("*pp_payload: 0x%x, "\
			"**pp_payload: 0x%x, %s:%d\n",\
			(uint32_t)*pp_payload,\
			**pp_payload, __FILE__, __LINE__))
#define cbor_decode_assert(expr, msgformat, ...) \
do { \
	if (!expr) { \
		printk("assertion failed at %s:%d:\n\n" #expr "\n" msgformat, \
			__VA_ARGS__); \
		return false; \
	} \
} while(0)
#define cbor_decode_print(...) printk(__VA_ARGS__)
#else
#define cbor_decode_trace()
#define cbor_decode_assert(...)
#define cbor_decode_print(...)
#endif

/** Function pointer type used with multi_decode.
 *
 * This type is compatible with all decoding functions here and in the generated
 * code, except for multi_decode.
 */
typedef bool(decoder_t)(uint8_t const **, uint8_t const *const, size_t *const,
			void *, void *, void *);

/** Decode a PINT/NINT into a int32_t.
 *
 * @param[inout] pp_payload     The current place in the payload. Will be
 *                              updated if the element is correctly decoded.
 * @param[in]    p_payload_end  The end of the payload. This will be checked
 *                              against pp_payload before decoding.
 * @param[inout] p_elem_count   The current element is part of a LIST or a MAP,
 *                              and this keeps count of how many elements are
 *                              expected. This will be checked before decoding
 *                              decremented if the element is correctly decoded.
 * @param[out]   p_result       Where to place the decoded value.
 * @param[in]    p_min_value    The minimum acceptable value. This is checked
 *                              after decoding, and if the decoded value is
 *                              outside the range, the decoding will fail.
 *                              A NULL value here means there is no restriction.
 * @param[in]    p_max_value    The maximum acceptable value. This is checked
 *                              after decoding, and if the decoded value is
 *                              outside the range, the decoding will fail.
 *                              A NULL value here means there is no restriction.
 *
 * @retval true   If the value was decoded correctly.
 * @retval false  If the value has the wrong type, the payload overflowed, the
 *                element count was exhausted, the value was not within the
 *                acceptable range, or the value was larger than can fit in the
 *                result variable.
 */
bool intx32_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		int32_t *p_result, void *p_min_value, void *p_max_value);

/** Decode a PINT into a uint32_t.
 *
 * @details See @ref intx32_decode for information about parameters and return
 *          values.
 */
bool uintx32_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		uint32_t *p_result, void *p_min_value, void *p_max_value);

/** Decode a BSTR or TSTR, but leave pp_payload pointing at the payload.
 *
 * @details See @ref intx32_decode for information about parameters and return
 *          values. For strings, the value refers to the length of the string.
 */
bool strx_start_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		cbor_string_type_t *p_result, void *p_min_len, void *p_max_len);

/** Decode a BSTR or TSTR, and move pp_payload to after the payload.
 *
 * @details See @ref intx32_decode for information about parameters and return
 *          values. For strings, the value refers to the length of the string.
 */
bool strx_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		cbor_string_type_t *p_result, void *p_min_len, void *p_max_len);

/** Decode a LIST or MAP, but leave pp_payload pointing at the payload.
 *
 * @details See @ref intx32_decode for information about parameters and return
 *          values. For lists and maps, the value refers to the number of
 *          elements.
 */
bool list_start_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		size_t *p_result, size_t min_num, size_t max_num);

/** Decode a primitive value.
 *
 * @details See @ref intx32_decode for information about parameters and return
 *          values.
 */
bool primx_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		uint8_t *p_result, void *p_min_result, void *p_max_result);

/** Decode a boolean primitive value.
 *
 * @details See @ref intx32_decode for information about parameters and return
 *          values. The result is translated internally from the primitive
 *          values for true/false (20/21) to 0/1.
 */
bool boolx_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		bool *p_result, void *p_min_result, void *p_max_result);

/** Decode a float
 *
 * @warning This function has not been tested, and likely doesn't work.
 *
 * @details See @ref intx32_decode for information about parameters and return
 *          values.
 */
bool float_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		double *p_result, void *p_min_result, void *p_max_result);

/** Skip a single element, regardless of type and value.
 *
 * @details See @ref intx32_decode for information about parameters and return
 *          values. @p p_result, @p p_min_result, and @p p_max_result must be
 *          NULL.
 */
bool any_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		void *p_result, void *p_min_result, void *p_max_result);

/** Decode 0 or more elements with the same type and constraints.
 *
 * @details This must not necessarily decode all elements in a list. E.g. if
 *          the list contains 3 INTS between 0 and 100 followed by 0 to 2 BSTRs
 *          with length 8, that could be done with:
 *
 *          @code{c}
 *              size_t elem_count = 5;
 *              uint32_t int_min = 0;
 *              uint32_t int_max = 100;
 *              size_t bstr_size = 8;
 *              uint32_t ints[3];
 *              cbor_string_type_t bstrs[2];
 *
 *              list_start_decode(pp_payload, p_payload_end, &parent_elem_count,
 *                                &elem_count, NULL, NULL);
 *              multi_decode(3, 3, &num_decode, uintx32_decode, pp_payload,
 *                           p_payload_end, ints, &int_min, &int_max, 4);
 *              multi_decode(0, 2, &num_decode, strx_decode, pp_payload,
 *                           p_payload_end, bstrs, &bstr_size, &bstr_size,
 *                           sizeof(cbor_string_type_t));
 *          @endcode
 *
 *          See @ref intx32_decode for information about the undocumented
 *          parameters.
 *
 * @param[in]  min_decode    The minimum acceptable number of elements.
 * @param[in]  max_decode    The maximum acceptable number of elements.
 * @param[out] p_num_decode  The actual number of elements.
 * @param[in]  decoder       The decoder function to call under the hood. This
 *                           function will be called with the provided arguments
 *                           repeatedly until the function fails (returns false)
 *                           or until it has been called @p max_decode times.
 *                           p_result is moved @p result_len bytes for each call
 *                           to @p decoder, i.e. @p p_result refers to an array
 *                           of result variables.
 * @param[out] p_result      Where to place the decoded values. Must be an array
 *                           of length at least @p max_decode.
 * @param[in]  result_len    The length of the result variables. Must be the
 *                           length expected by the @p decoder.
 *
 * @retval true   If at least @p min_decode variables were correctly decoded.
 * @retval false  If @p decoder failed before having decoded @p min_decode
 *                values.
 */
bool multi_decode(size_t min_decode, size_t max_decode, size_t *p_num_decode,
		decoder_t decoder, uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		void *p_result, void *p_min_result, void *p_max_result,
		size_t result_len);

#endif
