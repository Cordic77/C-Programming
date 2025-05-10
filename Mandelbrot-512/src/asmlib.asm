;****************************************************************************
%ifdef COMMENT

  TODO:           x64: results in an application crash (works fine in x86)!

  Author:         Michael Stumpfl

  Date:           2011-12-08

  API:            POSIX(, Win32)

  Environment:
[OSX] /etc/bashrc       NASMENV=";-i/usr/include/nasmx/inc/"
[LIN] /etc/environment  NASMENV=";-i/usr/include/nasmx/inc/"
[WIN] <env. variables>  NASMENV=;-ic:\Program Files (x86)\Nasm\nasmx\inc\

  Preprocessor:
[WIN]             -dLIBCFILE=MSVCRT

  Tested on:
[Windows 7]                   x64  (Windows NT 6.1.7601, SP1)

  Libraries:

 1. OS Libraries:
[WIN] kernel32.lib

      kernel32.lib
        ExitProcess

 2. C Runtime:

[WIN] msvcrtd.lib (http://msdn.microsoft.com/en-us/library/abx4dbyh(v=VS.100).aspx)
        C Run-time Library (Multithreaded DLL)

 3. Other dependencies:

[WIN] oldnames.lib
        _getch

  Functionality:  Assembly helper module.

  Known Bugs:     ---

  Last Modified:  2018-12-29

%endif
;****************************************************************************
  %use "altreg"                       ; Enable Intel-defined aliases R8L-R15L

  DEFAULT  rel                        ; x64: [memory addresses] are RIP-relative


;===  Function prototypes  ==================================================
  %include "nasmx.inc"                ; Collection of useful NASM macros

  %define __linux__        0
  %define __MACH__         0
  %define _WINDOWS         0

; LINUX:
%if __NASMX_DECLSPEC_SYSTYPE_DEFAULT__ == __NASMX_DECLSPEC_SYSTYPE_LINUX__
  %include "linux/errorno.inc"        ; System error numbers
  %include "linux/libc.inc"           ; Basic libc definitions

  %include "linux/syscall.inc"        ; Linux system calls

  %define __linux__        1
  %define _rdata rodata               ; Read-only data segment

; MAC OS:
%elif __NASMX_DECLSPEC_SYSTYPE_DEFAULT__ == __NASMX_DECLSPEC_SYSTYPE_MACHOS__
  %include "linux/errorno.inc"        ; System error numbers
  %include "linux/libc.inc"           ; Basic libc definitions

  %define __MACH__         1
  %define _rdata rodata               ; Read-only data segment

; WINDOWS:
%elif __NASMX_DECLSPEC_SYSTYPE_DEFAULT__ == __NASMX_DECLSPEC_SYSTYPE_WINDOWS__
  %include "win32/msvcrt.inc"         ; "C" runtime library

; %include "win32/windows.inc"        ; Defines, Structures and Typedefs

  %include "win32/kernel32.inc"       ; Windows Kernel functions
  %include "win32/user32.inc"         ; User Interface functions
; %include "win32/gdi32.inc"          ; Graphics Device Interface
; %include "win32/advapi32.inc"       ; Registry Access, etc.

  %include "win32/unicode.inc"        ; %define MessageBox MessageBox(A|W)

  %define _WINDOWS         1
  %define _rdata rdata                ; Read-only data segment

%else
  %error "Other OS are not supported at this time!"
%endif


;===  Imported symbols  =====================================================

  ; void exit (int status);
  %if _WINDOWS
  %define exit ExitProcess            ; Win32 API
  %endif
  EXTERN  exit                        ; POSIX API

;***  Variables:
  %imacro CEXTERN 1.nolist
    %if __MACH__ || (_WINDOWS && __BITS__ < 64)
      ; OSX86/64, Win32 - exported symbols are prepended with an underscore!
      %xdefine %[%1] _%[%1]
    %else
      ; Linux, Win64 - exported symbols are exported as-is!
    %endif
    EXTERN %[%1]
  %endmacro

  CEXTERN  screen_width
  CEXTERN  screen_height
; CEXTERN  vmode_colors
  CEXTERN  pixel_data

  CEXTERN  leftUpper
  CEXTERN  rightLower

  CEXTERN  real_factor
  CEXTERN  imag_factor


;===  Exported symbols  =====================================================
  %imacro CGLOBAL 1.nolist
    %if __MACH__ || (_WINDOWS && __BITS__ < 64)
      ; OSX86/64, Win32 - exported symbols are prepended with an underscore!
      %xdefine %[%1] _%[%1]
    %else
      ; Linux, Win64 - exported symbols are exported as-is!
    %endif
    global %[%1]
  %endmacro

; MandelbrotPixelColor{SSE,AVX*}
  CGLOBAL  escape_radius

; Function definitions:
  CGLOBAL  MandelbrotPixelColorFIXED
  CGLOBAL  FractalDrawFIXED

  CGLOBAL  FPUSetControlWord
  CGLOBAL  SSESetControlWord
  CGLOBAL  MandelbrotPixelColorFLOAT
  CGLOBAL  FractalDrawFLOAT

  CGLOBAL  MandelbrotPixelColor4SSE1
  CGLOBAL  MandelbrotPixelColor8AVX1
  CGLOBAL  MandelbrotPixelColor8AVX2
  CGLOBAL  MandelbrotPixelColor16AVX512F
  CGLOBAL  MandelbrotPixelColor32AVX1024


;===  Defines & Macros  =====================================================

;***  Standard defines:
  %idefine offset                     ; Treat offset keyword as a no-op

  %if (__BITS__ < 64)
  %define  STACK_ARGS      1
  %define  REG_ARGS        0          ; Arguments are passed on the stack
  %define  PTR_OVERHEAD    0          ; x86: pointers are 32-bit in size
  %else
  %define  STACK_ARGS      0
  %define  REG_ARGS        1          ; Arguments are passed in registers
  %define  PTR_OVERHEAD sizeof(dword) ; x86-64: pointer size increases by 32-bit
  %endif

;***  Stack-based parameter passing  access arguments relative to __BP:
; ---------------------------------------------------------------------------
;
; e.g. Win32: printf ("%s", &argv [0]);
;                   ------------------------------------------
;           1BFC04  |              ...                       |  sub     ESP, 12
;           1BFBF8  |  12 bytes alignment (empty)            |  push    [argv]
; EBP + 12  1BFBF4  |  offset of argv [0] (pointer)          |  push    offset lt_s
; EBP + 8   1BFBF0  |  offset of "%s" (pointer)              |  call    printf
; EBP + 4   1BFBEC  |  <return address to caller>            |  push    EBP         ; Prologue
;                                                               mov     EBP, ESP    ; Prologue
; EBP  ==>  1BFBE8  |  backup of EBP (callers base pointer)  |  sub     ESP, (16+8) ; (assume 4 local variables)
; EBP - 4   1BFBE4  |  <local variable 1>                    |
;                   |               :                        |
; EBP - 12  1BFBDC  |  <local variable 3>                    |
; ESP  ==>  1BFBD8  |  <local variable 4>                    |
;                   ------------------------------------------
;                                   :
;                   ------------------------------------------  mov     ESP, EBP    ; Epilogue
; ESP  ==>  1BFBE8  |  backup of EBP (callers base pointer)  |  pop     EBP         ; Epilogue
; ESP  ==>  1BFBEC  |  <return address to caller>            |  ret
;                   ------------------------------------------

  %if (__BITS__ < 64) || !_WINDOWS
  %define  SHADOW_SPACE    0          ; No shadow space by default
  %else
  %define  SHADOW_SPACE    32         ; Win64 requires caller to reserve 32 byte
  %endif                              ; of shadow stack space (before any args)

  %define  ALIGN_MIN       (__BITS__ / 8)
  %define  ALIGN_DEF       16

  %if _WINDOWS && (__BITS__ < 64)     ; Win32 requires stack to be aligned on
  %define  STACK_ALIGN     ALIGN_MIN  ; dword boundary before call
  %else
  %define  STACK_ALIGN     ALIGN_DEF  ; All unixes / Win64 require stack to be
  %endif                              ; aligned on double qword boundary b/call

; void  astack (int size_largest_arg);
  %imacro astack 1
    %if %1 > STACK_ALIGN
    and   __SP, -%1                   ; Align by size of largest argument
    %else
    and   __SP, -STACK_ALIGN          ; Use default stack alignment
    %endif
  %endmacro

; Access function arguments relative to __BP:
  %if __BITS__ < 64
  %define  bkp_baseptr     1          ; backup/restore __BP in enter/leave?
  %else
  %define  bkp_baseptr     0
  %endif
  %define  bparg1          __BP + (1+bkp_baseptr)*sizeof(NASMX_PTR) ; "C" calling convention: argument offset
  %define  bploc1          __BP

  ; Access function arguments relative to __SP:
  %define  sparg1          __SP + %$res_funcargs + %$res_locals + %$res_alignment + sizeof(NASMX_PTR)
  %define  sploc1          __SP + %$res_funcargs

; void  enter (int res_locals, int res_funcargs, bool force_stackframe=bkp_baseptr);
  %imacro enter 2-3 bkp_baseptr
    %push   enter_ctx                 ; push new context 'enter_ctx'
      %if bkp_baseptr || %3 != 0
      push  __BP
      mov   __BP, __SP
      %define %$res_ebp sizeof(size_t); backup of callers base pointer
      %else
      %define %$res_ebp         0
      %endif

      ; Reserve stack space for local variables (i.a.)?
      %define %$res_funcargs    %2    ; stack space for any function arguments
      %define %$res_locals      %1    ; stack space for all local variables
      %define %$res_alignment   0     ; stack space used for alignment

      %if %$res_funcargs < SHADOW_SPACE
      %define %$res_funcargs    SHADOW_SPACE
      %endif

      %define %$local_size  (%$res_funcargs + %$res_locals + %$res_alignment)

      %if %$res_alignment == 0
      %xdefine %$res_alignment  ((STACK_ALIGN - (sizeof(NASMX_PTR) + %$res_ebp + %$local_size) % STACK_ALIGN) % STACK_ALIGN)
      %endif

      %if %$local_size > 0
      sub   __SP, %$local_size
      %endif
  %endmacro

; void  leave (void);
  %imacro leave 0
    %ifctx  enter_ctx
      %if %$res_ebp > 0               ; Pop stack frame?
      mov   __SP, __BP
      pop   __BP

      %elif %$local_size > 0
      add   __SP, %$local_size
      %endif

      %pop
    %else
      %error "expected 'enter' before 'leave'"
    %endif
  %endmacro

;***  Floating-point Unit:
  %idefine FPOP            FSTP ST0
  %idefine FPOP2           FCOMPP

;***  Application-specific:
  %define  SSE_SUPPORT_41

; %define  MAX_ITERATIONS               [vmode_colors]
  %define  MAX_ITERATIONS_8BPP          255

  ; max(radius(fractview=1)) = 34.77 <=> (34,77*8192)^2 = 48305^2 = -1961594271 = 2333373025
  %define  FIXED_PRECISION              13

  %define  MANDEL_CARDIOID_PERIOD2BULB
  %define  MANDEL_CARDIOID_ITER         3

  %macro  ds_prefix 0
    DB      3Eh
  %endmacro


;===  Constant initialized data:  ===========================================
SECTION .%[_rdata] align=64 ; rdata align=8
; MandelbrotPixelColor{SSE,AVX*}
  escape_radius  times  32  declare(float32_t)    4.0

; MandelbrotPixelColorFLOAT
  cardioid_sub              declare(float32_t)    0.25
  period2_add               declare(float32_t)    1.25
  period2_mul               declare(float32_t)    16.0


;===  Writable initialized data:  ===========================================
SECTION .data align=32 ; data align=4


;===  Reserved uninitialized data:  =========================================
SECTION .bss align=32 ; bss align=4
  bkp_xmm6        reserve(float32_t)    16
  bkp_xmm7        reserve(float32_t)    16


;===  Non-writable executable code:  ========================================
;
; Application binary interface (ABI) - Calling Convention:
; --------------------------------------------------------
;
; Direction flag (DF):       must be cleared before any call or return.
;
; FPU registers:             the floating point register stack (ST0 - ST7) must
;                            be emptied before any call or return (EMMS), except
;                            for any return value in ST0.
;
; FPU control word:          the floating-point unit control word must be saved
;                            and restored before any call or return by any func-
;                            tion modifying it (FSTCW/FNSTCW and FLDCW)
;
; SSE control/status MXCSR:  the SIMD floating-point control and status register
;                            MXCSR must be saved and restored before any call or
;                            return by any function modifying bits 6-15 (STMXCSR
;                            and LDMXCSR).
;
; MMX registers:             can be used freely by any function that DOES NOT use
;                            floating point registers; if used, however, the FPU
;                            tag word needs to be cleared by issuing EMMS before
;                            any call or return.
;
; YMM registers:             functions using these registers should issue the
;                            VZEROUPPER/VZEROALL instructions before any call
;                            or return (except if the YMM registers are used
;                            for parameters or the return value).
;
;                            32-BIT OS              64-BIT OS
; ---------------------------------------------------------------------------
; 1. Scratch registers    :  EAX, ECX, EDX          RAX, RCX, RDX
;       [lin64,bsd64,osx64]                         RSI, RDI
;                                                   R8 - R11
;                            ST0 - ST7              ST0 - ST7
;                            XMM0 - XMM7            XMM0 - XMM5
;       [lin64,bsd64,osx64]                                XMM6 - XMM15
;                            YMM0 - YMM7            YMM0 - YMM5
;       [lin64,bsd64,osx64]                                YMM6 - YMM15
;                   [win64]                                YMM6H - YMM15H
; ---------------------------------------------------------------------------
; 2. Callee-save registers:  EBX, ESI, EDI, EBP     RBX, RSI, RDI, RBP
;                                                   R12 - R15
;                   [win64]                         XMM6 - XMM15
; ---------------------------------------------------------------------------
; 3. Parameter registers  :  <stack>
;            [all-fastcall]  ECX, EDX
;       [lin64,bsd64,osx64]     parameters 1 - 14:  RDI, RSI, RDX, RCX, R8, R9, XMM0 - XMM7, YMM0 - YMM7
;                   [win64]     parameters 1 -  4:  RCX / XMM0, RDX / XMM1, R8 / XMM2, R9 / XMM3
;
;  * Remark 1:
;    The stack pointer must be aligned by the stack word size (16-, 32, or
;    64-bit) at all times.
;
;  * Remark 2 (float, double):
;    Float and double types are transferred in XMM registers in 64 bit mode,
;    otherwise on the stack.
;
;  * Remark 3 (all 64 bit systems):
;    All systems require the stack pointer to be aligned by 16 at every func-
;    tion call instruction (except if at least one function parameter of type
;    __m356 is transferred on the stack, in which case Unix systems align the
;    parameter by 32 before the call).
;
;  * Remark 4 (only Windows 64):
;    Before passing any arguments, the caller must reserve 32 bytes of stack
;    space as "shadow space" (where the callee may save parameters 1-4), even
;    if there are no parameters.
; ---------------------------------------------------------------------------
; 4. Registers for return:   EAX                    RAX
;                   [win32]  EDX (upper 32-bit)
;       [lin64,bsd64,osx64]                         RDX
;                            ST0                    ST0
;                   [lin32]  XMM0, YMM0
;                                                   XMM0
;                                                   YMM0
;
;  * Remark (all systems):
;    Objects that do not fit into return registers are returned to a caller-
;    supplied storage space, whose address is passed in an implicit parameter.
; ***************************************************************************
SECTION .text ; code align=16


; WIN-x86/x64, LIN-x64 TESTED OK; [however: program ends with "segmentation fault" under LIN-x64?]
;----------------------------------------------------------------------------
; // Use integer math when the pixel spacing is greater than the Escape
; // Radius (2.0) divided by 2^16 <=> pixel spacing > 0,000030517578125
; extern void MandelbrotPixelColorFIXED (unsigned long x: ECX, unsigned long y: EDX, BOOL screenCoords: ESI);
;----------------------------------------------------------------------------
ALIGN 16
MandelbrotPixelColorFIXED:
  enter   32, 0, 1
  astack  ALIGN_DEF                           ; Inlining detection

  %define _bkp_ebp  sploc1                                                  ; bkp(EBP)
  %define _bkp_ebx  sploc1 + 1*sizeof(dword)+PTR_OVERHEAD                   ; bkp(EBX)
  %define _bkp_esi  sploc1 + 1*sizeof(dword)+PTR_OVERHEAD + 1*sizeof(qword) ; bkp(ESI)
  %define _c_imag   sploc1 + 1*sizeof(dword)+PTR_OVERHEAD + 2*sizeof(qword) ; fixed c_imag
  %define _c_curr   sploc1 + 2*sizeof(dword)+PTR_OVERHEAD + 2*sizeof(qword) ; fixed c_curr

  mov     [_bkp_ebx], __BX
  mov     [_bkp_esi], __SI
  mov     [_bkp_ebp], __BP

  %if __BITS__ < 64
    mov     EDX, [bparg1 + 1*sizeof(dword)]   ; EDX(Z_imag) = y = _c_imag
    mov     EBP, [bparg1]                     ; EBP(Z_real) = x = _c_curr = _c_real
  %else
    %if _WINDOWS
;   mov     EDX, EDX
    mov     EBP, ECX
    %else
    mov     EDX, ESI
    mov     EBP, EDI
    %endif
  %endif

  mov     [_c_curr], EBP                      ; _c_curr = Z_real  ; see ‘_MandelbrotPixelColorFIXED_Enter:’
  mov     [_c_imag], EDX                      ; _c_imag = Z_imag

  jmp     _MandelbrotPixelColorFIXED_Enter
MandelbrotPixelColorFIXED_Leave:

  mov     __BP, [_bkp_ebp]  ; pop     EBP     ; x64: instruction not supported in 64-bit mode
  mov     __SI, [_bkp_esi]  ; pop     ESI     ; x64: instruction not supported in 64-bit mode
  mov     __BX, [_bkp_ebx]  ; pop     EBX     ; x64: instruction not supported in 64-bit mode
  leave
  RET


; WIN-x86/x64; LIN-x64: TESTED OK
;----------------------------------------------------------------------------
; extern BOOL FractalDrawFIXED (unsigned long l: ECX, unsigned long u: EDX,
;                               unsigned long r: (ESI), unsigned long b: (EDI));
;
;         : c_imag
;         : c_real
;   EDX   : Z_imag
;   EBP   : Z_real
;   ---------------------
;   EBX   : Z_real^2  <temporary to each iteration>
;   ECX   : Z_imag^2  <temporary to each iteration>
;   ---------------------
;         : <temoprary>
;   ESI   : MAX_ITERATIONS
;         : screen_width
;   EDI   : pixel_ptr
;----------------------------------------------------------------------------
FractalDrawFIXED:
  push    __BX    ; push    EBX               ; x64: instruction not supported in 64-bit mode
; %if (__BITS__ < 64) || _WINDOWS
  push    __SI    ; push    ESI               ; x64: instruction not supported in 64-bit mode
  push    __DI    ; push    EDI               ; x64: instruction not supported in 64-bit mode
; %endif

  enter   32, 0, 1
  astack  ALIGN_DEF                           ; Inlining detection
  push    __BP    ; push    EBP               ; x64: instruction not supported in 64-bit mode

  %if __BITS__ < 64
    mov     ECX, [bparg1 + 3*sizeof(dword)]
    mov     EDX, [bparg1 + 4*sizeof(dword)]
    mov     ESI, [bparg1 + 5*sizeof(dword)]
    mov     EDI, [bparg1 + 6*sizeof(dword)]
  %else
    %if _WINDOWS
    mov     ESI, R8D
    mov     EDI, R9D
    %else
    mov     R8D, EDX              ; [unx64] ECX, EDX, ESI, EDI are scratch registers!
    mov     R9D, ECX
    mov     ECX, EDI
    mov     EDX, ESI
    mov     ESI, R8D
    mov     EDI, R9D
    %endif
  %endif

  %define _bkp_ebp  sploc1                                        ; bkp(EBP)
  %define _l_width  sploc1 + sizeof(NASMX_PTR)                    ; int   l_width
  %define _dummy    sploc1 + sizeof(NASMX_PTR) + 1*sizeof(dword)  ; <filler>
  %define _x        sploc1 + sizeof(NASMX_PTR) + 2*sizeof(dword)  ; int   x
  %define _y        sploc1 + sizeof(NASMX_PTR) + 3*sizeof(dword)  ; int   y
  %define _c_imag   sploc1 + sizeof(NASMX_PTR) + 4*sizeof(dword)  ; fixed c_imag
  %define _c_curr   sploc1 + sizeof(NASMX_PTR) + 5*sizeof(dword)  ; fixed c_curr
  %define _c_real   sploc1 + sizeof(NASMX_PTR) + 6*sizeof(dword)  ; fixed c_real
  %define _n_line   sploc1 + sizeof(NASMX_PTR) + 7*sizeof(dword)  ; int   n_line

  ; n_line := [byte] increment to move to column ECX(=l) in the next line
  mov     EAX, [screen_width]                 ; EAX(_n_line) = screen_width
  mov     [_l_width], EAX                     ; _l_width = screen_width
  dec     EAX                                 ; EAX = screen_width-1
  sub     EAX, ESI                            ; EAX = screen_width-1 - r
  add     EAX, ECX                            ; EAX = screen_width-1 - r + l
  mov     [_n_line], EAX                      ; _n_line = screen_width-1 - r + l

  ; l_width := counter value to loop over all bytes(=r-l+1) in a single line
  sub     [_l_width], EAX                     ; _l_width = screen_width - n_line

  ; _y := b - u
  mov     EAX, EDI                            ; EAX = b
  sub     EAX, EDX                            ; EBP = b - u
  mov     [_y], EAX                           ; _y = screen_height

  ; __DI : pixel_ptr
  mov     EDI, [screen_width]                 ; EAX = pitch_bytes
  imul    EDI, EDX                            ; EAX = u*pitch_bytes
  add     EDI, ECX                            ; EAX = u*pitch bytes + l
  add     __DI, [pixel_data]                  ; __DI = pixel_data + u*pitch bytes + l

  ; EDX(Z_imag) = _c_imag
; mov     EDX, [bploc1 + 5*sizeof(dword)]     ; EDX(Z_imag) = u  // argument u is already in EDX!
  imul    EDX, [imag_factor]                  ; EDX(Z_imag) = imag_factor * u
  xor     EAX, EAX                            ; EAX(fractview) = 0
; add     EDX, [EAX*8 + leftUpper + 4]        ; x64: 'ADDR32' relocation to 'leftUpper' invalid
  lea     __SI, [rel leftUpper]
  add     EDX, [__AX*8 + __SI + 4]            ; EDX(Z_imag) = leftUpper [fractview].imag + u*imag_factor
  mov     [_c_imag], EDX                      ; _c_imag = leftUpper [fractview].imag + u*imag_factor

  ; EBP(Z_real) = _c_real
  mov     EBP, ECX                            ; EBP(Z_real) = l
  mov     ECX, [_l_width]                     ; ECX(_l_width)
  imul    EBP, [real_factor]                  ; EBP(Z_real) = real_factor * l
; add     EBP, [EAX*8 + _leftUpper]           ; x64: 'ADDR32' relocation to 'leftUpper' invalid
  add     EBP, [__AX*8 + __SI]                ; EBP(Z_real) = leftUpper [fractview].real + l*real_factor;
; ds_prefix                                   ; align .for_col to paragraph(?)
  mov     [_c_real], EBP                      ; _c_real = leftUpper [fractview].imag + u*imag_factor

  FractalDrawFIXED.for_row:
    mov     [_x], ECX                         ; _x = _l_width

    FractalDrawFIXED.for_col:
      ; Inlining detection expects ESP%ALIGN_DEF!=0 at beginning of _MandelbrotPixelColorFIXED_Enter:
      _MandelbrotPixelColorFIXED_Enter:
    ; {
        mov     [_c_curr], EBP                ; _c_curr = _c_real
        mov     ESI, MAX_ITERATIONS_8BPP      ; ESI(255)

      .iter_func:
        %if FIXED_PRECISION <= 13
        mov     ECX, EDX                      ; ECX(Z_imag)
        mov     EBX, EBP                      ; EBX(Z_real)

        imul    ECX, ECX                      ; ECZ = Z_imag^2
        shr     ECX, FIXED_PRECISION          ; unsigned ECX: Fixed-point correction
        mov     EAX, ECX                      ; EAX(radius) = Z_imag^2

        imul    EBX, EBX                      ; ECX = Z_real^2
        shr     EBX, FIXED_PRECISION          ; unsigned ECX: Fixed-point correction
        add     EAX, EBX                      ; EAX(radius) = Z_imag^2 + Z_real^2

        cmp     EAX, (4 << FIXED_PRECISION)   ; radius ? 4.0
        ja      .calc_finished                ; if (radius > 4.0) break

        mov     EAX, EDX                      ; EAX = Z_imag
        sub     EBX, ECX                      ; EBX = Z_real^2 - Z_imag^2
        add     EBX, [_c_curr]                ; EBX = Z_real^2 - Z_imag^2 + c_real

        imul    EAX, EBP                      ; EAX = Z_imag*Z_real
        sar     EAX, (FIXED_PRECISION-1)      ; signed EAX: Fixed-point correction
        add     EAX, [_c_imag]                ; EAX = 2.0*Z_imag*Z_real + c_imag

        mov     EDX, EAX
        mov     EBP, EBX
        %else
        %error "Unsupported fixed-precision setting"
        %endif
        sub     ESI, 1                        ; i--
        jnz     .iter_func

      .calc_finished:
        mov     EAX, ESI                      ; EAX(i)

        ; Test for inlined call:
        test    ESP, (ALIGN_DEF - 1)          ; (ESP % ALIGN_DEF) == 0?
        jz      MandelbrotPixelColorFIXED_Leave; return
    ; }

      mov     byte [__DI], AL                 ; PutPixel (x, y, color);

      mov     EBP, [_c_curr]                  ; EBP(_c_curr)
      add     EBP, [real_factor]              ; _c_curr += real_factor
      inc     __DI                            ; pixel_ptr++
      sub     dword [_x], 1                   ; _x--
      mov     EDX, [_c_imag]                  ; _c_imag = leftUpper [fractview].imag + u*imag_factor

      jnz     FractalDrawFIXED.for_col

    mov     EAX, [_n_line]                    ; EAX = _n_line
    add     __DI, __AX                        ; __DI(pixel_ptr) += _n_line
    add     EDX, [imag_factor]                ; EDX(c_imag)
    sub     dword [_y], 1                     ; _y--
    mov     [_c_imag], EDX                    ; c_imag += imag_factor
    mov     EBP, [_c_real]                    ; c_curr = c_real
    mov     ECX, [_l_width]                   ; ECX(_l_width)
    jns     FractalDrawFIXED.for_row

  mov     __BP, [__SP]   ; mov     EBP, [ESP] ; x64: instruction not supported in 64-bit mode
  leave

; %if (__BITS__ < 64) || _WINDOWS
  pop     __DI           ; pop     EDI        ; x64: instruction not supported in 64-bit mode
  pop     __SI           ; pop     ESI        ; x64: instruction not supported in 64-bit mode
; %endif
  pop     __BX           ; pop     EBX        ; x64: instruction not supported in 64-bit mode

  mov     EAX, 1                              ; retval = TRUE
  RET


; WIN-x86/x64; LIN-x64: TESTED OK
;----------------------------------------------------------------------------
; // x87 FPU Control Word:
; extern unsigned int FPUSetControlWord (unsigned int control_word : ECX, unsigned int control_mask : EDX);
;----------------------------------------------------------------------------
FPUSetControlWord:
  enter   2, 0
  %if __BITS__ < 64
    mov     ECX, [bparg1]
    mov     EDX, [bparg1 + sizeof(uint32_t)]
  %elif !_WINDOWS
    mov     ECX, EDI              ; [unx64] ECX, EDX are scratch registers!
    mov     EDX, ESI
  %endif

  ; local short fpu_cw = REG_CW
  fwait
  fnstcw  word [__SP]
  movzx   EAX, word [__SP]

  ; ECX = ECX(new) & EDX(mask)
  and     ECX, EDX

  ; EDX = ~EDX(mask) & EAX
  not     EDX
  and     EDX, EAX

  ; EDX = (~EDX(mask) & EAX) | (ECX(new) & EDX(mask))
  or      EDX, ECX

  ; REG_CW = DX
  mov     [__SP], DX
  fldcw   word [__SP]

  leave
  RET


; WIN-x86/x64; LIN-x64: TESTED OK
;----------------------------------------------------------------------------
; // x87 FPU Control Word:
; extern unsigned int SSESetControlWord (unsigned int control_word : ECX, unsigned int control_mask : EDX);
;----------------------------------------------------------------------------
SSESetControlWord:
  enter   4, 0
  %if __BITS__ < 64
    mov     ECX, [bparg1]
    mov     EDX, [bparg1 + sizeof(uint32_t)]
  %elif !_WINDOWS
    mov     ECX, EDI              ; [unx64] ECX, EDX are scratch registers!
    mov     EDX, ESI
  %endif

  ; local int sse_cw = REG_CW
  stmxcsr dword [__SP]
  mov     EAX, dword [__SP]

  ; ECX = ECX(new) & EDX(mask)
  and     ECX, EDX

  ; EDX = ~EDX(mask) & EAX
  not     EDX
  and     EDX, EAX

  ; EDX = (~EDX(mask) & EAX) | (ECX(new) & EDX(mask))
  or      EDX, ECX

  ; REG_CW = DX
  ldmxcsr dword [__SP]

  leave
  RET


; WIN-x86/x64, LIN-x64 TESTED OK; [however: program ends with "segmentation fault" under LIN-x64?]
;----------------------------------------------------------------------------
; extern void MandelbrotPixelColorFLOAT (unsigned long x: ECX, unsigned long y: EDX, BOOL screenCoords: ESI);
;----------------------------------------------------------------------------
ALIGN 16
MandelbrotPixelColorFLOAT:
  enter   24, 0, 1
  astack  ALIGN_DEF                           ; Inlining detection

  %define _bkp_ebx       sploc1                                     ; bkp(EBX)
  %define _bkp_esi       sploc1 + 1*sizeof(qword)                   ; bkp(ESI)
  %define _cardioid_im2  sploc1 + 2*sizeof(qword)                   ; float card_im2
  %define _calc_radius   sploc1 + 2*sizeof(qword) + 1*sizeof(dword) ; float calc_radius
  %define _temp          _calc_radius

  mov     [_bkp_ebx], __BX
  mov     [_bkp_esi], __SI

  %if __BITS__ < 64
    mov     ECX, [bparg1]                     ; ECX(c_real)
    mov     EDX, [bparg1 + 1*sizeof(dword)]   ; EDX(c_imag)
  %endif

; fld     dword [imag_factor]                 ; ST3(imag_factor)
  fld     dword [real_factor]                 ; ST2(real_factor)

  %if __BITS__ < 64
  mov     [_temp], EDX
  %else
  movss   [_temp], xmm1
  %endif
  fld     dword [_temp]                       ; ST1(c_imag)

  %if __BITS__ < 64
  mov     [_temp], ECX
  %else
  movss   [_temp], xmm0
  %endif
  fld     dword [_temp]                       ; ST0(c_real)

  %ifdef MANDEL_CARDIOID_PERIOD2BULB
  fld     ST1                                 ; ST0(c_imag)
  fmul    ST0                                 ; ST0 = c_imag^2
  fstp    dword [_cardioid_im2]               ; cardioid_im2 = c_imag^2
  %endif
  xor     ECX, ECX                            ; ECX(l-r)

  jmp     _MandelbrotPixelColorFLOAT_Enter
MandelbrotPixelColorFLOAT_Leave:
  mov     EAX, EBX                            ; EAX(iter_count)

  fpop                                        ; fpop(ST0 = c_real)
  fpop                                        ; fpop(ST1 = c_imag)
  fpop                                        ; fpop(ST2 = real_factor)
; fpop                                        ; fpop(ST3 = imag_factor)

  mov     __SI, [_bkp_esi]  ; pop     ESI     ; x64: instruction not supported in 64-bit mode
  mov     __BX, [_bkp_ebx]  ; pop     EBX     ; x64: instruction not supported in 64-bit mode
  leave
  RET


; WIN-x86/x64; LIN-x64: TESTED OK
;----------------------------------------------------------------------------
; extern BOOL FractalDrawFLOAT (unsigned long l: ECX, unsigned long u: EDX,
;                               unsigned long r: (ESI), unsigned long b: (EDI));
;
;  (ST8   | imag_factor |)
;   ST7   | real_factor |
;   ST6   | c_imag      |
;   ST5   | c_real      |
;   ST4   | Z_imag      |
;   ST3   | Z_real      |
;   ---------------------
;   ST2   | Z_real^2    |  <temporary to each iteration>
;   ST1   | Z_imag^2    |  <temporary to each iteration>
;   ST0   | radius      |  <temporary to each iteration>
;   ---------------------
;   EAX   : <temoprary>
;   EBX   : MAX_ITERATIONS
;   ECX   : l - r
;   EDX   : pixel_ptr
;----------------------------------------------------------------------------
ALIGN 16
FractalDrawFLOAT:
  push    __BX
; %if (__BITS__ < 64) || _WINDOWS
  push    __SI    ; push    ESI               ; x64: instruction not supported in 64-bit mode
  push    __DI    ; push    EDI               ; x64: instruction not supported in 64-bit mode
; %endif

  enter   24, 0, 1
  astack  ALIGN_DEF                           ; Inlining detection
  sub     __SP, 8

  %if __BITS__ < 64
    mov     ECX, [bparg1 + 3*sizeof(dword)]
    mov     EDX, [bparg1 + 4*sizeof(dword)]
    mov     ESI, [bparg1 + 5*sizeof(dword)]
    mov     EDI, [bparg1 + 6*sizeof(dword)]
  %else
    %if _WINDOWS
    mov     ESI, R8D
    mov     EDI, R9D
    %else
    mov     R8D, EDX              ; [unx64] ECX, EDX, ESI, EDI are scratch registers!
    mov     R9D, ECX
    mov     ECX, EDI
    mov     EDX, ESI
    mov     ESI, R8D
    mov     EDI, R9D
    %endif
  %endif

  %define _c_real        sploc1                   ; float c_real
  %define _l_width       sploc1 + 1*sizeof(dword) ; int   l_width
  %define _n_line        sploc1 + 2*sizeof(dword) ; int   n_line
  %define _y             sploc1 + 3*sizeof(dword) ; int   y
  %define _cardioid_im2  sploc1 + 4*sizeof(dword) ; float cardioid_im2
  %define _calc_radius   sploc1 + 5*sizeof(dword) ; float calc_radius
  %define _temp          _calc_radius

  ; n_line := [byte] increment to move to column ECX(=l) in the next line
  mov     EAX, [screen_width]                 ; EAX(_n_line) = screen_width
  mov     [_l_width], EAX                     ; _l_width = screen_width
  dec     EAX                                 ; EAX = screen_width-1
  sub     EAX, ESI                            ; EAX = screen_width-1 - r
  add     EAX, ECX                            ; EAX = screen_width-1 - r + l
  mov     [_n_line], EAX                      ; _n_line = screen_width-1 - r + l

  ; l_width := counter value to loop over all bytes(=r-l+1) in a single line
  sub     [_l_width], EAX                     ; _l_width = screen_width - n_line

  ; _y := b - u
  mov     EAX, EDI                            ; EAX = b
  sub     EAX, EDX                            ; EBP = b - u
  mov     [_y], EAX                           ; _y = screen_height

  ; __DI : pixel_ptr
  mov     EDI, [screen_width]                 ; EAX = pitch_bytes
  imul    EDI, EDX                            ; EAX = u*pitch_bytes
  add     EDI, ECX                            ; EAX = u*pitch bytes + l
  add     __DI, [pixel_data]                  ; __DI = pixel_data + u*pitch bytes + l

  ; ST1 = leftUpper [fractview].real + l*real_factor
  mov     [_temp], ECX                        ; _temp = l
  xor     __AX, __AX                          ; EAX(fractview) = 0
  fld     dword [real_factor]                 ; ST2(real_factor)
; fld     dword [EAX*8 + leftUpper]           ; x64: 'ADDR32' relocation to 'leftUpper' invalid
  lea     __SI, [rel leftUpper]
  fld     dword [__SI + __AX*8]               ; ST1(c_real) = leftUpper [fractview].real
  fld     ST1                                 ; ST0(real_factor)
  fimul   dword [_temp]                       ; ST0 = real_factor * l
  faddp   ST1, ST0                            ; ST1 += real_factor * l
  fst     dword [_c_real]                     ; _c_real = leftUpper [fractview].real + l*real_factor

  ; ST0 = leftUpper [fractview].imag + u*imag_factor
  mov     [_temp], EDX                        ; _temp = u
; fld     dword [EAX*8 + _leftUpper + 4]      ; x64: 'ADDR32' relocation to 'leftUpper' invalid
  fld     dword [__SI + __AX*8 + 4]           ; ST1(c_imag) = leftUpper [fractview].imag
  fld     dword [imag_factor]                 ; ST0(imag_factor)
  fimul   dword [_temp]                       ; ST0 = imag_factor * u
  faddp   ST1, ST0                            ; ST1 = leftUpper [fractview].imag + u*imag_factor

; ds_prefix                                   ; align .for_col to paragraph
  mov     ECX, [_l_width]                     ; ECX = screen_width - n_line
  fxch    ST0, ST1                            ; ST0(c_real), ST1(c_imag)

  FractalDrawFLOAT.for_row:
    %ifdef MANDEL_CARDIOID_PERIOD2BULB
    fld     ST1                               ; ST0(c_imag)
    fmul    ST0                               ; ST0 = c_imag^2
    fstp    dword [_cardioid_im2]             ; cardioid_im2 = c_imag^2
    %endif

    FractalDrawFLOAT.for_col:
      ; Inlining detection expects ESP%ALIGN_DEF!=0 at beginning of _MandelbrotPixelColorFLOAT_Enter:
      _MandelbrotPixelColorFLOAT_Enter:
    ; {
        fld     ST1                           ; ST4(Z_imag) = c_imag
        fld     ST1                           ; ST3(Z_real) = c_real
        %ifdef MANDEL_CARDIOID_PERIOD2BULB
        mov     EBX, MANDEL_CARDIOID_ITER     ; EBX(i) = MANDEL_CARDIOID_ITER (60% terminate within 3 iterations)
        %else
        mov     EBX, MAX_ITERATIONS_8BPP      ; ESI(255)
        %endif

      .iter_func:
        fld     ST0                           ; ST2(Z_real)
        fmul    ST0                           ; Z_real^2
        fld     ST2                           ; ST1(Z_imag)
        fmul    ST0                           ; Z_imag^2
        fld     ST0                           ; ST0(Z_imag^2)
        fadd    ST2                           ; ST0(radius) = Z_imag^2 + Z_real^2
      ; fcomip  dword [escape_radius]
        fistp   int32_t [_calc_radius]        ; [calc_radius] = ST0
        cmp     int32_t [_calc_radius], 4     ; radius ? 4.0

        fld     ST2                           ; ST0(Z_real)
        fadd    ST0                           ; ST0 = 2*Z_real
        fmulp   ST4, ST0                      ; ST4(Z_imag) *= (2 * Z_real)
        fsubp   ST1, ST0                      ; ST0 = Z_real^2 - Z_imag^2
        fstp    ST1                           ; ST1(Z_real) = Z_real^2 - Z_imag^2
        jbe     .next_iteration               ; if (radius > 4.0)
                                              ; {
        %ifdef MANDEL_CARDIOID_PERIOD2BULB
        or      ECX, ECX                      ;   UPD_FLAGS
        jns     .adjust_count                 ;   if (ECX > 0) { ROUND1: goto adjust_count }
        %endif
        jmp     .calc_finished                ;   break
                                              ; }
      .next_iteration:
        sub     EBX, 1                        ; i--
        fadd    ST2                           ; ST0(Z_real) = Z_real^2 - Z_imag^2 + c_real
        fxch    ST1                           ; ST0 <-> ST1
        fadd    ST3                           ; ST0(Z_imag) = 2*Z_real*Z_imag + c_imag
        fxch    ST1                           ; ST0 <-> ST1
        jnz     .iter_func                    ; (i > 0)?

        %ifdef MANDEL_CARDIOID_PERIOD2BULB
      .cardioid_test:
        bts     ECX, 31                       ; CF=ECX[31]; ECX[31]=1
        jc      .calc_finished                ; ECX<0? => Finished!
        mov     EBX, -(MAX_ITERATIONS_8BPP-MANDEL_CARDIOID_ITER); EBX = -252

        ; Cardioid test:
        fld     ST2                           ; ST2(x_) = c_real
        fsub    dword [cardioid_sub]          ; ST2(x_) = c_real-0.25
        fld     ST0                           ; ST1(q) = c_real-0.25
        fmul    ST0                           ; ST1(q) = (c_real-0.25)^2
        fadd    dword [_cardioid_im2]         ; ST1(q) = (c_real-0.25)^2 + c_imag^2
        fld     ST0                           ; ST0(test) = q
        fadd    ST2                           ; ST0(test) = q + x_
        fmul    ST1                           ; ST0(test) = q*(q + x_)
        fmul    dword [escape_radius]         ; ST0(test) = 4.0f * q*(q + x_)

        fcomp   dword [_cardioid_im2]         ; 4.0f * q*(q + x_) ? c_imag^2
        fstsw   ax                            ; AX = fpu[status_word]
      ; test    ax, 0100h                     ; Intel 1 - Basic Architecture, Table 8.8
        sahf                                  ; AH = EFLAGS(SF:ZF:0:AF:0:PF:1:CF)
        fpop                                  ; fpop(ST1 = q)
        jb      .cardioid_within
                                              ; if (4.0f * q*(q + x_) >= c_imag^2)
        ; Period-2 bulb:                      ; {
        fadd    dword [period2_add]           ;   ST1(x_) = c_real+1.0
        fmul    ST0                           ;   ST1 = (c_real+1.0)^2
        fadd    dword [_cardioid_im2]         ;   ST1 = (c_real+1.0)^2 + c_imag^2
        fmul    dword [period2_mul]           ;   ST1 = 16.0f * ((c_real+1.0)^2 + c_imag^2)
        fld1                                  ;   ST0 = 1.0f

        fcomip  ST1                           ;   1.0f ? 16.0f * ((c_real+1.0)^2 + c_imag^2)
        fpop                                  ;   fpop(ST1 = (c_real+1.0)^2 + c_imag^2)
        ja      .period2_within               ;   if (1.0f <= 16.0f * ((c_real+1.0)^2 + c_imag^2)
                                              ;   {
        neg     EBX                           ;     EBX = (MAX_ITERATIONS-MANDEL_CARDIOID_ITER)  // ROUND 1
        jmp     .iter_func                    ;     goto .iter_func
                                              ;   }
      .cardioid_within:                       ; }
        fpop                                  ; fpop(ST2 = c_real-0.25)
      .period2_within:
      .adjust_count:
        add     EBX, (MAX_ITERATIONS_8BPP-MANDEL_CARDIOID_ITER) ; ROUND1: EBX += 252
        %endif

      .calc_finished:
        fpop                                  ; fpop(ST3 = Z_real)
        fpop                                  ; fpop(ST4 = Z_imag)
        mov     EAX, EBX                      ; EAX(i)

        ; Test for inlined call:
        test    ESP, (ALIGN_DEF - 1)          ; (ESP % ALIGN_DEF) == 0?
        jz      MandelbrotPixelColorFLOAT_Leave; return
    ; }

      %ifdef MANDEL_CARDIOID_PERIOD2BULB
      and     ECX, 7FFF_FFFFh                 ; SIGN=0; ECX>=0: Round2
      %endif
      mov     byte [__DI], AL                 ; PutPixel (x, y, color);
      add     __DI, 1                         ; pixel_ptr++
      fadd    ST0, ST2                        ; c_real += real_factor

      sub     ECX, 1                          ; w--
      jnz     FractalDrawFLOAT.for_col

    mov     EAX, [_n_line]                    ; EAX = _n_line
    add     __DI, __AX                        ; __DI(pixel_ptr) += _n_line
    fpop                                      ; fpop(ST0 = c_real)
    fadd    dword [imag_factor]               ; ST1 += imag_factor
    fld     dword [_c_real]                   ; ST0(_c_curr) = c_real
    sub     dword [_y], 1                     ; _y--
    mov     ECX, [_l_width]                   ; ECX(_l_width)
    jns     FractalDrawFLOAT.for_row

  fpop                                        ; fpop(ST0 = c_real)
  fpop                                        ; fpop(ST1 = c_imag)
  fpop                                        ; fpop(ST2 = real_factor)
  leave

; %if (__BITS__ < 64) || _WINDOWS
  pop     __DI           ; pop     EDI        ; x64: instruction not supported in 64-bit mode
  pop     __SI           ; pop     ESI        ; x64: instruction not supported in 64-bit mode
; %endif
  pop     __BX           ; pop     EBX        ; x64: instruction not supported in 64-bit mode

  mov     EAX, 1                              ; retval = TRUE
  RET


;----------------------------------------------------------------------------
; extern void MandelbrotPixelColor4SSE1 (SIMD_FLT *c_real: RCX, SIMD_FLT *c_imag: RDX, SIMD_INT *result: R8);
;
;   XMM7  | c_imag        |
;   XMM6  | c_real        |
;   XMM5  | Z_imag        |  NEW(Z_imag)
;   XMM4  | Z_real        |  NEW(Z_real)
;   -----------------------
;   XMM3  | iter_count    |
;   XMM2  | iter_incr     |
;   XMM1  | radius        |  <temporary to each iteration>
;   XMM0  | Bkp(Z_imag^2) |  <temporary to each iteration>
;   -----------------------
;   EAX   : SIMD_FLT *c_real
;   ECX   : SIMD_FLT *c_imag
;   EDX   : iter_count
;----------------------------------------------------------------------------
ALIGN 16
MandelbrotPixelColor4SSE1:
  enter   40, 0

  %if __BITS__ < 64
    mov     ECX, [__SP + 40 + 2*sizeof(NASMX_PTR)]  ; SIMD_FLT *c_real
    mov     EDX, [__SP + 40 + 3*sizeof(NASMX_PTR)]  ; SIMD_FLT *c_imag
  %else
    %if _WINDOWS                              ; [win64] callee-save registers
    movaps  [__SP     ], XMM6                 ; bkp(XMM6)
    movaps  [__SP + 16], XMM7                 ; bkp(XMM7)
    %else
    mov      R8, RDX                          ; [unx64] RDX, RSI, RDI are scratch registers!
    mov     RDX, RSI
    mov     RCX, RDI
    %endif
  %endif

  mov     __AX, __CX                          ; EAX(->c_real)
; mov     __DX, __DX                          ; EDX(->c_imag)
  mov     ECX, MAX_ITERATIONS_8BPP            ; MAX_ITERATIONS
  sub     ECX, 1                              ; ECX(vmode_colors-1)
  movaps  XMM6, [__AX]                        ; XMM6(c_real)
  movaps  XMM7, [__DX]                        ; XMM7(c_imag)
  movaps  XMM4, XMM6                          ; XMM4(Z_real) = c_real
  movaps  XMM5, XMM7                          ; XMM5(Z_imag) = c_imag
  movd    XMM3, ECX                           ; XMM3[31..0] = vmode_colors-1
  pshufd  XMM3, XMM3, 0                       ; Shuffle low 32 bits into all vector positions
  pcmpeqd XMM2, XMM2                          ; XMM2(iter_inc) = <iteration increment> = -1

.iter_func:
    movaps    XMM1, XMM5                      ; XMM1(radius) = Z_imag
    mulps     XMM1, XMM1                      ; XMM1(radius) = Z_imag^2
    addps     XMM5, XMM5                      ; XMM5(NEW(Z_imag)) = 2*Z_imag
    mulps     XMM5, XMM4                      ; XMM5(NEW(Z_imag)) = 2*Z_imag*Z_real
    movaps    XMM0, XMM1                      ; XMM0(Backup) = Z_imag^2
    subps     XMM0, XMM6                      ; XMM0(Backup) = Z_imag^2 - c_real [Variant 2: faster to subtract c_real, than to add it later.]
    mulps     XMM4, XMM4                      ; XMM4(NEW(Z_real)) = Z_Real^2
    addps     XMM5, XMM7                      ; XMM5(NEW(Z_imag)) = 2*Z_imag*Z_real + c_imag
    addps     XMM1, XMM4                      ; XMM1(radius) = Z_imag^2 + Z_real^2
    subps     XMM4, XMM0                      ; XMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2 (+ c_real)
;   addps     XMM4, XMM6                      ; XMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2 + c_real [Variant 1]

    ; The result of each comparison is a doubleword mask of
    ; all 1s (comparison true) or all 0s (comparison false):
    cmpleps   XMM1, [escape_radius]           ; XMM1 = radius <= 4.0?

    xor       EDX, EDX                        ; EDX = 0
    sub       ECX, 1                          ; i--
    pand      XMM2, XMM1                      ; XMM2(iter_inc) &= (radius <= 4.0)
    setnz     DL                              ; DL = (i != 0)
%ifdef SSE_SUPPORT_41 ; SSE 4.1:
    xor       EAX, EAX                        ; EAX = 0
    ptest     XMM2, XMM2                      ; ZF = (XMM2 & XMM2) == 0
    setnz     AL                              ; AL = (XMM2 != 0)
%else ; SSE1:
    pmovmskb  EAX, XMM2                       ; EAX = MSB(XMM2)
    or        EAX, EAX
    setnz     AL
%endif
    and       EDX, EAX                        ; EDX = (i!=0) & (XMM2!=0)
    paddd     XMM3, XMM2                      ; iter_count--
    jnz       .iter_func

  %if __BITS__ < 64
    mov     EAX, [__SP + 40 + 4*sizeof(NASMX_PTR)]  ; SIMD_INT *result
    movdqa  [EAX], XMM3
  %else
    %if _WINDOWS                              ; [win64] callee-save registers
    movaps  XMM6, [__SP     ]                 ; rst(XMM6)
    movaps  XMM7, [__SP + 16]                 ; rst(XMM7)
    %endif
    movdqa  [R8], XMM3
  %endif

  leave
  RET


;----------------------------------------------------------------------------
; extern void MandelbrotPixelColor8AVX1 (SIMD_FLT *c_real: RCX, SIMD_FLT *c_imag: RDX, SIMD_INT *result: R8);
;
;   YMM7  | c_imag        |
;   YMM6  | c_real        |
;   YMM5  | Z_imag        |
;   YMM4  | Z_real        |  NEW(Z_real)
;   -----------------------
;   YMM3  | iter_count    |
;   YMM2  | iter_incr     |
;   YMM1  | radius        |  <temporary to each iteration>
;   YMM0  | Bkp(Z_imag^2) |  <temporary to each iteration>
;   -----------------------
;   EAX   : SIMD_FLT *c_real
;   ECX   : SIMD_FLT *c_imag
;   EDX   : iter_count
;----------------------------------------------------------------------------
ALIGN 16
%if (__BITS__ < 64)         ; align .iter_func: with start of new memory paragraph
; times 15 nop              ; [x86] nearly aligned, nothing to do
%else
  times 6 nop               ; [x64]
%endif
MandelbrotPixelColor8AVX1:
  enter   56, 0

  %if __BITS__ < 64
    mov         ECX, [__SP + 56 + 2*sizeof(NASMX_PTR)]  ; SIMD_FLT *c_real
    mov         EDX, [__SP + 56 + 3*sizeof(NASMX_PTR)]  ; SIMD_FLT *c_imag
  %else
    %if _WINDOWS                              ; [win64] callee-save registers
    movaps      [__SP     ], XMM6             ; bkp(XMM6)
    movaps      [__SP + 16], XMM7             ; bkp(XMM7)
    %if (__BITS__ >= 64)
    movaps      [__SP + 32], XMM8             ; bkp(XMM8)
    %endif
    %else
    mov          R8, RDX                      ; [unx64] RDX, RSI, RDI are scratch registers!
    mov         RDX, RSI
    mov         RCX, RDI
    %endif
  %endif

; Transition YMM register set to state (A):
; vzeroupper                                  ; Zero upper bits of YMM registers [all YMM registers get reloaded anyway!]

  mov           __AX, __CX                    ; EAX(->c_real)
; mov           __DX, __DX                    ; EDX(->c_imag)
  mov           ECX, MAX_ITERATIONS_8BPP      ; MAX_ITERATIONS
  sub           ECX, 1                        ; ECX(vmode_colors-1)
  %if (__BITS__ >= 64)
  vbroadcastss  YMM8, dword [escape_radius]   ; ZMM8(radius) = 4.0
  %endif
  vmovaps       YMM6, [__AX]                  ; YMM6(c_real)
  vmovaps       YMM7, [__DX]                  ; YMM7(c_imag)
  vmovaps       YMM4, YMM6                    ; YMM4(Z_real) = c_real
  vmovaps       YMM5, YMM7                    ; YMM5(Z_imag) = c_imag

  vcvtsi2ss     XMM3, XMM3, ECX               ; YMM3[31..0] = (float)(vmode_colors-1)
  vshufps       XMM3, XMM3, XMM3, 0           ; Shuffle low 32 bits into all vector positions in XMM3
  vinsertf128   YMM3, YMM3, XMM3, 1           ; YMM3(iter_count) = YMM3[255:128] | XMM3[127:0] = 255.0
  mov           EDX, -1                       ; EDX = -1
  vcvtsi2ss     XMM2, XMM2, EDX               ; YMM2[31..0] = (float)-1
  vshufps       XMM2, XMM2, XMM2, 0           ; Shuffle low 32 bits into all vector positions in XMM2
  vinsertf128   YMM2, YMM2, XMM2, 1           ; YMM2(iter_count) = YMM2[255:128] | XMM2[127:0] = 255.0

  xor           EDX, EDX                      ; EDX = 0
  xor           EAX, EAX                      ; EAX = 0

.iter_func:
    vmovaps       YMM1, YMM5                  ; YMM1(radius) = Z_imag
    vmulps        YMM1, YMM1, YMM1            ; YMM1(radius) = Z_imag^2
    vaddps        YMM5, YMM5, YMM5            ; YMM5(NEW(Z_imag)) = 2*Z_imag
    vmulps        YMM5, YMM5, YMM4            ; YMM5(NEW(Z_imag)) = 2*Z_imag*Z_real
    vmovaps       YMM0, YMM1                  ; YMM0(Bkp(Z_imag^2))
    vsubps        YMM0, YMM0, YMM6            ; XMM0(Backup) = Z_imag^2 - c_real [Variant 2: faster to subtract c_real, than to add it later.]
    vmulps        YMM4, YMM4, YMM4            ; YMM4(NEW(Z_real)) = Z_Real^2
    vaddps        YMM5, YMM5, YMM7            ; YMM5(NEW(Z_imag)) = 2*Z_imag*Z_real + c_imag
    vaddps        YMM1, YMM1, YMM4            ; YMM1(radius) = Z_imag^2 + Z_real^2

    vsubps        YMM4, YMM4, YMM0            ; YMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2
;   vaddps        YMM4, YMM6                  ; YMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2 + c_real [Variant 1]

    ; The result of each comparison is a doubleword mask of
    ; all 1s (comparison true) or all 0s (comparison false):
    %if (__BITS__ < 64)                       ; YMM1 = radius <= 4.0?
    vcmpleps      YMM1, YMM1, [escape_radius]
    %else
    vcmpleps      YMM1, YMM1, YMM8
    %endif

    sub           ECX, 1                      ; i--
    vandps        YMM2, YMM2, YMM1            ; YMM2(iter_inc) &= (radius <= 4.0)
    setnz         DL                          ; DL = (i != 0)

    vptest        YMM2, YMM2                  ; ZF = (YMM2 & YMM2) == 0
    setnz         AL                          ; AL = (YMM2 != 0)

    and           EDX, EAX                    ; EDX = (i!=0) & (YMM2!=0)
    vaddps        YMM3, YMM3, YMM2            ; iter_count--
    jnz           .iter_func

  vcvttps2dq    YMM0, YMM3                    ; YMM0 = Truncate(YMM3)
  %if __BITS__ < 64
    mov         EAX, [__SP + 56 + 4*sizeof(NASMX_PTR)]  ; SIMD_INT *result
    vmovdqa     [EAX], YMM0                   ; *a_result = YMM0
  %else
    %if _WINDOWS                              ; [win64] callee-save registers
    movaps      XMM6, [__SP     ]             ; rst(XMM6)
    movaps      XMM7, [__SP + 16]             ; rst(XMM7)
    %if (__BITS__ >= 64)
    movaps      XMM8, [__SP + 32]             ; bkp(XMM8)
    %endif
    %endif
    vmovdqa     [R8], YMM0                    ; *a_result = YMM0
  %endif

; Transition YMM register set to state (A):
  vzeroupper                                  ; Zero upper bits of YMM registers
  leave
  RET


;----------------------------------------------------------------------------
; extern void MandelbrotPixelColor8AVX2 (SIMD_FLT *c_real: RCX, SIMD_FLT *c_imag: RDX, SIMD_INT *result: R8);
;
;   YMM8  | escape_radius |
;   -----------------------
;   YMM7  | c_imag        |
;   YMM6  | c_real        |
;   YMM5  | Z_imag        |
;   YMM4  | Z_real        |  NEW(Z_real)
;   -----------------------
;   YMM3  | iter_count    |
;   YMM2  | iter_incr     |
;   YMM1  | radius        |  <temporary to each iteration>
;   YMM0  | Bkp(Z_imag^2) |  <temporary to each iteration>
;   -----------------------
;   EAX   : SIMD_FLT *c_real
;   ECX   : SIMD_FLT *c_imag
;   EDX   : iter_count
;----------------------------------------------------------------------------
ALIGN 16
%if (__BITS__ < 64)         ; align .iter_func: with start of new memory paragraph
  times 5 nop               ; [x86]
%else
  times 6 nop               ; [x64]
%endif
MandelbrotPixelColor8AVX2:
  enter  56, 0

  %if __BITS__ < 64
    mov         ECX, [__SP + 56 + 2*sizeof(NASMX_PTR)]  ; SIMD_FLT *c_real
    mov         EDX, [__SP + 56 + 3*sizeof(NASMX_PTR)]  ; SIMD_FLT *c_imag
  %else
    %if _WINDOWS                              ; [win64] callee-save registers
    movaps      [__SP     ], XMM6             ; bkp(XMM6)
    movaps      [__SP + 16], XMM7             ; bkp(XMM7)
    %if (__BITS__ >= 64)
    movaps      [__SP + 32], XMM8             ; bkp(XMM8)
    %endif
    %else
    mov          R8, RDX                      ; [unx64] RDX, RSI, RDI are scratch registers!
    mov         RDX, RSI
    mov         RCX, RDI
    %endif
  %endif

; Transition YMM register set to state (A):
; vzeroupper                                  ; Zero upper bits of YMM registers [all YMM registers get reloaded anyway!]

  mov           __AX, __CX                    ; EAX(->c_real)
; mov           __DX, __DX                    ; EDX(->c_imag)
  mov           ECX, MAX_ITERATIONS_8BPP      ; MAX_ITERATIONS
  sub           ECX, 1                        ; ECX(vmode_colors-1)
  %if (__BITS__ >= 64)
  vbroadcastss  YMM8, dword [escape_radius]   ; ZMM8(radius) = 4.0
  %endif
  vmovaps       YMM6, [__AX]                  ; YMM6(c_real)
  vmovaps       YMM7, [__DX]                  ; YMM7(c_imag)
  vmovaps       YMM4, YMM6                    ; YMM4(Z_real) = c_real
  vmovaps       YMM5, YMM7                    ; YMM5(Z_imag) = c_imag
  vmovd         XMM3, ECX                     ; XMM3[31..0] = vmode_colors-1
  vpbroadcastd  YMM3, XMM3                    ; Broadcast low 32 bits into all vector positions
  vpcmpeqd      YMM2, YMM2                    ; YMM2(iter_inc) = -1
  xor           EDX, EDX                      ; EDX = 0
  xor           EAX, EAX                      ; EAX = 0

; Vol. 2B, 4-440:
;   vfmadd213ps   dst, src2, src3  <=>  dst = src2*dst + src3
;   vfmadd231ps   dst, src2, src3  <=>  dst = src2*src3 + dst

; Vol. 2B, 4-462:
;   vfmsub213ps   dst, src2, src3  <=>  dst = src2*dst - src3

.iter_func:
    vmovaps       YMM1, YMM5                  ; YMM1(radius) = Z_imag
    vmulps        YMM1, YMM1, YMM1            ; YMM1(radius) = Z_imag^2
    vaddps        YMM5, YMM5, YMM5            ; YMM5(NEW(Z_imag)) = 2*Z_imag
    vfmadd213ps   YMM5, YMM4, YMM7            ; YMM5(NEW(Z_imag)) = 2*Z_imag*Z_real + c_imag
    vmovaps       YMM0, YMM1                  ; YMM0(Bkp(Z_imag^2))
    vfmadd231ps   YMM1, YMM4, YMM4            ; YMM1(radius) = Z_imag^2 + Z_real^2
    vfmsub213ps   YMM4, YMM4, YMM0            ; YMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2
    vaddps        YMM4, YMM6                  ; YMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2 + c_real

    ; The result of each comparison is a doubleword mask of
    ; all 1s (comparison true) or all 0s (comparison false):
    %if (__BITS__ < 64)                       ; YMM1 = radius <= 4.0?
    vcmpleps      YMM1, YMM1, [escape_radius]
    %else
    vcmpleps      YMM1, YMM1, YMM8
    %endif

    sub           ECX, 1                      ; i--
    vpand         YMM2, YMM2, YMM1            ; YMM2(iter_inc) &= (radius <= 4.0)
    setnz         DL                          ; DL = (i != 0)

    vptest        YMM2, YMM2                  ; ZF = (YMM2 & YMM2) == 0
    setnz         AL                          ; AL = (YMM2 != 0)

    and           EDX, EAX                    ; EDX = (i!=0) & (YMM2!=0)
    vpaddd        YMM3, YMM3, YMM2            ; iter_count--
    jnz           .iter_func

  %if __BITS__ < 64
    mov         EAX, [__SP + 56 + 4*sizeof(NASMX_PTR)]  ; SIMD_INT *result
    vmovdqa     [EAX], YMM3                   ; *a_result = YMM3
  %else
    %if _WINDOWS                              ; [win64] callee-save registers
    movaps      XMM6, [__SP     ]             ; rst(XMM6)
    movaps      XMM7, [__SP + 16]             ; rst(XMM7)
    %if (__BITS__ >= 64)
    movaps      XMM8, [__SP + 32]             ; bkp(XMM8)
    %endif
    %endif
    vmovdqa     [R8], YMM3                    ; *a_result = YMM3
  %endif

; Transition YMM register set to state (A):
  vzeroupper                                  ; Zero upper bits of YMM registers
  leave
  RET


;----------------------------------------------------------------------------
; extern void MandelbrotPixelColor16AVX512F (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result);
;
;  (ZMM8  | imag_factor   |)
;  (ZMM7  | real_factor   |)
;   ZMM7  | c_imag        |
;   ZMM6  | c_real        |
;   ZMM5  | Z_imag        |
;   ZMM4  | Z_real        |
;   -----------------------
;   ZMM3  | iter_count    |
;   ZMM2  | iter_incr     |
;   YMM1  | radius        |  <temporary to each iteration>
;   YMM0  | Bkp(Z_imag^2) |  <temporary to each iteration>
;   -----------------------
;   EAX   : SIMD_FLT *c_real
;   ECX   : SIMD_FLT *c_imag
;   EDX   : iter_count
;----------------------------------------------------------------------------
ALIGN 16
%if (__BITS__ < 64)         ; align .iter_func: with start of new memory paragraph
  times 2 nop               ; [x86]
%else
  times 2 nop               ; [x64]
%endif
MandelbrotPixelColor16AVX512F:
  enter  56, 0

  %if __BITS__ < 64
    mov         ECX, [__SP + 56 + 2*sizeof(NASMX_PTR)]  ; SIMD_FLT *c_real
    mov         EDX, [__SP + 56 + 3*sizeof(NASMX_PTR)]  ; SIMD_FLT *c_imag
  %else
    %if _WINDOWS                              ; [win64] callee-save registers
    movaps      [__SP     ], XMM6             ; bkp(XMM6)
    movaps      [__SP + 16], XMM7             ; bkp(XMM7)
    %if (__BITS__ >= 64)
    movaps      [__SP + 32], XMM8             ; bkp(XMM8)
    %endif
    %else
    mov          R8, RDX                      ; [unx64] RDX, RSI, RDI are scratch registers!
    mov         RDX, RSI
    mov         RCX, RDI
    %endif
  %endif

; Transition ZMM register set to state (A):
; vzeroupper                                  ; Zero upper bits of ZMM registers [all ZMM registers get reloaded anyway!]

  mov           __AX, __CX                    ; EAX(->c_real)
; mov           __DX, __DX                    ; EDX(->c_imag)
  mov           ECX, MAX_ITERATIONS_8BPP      ; MAX_ITERATIONS
  sub           ECX, 1                        ; ECX(vmode_colors-1)
  %if (__BITS__ >= 64)
  vbroadcastss  ZMM8, dword [escape_radius]   ; ZMM8(radius) = 4.0
  %endif
  vmovaps       ZMM6, [__AX]                  ; ZMM6(c_real)
  vmovaps       ZMM7, [__DX]                  ; ZMM7(c_imag)
  vmovaps       ZMM4, ZMM6                    ; ZMM4(Z_real) = c_real
  vmovaps       ZMM5, ZMM7                    ; ZMM5(Z_imag) = c_imag
  mov           EAX, -1                       ; set mask of all 1 bits
  kmovw         k1, EAX                       ; copy 16 bits to mask register k1
  vpbroadcastd  ZMM3, ECX                     ; XMM3[31..0] = vmode_colors-1
  vpbroadcastd  ZMM2, EAX                     ; YMM2(iter_inc) = -1
  xor           EDX, EDX                      ; EDX = 0
  xor           EAX, EAX                      ; EAX = 0

; Vol. 2B, 4-440:
;   vfmadd213ps   dst, src2, src3  <=>  dst = src2*dst + src3
;   vfmadd231ps   dst, src2, src3  <=>  dst = src2*src3 + dst

; Vol. 2B, 4-462:
;   vfmsub213ps   dst, src2, src3  <=>  dst = src2*dst - src3

.iter_func:
    vmovaps       ZMM1 {k1}{z}, ZMM5          ; ZMM1(radius) = Z_imag
    vmulps        ZMM1 {k1}, ZMM1, ZMM1       ; ZMM1(radius) = Z_imag^2
    vaddps        ZMM5 {k1}, ZMM5, ZMM5       ; ZMM5(NEW(Z_imag)) = 2*Z_imag
    vfmadd213ps   ZMM5 {k1}{z}, ZMM4, ZMM7    ; ZMM5(NEW(Z_imag)) = 2*Z_imag*Z_real + c_imag
    vmovaps       ZMM0, ZMM1                  ; ZMM0(Bkp(Z_imag^2))
    vfmadd231ps   ZMM1 {k1}, ZMM4, ZMM4       ; ZMM1(radius) = Z_imag^2 + Z_real^2
    vfmsub213ps   ZMM4 {k1}, ZMM4, ZMM0       ; ZMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2
    vaddps        ZMM4 {k1}{z}, ZMM6          ; ZMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2 + c_real

    ; The result of each comparison is a doubleword mask of
    ; all 1s (comparison true) or all 0s (comparison false):
    %if (__BITS__ < 64)                       ; YMM1 = radius <= 4.0? [CMPLEPS xmm1, xmm2 <=> CMPPS xmm1, xmm2, 2]
    vcmpps        k1 {k1}, ZMM1, dword [escape_radius] {1to16}, 2
    %else
    vcmpps        k1 {k1}, ZMM1, ZMM8, 2
    %endif

    sub           ECX, 1                      ; i--
    setnz         DL                          ; DL = (i != 0)

    kortestw      k1, k1                      ; k1[15:0] == 0?
    setnz         AL                          ; AL = (ZMM2 != 0)

    and           EDX, EAX                    ; EDX = (i!=0) & (ZMM2!=0)
    vpaddd        ZMM3 {k1}, ZMM3, ZMM2       ; iter_count--
    jnz           .iter_func

  %if __BITS__ < 64
    mov         EAX, [__SP + 56 + 4*sizeof(NASMX_PTR)]  ; SIMD_INT *result
    vmovdqa32   [EAX], ZMM3                   ; *a_result = ZMM3
  %else
    %if _WINDOWS                              ; [win64] callee-save registers
    movaps      XMM6, [__SP     ]             ; rst(XMM6)
    movaps      XMM7, [__SP + 16]             ; rst(XMM7)
    %if (__BITS__ >= 64)
    movaps      XMM8, [__SP + 32]             ; bkp(XMM8)
    %endif
    %endif
    vmovdqa32   [R8], ZMM3                    ; *a_result = ZMM3
  %endif

; Transition ZMM register set to state (A):
  vzeroupper                                  ; Zero upper bits of ZMM registers
  leave
  RET


;----------------------------------------------------------------------------
; extern void MandelbrotPixelColor32AVX1024 (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result);
;
;  (ZMM8  | imag_factor |)
;  (ZMM7  | real_factor |)
;   ZMM7  | c_imag      |
;   ZMM6  | c_real      |
;   ZMM5  | Z_imag      |
;   ZMM4  | Z_real      |
;   ---------------------
;   ZMM3  | iter_count  |
;   ZMM2  | iter_incr   |
;   ZMM1  | iter_mask   |  <temporary to each iteration>
;   ZMM0  | radius      |  <temporary to each iteration>
;   ---------------------
;   EAX   : SIMD_FLT *c_real
;   ECX   : SIMD_FLT *c_imag
;   EDX   : iter_count
;----------------------------------------------------------------------------
ALIGN 16
MandelbrotPixelColor32AVX1024:
  %define a_real    __SP + sizeof(NASMX_PTR) + 0*sizeof(NASMX_PTR)  ; SIMD_FLT *c_real
  %define a_imag    __SP + sizeof(NASMX_PTR) + 1*sizeof(NASMX_PTR)  ; SIMD_FLT *c_imag
  %define a_result  __SP + sizeof(NASMX_PTR) + 2*sizeof(NASMX_PTR)  ; SIMD_FLT *c_result

; [0]:
  mov     EAX, [a_real]                       ; EAX(->c_real)
  mov     EDX, [a_imag]                       ; EDX(->c_imag)
  mov     ECX, MAX_ITERATIONS_8BPP            ; MAX_ITERATIONS
  sub     ECX, 1                              ; ECX(vmode_colors-1)
  movaps  XMM6, [EAX]                         ; XMM6(c_real)
  movaps  XMM7, [EDX]                         ; XMM7(c_imag)
  movaps  XMM4, XMM6                          ; XMM4(Z_real) = c_real
  movaps  XMM5, XMM7                          ; XMM5(Z_imag) = c_imag
  movd    XMM3, ECX                           ; XMM3[31..0] = vmode_colors-1
  pshufd  XMM3, XMM3, 0                       ; Shuffle low 32 bits into all vector positions
  pcmpeqd XMM2, XMM2                          ; XMM2(iter_inc) = <iteration increment> = -1

.iter_func1:
    movaps    XMM0, XMM4                      ; XMM0(NEW(Z_imag)) = Z_real
    mulps     XMM4, XMM4                      ; XMM4(NEW(Z_real)) = Z_real^2
    addps     XMM0, XMM0                      ; XMM0(NEW(Z_imag)) = 2*Z_real
    movaps    XMM1, XMM4                      ; XMM1(radius) = Z_real^2
    mulps     XMM0, XMM5                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag
    addps     XMM4, XMM6                      ; XMM4(NEW(Z_real)) = Z_real^2 + c_real
    mulps     XMM5, XMM5                      ; XMM5(Z_imag^2)
    addps     XMM0, XMM7                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag + c_imag
    addps     XMM1, XMM5                      ; XMM1(radius) = Z_real^2 + Z_imag^2
    subps     XMM4, XMM5                      ; XMM4(Z_real) = Z_real^2 + c_real - Z_imag^2
    cmpleps   XMM1, [escape_radius]           ; XMM1 = radius <= 4.0

    xor       EDX, EDX                        ; EDX = 0
    sub       ECX, 1                          ; i--
    pand      XMM2, XMM1                      ; XMM2(iter_inc) &= (radius <= 4.0)
    setnz     DL                              ; DL = (i != 0)
%ifdef SSE_SUPPORT_41 ; SSE 4.1:
    xor       EAX, EAX                        ; EAX = 0
    ptest     XMM2, XMM2                      ; (JZ,JC) = (XMM2 & XMM2)
    setnz     AL                              ; AL = (XMM2 != 0)
%else ; SSE1:
    pmovmskb  EAX, XMM2                       ; EAX = MSB(XMM2)
    or        EAX, EAX
    setnz     AL
%endif
    movaps    XMM5, XMM0                      ; XMM5(Z_imag) = 2*Z_real*Z_imag + c_imag
    and       EDX, EAX                        ; EDX = (i!=0) & (XMM2!=0)
    paddd     XMM3, XMM2                      ; iter_count--
    jnz       .iter_func1

  mov     EAX, [a_result]
  movdqa  [EAX], XMM3

; [1]:
  mov     EAX, [a_real]                       ; EAX(->c_real)
  mov     EDX, [a_imag]                       ; EDX(->c_imag)
  add     EAX, 16
  add     EDX, 16
  mov     ECX, MAX_ITERATIONS_8BPP            ; MAX_ITERATIONS
  sub     ECX, 1                              ; ECX(vmode_colors-1)
  movaps  XMM6, [EAX]                         ; XMM6(c_real)
  movaps  XMM7, [EDX]                         ; XMM7(c_imag)
  movaps  XMM4, XMM6                          ; XMM4(Z_real) = c_real
  movaps  XMM5, XMM7                          ; XMM5(Z_imag) = c_imag
  movd    XMM3, ECX                           ; XMM3[31..0] = vmode_colors-1
  pshufd  XMM3, XMM3, 0                       ; Shuffle low 32 bits into all vector positions
  pcmpeqd XMM2, XMM2                          ; XMM2(iter_inc) = <iteration increment> = -1

.iter_func2:
    movaps    XMM0, XMM4                      ; XMM0(NEW(Z_imag)) = Z_real
    mulps     XMM4, XMM4                      ; XMM4(NEW(Z_real)) = Z_real^2
    addps     XMM0, XMM0                      ; XMM0(NEW(Z_imag)) = 2*Z_real
    movaps    XMM1, XMM4                      ; XMM1(radius) = Z_real^2
    mulps     XMM0, XMM5                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag
    addps     XMM4, XMM6                      ; XMM4(NEW(Z_real)) = Z_real^2 + c_real
    mulps     XMM5, XMM5                      ; XMM5(Z_imag^2)
    addps     XMM0, XMM7                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag + c_imag
    addps     XMM1, XMM5                      ; XMM1(radius) = Z_real^2 + Z_imag^2
    subps     XMM4, XMM5                      ; XMM4(Z_real) = Z_real^2 + c_real - Z_imag^2
    cmpleps   XMM1, [escape_radius]           ; XMM1 = radius <= 4.0

    xor       EDX, EDX                        ; EDX = 0
    sub       ECX, 1                          ; i--
    pand      XMM2, XMM1                      ; XMM2(iter_inc) &= (radius <= 4.0)
    setnz     DL                              ; DL = (i != 0)
%ifdef SSE_SUPPORT_41 ; SSE 4.1:
    xor       EAX, EAX                        ; EAX = 0
    ptest     XMM2, XMM2                      ; (JZ,JC) = (XMM2 & XMM2)
    setnz     AL                              ; AL = (XMM2 != 0)
%else ; SSE1:
    pmovmskb  EAX, XMM2                       ; EAX = MSB(XMM2)
    or        EAX, EAX
    setnz     AL
%endif
    movaps    XMM5, XMM0                      ; XMM5(Z_imag) = 2*Z_real*Z_imag + c_imag
    and       EDX, EAX                        ; EDX = (i!=0) & (XMM2!=0)
    paddd     XMM3, XMM2                      ; iter_count--
    jnz       .iter_func2

  mov     EAX, [a_result]
  add     EAX, 16
  movdqa  [EAX], XMM3

; [2]:
  mov     EAX, [a_real]                       ; EAX(->c_real)
  mov     EDX, [a_imag]                       ; EDX(->c_imag)
  add     EAX, 32
  add     EDX, 32
  mov     ECX, MAX_ITERATIONS_8BPP            ; MAX_ITERATIONS
  sub     ECX, 1                              ; ECX(vmode_colors-1)
  movaps  XMM6, [EAX]                         ; XMM6(c_real)
  movaps  XMM7, [EDX]                         ; XMM7(c_imag)
  movaps  XMM4, XMM6                          ; XMM4(Z_real) = c_real
  movaps  XMM5, XMM7                          ; XMM5(Z_imag) = c_imag
  movd    XMM3, ECX                           ; XMM3[31..0] = vmode_colors-1
  pshufd  XMM3, XMM3, 0                       ; Shuffle low 32 bits into all vector positions
  pcmpeqd XMM2, XMM2                          ; XMM2(iter_inc) = <iteration increment> = -1

.iter_func3:
    movaps    XMM0, XMM4                      ; XMM0(NEW(Z_imag)) = Z_real
    mulps     XMM4, XMM4                      ; XMM4(NEW(Z_real)) = Z_real^2
    addps     XMM0, XMM0                      ; XMM0(NEW(Z_imag)) = 2*Z_real
    movaps    XMM1, XMM4                      ; XMM1(radius) = Z_real^2
    mulps     XMM0, XMM5                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag
    addps     XMM4, XMM6                      ; XMM4(NEW(Z_real)) = Z_real^2 + c_real
    mulps     XMM5, XMM5                      ; XMM5(Z_imag^2)
    addps     XMM0, XMM7                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag + c_imag
    addps     XMM1, XMM5                      ; XMM1(radius) = Z_real^2 + Z_imag^2
    subps     XMM4, XMM5                      ; XMM4(Z_real) = Z_real^2 + c_real - Z_imag^2
    cmpleps   XMM1, [escape_radius]           ; XMM1 = radius <= 4.0

    xor       EDX, EDX                        ; EDX = 0
    sub       ECX, 1                          ; i--
    pand      XMM2, XMM1                      ; XMM2(iter_inc) &= (radius <= 4.0)
    setnz     DL                              ; DL = (i != 0)
%ifdef SSE_SUPPORT_41 ; SSE 4.1:
    xor       EAX, EAX                        ; EAX = 0
    ptest     XMM2, XMM2                      ; (JZ,JC) = (XMM2 & XMM2)
    setnz     AL                              ; AL = (XMM2 != 0)
%else ; SSE1:
    pmovmskb  EAX, XMM2                       ; EAX = MSB(XMM2)
    or        EAX, EAX
    setnz     AL
%endif
    movaps    XMM5, XMM0                      ; XMM5(Z_imag) = 2*Z_real*Z_imag + c_imag
    and       EDX, EAX                        ; EDX = (i!=0) & (XMM2!=0)
    paddd     XMM3, XMM2                      ; iter_count--
    jnz       .iter_func3

  mov     EAX, [a_result]
  add     EAX, 32
  movdqa  [EAX], XMM3

; [3]:
  mov     EAX, [a_real]                       ; EAX(->c_real)
  mov     EDX, [a_imag]                       ; EDX(->c_imag)
  add     EAX, 48
  add     EDX, 48
  mov     ECX, MAX_ITERATIONS_8BPP            ; MAX_ITERATIONS
  sub     ECX, 1                              ; ECX(vmode_colors-1)
  movaps  XMM6, [EAX]                         ; XMM6(c_real)
  movaps  XMM7, [EDX]                         ; XMM7(c_imag)
  movaps  XMM4, XMM6                          ; XMM4(Z_real) = c_real
  movaps  XMM5, XMM7                          ; XMM5(Z_imag) = c_imag
  movd    XMM3, ECX                           ; XMM3[31..0] = vmode_colors-1
  pshufd  XMM3, XMM3, 0                       ; Shuffle low 32 bits into all vector positions
  pcmpeqd XMM2, XMM2                          ; XMM2(iter_inc) = <iteration increment> = -1

.iter_func4:
    movaps    XMM0, XMM4                      ; XMM0(NEW(Z_imag)) = Z_real
    mulps     XMM4, XMM4                      ; XMM4(NEW(Z_real)) = Z_real^2
    addps     XMM0, XMM0                      ; XMM0(NEW(Z_imag)) = 2*Z_real
    movaps    XMM1, XMM4                      ; XMM1(radius) = Z_real^2
    mulps     XMM0, XMM5                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag
    addps     XMM4, XMM6                      ; XMM4(NEW(Z_real)) = Z_real^2 + c_real
    mulps     XMM5, XMM5                      ; XMM5(Z_imag^2)
    addps     XMM0, XMM7                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag + c_imag
    addps     XMM1, XMM5                      ; XMM1(radius) = Z_real^2 + Z_imag^2
    subps     XMM4, XMM5                      ; XMM4(Z_real) = Z_real^2 + c_real - Z_imag^2
    cmpleps   XMM1, [escape_radius]           ; XMM1 = radius <= 4.0

    xor       EDX, EDX                        ; EDX = 0
    sub       ECX, 1                          ; i--
    pand      XMM2, XMM1                      ; XMM2(iter_inc) &= (radius <= 4.0)
    setnz     DL                              ; DL = (i != 0)
%ifdef SSE_SUPPORT_41 ; SSE 4.1:
    xor       EAX, EAX                        ; EAX = 0
    ptest     XMM2, XMM2                      ; (JZ,JC) = (XMM2 & XMM2)
    setnz     AL                              ; AL = (XMM2 != 0)
%else ; SSE1:
    pmovmskb  EAX, XMM2                       ; EAX = MSB(XMM2)
    or        EAX, EAX
    setnz     AL
%endif
    movaps    XMM5, XMM0                      ; XMM5(Z_imag) = 2*Z_real*Z_imag + c_imag
    and       EDX, EAX                        ; EDX = (i!=0) & (XMM2!=0)
    paddd     XMM3, XMM2                      ; iter_count--
    jnz       .iter_func4

  mov     EAX, [a_result]
  add     EAX, 48
  movdqa  [EAX], XMM3

; [4]:
  mov     EAX, [a_real]                       ; EAX(->c_real)
  mov     EDX, [a_imag]                       ; EDX(->c_imag)
  add     EAX, 64
  add     EDX, 64
  mov     ECX, MAX_ITERATIONS_8BPP            ; MAX_ITERATIONS
  sub     ECX, 1                              ; ECX(vmode_colors-1)
  movaps  XMM6, [EAX]                         ; XMM6(c_real)
  movaps  XMM7, [EDX]                         ; XMM7(c_imag)
  movaps  XMM4, XMM6                          ; XMM4(Z_real) = c_real
  movaps  XMM5, XMM7                          ; XMM5(Z_imag) = c_imag
  movd    XMM3, ECX                           ; XMM3[31..0] = vmode_colors-1
  pshufd  XMM3, XMM3, 0                       ; Shuffle low 32 bits into all vector positions
  pcmpeqd XMM2, XMM2                          ; XMM2(iter_inc) = <iteration increment> = -1

.iter_func5:
    movaps    XMM0, XMM4                      ; XMM0(NEW(Z_imag)) = Z_real
    mulps     XMM4, XMM4                      ; XMM4(NEW(Z_real)) = Z_real^2
    addps     XMM0, XMM0                      ; XMM0(NEW(Z_imag)) = 2*Z_real
    movaps    XMM1, XMM4                      ; XMM1(radius) = Z_real^2
    mulps     XMM0, XMM5                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag
    addps     XMM4, XMM6                      ; XMM4(NEW(Z_real)) = Z_real^2 + c_real
    mulps     XMM5, XMM5                      ; XMM5(Z_imag^2)
    addps     XMM0, XMM7                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag + c_imag
    addps     XMM1, XMM5                      ; XMM1(radius) = Z_real^2 + Z_imag^2
    subps     XMM4, XMM5                      ; XMM4(Z_real) = Z_real^2 + c_real - Z_imag^2
    cmpleps   XMM1, [escape_radius]           ; XMM1 = radius <= 4.0

    xor       EDX, EDX                        ; EDX = 0
    sub       ECX, 1                          ; i--
    pand      XMM2, XMM1                      ; XMM2(iter_inc) &= (radius <= 4.0)
    setnz     DL                              ; DL = (i != 0)
%ifdef SSE_SUPPORT_41 ; SSE 4.1:
    xor       EAX, EAX                        ; EAX = 0
    ptest     XMM2, XMM2                      ; (JZ,JC) = (XMM2 & XMM2)
    setnz     AL                              ; AL = (XMM2 != 0)
%else ; SSE1:
    pmovmskb  EAX, XMM2                       ; EAX = MSB(XMM2)
    or        EAX, EAX
    setnz     AL
%endif
    movaps    XMM5, XMM0                      ; XMM5(Z_imag) = 2*Z_real*Z_imag + c_imag
    and       EDX, EAX                        ; EDX = (i!=0) & (XMM2!=0)
    paddd     XMM3, XMM2                      ; iter_count--
    jnz       .iter_func5

  mov     EAX, [a_result]
  add     EAX, 64
  movdqa  [EAX], XMM3

; [5]:
  mov     EAX, [a_real]                       ; EAX(->c_real)
  mov     EDX, [a_imag]                       ; EDX(->c_imag)
  add     EAX, 80
  add     EDX, 80
  mov     ECX, MAX_ITERATIONS_8BPP            ; MAX_ITERATIONS
  sub     ECX, 1                              ; ECX(vmode_colors-1)
  movaps  XMM6, [EAX]                         ; XMM6(c_real)
  movaps  XMM7, [EDX]                         ; XMM7(c_imag)
  movaps  XMM4, XMM6                          ; XMM4(Z_real) = c_real
  movaps  XMM5, XMM7                          ; XMM5(Z_imag) = c_imag
  movd    XMM3, ECX                           ; XMM3[31..0] = vmode_colors-1
  pshufd  XMM3, XMM3, 0                       ; Shuffle low 32 bits into all vector positions
  pcmpeqd XMM2, XMM2                          ; XMM2(iter_inc) = <iteration increment> = -1

.iter_func6:
    movaps    XMM0, XMM4                      ; XMM0(NEW(Z_imag)) = Z_real
    mulps     XMM4, XMM4                      ; XMM4(NEW(Z_real)) = Z_real^2
    addps     XMM0, XMM0                      ; XMM0(NEW(Z_imag)) = 2*Z_real
    movaps    XMM1, XMM4                      ; XMM1(radius) = Z_real^2
    mulps     XMM0, XMM5                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag
    addps     XMM4, XMM6                      ; XMM4(NEW(Z_real)) = Z_real^2 + c_real
    mulps     XMM5, XMM5                      ; XMM5(Z_imag^2)
    addps     XMM0, XMM7                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag + c_imag
    addps     XMM1, XMM5                      ; XMM1(radius) = Z_real^2 + Z_imag^2
    subps     XMM4, XMM5                      ; XMM4(Z_real) = Z_real^2 + c_real - Z_imag^2
    cmpleps   XMM1, [escape_radius]           ; XMM1 = radius <= 4.0

    xor       EDX, EDX                        ; EDX = 0
    sub       ECX, 1                          ; i--
    pand      XMM2, XMM1                      ; XMM2(iter_inc) &= (radius <= 4.0)
    setnz     DL                              ; DL = (i != 0)
%ifdef SSE_SUPPORT_41 ; SSE 4.1:
    xor       EAX, EAX                        ; EAX = 0
    ptest     XMM2, XMM2                      ; (JZ,JC) = (XMM2 & XMM2)
    setnz     AL                              ; AL = (XMM2 != 0)
%else ; SSE1:
    pmovmskb  EAX, XMM2                       ; EAX = MSB(XMM2)
    or        EAX, EAX
    setnz     AL
%endif
    movaps    XMM5, XMM0                      ; XMM5(Z_imag) = 2*Z_real*Z_imag + c_imag
    and       EDX, EAX                        ; EDX = (i!=0) & (XMM2!=0)
    paddd     XMM3, XMM2                      ; iter_count--
    jnz       .iter_func6

  mov     EAX, [a_result]
  add     EAX, 80
  movdqa  [EAX], XMM3

; [6]:
  mov     EAX, [a_real]                       ; EAX(->c_real)
  mov     EDX, [a_imag]                       ; EDX(->c_imag)
  add     EAX, 96
  add     EDX, 96
  mov     ECX, MAX_ITERATIONS_8BPP            ; MAX_ITERATIONS
  sub     ECX, 1                              ; ECX(vmode_colors-1)
  movaps  XMM6, [EAX]                         ; XMM6(c_real)
  movaps  XMM7, [EDX]                         ; XMM7(c_imag)
  movaps  XMM4, XMM6                          ; XMM4(Z_real) = c_real
  movaps  XMM5, XMM7                          ; XMM5(Z_imag) = c_imag
  movd    XMM3, ECX                           ; XMM3[31..0] = vmode_colors-1
  pshufd  XMM3, XMM3, 0                       ; Shuffle low 32 bits into all vector positions
  pcmpeqd XMM2, XMM2                          ; XMM2(iter_inc) = <iteration increment> = -1

.iter_func7:
    movaps    XMM0, XMM4                      ; XMM0(NEW(Z_imag)) = Z_real
    mulps     XMM4, XMM4                      ; XMM4(NEW(Z_real)) = Z_real^2
    addps     XMM0, XMM0                      ; XMM0(NEW(Z_imag)) = 2*Z_real
    movaps    XMM1, XMM4                      ; XMM1(radius) = Z_real^2
    mulps     XMM0, XMM5                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag
    addps     XMM4, XMM6                      ; XMM4(NEW(Z_real)) = Z_real^2 + c_real
    mulps     XMM5, XMM5                      ; XMM5(Z_imag^2)
    addps     XMM0, XMM7                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag + c_imag
    addps     XMM1, XMM5                      ; XMM1(radius) = Z_real^2 + Z_imag^2
    subps     XMM4, XMM5                      ; XMM4(Z_real) = Z_real^2 + c_real - Z_imag^2
    cmpleps   XMM1, [escape_radius]           ; XMM1 = radius <= 4.0

    xor       EDX, EDX                        ; EDX = 0
    sub       ECX, 1                          ; i--
    pand      XMM2, XMM1                      ; XMM2(iter_inc) &= (radius <= 4.0)
    setnz     DL                              ; DL = (i != 0)
%ifdef SSE_SUPPORT_41 ; SSE 4.1:
    xor       EAX, EAX                        ; EAX = 0
    ptest     XMM2, XMM2                      ; (JZ,JC) = (XMM2 & XMM2)
    setnz     AL                              ; AL = (XMM2 != 0)
%else ; SSE1:
    pmovmskb  EAX, XMM2                       ; EAX = MSB(XMM2)
    or        EAX, EAX
    setnz     AL
%endif
    movaps    XMM5, XMM0                      ; XMM5(Z_imag) = 2*Z_real*Z_imag + c_imag
    and       EDX, EAX                        ; EDX = (i!=0) & (XMM2!=0)
    paddd     XMM3, XMM2                      ; iter_count--
    jnz       .iter_func7

  mov     EAX, [a_result]
  add     EAX, 96
  movdqa  [EAX], XMM3

; [7]:
  mov     EAX, [a_real]                       ; EAX(->c_real)
  mov     EDX, [a_imag]                       ; EDX(->c_imag)
  add     EAX, 112
  add     EDX, 112
  mov     ECX, MAX_ITERATIONS_8BPP            ; MAX_ITERATIONS
  sub     ECX, 1                              ; ECX(vmode_colors-1)
  movaps  XMM6, [EAX]                         ; XMM6(c_real)
  movaps  XMM7, [EDX]                         ; XMM7(c_imag)
  movaps  XMM4, XMM6                          ; XMM4(Z_real) = c_real
  movaps  XMM5, XMM7                          ; XMM5(Z_imag) = c_imag
  movd    XMM3, ECX                           ; XMM3[31..0] = vmode_colors-1
  pshufd  XMM3, XMM3, 0                       ; Shuffle low 32 bits into all vector positions
  pcmpeqd XMM2, XMM2                          ; XMM2(iter_inc) = <iteration increment> = -1

.iter_func8:
    movaps    XMM0, XMM4                      ; XMM0(NEW(Z_imag)) = Z_real
    mulps     XMM4, XMM4                      ; XMM4(NEW(Z_real)) = Z_real^2
    addps     XMM0, XMM0                      ; XMM0(NEW(Z_imag)) = 2*Z_real
    movaps    XMM1, XMM4                      ; XMM1(radius) = Z_real^2
    mulps     XMM0, XMM5                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag
    addps     XMM4, XMM6                      ; XMM4(NEW(Z_real)) = Z_real^2 + c_real
    mulps     XMM5, XMM5                      ; XMM5(Z_imag^2)
    addps     XMM0, XMM7                      ; XMM0(NEW(Z_imag)) = 2*Z_real*Z_imag + c_imag
    addps     XMM1, XMM5                      ; XMM1(radius) = Z_real^2 + Z_imag^2
    subps     XMM4, XMM5                      ; XMM4(Z_real) = Z_real^2 + c_real - Z_imag^2
    cmpleps   XMM1, [escape_radius]           ; XMM1 = radius <= 4.0

    xor       EDX, EDX                        ; EDX = 0
    sub       ECX, 1                          ; i--
    pand      XMM2, XMM1                      ; XMM2(iter_inc) &= (radius <= 4.0)
    setnz     DL                              ; DL = (i != 0)
%ifdef SSE_SUPPORT_41 ; SSE 4.1:
    xor       EAX, EAX                        ; EAX = 0
    ptest     XMM2, XMM2                      ; (JZ,JC) = (XMM2 & XMM2)
    setnz     AL                              ; AL = (XMM2 != 0)
%else ; SSE1:
    pmovmskb  EAX, XMM2                       ; EAX = MSB(XMM2)
    or        EAX, EAX
    setnz     AL
%endif
    movaps    XMM5, XMM0                      ; XMM5(Z_imag) = 2*Z_real*Z_imag + c_imag
    and       EDX, EAX                        ; EDX = (i!=0) & (XMM2!=0)
    paddd     XMM3, XMM2                      ; iter_count--
    jnz       .iter_func8

  mov     EAX, [a_result]
  add     EAX, 112
  movdqa  [EAX], XMM3
  RET


;===  Linker directives:  ===================================================
%if _WINDOWS

%ifdef LIBCFILE
  %defstr LIBCFILE LIBCFILE
%else
  %defstr LIBCFILE MSVCRT               ; DEFAULT: assume Multithreaded, dynamic link
%endif

SECTION .drectve info ; align=1
  db  '/DEFAULTLIB:"',LIBCFILE,'" /DEFAULTLIB:"OLDNAMES" '
  db  '/DEFAULTLIB:"KERNEL32" /DEFAULTLIB:"USER32" /DEFAULTLIB:"GDI32" /DEFAULTLIB:"ADVAPI32"'

%endif
