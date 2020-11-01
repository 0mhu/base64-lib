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

static void base64_init(void)
{
	if (base64_lib_initialized)
		return;

	initialize_encode_lookup_table();

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
