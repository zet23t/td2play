#include <test_FixedMath.h>
#include <test_math.h>

int analogRead(int) {
    return 0;
}
int main() {
    TestFixedMath();
    TestMath();
    return 0;
}
