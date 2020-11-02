#include <catch.hpp>
#include <string>
#include <iostream>

extern "C" {
#include <base64-lib/base64-lib.h>
}

std::string lorem_ipsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque tristique eu nibh eget efficitur."
			  " Proin pretium, massa in varius malesuada, purus ante pellentesque libero, ut dictum justo enim vel elit."
			  " Nullam convallis lectus non placerat lobortis. Aliquam consequat mi nulla, eget ultricies nibh ullamcorper at."
			  " Ut vitae commodo tellus, ut tempor lectus. Etiam vel cursus purus. Etiam ac sagittis nisl, ac mollis mauris."
			  " Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos."
			  " Fusce imperdiet malesuada tellus sit amet hendrerit."
			  " Suspendisse et lacus vehicula, viverra mauris id, scelerisque diam."
			  " Donec hendrerit mattis neque, non iaculis nisi accumsan quis.";
std::string base64_reference = "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdC4g"
		"UXVpc3F1ZSB0cmlzdGlxdWUgZXUgbmliaCBlZ2V0IGVmZmljaXR1ci4gUHJvaW4gcHJldGl1bSwg"
		"bWFzc2EgaW4gdmFyaXVzIG1hbGVzdWFkYSwgcHVydXMgYW50ZSBwZWxsZW50ZXNxdWUgbGliZXJv"
		"LCB1dCBkaWN0dW0ganVzdG8gZW5pbSB2ZWwgZWxpdC4gTnVsbGFtIGNvbnZhbGxpcyBsZWN0dXMg"
		"bm9uIHBsYWNlcmF0IGxvYm9ydGlzLiBBbGlxdWFtIGNvbnNlcXVhdCBtaSBudWxsYSwgZWdldCB1"
		"bHRyaWNpZXMgbmliaCB1bGxhbWNvcnBlciBhdC4gVXQgdml0YWUgY29tbW9kbyB0ZWxsdXMsIHV0"
		"IHRlbXBvciBsZWN0dXMuIEV0aWFtIHZlbCBjdXJzdXMgcHVydXMuIEV0aWFtIGFjIHNhZ2l0dGlz"
		"IG5pc2wsIGFjIG1vbGxpcyBtYXVyaXMuIENsYXNzIGFwdGVudCB0YWNpdGkgc29jaW9zcXUgYWQg"
		"bGl0b3JhIHRvcnF1ZW50IHBlciBjb251YmlhIG5vc3RyYSwgcGVyIGluY2VwdG9zIGhpbWVuYWVv"
		"cy4gRnVzY2UgaW1wZXJkaWV0IG1hbGVzdWFkYSB0ZWxsdXMgc2l0IGFtZXQgaGVuZHJlcml0LiBT"
		"dXNwZW5kaXNzZSBldCBsYWN1cyB2ZWhpY3VsYSwgdml2ZXJyYSBtYXVyaXMgaWQsIHNjZWxlcmlz"
		"cXVlIGRpYW0uIERvbmVjIGhlbmRyZXJpdCBtYXR0aXMgbmVxdWUsIG5vbiBpYWN1bGlzIG5pc2kg"
		"YWNjdW1zYW4gcXVpcy4=";

TEST_CASE("base64/encode", "[ENCODE]")
{
	int res;
	char base64_encoded[2048];
	size_t output_len;

	res = base64_encode(lorem_ipsum.c_str(), base64_encoded, lorem_ipsum.length(), sizeof(base64_encoded), &output_len);
	REQUIRE(res == 0);

	if (!res) {
		base64_encoded[output_len] = 0;
		REQUIRE(output_len == base64_reference.length());
		REQUIRE(base64_reference == std::string(base64_encoded));
	}
}

TEST_CASE("base64/encoded_size_calc", "[ENCODE]")
{
	size_t res;

	res = base64_calculate_encoded_size(1U);
	REQUIRE(res == 4);

	res = base64_calculate_encoded_size(2U);
	REQUIRE(res == 4);

	res = base64_calculate_encoded_size(3U);
	REQUIRE(res == 4);

	res = base64_calculate_encoded_size(4U);
	REQUIRE(res == 8);

	res = base64_calculate_encoded_size(120U);
	REQUIRE(res == 160);

	res = base64_calculate_encoded_size(119U);
	REQUIRE(res == 160);
}

TEST_CASE("base64/base64_calculate_maximum_decoded_size", "[DECODE]")
{
	REQUIRE(base64_calculate_maximum_decoded_size(3) == 0);
	REQUIRE(base64_calculate_maximum_decoded_size(4) == 3);
	REQUIRE(base64_calculate_maximum_decoded_size(160) == 120);
}

TEST_CASE("base64/base64_decode", "[DECODE]")
{
	char decoded[2048];
	size_t written;
	int ret;

	ret = base64_decode(base64_reference.c_str(), decoded, base64_reference.length(), lorem_ipsum.length(), &written);
	REQUIRE(ret == 0);

	REQUIRE(written == lorem_ipsum.length());

	if (!ret) {
		decoded[written] = 0;
		REQUIRE(std::string(decoded) == lorem_ipsum);
	}
}

TEST_CASE("base64/base64_decode/error_src_to_short", "[DECODE]")
{
	char decoded[2048];
	size_t written;
	int ret;

	ret = base64_decode(base64_reference.c_str(), decoded, base64_reference.length() -1, sizeof(decoded), &written);
	REQUIRE(ret == -1);
	REQUIRE(written == 0);
}

TEST_CASE("base64/base64_decode/error_dest_to_short", "[DECODE]")
{
	char decoded[2048];
	size_t written;
	int ret;

	ret = base64_decode(base64_reference.c_str(), decoded, base64_reference.length(), 12, &written);
	REQUIRE(ret == -2);
	REQUIRE(written == 0);
}

TEST_CASE("base64/base64_decode/invalid_char", "[DECODE]")
{
	char decoded[2048];
	size_t written;
	int ret;

	std::string ref = base64_reference;
	ref[12] = '!';

	ret = base64_decode(ref.c_str(), decoded, ref.length(), 2048, &written);
	REQUIRE(ret == -3);
	REQUIRE(written == 9);
}
