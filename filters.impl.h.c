#include "filters.h"
#include "utils.h"

#include <immintrin.h>

/*
    // AT&T/UNIX GCC Inline Assembly Sample

    static const char Argument[] =                   // C constants
        "some data";
    static const unsigned long Another_Argument =
        sizeof(Argument);
    long result;                                     // a C variable

    // x86, x86-64
    __asm__ __volatile__ (
        "op-code<length suffix> %%src_register, %%dest_register\n\t"
        "op-code<length suffix> $immediate, %%dest_register\n\t"
        // ...
        "op-code<length suffix> %<argument number>, %%dest_register\n\t"
        "op-code"
        : "=a" (result)                              // output argument/s
        : "D" ((unsigned long) file_descriptor),     // input arguments
            "S" (buffer),
            "d" (buffer_size),
            "r" (Argument), "r" (Another_Argument)
        : "%used register", "%another used register" // clobbered registers
    );

    // The ARM assembly syntax uses the `#` symbol for constants and NOT
    // the `$` symbol. Registers `r` or `x` (for the 32-bit or 64-bit
    // architecture) do not need a `%%` prefix.
    //
    // `__asm__` and `__volatile__` could also be written as `asm` and `volatile`.
    //
    // The `volatile` modifier tells the compiler not to remove or reorder
    // the inlined assembly block during the compiler optimization step.
    //
    // <length suffixes>
    //     'b'    'w'     's'     'l'     'q'
    //      8 bit  16 bit  16 bit  32 bit  64 bit  integers
    //                     32 bit  64 bit          floating point numbers
    //
    // Length suffixes are not required for the ARM assembly syntax.
    // Argument numbers go from top to bottom, from left to right
    // starting from zero.
    //
    //     result           : %<argument number> = %0
    //     file_descriptor  : ...                = %1
    //     buffer           :                    = %2
    //     buffer_size      :                    = %3
    //     Argument         :                    = %4
    //     Another_Argument :                    = %5
    //
    // The first quoted letter before the argument in brackets is a
    // register constraint. It tells the compiler to provide the
    // argument through that register.
    //
    // On X86/-64 the following register constraints are possible
    // +---+--------------------------+
    // | r :   any register           |
    // +---+--------------------------+
    // | a :   %rax, %eax, %ax, %al   |
    // | b :   %rbx, %ebx, %bx, %bl   |
    // | c :   %rcx, %ecx, %cx, %cl   |
    // | d :   %rdx, %edx, %dx, %dl   |
    // | S :   %rsi, %esi, %si        |
    // | D :   %rdi, %edi, %di        |
    // +---+--------------------------+
    //
    // On ARM, the `r` constraint will work for all general purpose
    // registers. The input variable's register can be specified after the
    // variable's declaration wrapped in quotes and parentheses.
    //
    //     register long result ("r7"); // 32-bit ARM
    //     register long result ("x0"); // 64-bit ARM
    //
    // All registers used as input or output arguments should not be
    // listed as clobbered.
    //
    // https://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
*/

