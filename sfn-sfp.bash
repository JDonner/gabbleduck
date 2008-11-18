#!/usr/bin/env bash

# &&& later, use this instead of feeder, maybe. But, we need threading no?
# Just feed a list of files and parameters and let it deal?
# Would rather in fact do it from some shell script.


rm "stats.txt"

for vfile in output/*.vertices
do
    vfile_base=$(basename $vfile .vertices)

    vfile_bits_string=$(echo $vfile_base | sed "s/-/ /g")
    # eg: bits=[1AGW bt=5.000 sig=1.000 wnd=5.000 bfal=0.100 pts=0]
    echo "bits=[$vfile_bits_string]"
    declare -a vfile_bits
    vfile_bits=($vfile_bits_string)

    protein=${vfile_bits[0]}
    echo "protein=[$protein]"

    declare -a sfn_sfp
    # array (thus splitting) of stdout output
    sfn_sfp=($(./sfn-sfp $protein.carbons $vfile))

    # output looks like: SFN 1.2 SFP 3.4. Pick out just numbers
    echo $vfile_bits_string ${sfn_sfp[0]}=${sfn_sfp[1]} ${sfn_sfp[2]}=${sfn_sfp[3]} >> stats.txt
    chmod a+rw stats.txt
done
