#ifndef VERILATOR_VERILATED_LUAJIT_H_
#define VERILATOR_VERILATED_LUAJIT_H_

#ifdef VL_IN_LUAJIT_FFI_MODULE
typedef int8_t vlsint8_t;  ///< 8-bit signed type
typedef uint8_t vluint8_t;  ///< 8-bit unsigned type
typedef int16_t vlsint16_t;  ///< 16-bit signed type
typedef uint16_t vluint16_t;  ///< 16-bit unsigned type
typedef int32_t vlsint32_t;  ///< 32-bit signed type
typedef uint32_t vluint32_t;  ///< 32-bit unsigned type
typedef int64_t vlsint64_t;  ///< 64-bit signed type
typedef uint64_t vluint64_t;  ///< 64-bit unsigned type

typedef vluint8_t CData;  ///< Data representing 'bit' of 1-8 packed bits
typedef vluint16_t SData;  ///< Data representing 'bit' of 9-16 packed bits
typedef vluint32_t IData;  ///< Data representing 'bit' of 17-32 packed bits
typedef vluint64_t QData;  ///< Data representing 'bit' of 33-64 packed bits
typedef vluint32_t EData;  ///< Data representing one element of WData array
typedef EData WData;  ///< Data representing >64 packed bits (used as pointer)

#define VL_SIG8(name, msb, lsb) CData name  ///< Declare signal, 1-8 bits
#define VL_SIG16(name, msb, lsb) SData name  ///< Declare signal, 9-16 bits
#define VL_SIG64(name, msb, lsb) QData name  ///< Declare signal, 33-64 bits
#define VL_SIG(name, msb, lsb) IData name  ///< Declare signal, 17-32 bits
#define VL_SIGW(name, msb, lsb, words) WData name[words]  ///< Declare signal, 65+ bits
#define VL_IN8(name, msb, lsb) CData name  ///< Declare input signal, 1-8 bits
#define VL_IN16(name, msb, lsb) SData name  ///< Declare input signal, 9-16 bits
#define VL_IN64(name, msb, lsb) QData name  ///< Declare input signal, 33-64 bits
#define VL_IN(name, msb, lsb) IData name  ///< Declare input signal, 17-32 bits
#define VL_INW(name, msb, lsb, words) WData name[words]  ///< Declare input signal, 65+ bits
#define VL_INOUT8(name, msb, lsb) CData name  ///< Declare bidir signal, 1-8 bits
#define VL_INOUT16(name, msb, lsb) SData name  ///< Declare bidir signal, 9-16 bits
#define VL_INOUT64(name, msb, lsb) QData name  ///< Declare bidir signal, 33-64 bits
#define VL_INOUT(name, msb, lsb) IData name  ///< Declare bidir signal, 17-32 bits
#define VL_INOUTW(name, msb, lsb, words) WData name[words]  ///< Declare bidir signal, 65+ bits
#define VL_OUT8(name, msb, lsb) CData name  ///< Declare output signal, 1-8 bits
#define VL_OUT16(name, msb, lsb) SData name  ///< Declare output signal, 9-16 bits
#define VL_OUT64(name, msb, lsb) QData name  ///< Declare output signal, 33-64bits
#define VL_OUT(name, msb, lsb) IData name  ///< Declare output signal, 17-32 bits
#define VL_OUTW(name, msb, lsb, words) WData name[words]  ///< Declare output signal, 65+ bits
#endif

#ifndef VL_IN_LUAJIT_FFI_MODULE
extern "C" {
#endif

#ifdef VL_IN_LUAJIT_FFI_MODULE
struct VerilatedContext {
    char unused[0];
};
#endif

void* VerilatedContext_new();
void VerilatedContext_delete(void* p);
void VerilatedContext_commandArgs(void* p, int argc, const char** argv);
bool VerilatedContext_gotFinish(void* p);
void VerilatedContext_traceEverOn(void* p, bool flag);

#ifndef VL_IN_LUAJIT_FFI_MODULE
}
#endif

#endif
