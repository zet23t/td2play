#include <stdio.h>
#include <assert.h>
#include "lib_math.h"

#define F4 FixedNumber16<4>

class TestMath {
public:
    TestMath() {
        testRand();
        testRandVisual();
        testRandRepeat();
    }

    void testRandRepeat() {
        // this test runs a while - it tests the PRNG for repeating patterns
        // it's not a thorough test.
        int seqN = 0x100;
        int seeds[seqN * 2];
        Math::setSeed(0,0);
        for (int i = 0; i<1000000; i+=1) {
            Math::randInt();
            int a,b;
            Math::getSeed(a,b);
            if (i < seqN) {
                seeds[i*2] = a;
                seeds[i*2+1] = b;
            } else {
                for (int j=0;j<seqN;j+=1) {
                    if (seeds[j*2] == a && seeds[j*2+1] == b) {
                        printf("Warning: PRNG repeat after %d iterations\n",i);
                        return;
                    }
                }
            }
            if (a == 0 && b == 0) {
                printf("Warning: PRNG repeat after %d iterations\n",i);
                break;
            }
        }
    }

    void testRandVisual() {
        printf("---Visual rand check---\n");
        Math::setSeed(0,0);
        int cnt[20 * 20];
        memset(cnt,0,sizeof(cnt));
        for (int i=0;i<200;i+=1) {
            int r = Math::randInt();
            cnt[r%400]+=1;
        }
        for (int i=0;i<400;i+=20) {
            for (int j=i; j<i+20;j+=1) {
                printf("%s",cnt[j] == 0 ? " " :"x");
            }
            printf("\n");
        }
    }

    void testRand() {
        for (int j=5;j < 100; j+=1) {
            Math::setSeed(-j,j);
            const int n = j;
            int cnt[n];
            memset(cnt,0,sizeof(cnt));
            for (int i=0;i<n * 100;i+=1) {
                cnt[Math::randInt()%n] += 1;
            }
            for (int i=0;i<n;i+=1) {
                if (cnt[i] < 50) {
                    printf("testRand Warning: n=%d, i=%2d, cnt[i]=%5d\n",n,i,cnt[i]);
                }
            }
        }

    }
};
