#include <stdio.h>

class Serial {
public:
    void print(const char* msg) {
        printf("%s",msg);
    }
    void println(const char *msg) {
        printf("%s\n",msg);
    }
    void println(double num) {
        printf("%f\n",num);
    }
};

Serial SerialUSB;
