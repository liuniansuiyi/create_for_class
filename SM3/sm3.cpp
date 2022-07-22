// Sample 1
// Input:"abc"
// Output:66c7f0f4 62eeedd9 d1f2d46b dc10e4e2 4167c487 5cf2f7a2 297da02b 8f4ba8e0

// Sample 2
// Input:"abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd"
// Outpuf:debe9ff9 2275b8a1 38604889 c18e5a4d 6fdb70e5 387e5765 293dcba3 9c0c5732

#include "sm3.h"
#include <cstring>
# include <immintrin.h>

/*
 * 32-bit integer manipulation macros (big endian)
 */

/* unsigned char b[i] = unsigned long n 的i  -  i+8位 */

void PUT_ULONG_BE(unsigned long n, unsigned char* b, int i)
{
    (b)[i    ] = (unsigned char) ( (n) >> 24 );
    (b)[i + 1] = (unsigned char) ( (n) >> 16 );
    (b)[i + 2] = (unsigned char) ( (n) >>  8 );
    (b)[i + 3] = (unsigned char) ( (n)       );
}

/*
 * SM3 context setup
 */
void sm3_starts( sm3_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x7380166F;
    ctx->state[1] = 0x4914B2B9;
    ctx->state[2] = 0x172442D7;
    ctx->state[3] = 0xDA8A0600;
    ctx->state[4] = 0xA96F30BC;
    ctx->state[5] = 0x163138AA;
    ctx->state[6] = 0xE38DEE4D;
    ctx->state[7] = 0xB0FB0E4E;
}

