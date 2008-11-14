#!/usr/bin/env python

# PDB Helix extracting stuff
global atom_defs_by_id
global sheet_defs
global helix_defs
global res_atoms_by_resid
global sorted_residues

atom_defs_by_id = {}
sheet_defs = []
helix_defs = []
res_atoms_by_resid = {}
# sorted_residues == (resid, (x,y,z))
sorted_residues = []


# 1-based, to make easier to match to spec
def _text_at(line, start, end):
    text = line[start - 1:end - 1 + 1]
    return text

# 1-based, to make easier to match to spec
def _tok_at(line, start, end):
#    print "line: %s, (%s)%d - (%s)%d" % (line, type(start), start, type(end), end)
    return line[start - 1: end - 1 + 1].strip()

# 1-based, to make easier to match to spec
def _float_at(line, start, end):
#    print "line:\n%s, %d - %d" % (line, start, end)
    return float(tok_at(line, start, end))

# 1-based, to make easier to match to spec
def _int_at(line, start, end):
    return int(tok_at(line, start, end))


# Keep chainno separate from resno
def make_resid(chainno, resno):
#    print "chainno: ", chainno, "resno:", resno
    return "%3.3s:%5.5s" % (chainno, resno)


#SHEET    2   A 5 ILE 1 184  ARG 1 190 -1  N  LEU 1 186   O  ILE 1 223
def load_helix_def(line):
    chain = _tok_at(line, 20, 20)
    start_res_no = _tok_at(line, 23, 26)
    start_res_id = make_resid(chain, start_res_no)
    end_res_no = _tok_at(line, 34, 37)
    end_res_id = make_resid(chain, end_res_no)
    return (start_res_id, end_res_id)


def load_sheet_def(line):
    chain = _tok_at(line, 22, 22)
    start_res_no = _tok_at(line, 23, 26)
    start_res_id = make_resid(chain, start_res_no)
    end_res_no = _tok_at(line, 34, 37)
    end_res_id = make_resid(chain, end_res_no)
    return (start_res_id, end_res_id)


def load_atom_def(line):
    atomid = _tok_at(line, 7, 11)

    # chain no, residue within that chain
    chain = _tok_at(line, 22, 22)
    resno = _tok_at(line, 23, 26)

    # ... not sure this id is unique!
    resid = make_resid(chain, resno)
    x = _float_at(line, 31, 38)
    y = _float_at(line, 39, 46)
    z = _float_at(line, 47, 54)

    symbol = _text_at(line, 77, 78).strip()
    return (atomid, resid, (x, y, z), symbol)


# manipulates globals
def load_pdb(file):
    nHelixLines = 0
    nSheetLines = 0
    nAtoms = 0
    for line in file:
        if line.startswith("HELIX"):
            nHelixLines += 1
            tup = load_helix_def(line)
            helix_defs.append(tup)
        elif line.startswith("SHEET"):
            nSheetLines += 1
            tup = load_sheet_def(line)
            sheet_defs.append(tup)
        elif line.startswith("ATOM"):
            nAtoms += 1
            tup = load_atom_def(line)
            # atom id, straight 1 - N index (in tup[0])
            atom_defs_by_id[tup[0]] = tup
            # tup[1] is a non-unique residue id
            res_atoms_by_resid.setdefault(tup[1], []).append(tup)

    print >> sys.stderr, "pdb: %d atoms, %d helix lines, %d sheet lines" % (nAtoms, nHelixLines, nSheetLines)

    process_residues()


# looks like it just finds center of mass
def process_residues():
    (xmin,ymin,zmin,xmax,ymax,zmax) = (1e38,1e38,1e38,-1e38,-1e38,-1e38)
    global sorted_residues
    for resid, atoms in res_atoms_by_resid.items():
        num_atoms = len(atoms)
        # x,y,z = center of mass it looks like
        (x, y, z) = (0.0, 0.0, 0.0)
        for a in atoms:
            x += a[2][0]; y += a[2][1]; z += a[2][2]
        x /= num_atoms; y /= num_atoms; z /= num_atoms
        xmax = max(x, xmax) ; ymax = max(y, ymax) ; zmax = max(z, zmax)
        xmin = min(x, xmin) ; ymin = min(y, ymin) ; zmin = min(z, zmin)
        sorted_residues.append((resid, (x, y, z)))
    # sort by id ([0])
    sorted_residues.sort(key=lambda r: r[0])
    print >> sys.stderr, "min res: %f, %f, %f; max res: %f, %f, %f" % (xmin, ymin, zmin, xmax, ymax, zmax)


def extract_carbons():
    print >> sys.stderr, "%d atoms..." % len(atom_defs_by_id.values())
#    print "some...", "\n".join(str(atom_defs_by_id.values()[0:5]))
    carbons = [atom[2] for atom in atom_defs_by_id.values() if atom[3] == 'C']
    print >> sys.stderr, "carbons[0]:", carbons[0]
    print >> sys.stderr, "carbons[0][0]:", carbons[0][0]
    assert isinstance(carbons[0][0], float)
    return carbons
