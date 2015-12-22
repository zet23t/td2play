#include "lib_math.h"
#include <assert.h>
#include <stdio.h>

#define F4 FixedNumber16<4>

class TestMath {
public:
    TestMath() {
        testRand();
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
