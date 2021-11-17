#include "camellia.h"
u64 FL(u64 FL_IN, u64 KE){
    u32 x1, x2;
    u32 k1, k2;
    u64 FL_OUT;
    x1 = FL_IN >> 32;
    x2 = FL_IN & MASK32;
    k1 = KE >> 32;
    k2 = KE & MASK32;
    x2 = x2 ^ ((x1 & k1) << 1 | (x1 & k1) >> (32-1));
    x1 = x1 ^ (x2 | k2);
    FL_OUT = (x1 << 32) | x2;
    return FL_OUT;
}

u64 FLINV(u64 FLINV_IN, u64 KE){
    u32 y1, y2;
    u32 k1, k2;
    u64 FLINV_OUT;
    y1 = FLINV_IN >> 32;
    y2 = FLINV_IN & MASK32;
    k1 = KE >> 32;
    k2 = KE & MASK32;
    y1 = y1 ^ (y2 | k2);
    y2 = y2 ^ ((y1 & k1) << 1 | (y1 & k1) >> (32-1));
    FLINV_OUT = (y1 << 32) | y2;
    return FLINV_OUT;
}

u64 F(u64 F_IN, u64 KE){
    u64 x;
    u8 t1,t2,t3,t4,t5,t6,t7,t8;
    u8 y1,y2,y3,y4,y5,y6,y7,y8;

    x = F_IN ^ KE;
    t1 = x >> 56;
    t2 = (x >> 48) & MASK8;
    t2 = (x >> 48) & MASK8;
    t2 = (x >> 48) & MASK8;
    t2 = (x >> 48) & MASK8;
    t2 = (x >> 48) & MASK8;
    t2 = (x >> 48) & MASK8;
    t8 = x         & MASK8;

    t1 = SBOX1[t1];
    t2 = SBOX2[t2];
    t3 = SBOX3[t3];
    t4 = SBOX4[t4];
    t5 = SBOX4[t5];
    t6 = SBOX3[t6];
    t7 = SBOX2[t7];
    t8 = SBOX1[t8];

    y1 = t1 ^ t3 ^ t4 ^ t6 ^ t7 ^ t8;
    y2 = t1 ^ t2 ^ t4 ^ t5 ^ t7 ^ t8;
    y3 = t1 ^ t2 ^ t3 ^ t5 ^ t6 ^ t8;
    y4 = t2 ^ t3 ^ t4 ^ t5 ^ t6 ^ t7;
    y5 = t1 ^ t2 ^ t6 ^ t7 ^ t8;
    y6 = t2 ^ t3 ^ t5 ^ t7 ^ t8;
    y7 = t3 ^ t4 ^ t5 ^ t6 ^ t8;
    y8 = t1 ^ t4 ^ t5 ^ t6 ^ t7;

    u64 F_OUT;
    F_OUT = (y1 << 56) | (y2 << 48) | (y3 << 40) | (y4 << 32) | (y5 << 24) | (y6 << 16) | (y7 << 8) | y8;
    return F_OUT;
}

void gen_SBOX(){
    for(int i = 0;i < 256; ++i){
        printf("%d ,", SBOX1[i] << 1);
        if(i % 16 == 0)
            printf("\\\n");
    }
}

// KL = KL[0] || KL[1]
void init_KA_KB(const u64 *KL, const u64 *KR, u64 *KA, u64 *KB){
    u64 D1, D2;
    const u64 Sigma1 = 0xA09E667F3BCC908B;
    const u64 Sigma2 = 0xB67AE8584CAA73B2;
    const u64 Sigma3 = 0xC6EF372FE94F82BE;
    const u64 Sigma4 = 0x54FF53A5F1D36F1C;
    const u64 Sigma5 = 0x10E527FADE682D1D;
    const u64 Sigma6 = 0xB05688C2B3E6C1FD;
    
    D1 = KL[0] ^ KR[0];  // (KL ^ KR) >> 64
    D2 = KL[1] ^ KR[1];  // (KL ^ KR) & MASK64
    D2 = D2 ^ F(D1, Sigma1);
    D1 = D1 ^ F(D2, Sigma2);
    D1 = D1 ^ (KL[0]);  // D1 ^ (KL >> 64)
    D2 = D2 ^ (KL[1]);  // D2 ^ (KL & MASK64)
    D2 = D2 ^ F(D1, Sigma3);
    D1 = D1 ^ F(D2, Sigma4);
    KA[0] = D1;        // KA = (D1 << 64) | D2
    KA[1] = D2;
    D1 = KA[0] ^ KR[0];  // (KA ^ KR) >> 64
    D2 = KA[1] ^ KR[0];  // (KA ^ KR) & MASK64
    D2 = D2 ^ F(D1, Sigma5);
    D1 = D1 ^ F(D2, Sigma6);
    KB[0] = KA;        // KB = (D1 << 64) | D2
    KB[1] = KB;
}

