#include <float.h>                /* _controlfp() */

#include "stdinc.h"
#include "FloatPoint.h"


/* Function definitions: */

/*
// fnstcw - default = 639
//
//  1 1 1 1 1 1
//  5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  ===============================
//        X R R P P     P U O Z D I
//          C C C C     M M M M M M
//  0 0 0 0 0 0 1 0 0 1 1 1 1 1 1 1
//
// + all FPU exceptions are masked
// + round to nearest
// + double precision
//
// REMARKS:
// 1. The control word needs to be set for every thread (even with pthreads)
// 2."On the x64 architecture, changing the floating point precision IS NOT
//    SUPPORTED. If the precision control mask is used on that platform, an
//    assertion and the invalid parameter handler is invoked, as described
//    in Parameter Validation."
*/
extern unsigned short FPU_set_control_word (unsigned short control_word, unsigned short control_mask)
{
/*
//fpu_oldcw = _controlfp (_RC_CHOP
//                        #ifndef _M_AMD64
//                      | _PC_24
//                        #endif
//                      , _MCW_RC
//                        #ifndef _M_AMD64
//                      | _MCW_PC
//                        #endif
//                       );
*/
  #ifdef _M_AMD64
  control_mask &= ~FPU_PC_MASK;
  #endif

  return ((unsigned short)FPUSetControlWord (control_word, control_mask));
}


/* stmxcsr - default = 8096 */
/*
//  3 ... 1 | 1 1 1 1 1 1
//  1 ... 6 | 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  =========================================
//  RESERVED  F R R P U O Z D I D P U O Z D I
//            Z C C M M M M M M A E E E E E E
//            0 0 0 1 1 1 1 1 1 0 1 0 0 0 0 0
//
// + all SSE exceptions are masked
// + round to nearest
*/
extern unsigned int SSE_set_control_word (unsigned int control_word, unsigned int control_mask)
{
  return (SSESetControlWord (control_word, control_mask));
}