static inline void filters_apply_brightness_contrast(
                       uint8_t *pixels,
                       size_t position,
                       float brightness,
                       float contrast
                   )
{
#if !defined FILTERS_C_IMPLEMENTATION       && \
    !defined FILTERS_X87_ASM_IMPLEMENTATION && \
    !defined FILTERS_SIMD_ASM_IMPLEMENTATION
#define FILTERS_C_IMPLEMENTATION 1
#endif

#if defined FILTERS_C_IMPLEMENTATION

    pixels[position] =
        (uint8_t) UTILS_CLAMP(pixels[position]     * contrast + brightness, 0.0f, 255.0f);
    pixels[position + 1] =
        (uint8_t) UTILS_CLAMP(pixels[position + 1] * contrast + brightness, 0.0f, 255.0f);
    pixels[position + 2] =
        (uint8_t) UTILS_CLAMP(pixels[position + 2] * contrast + brightness, 0.0f, 255.0f);

#elif defined FILTERS_X87_ASM_IMPLEMENTATION

#if defined x86_32_CPU

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subl $0x1c, %%esp\n\t"

        "xorl %%eax, %%eax\n\t"

        "movb (%2,%3), %%al\n\t"
        "movl %%eax, (%%esp)\n\t"

        "movb 0x1(%2,%3), %%al\n\t"
        "movl %%eax, 0x4(%%esp)\n\t"

        "movb 0x2(%2,%3), %%al\n\t"
        "movl %%eax, 0x8(%%esp)\n\t"

        "fildl (%%esp)\n\t"
        "fildl 0x4(%%esp)\n\t"
        "fildl 0x8(%%esp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpl 0x8(%%esp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpl 0x4(%%esp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpl (%%esp)\n\t"

        "movl $0xff, %%edx\n\t"
        "movl $0x0, %%edi\n\t"

        "movl (%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "cmpl %%edi, %%eax\n\t"
        "cmovbl %%edi, %%eax\n\t"
        "movb %%al, (%2,%3)\n\t"

        "movl 0x4(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "cmpl %%edi, %%eax\n\t"
        "cmovbl %%edi, %%eax\n\t"
        "movb %%al, 0x1(%2,%3)\n\t"

        "movl 0x8(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "cmpl %%edi, %%eax\n\t"
        "cmovbl %%edi, %%eax\n\t"
        "movb %%al, 0x2(%2,%3)\n\t"

        "addl $0x1c, %%esp\n\t"
    ::
        "S"(&brightness), "D"(&contrast),
        "b"(pixels), "c"(position)
    :
        "%eax", "%edx"
    );

#elif defined x86_64_CPU

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subq $0x38, %%rsp\n\t"

        "xorq %%rax, %%rax\n\t"

        "movb (%2,%3), %%al\n\t"
        "movq %%rax, (%%rsp)\n\t"

        "movb 0x1(%2,%3), %%al\n\t"
        "movq %%rax, 0x8(%%rsp)\n\t"

        "movb 0x2(%2,%3), %%al\n\t"
        "movq %%rax, 0x10(%%rsp)\n\t"

        "fildl (%%rsp)\n\t"
        "fildl 0x8(%%rsp)\n\t"
        "fildl 0x10(%%rsp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpq 0x10(%%rsp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpq 0x8(%%rsp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpq (%%rsp)\n\t"

        "movq $0xff, %%rdx\n\t"
        "movq $0x0, %%r8\n\t"

        "movq (%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "cmpq %%r8, %%rax\n\t"
        "cmovbq %%r8, %%rax\n\t"
        "movb %%al, (%2,%3)\n\t"

        "movq 0x8(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "cmpq %%r8, %%rax\n\t"
        "cmovbq %%r8, %%rax\n\t"
        "movb %%al, 0x1(%2,%3)\n\t"

        "movq 0x10(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "cmpq %%r8, %%rax\n\t"
        "cmovbq %%r8, %%rax\n\t"
        "movb %%al, 0x2(%2,%3)\n\t"

        "addq $0x38, %%rsp\n\t"
    ::
        "S"(&brightness), "D"(&contrast),
        "b"(pixels), "c"(position)
    :
        "%rax", "%rdx", "%r8"
    );

#else
#error "Unsupported processor architecture"
#endif

#elif defined FILTERS_SIMD_ASM_IMPLEMENTATION

#if defined x86_32_CPU

    // Process 16 color channels at the same time.
    __asm__ __volatile__ (
        "vbroadcastss (%0), %%zmm2\n\t"
        "vbroadcastss (%1), %%zmm1\n\t"
        "vpmovzxbd (%2, %3), %%zmm0\n\t"
        "vcvtdq2ps %%zmm0, %%zmm0\n\t"
        "vfmadd132ps %%zmm1, %%zmm2, %%zmm0\n\t"
        "vcvtps2dq %%zmm0, %%zmm0\n\t"
        "vpmovusdb %%zmm0, (%2, %3)\n\t"
    ::
        "S"(&brightness), "D"(&contrast), "b"(pixels), "c"(position)
    :
        "%zmm0", "%zmm1", "%zmm2"
    );

