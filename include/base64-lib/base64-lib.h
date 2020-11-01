#include <stddef.h>

int base64_encode(const char *src, char *dest, size_t src_size, size_t dest_size, size_t *output_length);

size_t base64_calculate_encoded_size(size_t unencoded_size);

size_t base64_calculate_maximum_decoded_size(size_t encoded_size);
