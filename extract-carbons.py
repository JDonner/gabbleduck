#!/usr/bin/env python

import sys
from pdb_format import load_pdb, atom_defs_by_id

def extract_carbons(f):
    load_pdb(f);
#    print >> sys.stderr, "%d atoms..." % len(atom_defs_by_id.values())
#    print "some...", "\n".join(str(atom_defs_by_id.values()[0:5]))
    carbons = [atom[2] for atom in atom_defs_by_id.values() if atom[3] == 'C']
#    print >> sys.stderr, "carbons[0]:", carbons[0]
#    print >> sys.stderr, "carbons[0][0]:", carbons[0][0]
    assert isinstance(carbons[0][0], float)
    return carbons

if __name__ == "__main__":
    in_fname = sys.argv[1]
    fin = open(in_fname)

    carbons = extract_carbons(fin)

    for carbon in carbons:
        print >> sys.stdout, carbon[0], carbon[1], carbon[2]