#elif defined x86_64_CPU

    // Process 16 color channels at the same time.
    __asm__ __volatile__ (
        "vbroadcastss (%0), %%zmm2\n\t"
        "vbroadcastss (%1), %%zmm1\n\t"
        "vpmovzxbd (%2, %3), %%zmm0\n\t"
        "vcvtdq2ps %%zmm0, %%zmm0\n\t"
        "vfmadd132ps %%zmm1, %%zmm2, %%zmm0\n\t"
        "vcvtps2dq %%zmm0, %%zmm0\n\t"
        "vpmovusdb %%zmm0, (%2, %3)\n\t"
    ::
        "S"(&brightness), "D"(&contrast), "b"(pixels), "c"(position)
    :
        "%zmm0", "%zmm1", "%zmm2"
    );

#else
#error "Unsupported processor architecture"
#endif

#endif
}

static inline void filters_apply_sepia(
                       uint8_t *pixels,
                       size_t position
                   )
{
    static const float Sepia_Coefficients[] = {
        0.272f, 0.534f, 0.131f,
        0.349f, 0.686f, 0.168f,
        0.393f, 0.769f, 0.189f
    };

#if !defined FILTERS_C_IMPLEMENTATION &&     \
    !defined FILTERS_SIMD_ASM_IMPLEMENTATION
#define FILTERS_C_IMPLEMENTATION 1
#endif

#if defined FILTERS_C_IMPLEMENTATION

    uint32_t blue =
        pixels[position];
    uint32_t green =
        pixels[position + 1];

    pixels[position] =
        (uint8_t) UTILS_MIN(
                      Sepia_Coefficients[0] * blue  +
                      Sepia_Coefficients[1] * green +
                      Sepia_Coefficients[2] * blue,
                      255.0f
                  );
    pixels[position + 1] =
        (uint8_t) UTILS_MIN(
                      Sepia_Coefficients[3] * blue  +
                      Sepia_Coefficients[4] * green +
                      Sepia_Coefficients[5] * blue,
                      255.0f
                  );
    pixels[position + 2] =
        (uint8_t) UTILS_MIN(
                      Sepia_Coefficients[6] * blue  +
                      Sepia_Coefficients[7] * green +
                      Sepia_Coefficients[8] * blue,
                      255.0f
                  );

#elif defined FILTERS_X87_ASM_IMPLEMENTATION


#if defined x86_32_CPU

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subl $0x14, %%esp\n\t"

        "xorl %%eax, %%eax\n\t"

        "movb (%1,%2), %%al\n\t"
        "movl %%eax, (%%esp)\n\t"

        "movb 0x1(%1,%2), %%al\n\t"
        "movl %%eax, 0x4(%%esp)\n\t"

        "filds (%%esp)\n\t"
        "filds 0x4(%%esp)\n\t"

        "flds (%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x4(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x8(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpl 0x8(%%esp)\n\t"

        "flds 0xc(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x10(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x14(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpl 0xc(%%esp)\n\t"

        "flds 0x18(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x1c(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x20(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpl 0x10(%%esp)\n\t"

        "fstp %%st\n\t"
        "fstp %%st\n\t"

        "movl $0xff, %%edx\n\t"

        "movl 0x8(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "movb %%al, (%1,%2)\n\t"

        "movl 0xc(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "movb %%al, 0x1(%1,%2)\n\t"

        "movl 0x10(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "movb %%al, 0x2(%1,%2)\n\t"

        "addl $0x14, %%esp\n\t"
    ::
        "S"(Sepia_Coefficients),
        "b"(pixels), "c"(position)
    :
        "%eax", "%edx"
    );

