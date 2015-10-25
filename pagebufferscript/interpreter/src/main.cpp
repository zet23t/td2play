#include <stdio.h>

#define PSC_MAX_STACKDEPTH 32
#define PSC_MAX_STACKSIZE 512

#define OP_push8bitLiteral 0x01
#define OP_push16bitLiteral 0x02
#define OP_push32bitLiteral 0x03
#define OP_pushStringLiteral 0x04
#define OP_push8bitValue 0x05
#define OP_push16bitValue 0x06
#define OP_pushStackValueUint8 0x07
#define OP_opMulUint8Uint8Uint16 0x08
#define OP_returnUint16 0x09
#define OP_call 0x10
#define OP_callNative 0x11
#define OP_returnVoid 0x12
#define OP_moveFunctionStackOffset 0x13
#define OP_castUint16Int16 0x14
#define OP_castUint16Int32 0x15


class PSCvm;

typedef void PSCVMFunc (PSCvm* vm);

class PSCvmFuncRecord {
public:
	PSCVMFunc *func;
	unsigned short bytesArgument;
	unsigned short bytesReturn;

	PSCvmFuncRecord (PSCVMFunc *func, unsigned short bytesArgument, unsigned short bytesReturn) {
        this->func = func;
        this->bytesArgument = bytesArgument;
        this->bytesReturn = bytesReturn;
	}
};

class PSCStackFrame {
public:
	unsigned char *functionStackOffset;
	unsigned char *stackPointer;
	unsigned char *instructionPointer;
};

class PSCvm {
private:
	PSCvmFuncRecord *nativeFunctionList;
	PSCStackFrame *currentFrame;
	unsigned char *code;
	unsigned short codeSize;
	PSCStackFrame frames[PSC_MAX_STACKDEPTH];
	unsigned char stack[PSC_MAX_STACKSIZE];

    void opPush8bitLiteral() {
        printf("  %s\n",__FUNCTION__);
        currentFrame->stackPointer++[0] = currentFrame->instructionPointer++[0];
    }

    void opCall() {
        int targetOffset = *(unsigned short*)currentFrame->instructionPointer;
        currentFrame->instructionPointer+=2;
        if (currentFrame == &frames[PSC_MAX_STACKDEPTH-1]) {
            // THROW
        }
        PSCStackFrame* nextFrame = currentFrame + 1;
        nextFrame->instructionPointer = &code[targetOffset];
        nextFrame->stackPointer = currentFrame->stackPointer;
        nextFrame->functionStackOffset = nextFrame->stackPointer;
        currentFrame = nextFrame;
        printf("  %s %x\n",__FUNCTION__,targetOffset);

    }

    void moveFunctionStackOffset() {
        int move = *(unsigned short*)currentFrame->instructionPointer;
        currentFrame->instructionPointer+=2;
        currentFrame->functionStackOffset -= move;
        printf("  %s %x\n",__FUNCTION__,move);
    }

    void pushStackValueUint8() {
        int offset = currentFrame->instructionPointer++[0];
        currentFrame->stackPointer++[0] = currentFrame->functionStackOffset[offset];
        printf("  %s %d\n",__FUNCTION__,currentFrame->functionStackOffset[offset]);
    }

    void mulUint8Uint8Uint16() {
        unsigned char a = currentFrame->stackPointer[-2];
        unsigned char b = currentFrame->stackPointer[-1];
        unsigned short* res = (unsigned short*) &currentFrame->stackPointer[-2];
        *res = a * b;
        printf("  %s %d * %d = %d\n",__FUNCTION__, a, b,*res);
    }

    void returnUint16() {
        unsigned short copyVal = *(unsigned short*) &currentFrame->stackPointer[-2];
        *((unsigned short*)currentFrame->functionStackOffset) = copyVal;
        PSCStackFrame* upFrame = currentFrame - 1;
        upFrame->stackPointer = currentFrame->functionStackOffset + 2;
        currentFrame = upFrame;
        printf("  %s %d\n",__FUNCTION__, copyVal);
    }

