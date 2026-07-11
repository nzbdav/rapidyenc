#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../rapidyenc.h"

#define CHECK(condition) do { \
	if(!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while(0)

static int test_version(void) {
	CHECK(rapidyenc_version() == RAPIDYENC_VERSION);
	CHECK(rapidyenc_encode_max_length(1024, 128) >= 2048);
	return 0;
}

#if !defined(RAPIDYENC_DISABLE_ENCODE) && !defined(RAPIDYENC_DISABLE_DECODE)
static int test_round_trips(void) {
	unsigned char input[1024];
	unsigned char *encoded;
	unsigned char *decoded;
	unsigned char *in_place;
	size_t capacity;
	size_t encoded_length;
	size_t decoded_length;
	size_t i;
	int column;
	const size_t split = 63;

	for(i = 0; i < sizeof(input); i++)
		input[i] = (unsigned char)(i * 73u + (i >> 2));
	input[split - 1] = ' ';
	input[split] = '\t';
	input[sizeof(input) - 2] = ' ';
	input[sizeof(input) - 1] = '\t';

	capacity = rapidyenc_encode_max_length(sizeof(input), 128);
	encoded = (unsigned char*)malloc(capacity);
	decoded = (unsigned char*)malloc(sizeof(input));
	in_place = (unsigned char*)malloc(capacity);
	CHECK(encoded != NULL);
	CHECK(decoded != NULL);
	CHECK(in_place != NULL);

	encoded_length = rapidyenc_encode(input, encoded, sizeof(input));
	decoded_length = rapidyenc_decode(encoded, decoded, encoded_length);
	CHECK(decoded_length == sizeof(input));
	CHECK(memcmp(decoded, input, sizeof(input)) == 0);

	memcpy(in_place, encoded, encoded_length);
	decoded_length = rapidyenc_decode_ex(1, in_place, in_place, encoded_length, NULL);
	CHECK(decoded_length == sizeof(input));
	CHECK(memcmp(in_place, input, sizeof(input)) == 0);

	column = 0;
	encoded_length = rapidyenc_encode_ex(
		128, &column, input, encoded, split, 0);
	encoded_length += rapidyenc_encode_ex(
		128, &column, input + split, encoded + encoded_length,
		sizeof(input) - split, 1);
	decoded_length = rapidyenc_decode(encoded, decoded, encoded_length);
	CHECK(decoded_length == sizeof(input));
	CHECK(memcmp(decoded, input, sizeof(input)) == 0);

	free(in_place);
	free(decoded);
	free(encoded);
	return 0;
}

static int test_line_length_edges(void) {
	static const unsigned char input[] = {
		0, '\r', '\n', '=', '.', ' ', '\t', 255, 1, 2, 3, 4
	};
	const int line_sizes[] = {1, 2, 7, 128};
	unsigned char *encoded;
	unsigned char decoded[sizeof(input)];
	size_t capacity;
	size_t encoded_length;
	size_t decoded_length;
	size_t i;

	for(i = 0; i < sizeof(line_sizes) / sizeof(line_sizes[0]); i++) {
		capacity = rapidyenc_encode_max_length(sizeof(input), line_sizes[i]);
		encoded = (unsigned char*)malloc(capacity);
		CHECK(encoded != NULL);
		encoded_length = rapidyenc_encode_ex(
			line_sizes[i], NULL, input, encoded, sizeof(input), 1);
		decoded_length = rapidyenc_decode(encoded, decoded, encoded_length);
		CHECK(decoded_length == sizeof(input));
		CHECK(memcmp(decoded, input, sizeof(input)) == 0);
		free(encoded);
	}
	return 0;
}
#endif

#if !defined(RAPIDYENC_DISABLE_DECODE)
static int test_control_end_state(void) {
	static const unsigned char first[] = {'K', '\r'};
	static const unsigned char lf[] = {'\n'};
	static const unsigned char equals[] = {'='};
	static const unsigned char end[] = {'y', 'r', 'e', 's', 't'};
	unsigned char output[8];
	const void *src;
	void *dest = output;
	RapidYencDecoderState state = RYDEC_STATE_CRLF;
	RapidYencDecoderEnd result;

	src = first;
	result = rapidyenc_decode_incremental(&src, &dest, sizeof(first), &state);
	CHECK(result == RYDEC_END_NONE);
	CHECK(state == RYDEC_STATE_CR);
	CHECK((const unsigned char*)src == first + sizeof(first));
	CHECK((unsigned char*)dest == output + 1);
	CHECK(output[0] == (unsigned char)('K' - 42));

	src = lf;
	result = rapidyenc_decode_incremental(&src, &dest, sizeof(lf), &state);
	CHECK(result == RYDEC_END_NONE);
	CHECK(state == RYDEC_STATE_CRLF);

	src = equals;
	result = rapidyenc_decode_incremental(&src, &dest, sizeof(equals), &state);
	CHECK(result == RYDEC_END_NONE);
	CHECK(state == RYDEC_STATE_CRLFEQ);

	src = end;
	result = rapidyenc_decode_incremental(&src, &dest, sizeof(end), &state);
	CHECK(result == RYDEC_END_CONTROL);
	CHECK((const unsigned char*)src == end + 1);
	CHECK((unsigned char*)dest == output + 1);
	return 0;
}

static int test_article_end_state(void) {
	static const unsigned char first[] = {'K', '\r', '\n'};
	static const unsigned char dot[] = {'.'};
	static const unsigned char cr[] = {'\r'};
	static const unsigned char end[] = {'\n', 'r', 'e', 's', 't'};
	unsigned char output[8];
	const void *src;
	void *dest = output;
	RapidYencDecoderState state = RYDEC_STATE_CRLF;
	RapidYencDecoderEnd result;

	src = first;
	result = rapidyenc_decode_incremental(&src, &dest, sizeof(first), &state);
	CHECK(result == RYDEC_END_NONE);
	CHECK(state == RYDEC_STATE_CRLF);
	CHECK((unsigned char*)dest == output + 1);
	CHECK(output[0] == (unsigned char)('K' - 42));

	src = dot;
	result = rapidyenc_decode_incremental(&src, &dest, sizeof(dot), &state);
	CHECK(result == RYDEC_END_NONE);
	CHECK(state == RYDEC_STATE_CRLFDT);

	src = cr;
	result = rapidyenc_decode_incremental(&src, &dest, sizeof(cr), &state);
	CHECK(result == RYDEC_END_NONE);
	CHECK(state == RYDEC_STATE_CRLFDTCR);

	src = end;
	result = rapidyenc_decode_incremental(&src, &dest, sizeof(end), &state);
	CHECK(result == RYDEC_END_ARTICLE);
	CHECK((const unsigned char*)src == end + 1);
	CHECK((unsigned char*)dest == output + 1);
	return 0;
}
#endif

#if !defined(RAPIDYENC_DISABLE_CRC)
static int test_crc32(void) {
	static const unsigned char vector[] = "123456789";
	static const unsigned char prefix[] = "1234";
	static const unsigned char suffix[] = "56789";
	static const unsigned char abc[] = "abc";
	unsigned char zero_extended[sizeof(abc) - 1 + 17];
	uint32_t vector_crc;
	uint32_t prefix_crc;
	uint32_t suffix_crc;
	uint32_t abc_crc;
	uint32_t extended_crc;
	uint32_t one;
	uint32_t power;
	size_t i;

	vector_crc = rapidyenc_crc(vector, sizeof(vector) - 1, 0);
	CHECK(vector_crc == UINT32_C(0xcbf43926));
	CHECK(rapidyenc_crc(vector + 4, 5, rapidyenc_crc(vector, 4, 0)) == vector_crc);

	prefix_crc = rapidyenc_crc(prefix, sizeof(prefix) - 1, 0);
	suffix_crc = rapidyenc_crc(suffix, sizeof(suffix) - 1, 0);
	CHECK(rapidyenc_crc_combine(prefix_crc, suffix_crc, sizeof(suffix) - 1) == vector_crc);

	memcpy(zero_extended, abc, sizeof(abc) - 1);
	for(i = sizeof(abc) - 1; i < sizeof(zero_extended); i++)
		zero_extended[i] = 0;
	abc_crc = rapidyenc_crc(abc, sizeof(abc) - 1, 0);
	extended_crc = rapidyenc_crc(zero_extended, sizeof(zero_extended), 0);
	CHECK(rapidyenc_crc_zeros(abc_crc, 17) == extended_crc);
	CHECK(rapidyenc_crc_unzero(extended_crc, 17) == abc_crc);

	one = rapidyenc_crc_2pow(0);
	CHECK(one == UINT32_C(0x80000000));
	CHECK(rapidyenc_crc_multiply(vector_crc, one) == vector_crc);
	CHECK(rapidyenc_crc_2pow(8 * 11) == rapidyenc_crc_256pow(11));
	power = rapidyenc_crc_multiply(
		rapidyenc_crc_2pow(19), rapidyenc_crc_2pow(23));
	CHECK(power == rapidyenc_crc_2pow(42));
	CHECK(rapidyenc_crc_multiply(
		rapidyenc_crc_2pow(31), rapidyenc_crc_2pow(-31)) == one);
	return 0;
}
#endif

int main(void) {
	if(test_version()) return 1;

#ifndef RAPIDYENC_DISABLE_ENCODE
	rapidyenc_encode_init();
#endif
#ifndef RAPIDYENC_DISABLE_DECODE
	rapidyenc_decode_init();
#endif
#ifndef RAPIDYENC_DISABLE_CRC
	rapidyenc_crc_init();
#endif

#if !defined(RAPIDYENC_DISABLE_ENCODE) && !defined(RAPIDYENC_DISABLE_DECODE)
	if(test_round_trips()) return 1;
	if(test_line_length_edges()) return 1;
#endif
#ifndef RAPIDYENC_DISABLE_DECODE
	if(test_control_end_state()) return 1;
	if(test_article_end_state()) return 1;
#endif
#ifndef RAPIDYENC_DISABLE_CRC
	if(test_crc32()) return 1;
#endif

	return 0;
}