#elif defined x86_64_CPU

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subq $0x28, %%rsp\n\t"

        "xorq %%rax, %%rax\n\t"

        "movb (%1,%2), %%al\n\t"
        "movq %%rax, (%%rsp)\n\t"

        "movb 0x1(%1,%2), %%al\n\t"
        "movq %%rax, 0x8(%%rsp)\n\t"

        "fildl (%%rsp)\n\t"
        "fildl 0x8(%%rsp)\n\t"

        "flds (%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x4(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x8(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpq 0x10(%%rsp)\n\t"

        "flds 0xc(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x10(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x14(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpq 0x18(%%rsp)\n\t"

        "flds 0x18(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x1c(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "flds 0x20(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpq 0x20(%%rsp)\n\t"

        "fstp %%st\n\t"
        "fstp %%st\n\t"

        "movq $0xff, %%rdx\n\t"

        "movq 0x10(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "movb %%al, (%1,%2)\n\t"

        "movq 0x18(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "movb %%al, 0x1(%1,%2)\n\t"

        "movq 0x20(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "movb %%al, 0x2(%1,%2)\n\t"

        "addq $0x28, %%rsp\n\t"
    ::
        "S"(Sepia_Coefficients),
        "b"(pixels), "c"(position)
    :
        "%rax", "%rdx"
    );

#else
#error "Unsupported processor architecture"
#endif

#elif defined FILTERS_SIMD_ASM_IMPLEMENTATION

float sorted_coeff[]=
       {
         0.272f, 0.349f, 0.393f, 1.0f,
         0.534f, 0.686f, 0.769f, 1.0f,
         0.131f, 0.168f, 0.189f, 1.0f,
       };

       //
       // 0.272f, 0.534f, 0.131f,
       // 0.349f, 0.686f, 0.168f,
       // 0.393f, 0.769f, 0.189f
       // +---+--------------------------+
       // | r :   any register           |
       // +---+--------------------------+
       // | a :   %rax, %eax, %ax, %al   |
       // | b :   %rbx, %ebx, %bx, %bl   |
       // | c :   %rcx, %ecx, %cx, %cl   |
       // | d :   %rdx, %edx, %dx, %dl   |
       // | S :   %rsi, %esi, %si        |
       // | D :   %rdi, %edi, %di        |
       // +---+--------------------------+
#if defined x86_32_CPU

__asm__ __volatile__ (
   /* "vmovups (%3), %%xmm1\n\t"
    "vmovups 0x10(%3), %%xmm2\n\t"  // array
    "vmovups 0x20(%3), %%xmm3\n\t"

    "movb 0x3(%1,%2), %%al\n\t"

    "movb (%1,%2), %%bl\n\t"
    "vpbroadcastd %%ebx, %%xmm5\n\t"  // Blue
    "vcvtdq2ps %%xmm5, %%xmm5\n\t"

    "movb 0x1(%1,%2), %%bl\n\t"
    "vpbroadcastd %%ebx, %%xmm6\n\t" // green
    "vcvtdq2ps %%xmm6, %%xmm6\n\t"

    "movb 0x2(%1,%2), %%bl\n\t"
    "vpbroadcastd %%eax, %%xmm7\n\t" // red
    "vcvtdq2ps %%xmm7, %%xmm7\n\t"

    "vmulps %%xmm1, %%xmm5, %%xmm7\n\t"
    "vmulps %%xmm2, %%xmm6, %%xmm6\n\t"
    "vmulps %%xmm3, %%xmm5, %%xmm5\n\t"

    "vaddps %%xmm7, %%xmm6, %%xmm0\n\t"
    "vaddps %%xmm5, %%xmm0, %%xmm0\n\t"
    "vcvtps2dq %%xmm0, %%xmm0\n\t"

    // "movl $0xff, %%ebx\n\t"
    // "vpbroadcastd %%ebx, %%xmm8\n\t"
    //
    // "vpcmpgtd %%xmm8, %%xmm0, %%k1\n\t"
    // "vmovdqa32 %%xmm8, %%xmm0%{%%k1%}\n\t"

    "vpmovusdb %%xmm0, (%1,%2)\n\t"
    "movb %%al, 0x3(%1,%2)\n\t"
    ::
      "S"(Sepia_Coefficients), "D"(pixels), "c"(position),
      "d"(sorted_coeff)
    :
    "%zmm1", "%zmm2","%zmm3","%zmm0"
	*/
);

