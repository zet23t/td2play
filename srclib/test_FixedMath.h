#include <stdio.h>
#include <assert.h>
#include "lib_FixedMath.h"

#define F4 FixedNumber16<4>

class TestFixedMath {
public:
    TestFixedMath() {
        testCompares();
        testAddition();
        testSubtraction();
        testMultiplication();
        testDivision();
        testGettersAndSetters();
        testCompare();
        testSqrt();
        testRandom();
        testRoundingErrorSymmetry();

        test2DCompare();
        test2DAdd();
        test2DSubtract();
        test2DScale();
        test2DRandomCircle();
        test2DSqrLen();
        test2DNormalization();
    }

    void testCompares() {
        assert(F4(7,7) != F4(9,3));
        assert(F4(23,2) == F4(23,2));
    }

    void testAddition() {
        assert(F4(16,10) == F4(7,7) + F4(9,3));
        assert(F4() == F4(-1,4) + F4(0,12));
    }

    void testSubtraction() {
        assert(F4(1,12) == F4(9,3) - F4(7,7));
        assert(F4(-2,4) == F4(7,7) - F4(9,3));
        assert(F4(0,8) == F4(0,15) - F4(0,7));
    }

    void testMultiplication() {
        assert(F4(1,0) == F4(0,8) * F4(2,0));
        assert(F4() == F4(0,0) * F4(2,0));
        assert(F4(-1,0) == F4(0,8) * F4(-2,0));
        assert(F4(1,0) == F4(-1,8) * F4(-2,0));
        assert(F4(2,0) == F4(1,0) * 2);
        assert(F4(-2,0) == F4(1,0) * -2);
        assert(F4(1,0) == F4(-2,0) * F4(-1,8));
        assert(F4(-1,0) == F4(0,8) * F4(-2,0));
        assert(F4(17,0) == F4(1,1) * F4(16,0));
        assert(F4(-17,0) == F4(1,1) * F4(-16,0));
        assert(F4(-17,0) == F4(-2,15) * F4(16,0));
    }

    void testDivision() {
        assert(F4(1,0) == F4(1,0) / F4(1,0));
        assert(F4(2,0) == F4(1,0) / F4(0,8));
        assert(F4(-2,0) == F4(1,0) / F4(-1,8));
    }

    void testRandom() {
        F4 n = F4(0,0);
        F4 fmin = F4(2,0), fmax = F4(-2,0);
        for (int i=0;i<1000;i+=1) {
            n.random(F4(-2,0),F4(2,0));
            if (n < fmin) fmin = n;
            if (n > fmax) fmax = n;
        }
        assert(fmax == F4(2,0));
        assert(fmin == F4(-2,0));

    }

    void testRoundingErrorSymmetry() {
        {
            F4 a = F4(1,3);
            F4 b = F4(0,8);
            F4 c = a * b;
            assert(F4(0,9) == c);
        }
        {
            F4 a = F4(1,3);
            F4 b = F4(-1,8);
            F4 c = a * b;
            assert(F4(-1,7) == c);
        }
    }

    void testSqrt() {
        assert(F4(1,0).sqrt() == F4(1,0));
        assert(F4(0,0).sqrt() == F4(0,0));
        assert(F4(4,0).sqrt() == F4(2,0));
        assert(F4(81,0).sqrt() == F4(9,0));
        assert((F4(21,0).sqrt() - F4(4,8)).absolute() < F4(0,2));
        assert(F4(30*30,0).sqrt() == F4(30,0));
    }

    void test2DCompare() {
        assert(Fixed2D4(4,1) == Fixed2D4(4,1));
        assert(!(Fixed2D4(4,2) == Fixed2D4(4,1)));
        assert(!(Fixed2D4(3,1) == Fixed2D4(4,1)));
        assert(Fixed2D4(4,2) != Fixed2D4(4,1));
        assert(Fixed2D4(5,1) != Fixed2D4(4,1));
        assert(!(Fixed2D4(4,2) != Fixed2D4(4,2)));
    }

    void test2DAdd() {
        assert(Fixed2D4(5,2,8,1) == Fixed2D4(3,2,4,0) + Fixed2D4(2,0,4,1));
    }

    void test2DSubtract() {
        assert(Fixed2D4(5,2,7,6) == Fixed2D4(10,4,9,6) - Fixed2D4(5,2,2,0));
    }

