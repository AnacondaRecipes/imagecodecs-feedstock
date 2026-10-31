/* TEMPORARY diagnostic harness: imcd_lzw_decode_size on MSVC ARM64.
   Built by recipe/bld.bat on win-arm64 only. Expected output for a correct build:
     e1 size=40  e2 size=16  e3 size=0  decode ret=40 'say hammer yo hammer mc hammer go hammer'
     refill e1: codes=256,115,97,...   (first 9-bit MSB codes of e1)
*/
#include "imcd.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifdef _MSC_VER
#  include <intrin.h>
#  define SWAP8BYTES(x) _byteswap_uint64(x)
#  define NOINLINE __declspec(noinline)
#else
#  define SWAP8BYTES(x) __builtin_bswap64(x)
#  define NOINLINE __attribute__((noinline))
#endif

static const unsigned char e1[] = {128,28,204,39,145,1,160,194,109,54,153,78,66,3,201,190,11,7,132,194,205,166,56,124,34,20,32,51,195,160,209,99,148,2,2,128};
static const unsigned char e2[] = {128,24,204,38,225,57,208,64,116,55,157,76,102,136,57,160,210,115};
static const unsigned char e3[] = {128,64,64};

/* Same refill/get-code logic as LZW_REFILL_MSB / LZW_GET_NEXT_CODE_MSB in
   imcd.c, copied here so it can be compiled with the same flags and its
   intermediate state (bitcnt/bitpos) printed. */
static NOINLINE void trace_codes(const uint8_t* src, const ssize_t srcsize, int n)
{
    ssize_t bitpos = 0;
    uint64_t bitbuf = 0;
    uint64_t bitw = 9;
    int32_t bitcnt = 0;
    uint32_t code = 0;
    int k;
    for (k = 0; k < n; k++) {
        if (bitcnt < (int)bitw) {
            if (bitpos + 8 <= srcsize) {
                uint64_t c;
                memcpy(&c, src + bitpos, 8);
                bitbuf |= SWAP8BYTES(c) >> bitcnt;
                bitpos += (63 - bitcnt) >> 3;
                bitcnt |= 56;
            }
            else {
                while (bitcnt <= 56 && bitpos < srcsize) {
                    bitbuf |= (uint64_t)src[bitpos++] << (56 - bitcnt);
                    bitcnt += 8;
                }
            }
        }
        if (bitcnt >= (int)bitw) {
            code = (uint32_t)(bitbuf >> (64 - (int)bitw))
                & (((uint32_t)1 << (int)bitw) - 1u);
            bitbuf <<= (int)bitw;
            bitcnt -= (int)bitw;
        }
        else {
            code = 257;
        }
        printf("  code[%d]=%u bitcnt=%d bitpos=%lld\n", k, (unsigned)code,
               (int)bitcnt, (long long)bitpos);
    }
}

int main(void)
{
    imcd_lzw_handle_t* h = imcd_lzw_new();
    unsigned char out[64];
    ssize_t s1, s2, s3, r;
    int bad = 0;

    s1 = imcd_lzw_decode_size(h, e1, (ssize_t)sizeof(e1));
    s2 = imcd_lzw_decode_size(h, e2, (ssize_t)sizeof(e2));
    s3 = imcd_lzw_decode_size(h, e3, (ssize_t)sizeof(e3));
    printf("e1 size=%lld (want 40)\n", (long long)s1);
    printf("e2 size=%lld (want 16)\n", (long long)s2);
    printf("e3 size=%lld (want 0)\n", (long long)s3);
    memset(out, 0, sizeof(out));
    r = imcd_lzw_decode(h, e1, (ssize_t)sizeof(e1), out, 40);
    printf("decode ret=%lld '%.40s'\n", (long long)r, out);
    printf("refill trace e1 (want 256,115,...):\n");
    trace_codes(e1, (ssize_t)sizeof(e1), 4);
    imcd_lzw_del(h);
    if (s1 != 40 || s2 != 16 || s3 != 0 || r != 40) bad = 1;
    printf("RESULT: %s\n", bad ? "BROKEN" : "OK");
    return bad;
}
