#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifeq ($(strip $(ROPKIT_PATH)),)
$(error "ROPKIT_PATH is not set.")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

.PHONY: clean all build_savedata

BUILDPREFIX	:=	
ROPINC_PATH	:=	ropinclude

export BOSSTOOLPARAMS	:=	$(BOSSTOOLPARAMS)

DEFINES	:=	-DTARGET_STACKFRAME_BEFORELROVERWRITE_SIZE=0x50 \
		-DROPBUF=0x32927e80 \
		-DROPKIT_LINEARMEM_REGIONBASE=0x30000000 \
		-DROPKIT_LINEARMEM_BUF=ROPBUF+0x100000 \
		"-DROPKIT_BINPAYLOAD_PATH=\"extdata:/payload.bin\"" \
		-DROPKIT_GSPTHREAD_OBJECTADDR=0x0072ac40 \
		-DROPKIT_BEFOREJUMP_CACHEBUFADDR=ROPKIT_LINEARMEM_BUF \
		-DROPKIT_BEFOREJUMP_CACHEBUFSIZE=0x800000 \
		-DROPKIT_OTHERAPP_NEWSP_ADDR=0x0FFC4000

#The above ROPBUF addr is for TFH USA, probably has to be updated for others. Likewise for ROPKIT_GSPTHREAD_OBJECTADDR.

all:
	@mkdir -p finaloutput_boss
	@mkdir -p build
	@for path in $(ROPINC_PATH)/*; do echo Building for title $$(basename "$$path")... && make build_title --no-print-directory TID=$$(basename "$$path"); done

clean:
	@rm -R -f finaloutput_boss
	@rm -R -f build

build_title:
	@mkdir -p finaloutput_boss/$(TID)
	@mkdir -p build/$(TID)
	@for path in $(ROPINC_PATH)/$(TID)/*; do echo Building $$(basename "$$path")... && make build_title_boss --no-print-directory BUILDPREFIX=$$(basename "$$path"); done

build_title_boss:	build/$(TID)/$(BUILDPREFIX).bin
	@bosstool --input=$< --output=finaloutput_boss/$(TID)/$(BUILDPREFIX).bin --programID=$(TID) --build --nsdataid=0x4e574b50 --datatype=0x18000 $(BOSSTOOLPARAMS)

build/$(TID)/$(BUILDPREFIX).bin:	build/$(TID)/$(BUILDPREFIX).elf
	@$(OBJCOPY) -O binary $< $@

build/$(TID)/$(BUILDPREFIX).elf:	../ctpkpwn.s
	@$(CC) -x assembler-with-cpp -nostartfiles -nostdlib -I$(ROPKIT_PATH) -include $(ROPINC_PATH)/$(TID)/$(BUILDPREFIX) $(DEFINES) $< -o $@