static void sm3_process(sm3_context* ctx, unsigned char data[64])
{
    // data[64],一个char是1子节 8bit,64刚好是512bit是一组
    int j;
    unsigned long SS1, SS2, TT1, TT2, W[68], W1[64];
    unsigned long A, B, C, D, E, F, G, H;
    unsigned long T[64];
    __m128i X, K, R;
    __m128i M = _mm_setr_epi32(0, 0, 0, 0xffffffff);
    __m128i V = _mm_setr_epi8(3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12);
    // M用于取X的前三个数据,V用于将X进行大端排序
#ifdef _DEBUG
    int i;
#endif

    // constant T
    for (j = 0; j < 16; j += 4)
    {
        T[j] = 0x79CC4519;
        T[j + 1] = 0x79CC4519;
        T[j + 2] = 0x79CC4519;
        T[j + 3] = 0x79CC4519;
    }
    for (j = 16; j < 64; j += 4)
    {
        T[j] = 0x7A879D8A;
        T[j + 1] = 0x7A879D8A;
        T[j + 2] = 0x7A879D8A;
        T[j + 3] = 0x7A879D8A;
    }

    for (j = 0; j < 16; j += 4) {
        X = _mm_loadu_si128((__m128i*)(data + j * 4));      // 取X为
        X = _mm_shuffle_epi8(X, V);
        _mm_storeu_si128((__m128i*)(W + j), X);
    }

#ifdef _DEBUG
    printf("Message with padding:\n");
    for (i = 0; i < 8; i++)
        printf("%08lx ", W[i]);
    printf("\n");
    for (i = 8; i < 16; i++)
        printf("%08lx ", W[i]);
    printf("\n");
#endif


    //消息扩展
    for (j = 16; j < 68; j += 4) {
        /* X = (W[j - 3], W[j - 2], W[j - 1], 0) */
        X = _mm_loadu_si128((__m128i*)(W + j - 3));
        X = _mm_andnot_si128(M, X);

        X = _mm_xor_si128(_mm_slli_epi32((X),(15)), _mm_srli_epi32((X),32-(15)));
        K = _mm_loadu_si128((__m128i*)(W + j - 9));
        X = _mm_xor_si128(X, K);
        K = _mm_loadu_si128((__m128i*)(W + j - 16));
        X = _mm_xor_si128(X, K);

        /* P1() */
        K = _mm_xor_si128(_mm_slli_epi32((X),(23 - 15)), _mm_srli_epi32((X),32-(23 - 15)));
        K = _mm_xor_si128(K, X);
        K = _mm_xor_si128(_mm_slli_epi32((K),(15)), _mm_srli_epi32((K),32-(15)));
        X = _mm_xor_si128(X, K);

        K = _mm_loadu_si128((__m128i*)(W + j - 13));
        K = _mm_xor_si128(_mm_slli_epi32((K),(7)), _mm_srli_epi32((K),32-(7)));
        X = _mm_xor_si128(X, K);
        K = _mm_loadu_si128((__m128i*)(W + j - 6));
        X = _mm_xor_si128(X, K);

        R = _mm_shuffle_epi32(X, 0);
        R = _mm_and_si128(R, M);
        K = _mm_xor_si128(_mm_slli_epi32((R),(15)), _mm_srli_epi32((R),32-(15)));
        K = _mm_xor_si128(K, R);
        K = _mm_xor_si128(_mm_slli_epi32((K),(9)), _mm_srli_epi32((K),32-(9)));
        R = _mm_xor_si128(R, K);
        R = _mm_xor_si128(_mm_slli_epi32((R),(6)), _mm_srli_epi32((R),32-(6)));
        X = _mm_xor_si128(X, R);

        _mm_storeu_si128((__m128i*)(W + j), X);
    }

#ifdef _DEBUG
    printf("Expanding message W0-67:\n");
    for (i = 0; i < 68; i++)
    {
        printf("%08x ", W[i]);
        if (((i + 1) % 8) == 0) printf("\n");
    }
    printf("\n");
#endif

    // loop unwinding
    for (j = 0; j < 64; j += 4)
    {
        W1[j] = W[j] ^ W[j + 4];
        W1[j + 1] = W[j + 1] ^ W[j + 5];
        W1[j + 2] = W[j + 2] ^ W[j + 6];
        W1[j + 3] = W[j + 3] ^ W[j + 7];
    }

#ifdef _DEBUG
    printf("Expanding message W'0-63:\n");
    for (i = 0; i < 64; i++)
    {
        printf("%08x ", W1[i]);
        if (((i + 1) % 8) == 0) printf("\n");
    }
    printf("\n");
#endif

    //迭代压缩
    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];
    F = ctx->state[5];
    G = ctx->state[6];
    H = ctx->state[7];
#ifdef _DEBUG
    printf("j     A       B        C         D         E        F        G       H\n");
    printf("   %08x %08x %08x %08x %08x %08x %08x %08x\n", A, B, C, D, E, F, G, H);
#endif

    // 压缩函数
    for (j = 0; j < 16; j++)
    {
        SS1 = ROTL((ROTL(A, 12) + E + ROTL(T[j], j)), 7);
        SS2 = SS1 ^ ROTL(A, 12);
        TT1 = FF0(A, B, C) + D + SS2 + W1[j];
        TT2 = GG0(E, F, G) + H + SS1 + W[j];
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = P0(TT2);
#ifdef _DEBUG
        printf("%02d %08x %08x %08x %08x %08x %08x %08x %08x\n", j, A, B, C, D, E, F, G, H);
#endif
    }

    for (j = 16; j < 64; j++)
    {
        SS1 = ROTL((ROTL(A, 12) + E + ROTL(T[j], j)), 7);
        SS2 = SS1 ^ ROTL(A, 12);
        TT1 = FF1(A, B, C) + D + SS2 + W1[j];
        TT2 = GG1(E, F, G) + H + SS1 + W[j];
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = P0(TT2);
#ifdef _DEBUG
        printf("%02d %08x %08x %08x %08x %08x %08x %08x %08x\n", j, A, B, C, D, E, F, G, H);
#endif
    }

    ctx->state[0] ^= A;
    ctx->state[1] ^= B;
    ctx->state[2] ^= C;
    ctx->state[3] ^= D;
    ctx->state[4] ^= E;
    ctx->state[5] ^= F;
    ctx->state[6] ^= G;
    ctx->state[7] ^= H;
