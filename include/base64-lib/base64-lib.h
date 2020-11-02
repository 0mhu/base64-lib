#ifndef _BASE64_LIB_H_
#define _BASE64_LIB_H_

#include <stddef.h>

/**
 * @defgroup base64-lib Base64 Encoding / Decoding Library
 * @addtogroup base64-lib
 * @{
 */

/**
 * @brief Encode a byte sequence to base 64
 * @param src The source sequence
 * @param dest The destination to write the base64 encoded data to.
 * @param src_size Source size
 * @param dest_size Available destination space
 * @param output_length Actual count of written data
 * @note The output is not null-terminated
 * @return 0 if successful, -1000 if parameter error, -1 if destination too small,
 */
int base64_encode(const char *src, char *dest, size_t src_size, size_t dest_size, size_t *output_length);

/**
 * @brief Calcualte encoded size of \p unencoded_size amount of unencoded bytes
 * @param unencoded_size Unencoded size
 * @return Encoded size
 */
size_t base64_calculate_encoded_size(size_t unencoded_size);

/**
 * @brief Calculate the maximum decoded size.
 *
 * The true size may be up to 2 bytes smaller, due to possible padding.
 *
 * @param encoded_size Encoded size
 * @return Decoded size
 */
size_t base64_calculate_maximum_decoded_size(size_t encoded_size);

/**
 * @brief Decode a base64 encoded string
 * @param base64_src Source data
 * @param dest Destination buffer
 * @param src_size Source size
 * @param dest_size Destination size
 * @param output_written Actual number of written bytes
 * @return 0 if successful, -1 if input length does not match a base64 string,
 * -2 if destination size is too small, -3 and -4 if a malformed character
 * is found.
 */
int base64_decode(const char *base64_src, char *dest, size_t src_size, size_t dest_size, size_t *output_written);

/** @} */

#endif /* _BASE64_LIB_H_ */