// generate 64-bit subkeys kw1,..,kw4, k1,...,k18, ke1,...,ke4
void key_scheduling_128(const u64 *key, u64 *kw, u64 *k, u64 *ke){
    u64 KA[2], KB[2];
    u64 KL[2], KR[2];

    KL[0] = key[0];
    KL[1] = key[1];
    KR[0] = 0;
    KR[1] = 0;
    init_KA_KB(KL, KR, KA, KB);  // generate 128-bit varrables KA and KB, which used in key scheduling
    kw[0] = KL[0];
    kw[1] = KL[1];
    k[0] = KA[0];
    k[1] = KA[1];
    k[2] = (KL[0] << 15) | (KL[1] >> (64-15));  // KL <<< 15
    k[3] = (KL[1] << 15) | (KL[0] >> (64-15));
    k[4] = (KA[0] << 15) | (KA[1] >> (64-15));  // KA <<< 15
    k[5] = (KA[1] << 15) | (KA[0] >> (64-15));
    ke[0] = (KA[0] << 30) | (KA[1] >> (64-30));  // KA <<< 30
    ke[1] = (KA[1] << 30) | (KA[0] >> (64-30));
    k[6] = (KL[0] << 45) | (KL[1] >> (64-45));  // KL <<< 45
    k[7] = (KL[1] << 45) | (KL[0] >> (64-45));
    k[8] = (KA[0] << 45) | (KA[1] >> (64-45));  // KA <<< 45 
    k[9] = (KL[1] << 60) | (KL[0] >> (64-60));  // KL <<< 60
    k[10] = (KA[0] << 60) | (KA[1] >> (64-60));  // KA <<< 60
    k[11] = (KA[1] << 60) | (KA[0] >> (64-60));
    ke[2] = (KL[1] << 13) | (KL[0] >> (64-13));  // KL <<< 77
    ke[3] = (KL[0] << 13) | (KL[1] >> (64-13));
    k[12] = (KL[1] << 30) | (KL[0] >> (64-30));  // KL <<< 94
    k[13] = (KL[0] << 30) | (KL[1] >> (64-30));
    k[14] = (KA[1] << 30) | (KA[0] >> (64-30));  // KA <<< 94
    k[15] = (KA[0] << 30) | (KA[1] >> (64-30));
    k[16] = (KL[1] << 47) | (KL[0] >> (64-47));  // KL <<< 111
    k[17] = (KL[0] << 47) | (KL[1] >> (64-47));
    kw[2] = (KA[1] << 47) | (KA[0] >> (64-47));  // KA <<< 111
    kw[3] = (KA[0] << 47) | (KA[1] >> (64-47));
}

u64* encrypt_128k(u64 *data, u64 *key, u64 *c){
    u64 D1, D2;
    u64 kw[4], k[18], ke[4];
    key_scheduling_128(key, kw, k, ke);
    D1 = D1 ^ kw[0];
    D2 = D2 ^ kw[1];
    D2 = D2 ^ F(D1, k[0]);
    D1 = D1 ^ F(D2, k[1]);
    D2 = D2 ^ F(D1, k[2]);
    D1 = D1 ^ F(D2, k[3]);
    D2 = D2 ^ F(D1, k[4]);
    D1 = D1 ^ F(D2, k[5]);
    D1 = FL(D1, ke[0]);     // FL
    D2 = FLINV(D2, ke[1]);  // FLINV
    D2 = D2 ^ F(D1, k[6]);
    D1 = D1 ^ F(D2, k[7]);
    D2 = D2 ^ F(D1, k[8]);
    D1 = D1 ^ F(D2, k[9]);
    D2 = D2 ^ F(D1, k[10]);
    D1 = D1 ^ F(D2, k[11]);
    D1 = FL(D1, ke[2]);     // FL
    D2 = FLINV(D2, ke[3]);  // FLINV
    D2 = D2 ^ F(D1, k[12]);
    D1 = D1 ^ F(D2, k[13]);
    D2 = D2 ^ F(D1, k[14]);
    D1 = D1 ^ F(D2, k[15]);
    D2 = D2 ^ F(D1, k[16]);
    D1 = D1 ^ F(D2, k[17]); 
    D2 = D2 ^ kw[2];
    D1 = D1 ^ kw[3];
    c[0] = D2;
    c[1] = D1;
}

int main(){
    u64 data[2] = {0x0123456789abcdef,0xfedcba9876543210};
    u64 key[2] = {0x0123456789abcdef,0xfedcba9876543210};
    u64 c[2];
    encrypt_128k(data, key, c);
    printf("%lx%lx",c[0],c[1]); 
    gen_SBOX();
    return 0;
}