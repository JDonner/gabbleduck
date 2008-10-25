#!/usr/bin/env bash

# Jing's original setting at 10A
# RES=10

# .. but the paper uses 8A
RES=8
for f in 1CID 1TIM 2PHH 1BVP 1AGW 1BBH 1C3W 1DXT 1IRK 1LGA
do
    pdb2mrc $f.pdb $f.mrc res=$RES apix=1.0 center
done