#elif defined x86_64_CPU
        // Rs = 0.393 * R + 0.769 * G + 0.189 * B
        // Gs = 0.349 * R + 0.686 * G + 0.168 * B
        // Bs = 0.272 * R + 0.534 * G + 0.131 * B
    float B[16], G[16], R[16];

    for(int i = 0, j = 0; i < 16; i++, j+=3){
      B[i] = (float)pixels[position + j];
      G[i] = (float)pixels[position + j + 1];
      R[i] = (float)pixels[position + j + 2];
    }

      // 0.272f, 0.534f, 0.131f,
      // 0.349f, 0.686f, 0.168f,
      // 0.393f, 0.769f, 0.189f

    __asm__ __volatile__ (

        "vmovaps (%3), %%zmm1\n\t"   // 16 pixels of Blue
        "vmovaps (%4), %%zmm2\n\t"   // 16 pixels of Green
        "vmovaps (%5), %%zmm3\n\t"   // 16 pixels of Red

        // 0.272f, 0.534f, 0.131f
        "vbroadcastss (%2), %%zmm4\n\t" // B
        "vbroadcastss 0x4(%2), %%zmm5\n\t" // G
        "vbroadcastss 0x8(%2), %%zmm6\n\t" // R

        // Bs = 0.272 * R + 0.534 * G + 0.131 * B
        "vmulps %%zmm1, %%zmm6, %%zmm9\n\t" // Bp * Rc
        "vmulps %%zmm2, %%zmm5, %%zmm8\n\t" // Gp * Gc
        "vmulps %%zmm3, %%zmm4, %%zmm7\n\t" // Rp * Bc

        "vaddps %%zmm9, %%zmm8, %%zmm8\n\t"
        "vaddps %%zmm8, %%zmm7, %%zmm7\n\t"
        "vmovaps %%zmm7, (%3)\n\t"   // 16 pixels of Red



        "vbroadcastss 0xc(%2), %%zmm4\n\t" // B
        "vbroadcastss 0x10(%2), %%zmm5\n\t" // G
        "vbroadcastss 0x14(%2), %%zmm6\n\t" // R

        // B
        "vmulps %%zmm1, %%zmm6, %%zmm9\n\t"
        "vmulps %%zmm2, %%zmm5, %%zmm8\n\t"
        "vmulps %%zmm3, %%zmm4, %%zmm7\n\t"

        "vaddps %%zmm9, %%zmm8, %%zmm8\n\t"
        "vaddps %%zmm8, %%zmm7, %%zmm7\n\t"
        "vmovaps %%zmm7, (%4)\n\t"   // 16 pixels of Red


        "vbroadcastss 0x18(%2), %%zmm4\n\t" // B
        "vbroadcastss 0x1c(%2), %%zmm5\n\t" // G
        "vbroadcastss 0x20(%2), %%zmm6\n\t" // R

        // B
        "vmulps %%zmm1, %%zmm6, %%zmm9\n\t"
        "vmulps %%zmm2, %%zmm5, %%zmm8\n\t"
        "vmulps %%zmm3, %%zmm4, %%zmm7\n\t"

        "vaddps %%zmm9, %%zmm8, %%zmm8\n\t"
        "vaddps %%zmm8, %%zmm7, %%zmm7\n\t"
        "vmovaps %%zmm7, (%5)\n\t"   // 16 pixels of Red

        ::
          "D"(pixels), "S"(position), // 0 1
          "d"(Sepia_Coefficients), "a"(B),     // 2 3
          "b"(G), "c"(R)              // 4 5
        :
        "%zmm1", "%zmm2","%zmm3","%zmm0", "%zmm4", "%zmm5", "%zmm6", "%zmm7", "%zmm8", "%zmm9"
    );


    for(int i = 0, j = 0; i < 16; i++, j+=3){
      pixels[position + j]     =  (uint8_t) UTILS_CLAMP(B[i], 0.0f, 255.0f);
      pixels[position + j + 1] =  (uint8_t) UTILS_CLAMP(G[i], 0.0f, 255.0f);
      pixels[position + j + 2] =  (uint8_t) UTILS_CLAMP(R[i], 0.0f, 255.0f);
    }

