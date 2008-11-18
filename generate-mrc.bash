#!/usr/bin/env bash

# Jing's original setting at 10A
# RES=10

# .. but the paper uses 8A
RES=8
# 'apix' is the size of each pixel, 1 = 1 Angstrom. I think the natural value
# is to be the same size as RES, but, we make things a little
# easier on ourselves. It doesn't make RES meaningless though,
# RES is the blurriness of the image, and apix being 'fine'
# like it is just means we get fine slices of realistic blurriness.
pdb2mrc $1.pdb $1.mrc res=$RES apix=1.0 center
