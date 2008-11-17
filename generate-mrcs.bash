#!/usr/bin/env bash

OUTPUTDIR=${1:"."}

# Jing's original setting at 10A
# RES=10

# .. but the paper uses 8A
RES=8
APIX=1
for f in 1CID 1TIM 2PHH 1BVP 1AGW 1BBH 1C3W 1DXT 1IRK 1LGA
do
    pdb2mrc $f.pdb $OUTPUTDIR/$f-res=$RES-apix=$APIX.mrc res=$RES apix=$APIX center
done