#ifdef _DEBUG
    printf("   %08x %08x %08x %08x %08x %08x %08x %08x\n", ctx->state[0], ctx->state[1], ctx->state[2],
        ctx->state[3], ctx->state[4], ctx->state[5], ctx->state[6], ctx->state[7]);
#endif
}

/*
 * SM3 process buffer
 */
void sm3_update( sm3_context *ctx, unsigned char *input, int ilen )
{
    int fill;
    unsigned long left;

    if( ilen <= 0 )
        return;

    left = ctx->total[0] & 0x3F;    // 从ctx->buffer左边第几位开始复制, & 0x3F 为了截取最后一个块
    fill = 64 - left;               // 需要填充的位数

    ctx->total[0] += ilen;          // ilen是有意义的填充长度(相较于fill而言,fill中从ilen之后的填充都是无意义的),ctx->total[0]代表现在已经填充了的有意义的长度

    // if 成立的条件是:ctx->total[0]在加这一轮ilen后超过了0xFFFFFFFF,此时已经在填充下一个B,因此ctx->total[1] += 1
    if( ctx->total[0] < (unsigned long) ilen )
        ctx->total[1]++;

    // 填充的左端点不是0,保证不是在填充消息内容,有意义的填充长度大于等于需要填充的位数
    if( left && ilen >= fill )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );     // 在最后填充消息长度
        sm3_process( ctx, ctx->buffer );        // 将最后一段B进行加密
        input += fill;
        ilen  -= fill;
//        left = 0;                                   // 猜测,清理内存,这里最后一个B加密后应该没有后续内容了的
    }


    while( ilen >= 64 )     // 如果消息长度本身大于64,541bit,那么直接拿前64字节进行sm3一轮加密,然后取input的64字节后的内容作为输入,长度-64
    {
        sm3_process( ctx, input );
        input += 64;
        ilen  -= 64;
    }

    if( ilen > 0 )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, ilen );     // 将input的ilen长度填充给ctx->buffer的left开始的位置
    }
}

static const unsigned char sm3_padding[64] =
        {
                0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };

/*
 * SM3 final digest
 */
void sm3_finish( sm3_context *ctx, unsigned char output[32] )
{
    unsigned long last, padn;
    unsigned long high, low;
    unsigned char msglen[8];        // 消息长度

    high = ( ctx->total[0] >> 29 )
           | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );     // ctx->total[0]字符数,一个字符8bit,左移三位

    PUT_ULONG_BE( high, msglen, 0 );
    PUT_ULONG_BE( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );      // 若长度大于等于56,则需要在下一个块中填充数据长度

    sm3_update( ctx, (unsigned char *) sm3_padding, padn );     // 用padding对ctx->buffer进行填充
    sm3_update( ctx, msglen, 8 );                               // 用msglen对ctx->buffer进行填充

    PUT_ULONG_BE( ctx->state[0], output,  0 );
    PUT_ULONG_BE( ctx->state[1], output,  4 );
    PUT_ULONG_BE( ctx->state[2], output,  8 );
    PUT_ULONG_BE( ctx->state[3], output, 12 );
    PUT_ULONG_BE( ctx->state[4], output, 16 );
    PUT_ULONG_BE( ctx->state[5], output, 20 );
    PUT_ULONG_BE( ctx->state[6], output, 24 );
    PUT_ULONG_BE( ctx->state[7], output, 28 );
}

/*
 * output = SM3( input buffer )
 */
void sm3( unsigned char *input, int ilen,
          unsigned char output[32] )
{
    sm3_context ctx;

    sm3_starts( &ctx );     // init ctx
    sm3_update( &ctx, input, ilen );
    sm3_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sm3_context ) );
}