    void test2DScale() {
        assert(Fixed2D4(1,2) == Fixed2D4(2,1).scale(Fixed2D4(0,8,2,0)));
        assert(Fixed2D4(2,0,0,0) == Fixed2D4(2,0,0,0).scale(1,0));
        assert(Fixed2D4(3,0,0,0) == Fixed2D4(2,0,0,0).scale(1,8));
        assert(Fixed2D4(-3,0,0,0) == Fixed2D4(-2,0,0,0).scale(1,8));

        {

            Fixed2D4 a = Fixed2D4(0,0);
            Fixed2D4 b = Fixed2D4(0,8,0,0);
            a += b * FixedNumber16<4>(2,0);
            a.scale(0,8);
            assert(Fixed2D4(0,8,0,0) == a);
            a += b * FixedNumber16<4>(2,0);
            a.scale(0,8);
            assert(Fixed2D4(0,12,0,0) == a);
            printf("%f\n",a.x.asFloat());
        }
        {

            Fixed2D4 a = Fixed2D4(0,0);
            Fixed2D4 b = Fixed2D4(-1,8,0,0);
            a += b * FixedNumber16<4>(2,0);
            a.scale(0,8);
            assert(Fixed2D4(-1,8,0,0) == a);
            a += b * FixedNumber16<4>(2,0);
            a.scale(0,8);
            assert(Fixed2D4(-1,4,0,0) == a);
            printf("%f\n",a.x.asFloat());
        }
    }

    void testGettersAndSetters() {
        assert(F4(5,9).getIntegerPart() == 5);
        assert(F4(5,9).getFractionPart() == 9);
        assert(F4(5,9).setFractionPart(3).getFractionPart() == 3);
        assert(F4(5,9).setFractionPart(3).getIntegerPart() == 5);
        assert(F4(5,9).setIntegerPart(2).getFractionPart() == 9);
        assert(F4(5,9).setIntegerPart(2).getIntegerPart() == 2);
    }

    void test2DRandomCircle() {
        int hit[400];
        memset(hit,0,sizeof(hit));
        printf("--- test2DRandomCircle ---\n");
        for (int i=0;i<200;i+=1) {
            Fixed2D4 f;
            f.randomCircle(FixedNumber16<4>(8,0));
            int x = f.x.getIntegerPart() + 10;
            int y = f.y.getIntegerPart() + 10;
            if (x < 0 || x >= 20 || y < 0 || y >= 20) {
            //    assert(0);
            } else {
                hit[x + y * 20] = 1;
            }
        }
        for (int i=0;i<400;i+=1) {
            printf(hit[i] ? "x" : " ");
            if (i % 20 == 19) printf("\n");
        }
    }

    void testCompare() {
        assert(F4(0,8) < F4(0,10));
        assert(!(F4(0,8) > F4(0,10)));
        assert(F4(-1,8) < F4(-1,10));
        assert(F4(-1,8) >= F4(-1,8));
        assert(F4(-1,8) < F4(1,8));
        assert(F4(2,8) >= F4(1,8));
    }

    void test2DSqrLen() {
        assert(FixedNumber16<4>(1,0) == Fixed2D4(1,0).calcSqrLen());
        assert(FixedNumber16<4>(1,0) == Fixed2D4(0,1).calcSqrLen());
        assert(FixedNumber16<4>(1,0) == Fixed2D4(0,-1).calcSqrLen());
        assert(FixedNumber16<4>(8,0) == Fixed2D4(2,2).calcSqrLen());
    }

    void test2DNormalization() {
        assert(FixedNumber16<4>(1,0) == Fixed2D4(1,0).normalize().calcSqrLen());
        assert(FixedNumber16<4>(1,0) == Fixed2D4(-1,0).normalize().calcSqrLen());
        assert(FixedNumber16<4>(1,0) == Fixed2D4(0,1).normalize().calcSqrLen());
        assert(FixedNumber16<4>(1,0) == Fixed2D4(0,-1).normalize().calcSqrLen());
        assert(FixedNumber16<4>(1,0) == Fixed2D4(F4(0,1),F4(-1,0)).normalize().calcSqrLen());
        assert(FixedNumber16<4>(1,0) == Fixed2D4(F4(-1,0),F4(0,0)).normalize().calcSqrLen());
    }
};