#else
#error "Unsupported processor architecture"
#endif

#endif
}

static int _filters_compare_color_channels(const void *a, const void *b)
{
    return *((const uint8_t *) a) - *((const uint8_t *) b);
}

static inline void filters_apply_median(
                       uint8_t *source_pixels,
                       uint8_t *destination_pixels,
                       size_t position,
                       size_t x,
                       size_t y,
                       size_t width,
                       size_t height
                   )
{
#if !defined FILTERS_C_IMPLEMENTATION &&     \
    !defined FILTERS_SIMD_ASM_IMPLEMENTATION
#define FILTERS_C_IMPLEMENTATION 1
#endif

    const size_t window_width =
        FILTERS_MEDIAN_WINDOW_SIZE % 2 == 0 ?
            FILTERS_MEDIAN_WINDOW_SIZE + 1 :
            FILTERS_MEDIAN_WINDOW_SIZE;
    const size_t window_height =
        window_width;
    const size_t window_center_shift_x =
        window_width / 2;
    const size_t window_center_shift_y =
        window_height / 2;
    const size_t window_size =
        window_width * window_height;
    const size_t window_center =
        window_size / 2;

    uint8_t window[window_size];
    for (size_t channel = 0; channel < 3; ++channel) {
        for (size_t wy = 0; wy < window_height; ++wy) {
            for (size_t wx = 0; wx < window_width; ++wx) {
                ssize_t adjusted_x =
                    (ssize_t) x - (ssize_t) window_center_shift_x + (ssize_t) wx;
                ssize_t adjusted_y =
                    (ssize_t) y - (ssize_t) window_center_shift_y + (ssize_t) wy;

                window[wy * window_width + wx] =
                    bmp_sample_pixel(
                        source_pixels,
                        adjusted_x,
                        adjusted_y,
                        width,
                        height
                    )[channel];
            }
        }

#if defined FILTERS_C_IMPLEMENTATION

        qsort(window, window_size, sizeof(*window), _filters_compare_color_channels);

#elif defined FILTERS_SIMD_ASM_IMPLEMENTATION

#if defined x86_32_CPU

        __asm__ __volatile__ (
            "\n\t" :::
        );

#elif defined x86_64_CPU
      int64_t a[] = {6, 7, 4, 5, 2, 3, 0, 1};
      int64_t b[] = {4, 5, 6, 7, 0, 1, 2, 3};
      int64_t c[] = {0, 1, 2, 3, 4, 5, 6, 7};
      int64_t d[] = {5, 4, 7, 6, 1, 0, 3, 2};

      __asm__ __volatile__ (
          "vmovapd   (%0), %%zmm0\n\t" // result = window
          "vmovdqa64 (%1), %%zmm1\n\t" // a[]
          "vmovdqa64 (%2), %%zmm2\n\t" // b[]
          "vmovdqa64 (%3), %%zmm3\n\t" // c[]
          "vmovdqa64 (%4), %%zmm4\n\t" // d[]

//__m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
          "vpermpd %%zmm1, %%zmm0, %%zmm5%{%%k%}\n\t"
//__m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
          "vminpd  %%zmm5, %%zmm0, %%zmm6%{%%k%}\n\t"
//__m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;
          "vmaxpd  %%zmm5, %%zmm0, %%zmm7%{%%k%}\n\t"

//  result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);

          ::"S"(window), "a"(a),
            "b"(b), "c"(c), "d"(d)
          :
      );


#else
#error "Unsupported processor architecture"
#endif

#endif

        uint8_t median =
            window_size % 2 == 0 ?
                (uint8_t) ((window[window_center - 1] + window[window_center]) * 0.5f) :
                window[window_center];
        destination_pixels[position + channel] =
            median;
    }
}
