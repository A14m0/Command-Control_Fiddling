#include "b64.h"

/* returns the size of the buffer 
given the `inlen` of the raw buffer */
size_t B64::enc_size(size_t inlen)
{
	size_t ret;

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}

/* returns the size of the decoded buffer
given the `in` string */
size_t B64::dec_size(const char *in)
{
	size_t len;
	size_t ret;
	size_t i;

	if (in == NULL)
		return 0;

	len = strlen(in);
	ret = len / 4 * 3;

	for (i=len; i-->0; ) {
		if (in[i] == '=') {
			ret--;
		} else {
			break;
		}
	}

	return ret;
}

/*Encodes the `in` buffer of size `len` 
and writes encoded string to `buff`*/
void B64::encode(const unsigned char *in, size_t len, char **buff)
{
	char   *out;
	size_t  elen;
	size_t  i;
	size_t  j;
	size_t  v;

	if (in == NULL || len == 0){
		printf("INNODE IS NULL!\n");
		return;
	}
	
	elen = B64::enc_size(len);
	out  = (char*)malloc(elen+1);
	out[elen] = '\0';

	for (i=0, j=0; i<len; i+=3, j+=4) {
		v = in[i];
		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

		out[j]   = B64::b64chars[(v >> 18) & 0x3F];
		out[j+1] = B64::b64chars[(v >> 12) & 0x3F];
		if (i+1 < len) {
			out[j+2] = B64::b64chars[(v >> 6) & 0x3F];
		} else {
			out[j+2] = '=';
		}
		if (i+2 < len) {
			out[j+3] = B64::b64chars[v & 0x3F];
		} else {
			out[j+3] = '=';
		}
	}

	*buff = out;
	return;
}

/* Checks if `c` is a valid character */
int B64::isvalidchar(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c >= 'a' && c <= 'z')
		return 1;
	if (c == '+' || c == '/' || c == '=')
		return 1;
	return 0;
}

/* Decodes `in` and writes to `out`, storing length in `outlen`*/
int B64::decode(const char *in, unsigned char *out, size_t outlen)
{
	size_t len;
	size_t i;
	size_t j;
	int    v;

	if (in == NULL || out == NULL)
		return 0;

	len = strlen(in);
	if (outlen < B64::dec_size(in) || len % 4 != 0)
		return 0;

	for (i=0; i<len; i++) {
		if (!B64::isvalidchar(in[i])) {
			return 0;
		}
	}

	for (i=0, j=0; i<len; i+=4, j+=3) {
		v = B64::b64invs[in[i]-43];
		v = (v << 6) | B64::b64invs[in[i+1]-43];
		v = in[i+2]=='=' ? v << 6 : (v << 6) | B64::b64invs[in[i+2]-43];
		v = in[i+3]=='=' ? v << 6 : (v << 6) | B64::b64invs[in[i+3]-43];

		out[j] = (v >> 16) & 0xFF;
		if (in[i+2] != '=')
			out[j+1] = (v >> 8) & 0xFF;
		if (in[i+3] != '=')
			out[j+2] = v & 0xFF;
	}

	return 1;
}

const char B64::b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const int B64::b64invs[] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };