typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[8];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

}
        sm3_context;

inline unsigned long FF0(unsigned long x, unsigned long y, unsigned long z) {    return x ^ y ^ z;      }

inline unsigned long FF1(unsigned long x, unsigned long y, unsigned long z) {    return (x & y) | (x & z) | (y & z);     }

inline unsigned long GG0(unsigned long x, unsigned long y, unsigned long z) {    return x ^ y ^ z;      }

inline unsigned long GG1(unsigned long x, unsigned long y, unsigned long z) {    return (x & y) | (~x & z);      }

inline unsigned long SHL(unsigned long x, int n) {    return (x & 0xFFFFFFFF) << n;      }

inline unsigned long ROTL(unsigned long x, int n) {    return SHL(x, n) | (x >> (32 - n));      }

inline unsigned long P0(unsigned long x) {    return x ^ ROTL(x, 9) ^ ROTL(x, 17);     }

void sm3_starts( sm3_context *ctx );

void sm3_update( sm3_context *ctx, unsigned char *input, int ilen );

void sm3_finish( sm3_context *ctx, unsigned char output[32] );

void sm3( unsigned char *input, int ilen,
          unsigned char output[32]);