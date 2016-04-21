#include "lib_FixedMath.h"
#include "lib_math.h"
template<uint8_t shiftNum>
void FixedNumber16<shiftNum>::random(const FixedNumber16<shiftNum>& min, const FixedNumber16<shiftNum>& max) {
    const FixedNumber16<shiftNum> range = max - min;
    number = Math::randInt();
    if (number < 0) number = -number;
    number%= range.number + 1;
    number+= min.number;
}

Fixed2D4 Fixed2D4::randomCircle(const FixedNumber16<4>& radius) const {
    Fixed2D4 cp = *this;
    FixedNumber16<4> r2 = radius * radius;
    while(1) {
        cp.x.random(-radius,radius);
        cp.y.random(-radius,radius);
        FixedNumber16<4> dist = cp.x*cp.x + cp.y*cp.y;
        if (dist <= r2) break;
    }
    return cp;
}
