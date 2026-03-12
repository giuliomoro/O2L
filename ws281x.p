// select one of the two below
#define GPIO_PIN 19 // PB2 (Gem Stereo, Gem Multi) P2.17
// #define GPIO_PIN 6 // BBB (Bela, Ctag) P9.41 or PB (BelaMini) P2.28

.origin 0
.entrypoint START

.macro LD32
.mparam dst,src
    LBBO dst,src,#0x00,4
.endm

.macro LD16
.mparam dst,src
    LBBO dst,src,#0x00,2
.endm

.macro LD8
.mparam dst,src
    LBBO dst,src,#0x00,1
.endm

.macro ST32
.mparam src,dst
    SBBO src,dst,#0x00,4
.endm

.macro ST16
.mparam src,dst
    SBBO src,dst,#0x00,2
.endm

.macro ST8
.mparam src,dst
    SBBO src,dst,#0x00,1
.endm

.macro stack_init
    mov r0, (0x2000 - 0x200)
.endm

.macro push
.mparam reg, cnt
    sbbo reg, r0, 0, 4*cnt
    add r0, r0, 4*cnt
.endm

.macro pop
.mparam reg, cnt
    sub r0, r0, 4*cnt
    lbbo reg, r0, 0, 4*cnt
.endm

.macro SLEEPNS
.mparam ns,inst,lab
    MOV r7, (ns/10)-1-inst
lab:
    SUB r7, r7, 1
    QBNE lab, r7, 0
.endm

.macro WAITNS
.mparam ns,lab
    MOV r8, 0x22000
#ifdef IS_AM62 // 250 MHz
    MOV r29, (ns) / 4
#else // 200 MHz
    MOV r29, (ns) / 5
#endif
lab:
 LBBO r9, r8, 0xC, 4
 QBGT lab, r9, R29
.endm

.macro RESET_COUNTER

 MOV r8, 0x22000
 LBBO r9, r8, 0, 4
 CLR r9, r9, 3
 SBBO r9, r8, 0, 4

 MOV r27, 0
 SBBO r27, r8, 0xC, 4

 SET r9, r9, 3
 SBBO r9, r8, 0, 4

 LBBO r7, r8, 0xC, 4
.endm

START:
    LBCO r0, C4, 4, 4
    CLR r0, r0, 4
    SBCO r0, C4, 4, 4

    MOV r0, 0x00000120
    MOV r1, 0x22028
    ST32 r0, r1

    MOV r0, 0x00100000
    MOV r1, 0x2202C
    ST32 r0, r1

	MOV r2, #0x1
    SBCO r2, C24, 12, 4

 MOV r20, 0xFFFFFFFF

_LOOP:
    LBCO r0, C24, 0, 12
    QBEQ _LOOP, r2, #0

	RESET_COUNTER

    MOV r3, 0
    SBCO r3, C24, 8, 4
    QBEQ EXIT, r2, #0xFF

WORD_LOOP:

 MOV r6, 24

 BIT_LOOP:
  SUB r6, r6, 1

  LBBO r10, r0, 0, 4
  MOV r2, 0

  QBBS gpio0_r10_skip, r10, r6
 SET r2, r2, GPIO_PIN
 gpio0_r10_skip: 

  WAITNS 800, wait_one_time
  MOV r20, (0|(1<<GPIO_PIN))
  NOT r30, r20 // clear the bit

  WAITNS 1150, wait_frame_spacing_time
  RESET_COUNTER
  MOV r30, r20 // set the bit

  WAITNS 240, wait_zero_time
  NOT r30, r2 // conditionally clear the bit

  QBNE BIT_LOOP, r6, 0

 ADD r0, r0, 4
 SUB r1, r1, 1
 QBNE WORD_LOOP, r1, #0


 WAITNS 1000, end_of_frame_clear_wait
 MOV r20, (0|(1<<GPIO_PIN))
 NOT r30, r20 // clear the bit

    SLEEPNS 50000, 1, reset_time

    MOV r8, 0x22000
    LBBO r2, r8, 0xC, 4
    SBCO r2, C24, 12, 4

    QBA _LOOP

EXIT:
    MOV r2, #0xFF
    SBCO r2, C24, 12, 4
    MOV R31.b0, 19 +16
    HALT
