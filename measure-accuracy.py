#!/usr/bin/env python

# This needs Python >= 2.5 I believe
from __future__ import with_statement
import sys, math, itertools

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
# sorted_residues = (resid, (x,y,z))
sorted_residues = []


# 1-based, to make easier to match to spec
def text_at(line, start, end):
    text = line[start - 1:end - 1 + 1]
    return text

# 1-based, to make easier to match to spec
def tok_at(line, start, end):
#    print "line: %s, (%s)%d - (%s)%d" % (line, type(start), start, type(end), end)
    return line[start - 1: end - 1 + 1].strip()

# 1-based, to make easier to match to spec
def float_at(line, start, end):
#    print "line:\n%s, %d - %d" % (line, start, end)
    return float(tok_at(line, start, end))

# 1-based, to make easier to match to spec
def int_at(line, start, end):
    return int(tok_at(line, start, end))


def residue_index(res0, res1):
    global sorted_residues

    (first, last) = (-1, -1)

#    print "res0, res1: ", res0, res1
#    print "r[0]: ", sorted_residues[0][0]
    # find index of res0, res1
    first = map(lambda r: r[0], sorted_residues).index(res0)
    last = map(lambda r: r[0], sorted_residues).index(res1)
#    print "fst, lst: ", first, last
#    for x in range(first, last + 1):
#        print "res[", x, "]", sorted_residues[x][0]
    return (first, last)


# def calculate_res_pos(atoms):
#     (x, y, z) = (0.0, 0.0, 0.0)
#     for a in atoms:
#         x += a.x
#         y += a.y
#         z += a.z
#     x /= atoms.length()
#     y /= atoms.length()
#     z /= atoms.length()
#     return (x,y,z)

def make_resid(chainno, resno):
#    print "chainno: ", chainno, "resno:", resno
    return "%3.3s:%5.5s" % (chainno, resno)

#SHEET    2   A 5 ILE 1 184  ARG 1 190 -1  N  LEU 1 186   O  ILE 1 223
def load_helix_def(line):
    chain = tok_at(line, 20, 20)
    start_res_no = tok_at(line, 23, 26)
    start_res_id = make_resid(chain, start_res_no)
    end_res_no = tok_at(line, 34, 37)
    end_res_id = make_resid(chain, end_res_no)
    return (start_res_id, end_res_id)

def load_sheet_def(line):
    chain = tok_at(line, 22, 22)
    start_res_no = tok_at(line, 23, 26)
    start_res_id = make_resid(chain, start_res_no)
    end_res_no = tok_at(line, 34, 37)
    end_res_id = make_resid(chain, end_res_no)
    return (start_res_id, end_res_id)


def load_atom_def(line):
    atomid = tok_at(line, 7, 11)

    # chain no, residue within that chain
    chain = tok_at(line, 22, 22)
    resno = tok_at(line, 23, 26)

    # ... not sure this id is unique!
    resid = make_resid(chain, resno)
    x = float_at(line, 31, 38)
    y = float_at(line, 39, 46)
    z = float_at(line, 47, 54)

    symbol = text_at(line, 77, 78).strip()
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

    print "pdb: %d atoms, %d helix lines, %d sheet lines" % (nAtoms, nHelixLines, nSheetLines)

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
    print "min: %f, %f, %f; max: %f, %f, %f" % (xmin, ymin, zmin, xmax, ymax, zmax)


def euclidean_dist(one, two):
    dx = one[0] - two[0]
    dx = dx * dx

    dy = one[1] - two[1]
    dy = dy * dy

    dz = one[2] - two[2]
    dz = dz * dz

    return dx + dy + dz


def SFX(ys, xs):
    # 1st elt is distance, 2nd is index of winning vertex
    x_closest_y = [i for i in itertools.repeat(None, len(ys))]
    print "%d ys, %d xs" % (len(ys), len(xs))
    for (iy, y) in enumerate(ys):
        nearest_d2 = 1.0e38
        for (ix, x) in enumerate(xs):
            d2 = euclidean_dist(y, x)
            if (d2 < nearest_d2):
                x_closest_y[iy] = d2
                nearest_d2 = d2
    # ... something...
    sfx = sum(map(math.sqrt, x_closest_y))
    sfx /= len(x_closest_y)
    return sfx


# Big, fat n^2
def SFN(carbons, vertices):
    sfn = SFX(carbons, vertices)
    return sfn


def SFP(vertices, carbons):
    sfp = SFX(vertices, carbons)
    return sfp


# ATOM is: (atomid, resid, (x, y, z), symbol)
def extract_carbons():
    print "%d atoms..." % len(atom_defs_by_id.values())
#    print "some...", "\n".join(str(atom_defs_by_id.values()[0:5]))
    carbons = [atom[2] for atom in atom_defs_by_id.values() if atom[3] == 'C']
    print "carbons[0]:", carbons[0]
    print "carbons[0][0]:", carbons[0][0]
    assert isinstance(carbons[0][0], float)
    return carbons

# Triangle vertices generated by the beta sheet finder (gabbleduck)
def read_vertices(fname):
    vertices = []
    with open(fname) as f:
        for line in f:
            (x, y, z) = line.split()
            vertices.append((float(x),float(y),float(z)))
    print "vertices[0]:", vertices[0]
    print "vertices[0][0]:", vertices[0][0]
    assert isinstance(vertices[0][0], float)
    return vertices


# -- &&& how do I know that the axes are even the same 'handedness'?
# -- (can see them trying at least, if pitifully, with sse.py)
# eg, 1AGW, [vol, laplace]
def main(args):
    pdb_fname = args[0]
    vertices_fname = args[1]
    pdb_file = open(pdb_fname)
    load_pdb(pdb_file)
    carbons = extract_carbons()
    vertices = read_vertices(vertices_fname)

    print "%d carbons" % len(carbons)
    print "%d vertices" % len(vertices)

    sfp = SFP(vertices, carbons)
    sfn = SFN(carbons, vertices)

    print "SFP %f" % sfp
    print "SFN %f" % sfn


if __name__ == "__main__":
    args = sys.argv[1:]
    main(args)
