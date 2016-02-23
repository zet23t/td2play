#ifndef __LIB_GEOM_H__
#define __LIB_GEOM_H__

#include <inttypes.h>

namespace Geom {
    template<class T>
    struct Rect {
        const T x1, y1, x2, y2;
        Rect(T x1, T y1, T x2, T y2):x1(x1),y1(y1),x2(x2),y2(y2) {}
        bool isIntersecting(const Rect &b) const {
            return !(x2 <= b.x1 || b.x2 <= x1 || y2 <= b.y1 || b.y2 <= y1);
        }
    };
}

#endif // __LIB_GEOM_H__
