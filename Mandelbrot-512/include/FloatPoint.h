#ifndef _FLOATPOINT_H_
#define _FLOATPOINT_H_


/* x87 FPU Control Word:
//  1 1 1 1 1 1
//  5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  ===============================
//        X R R P P     P U O Z D I
//          C C C C     M M M M M M
*/

/* Exception Mask Bits: */
#define FPU_INVALID_OPERATION_MASK    1
#define FPU_DENORMAL_OPERAND_MASK     2
#define FPU_ZERO_DIVIDE_MASK          4
#define FPU_OVERFLOW_MASK             8
#define FPU_UNDERFLOW_MASK           16
#define FPU_PRECISION_MASK           32

#define FPU_PC_MASK                 768
#define FPU_PC_SINGLE_PREC            0
#define FPU_PC_RESERVED             256
#define FPU_PC_DOUBLE_PREC          512
#define FPU_PC_DBLEXT_PREC          768

#define FPU_RC_MASK                3072
#define FPU_RC_ROUND_NEAREST          0
#define FPU_RC_ROUND_DOWN          1024
#define FPU_RC_ROUND_UP            2048
#define FPU_RC_ROUND_ZERO          3072

#define X287_INFINITY_CONTROL      4096

#define FPU_ALL_MASK             0xFFFFU

/* asmlib.asm */
EXTERN_C unsigned int
  FPUSetControlWord (unsigned int control_word, unsigned int control_mask);


/* MXCSR Multimedia Extensions Control and Status Register
//  3 ... 1 | 1 1 1 1 1 1
//  1 ... 6 | 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  =========================================
//  RESERVED  F R R P U O Z D I D P U O Z D I
//            Z C C M M M M M M A E E E E E E
//                              Z
*/
#define MXCSR_INVALID_OP_FLAG         1
#define MXCSR_DENORMAL_FLAG           2
#define MXCSR_ZERO_DIVIDE_FLAG        4
#define MXCSR_OVERFLOW_FLAG           8
#define MXCSR_UNDERFLOW_FLAG         16
#define MXCSR_PRECISION_FLAG         32

#define MXCSR_DENORMALS_ARE_ZEROS    64

#define MXCSR_INVALID_OP_MASK       128
#define MXCSR_DENORMAL_OP_MASK      256
#define MXCSR_ZERO_DIVIDE_MASK      512
#define MXCSR_OVERFLOW_MASK        1024
#define MXCSR_UNDERFLOW_MASK       2048
#define MXCSR_PRECISION_MASK       4096

#define MXCSR_RC_MASK             24576
#define MXCSR_RC_ROUND_NEAREST        0
#define MXCSR_RC_ROUND_DOWN        8192
#define MXCSR_RC_ROUND_UP         16384
#define MXCSR_RC_ROUND_ZERO       24576

#define MXCSR_FLUSH_TO_ZERO       32768

#define MXCSR_ALL_MASK       0xFFFFFFFFUL

/* asmlib.asm */
EXTERN_C unsigned int
  SSESetControlWord (unsigned int control_word, unsigned int control_mask);


/* Structures: */
PACK(struct FpControlWord
{
  unsigned int   sse;
  unsigned short fpu;
})


/* Function declarations: */
extern unsigned short
  FPU_set_control_word (unsigned short control_word, unsigned short control_mask);
extern unsigned int
  SSE_set_control_word (unsigned int control_word, unsigned int control_mask);

#endif
