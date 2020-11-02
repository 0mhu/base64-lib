#include <base64-lib/base64-lib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef BASE64_LOOKUP_TABLE_SECTION
#define LOOKUP_SECTION_ATTR __attribute__((section(BASE64_LOOKUP_TABLE_SECTION)))
#else
#define LOOKUP_SECTION_ATTR
#endif

static bool base64_lib_initialized = false;

static char LOOKUP_SECTION_ATTR base64_encode_table[64];
static char LOOKUP_SECTION_ATTR base64_decode_table[256];

static void initialize_encode_lookup_table(void)
{
	int idx = 0;
	char encoded_char;

	for (encoded_char = 'A'; encoded_char <= 'Z'; encoded_char++, idx++)
		base64_encode_table[idx] = encoded_char;
	for (encoded_char = 'a'; encoded_char <= 'z'; encoded_char++, idx++)
		base64_encode_table[idx] = encoded_char;
	for (encoded_char = '0'; encoded_char <= '9'; encoded_char++, idx++)
		base64_encode_table[idx] = encoded_char;
	base64_encode_table[62] = '+';
	base64_encode_table[63] = '/';
}

static void initialize_decode_lookup_table(void)
{
	uint8_t i;

	for (i = 0; i < 64; i++) {
		base64_decode_table[(unsigned int)base64_encode_table[i]] = (char)i;
	}
	base64_decode_table[(unsigned int)'='] = '\0';
}

static void base64_init(void)
{
	if (base64_lib_initialized)
		return;

	initialize_encode_lookup_table();
	initialize_decode_lookup_table();

	base64_lib_initialized = true;
}

int base64_encode(const char *src, char *dest, size_t src_size, size_t dest_size, size_t *output_length)
{
	unsigned int src_idx;
	unsigned int dest_idx;
	uint32_t first_byte, second_byte, third_byte;
	uint32_t three_bytes;
	uint8_t input_vals_padded;
	static const uint8_t padding_count_table[] = {0, 2, 1};

	if (!src || !dest || !output_length)
		return -1000;

	/* Base 64 converts 3 bytes to 4 output chars.
	 * The following formula ensures, that the value of in_size / 3 * 4 is rounded up.
	 */
	*output_length = base64_calculate_encoded_size(src_size);

	/* Check if the converted data fits into tzhe buffer */
	if (*output_length > dest_size)
		return -1;

	/* Init base64 module if not already initialized */
	base64_init();

	/* Loop over data */
	for (src_idx = 0, dest_idx = 0; src_idx < src_size;) {
		/* Extract 3 bytes from the input data. Pad with zeroes if end is reached.
		 * uint32_t variables are used, because they make concatenating the bytes to one
		 * 32 bit number easier.
		 */
		first_byte = (uint32_t)src[src_idx++];
		second_byte = (src_idx <  src_size ? (uint32_t)src[src_idx++] : 0UL);
		third_byte = (src_idx <  src_size ? (uint32_t)src[src_idx++] : 0UL);

		/* Concatenate the three input values */
		three_bytes = (first_byte << 16U) | (second_byte << 8U) | third_byte;

		/* Extract 6 bit blocks and convert them to the ouput data */
		dest[dest_idx++] = base64_encode_table[(three_bytes >> 3 * 6) & 0x3F];
		dest[dest_idx++] = base64_encode_table[(three_bytes >> 2 * 6) & 0x3F];
		dest[dest_idx++] = base64_encode_table[(three_bytes >> 1 * 6) & 0x3F];
		dest[dest_idx++] = base64_encode_table[(three_bytes) & 0x3F];
	}

	/* If the last values have been padded, the last one or two symbols in the ouput stream are incorrect
	 * and have to be replaced with '='
	 */
	input_vals_padded = padding_count_table[src_size % 3];
	for (dest_idx-- ;input_vals_padded > 0; input_vals_padded--) {
		dest[dest_idx--] = '=';
	}

	return 0;
}

size_t base64_calculate_encoded_size(size_t unencoded_size)
{
	return (size_t)(4 * ((unencoded_size + 2) / 3));
}

size_t base64_calculate_maximum_decoded_size(size_t encoded_size)
{
	if (encoded_size % 4)
		return 0;
	return encoded_size / 4 * 3;
}

/**
 * @brief This function checks, if a char is a valid character for base64 encoded data
 * @note This function treats the padding '=' as an invalid character
 */
static bool is_valid_base64_character(char c)
{
	bool ret;

	switch (c) {
	case 'A' ... 'Z': /* FALLTHRU */
	case 'a' ... 'z': /* FALLTHRU */
	case '0' ... '9': /* FALLTHRU */
	case '+': /* FALLTHRU */
	case '/':
		ret = true;
		break;
	default:
		ret = false;
		break;
	}

	return ret;
}

static inline uint32_t base64_chars_to_three_bytes(char a, char b, char c, char d)
{
	return	((((uint32_t)base64_decode_table[(unsigned int)a] & 0x3F) << 18) ) |
			((((uint32_t)base64_decode_table[(unsigned int)b] & 0x3F) << 12)) |
			((((uint32_t)base64_decode_table[(unsigned int)c] & 0x3F) << 6)) |
			((((uint32_t)base64_decode_table[(unsigned int)d] & 0x3F)));
}

int base64_decode(const char *base64_src, char *dest, size_t src_size, size_t dest_size, size_t *output_written)
{
	size_t output_len;
	size_t idx;
	char first, second, third, fourth;
	uint32_t three_bytes;
	bool padded = false;
	size_t written = 0;
	int ret = 0;

	if (!base64_src || !dest || !src_size || !dest_size)
		return -1000;

	output_len = base64_calculate_maximum_decoded_size(src_size);
	if (output_len == 0) {
		ret = -1;
		goto exit;
	}

	base64_init();

	/* Check paddings and correct output length */
	if (base64_src[src_size-1] == '=') {
		padded = true;
		output_len--;
	}

	if (base64_src[src_size-2] == '=') {
		output_len--;
		padded = true;
	}

	if (dest_size < output_len) {
		ret = -2;
		goto exit;
	}

	/* Convert all full quartetts */
	for (idx = 0; idx < src_size - (padded ? 4 : 0);) {
		/* Due to the call to base64_calculate_maximum_decoded_size(), it is guaranteed, that src_size is
		 * a multiple of four characters */
		first = base64_src[idx++];
		second = base64_src[idx++];
		third = base64_src[idx++];
		fourth = base64_src[idx++];
		if (is_valid_base64_character(first) &&
		is_valid_base64_character(second) &&
		is_valid_base64_character(third) &&
		is_valid_base64_character(fourth)) {
			three_bytes = base64_chars_to_three_bytes(first, second, third, fourth);
			dest[written++] = (three_bytes >> 16) & 0xFF;
			dest[written++] = (three_bytes >> 8) & 0xFF;
			dest[written++] = (three_bytes) & 0xFF;
		} else {
			ret = -3;
			goto exit;
		}

	}

	if (padded) {
		first = base64_src[idx++];
		second = base64_src[idx++];
		third = base64_src[idx++];
		fourth = base64_src[idx++];
		if (!is_valid_base64_character(first) || !is_valid_base64_character(second)) {
			ret = -4;
			goto exit;
		}
		three_bytes = base64_chars_to_three_bytes(first, second, third, fourth);
		dest[written++] = (three_bytes >> 16) & 0xFF;
		if (third != '=')
			dest[written++] = (three_bytes >> 8) & 0xFF;
		if (fourth != '=')
			dest[written++] = (three_bytes) & 0xFF;
	}

exit:
	if (output_written)
		*output_written = written;

	return ret;
}