    void callNative() {
        unsigned short id = currentFrame->instructionPointer++[0];
        printf("  %s %d\n",__FUNCTION__, id);
        PSCvmFuncRecord* func = &nativeFunctionList[id];
        PSCStackFrame* nextFrame = currentFrame + 1;
        nextFrame->stackPointer = currentFrame->stackPointer;
        nextFrame->functionStackOffset = nextFrame->stackPointer - func->bytesArgument;
        currentFrame = nextFrame;

        func->func(this);

        PSCStackFrame* upFrame = currentFrame - 1;
        upFrame->stackPointer = currentFrame->functionStackOffset;
        for (int i=0;i<func->bytesReturn;i+=1) {
            upFrame->stackPointer++[0] = currentFrame->stackPointer[i];
        }
        currentFrame = upFrame;
    }

    void returnVoid() {

    }

    void callCastUint16Int32() {
        unsigned short num = *(unsigned short*)(&currentFrame->stackPointer[-2]);
        *(long*)&currentFrame->stackPointer[-2] = (long)num;
        currentFrame->stackPointer+=2;
    }
    void callCastUint16Int16() {
        // no need to do anything
    }

    bool nextInstruction() {
        unsigned char op = currentFrame->instructionPointer++[0];
        switch (op) {
        case OP_push8bitLiteral: opPush8bitLiteral(); break;
        case OP_call: opCall(); break;
        case OP_moveFunctionStackOffset: moveFunctionStackOffset(); break;
        case OP_pushStackValueUint8: pushStackValueUint8(); break;
        case OP_opMulUint8Uint8Uint16: mulUint8Uint8Uint16(); break;
        case OP_returnUint16: returnUint16(); break;
        case OP_callNative: callNative(); break;
        case OP_returnVoid:
            if (currentFrame == frames) {
                return false;
            } else {
                returnVoid();
            }
            break;
        case OP_castUint16Int16: callCastUint16Int16(); break;
        case OP_castUint16Int32: callCastUint16Int32(); break;
        default:
            printf("unkown nextInstructions: 0x%02x\n",op);
            return false;
        }
        return true;
    }

public:
	PSCvm (unsigned char *code, PSCvmFuncRecord *nativeFunctionList) {
        this->code = code;
        this->nativeFunctionList = nativeFunctionList;
        currentFrame = &frames[0];
        currentFrame->functionStackOffset = stack;
        currentFrame->stackPointer = stack;
        currentFrame->instructionPointer = code;
    }

    long getLong(unsigned char offset) {
        return *(long*)(&currentFrame->functionStackOffset[offset]);
    }
    short getShort(unsigned char offset) {
        return *(short*)(&currentFrame->functionStackOffset[offset]);
    }


    void execProg() {
        while (nextInstruction());
    }
};


void printInt16 (PSCvm *vm) {
	short num = vm->getShort(0);
	printf("%d\n",num);
}

void printInt32 (PSCvm *vm) {
	long num = vm->getLong(0);
	printf("%ld\n",num);
}

PSCvmFuncRecord _nativeFunctions[] = {
    PSCvmFuncRecord(printInt16, sizeof(short), 0),
    PSCvmFuncRecord(printInt32, sizeof(long), 0)
};


unsigned char code[] = {
//0x01, 0x05, 0x10, 0x0f, 0x00, 0x11, 0x00, 0x01, 0x06, 0x10, 0x0f, 0x00, 0x11, 0x00, 0x12, 0x13, 0x01, 0x00, 0x07, 0x00, 0x07, 0x00, 0x08, 0x09,
//0x01, 0x05, 0x10, 0x0f, 0x00, 0x11, 0x00, 0x01, 0x06, 0x10, 0x0f, 0x00, 0x11, 0x01, 0x12, 0x13, 0x01, 0x00, 0x07, 0x00, 0x07, 0x00, 0x08, 0x09,
0x01, 0x05, 0x10, 0x11, 0x00, 0x14, 0x11, 0x00, 0x01, 0x06, 0x10, 0x11, 0x00, 0x15, 0x11, 0x01, 0x12, 0x13, 0x01, 0x00, 0x07, 0x00, 0x07, 0x00, 0x08, 0x09,
};

PSCvm vm(code, _nativeFunctions);

int main() {
    vm.execProg();

    return 0;
}
