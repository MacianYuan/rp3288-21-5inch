#include "sha1.h"

void SHA1_Update(void __attribute__((unused)) *sha, unsigned char __attribute__((unused)) *buf, int __attribute__((unused)) size) {}

void SHA1_Final(unsigned char __attribute__((unused)) results[SHA1_DIGEST_LENGTH], void __attribute__((unused)) *sha) {
    int idx;
    
    for (idx = 0 ; idx < SHA1_DIGEST_LENGTH ; idx++)
        results[idx] = 0xa5;
}

void SHA1_Init(void __attribute__((unused)) *sha) {}
