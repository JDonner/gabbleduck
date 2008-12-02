#!/usr/bin/env python

import sys
import pdb_format as pdb


# This does not maintain any beta grouping, it's just individual atoms.
def carbons_of_beta_sheets(sheets):
    carbon_pts = []
    for sheet in sheets:
        carbon_pts.extend(carbons_of_beta_sheet(sheet))
    return carbon_pts


def carbons_of_beta_sheet(sheet):
    carbon_pts = []
    (start_res, end_res) = sheet
    (start_idx, end_idx) = pdb.residue_index(start_res, end_res)
    assert -1 < start_idx and -1 < end_idx, "strt: %s, end %s" % (start_res, end_res)
    residues = pdb.sorted_residues[start_idx : end_idx + 1]
    for res in residues:
        resid = res[0]
        # (atomid, resid, (x, y, z), symbol)
        carbon_pts.extend([atom[2]
                           for atom in pdb.res_atoms_by_resid[resid]
                           if atom[3] == 'C'])
    return carbon_pts


def extract_carbons(f):
    pdb.load_pdb(f)
    carbons = carbons_of_beta_sheets(pdb.sheet_defs)
#    print >> sys.stderr, "%d atoms..." % len(atom_defs_by_id.values())
#    print "some...", "\n".join(str(atom_defs_by_id.values()[0:5]))
    assert len(carbons) == 0 or isinstance(carbons[0][0], float)
    return carbons


# Use like:
#    extract-carbons.py something.pdb
if __name__ == "__main__":
    in_fname = sys.argv[1]
    fin = open(in_fname)

    carbons = extract_carbons(fin)

    for carbon in carbons:
        print >> sys.stdout, carbon[0], carbon[1], carbon[2]
