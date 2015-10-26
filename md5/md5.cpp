#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Initialize variables:
const unsigned int h0 = 0x67452301;
const unsigned int h1 = 0xEFCDAB89;
const unsigned int h2 = 0x98BADCFE;
const unsigned int h3 = 0x10325476;

const int SIZE_OF_GROUP = 512;
const int NUM_OF_MIN_GROUP = 16;
const int SIZE_OF_MIN_GROUP = 32;
    
// rotate_bit specifies the per-round shift amounts
const unsigned int rotate_bit[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

// consts 
const unsigned int CONSTS[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};


// datas
int min_groups[16];

// variables
long long fp;
int ep;
int current_group;
bool flag;
bool END_LAST_TIME;
long long len_of_src;
char result[17];
char input[65];

unsigned int A, B, C, D;
unsigned int a0, b0, c0, d0;
unsigned int output[4];

#define F(x,y,z) ((x & y) | (~x & z))  
#define G(x,y,z) ((x & z) | (y & ~z))  
#define H(x,y,z) (x^y^z)  
#define I(x,y,z) (y ^ (x | ~z))  
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))

void init_var() {
    fp = 0;
    current_group = 0;

    a0 = h0;
    b0 = h1;
    c0 = h2;
    d0 = h3;

    flag = true;
    END_LAST_TIME = false;
}

void extend_string(char* src) {
    int i;
    for (i = 0; i < 64 && src[fp] != '\0'; ++i)
    {   
        input[i] = src[fp++];
    }
    if (i != 64) {
        int empty_bytes = 64 - i;

        if (empty_bytes < 8) {
            input[i++] = 0x80;
            empty_bytes--;
            while (empty_bytes) {
                input[i++] = 0x00;
                empty_bytes--;
            }
            END_LAST_TIME = true;
        } else if (!END_LAST_TIME){
            flag = false;
            len_of_src = fp*8;

            if (empty_bytes > 8) {
                input[i++] = 0x80;
                empty_bytes--;
                while (empty_bytes > 8) {
                    input[i++] = 0x00;
                    empty_bytes--;
                }
                
                for (int p = 0; p < 8; ++p)
                {
                    input[i++] = (len_of_src >> (p*8)) & 0xff;
                }
            } else {
                for (int p = 0; p < 8; ++p)
                {
                    input[i++] = (len_of_src >> (p*8)) & 0xff;
                }
            }
        } else {
            flag = false;
            len_of_src = fp*8;

            while(empty_bytes > 8) {
                input[i++] = 0x00;
                empty_bytes--;
            }

            for (int p = 0; p < 8; ++p)
            {
                input[i++] = (len_of_src >> (p*8)) & 0xff;
            }
        }
    }
    input[i] = '\0';
}

void char_to_int(char* src) {
    int k = 0;
    extend_string(src);
    for (int i = 0; i < 16; ++i)
    {
        min_groups[i] = (input[k] & 0xff) |
                        ((input[k+1] << 8) & 0xff00) |
                        ((input[k+2] << 16) & 0xff0000) |
                        ((input[k+3] << 24) & 0xff000000);
        k += 4;
    }
}

void little_to_output() {
    unsigned int tem[4] = {a0, b0, c0, d0};

    for (int i = 0; i < 4; ++i)
    {
        output[i] = tem[i] << 24 |
                    ((tem[i] >> 8) & 0xff) << 16 |
                    ((tem[i] >> 16) & 0xff) << 8 |
                    ((tem[i] >> 24)& 0xff);
    }
}

void get_md5_for_src(char* src) {
    int f;

    init_var();

    while (flag) {
        char_to_int(src);
        int g;
        A = a0;
        B = b0;
        C = c0;
        D = d0;

        for (int i = 0; i < 64; ++i)
        {
            if (i <= 15) {
                f = F(B, C, D);
                g = i;
            } else if (i <= 31) {
                f = G(B, C, D);
                g = (5*i + 1) % 16;
            } else if (i <= 47) {
                f = H(B, C, D);
                g = (3*i + 5) % 16;
            } else {
                f = I(B, C, D);
                g = (7*i) % 16;
            }

            int temp = D;
            D = C;
            C = B;
            B = B + ROTATE_LEFT((A+f+CONSTS[i]+min_groups[g]), rotate_bit[i]);
            A = temp;
        }
        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }
    little_to_output();
}

int main() {
    char input[1000];
    while (scanf("%s", input)) {
        
        get_md5_for_src(input);

        for (int i = 0; i < 4; ++i)
        {
            printf("%08x", output[i]);
        }
        printf("\n");
    }
    return 0;
}

