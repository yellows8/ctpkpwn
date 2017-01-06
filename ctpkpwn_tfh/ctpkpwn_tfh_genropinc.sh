# Usage: ./ctpkpwn_tfh_genropinc.sh <path to yellows8github/3ds_ropkit repo> <codebin path>

$1/generate_ropinclude.sh $2 $1 --disableblacklist
if [[ $? -ne 0 ]]; then
	echo "//ERROR: 3ds_ropkit generate_ropinclude.sh returned an error."
	exit 1
fi

echo ""

# Locate the following: ldmdb r6!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, ip, sp, lr, pc}

printstr=`ropgadget_patternfinder $2 --baseaddr=0x100000 --patterntype=sha256 --patterndata=296db7c6db3c00e52c7e66c93d38804f40fb905a9f94a572db225df5d6df20bd --patternsha256size=0x4 "--plainout=#define STACKPIVOT_ADR "`

if [[ $? -eq 0 ]]; then
	echo "$printstr"
else
	echo "//ERROR: STACKPIVOT_ADR not found."
	exit 1
fi

