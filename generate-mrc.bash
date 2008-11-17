#!/usr/bin/env bash

# Jing's original setting at 10A
# RES=10

# .. but the paper uses 8A
RES=8
# &&& what is apix...?
pdb2mrc $1.pdb $1.mrc res=$RES apix=1.0 center
