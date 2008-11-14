#!/usr/bin/env python

from __future__ import with_statement
import sys


def main(args):
    fname = args[0]

    # eg {'1AGW':[vals1, vals2, ...], 'BVPP':[vals1, vals2, ...], ...}
    valses_by_protein = {}
    with open(fname) as f:
        for line in f:
            toks = line.split()
            protein = toks[0]
            parms = toks[1:]
#            print "%d parms" % len(parms)
            pairs = [(pair.split("=")[0], float(pair.split("=")[1])) for pair in parms]
            vals = dict(pairs)

            valses_by_protein.setdefault(protein, []).append(vals)

    ordered_valses_by_protein = sort_by_best(valses_by_protein)

    for protein, valses in ordered_valses_by_protein.items():
        for i in range(5):
            print protein, "\t".join(["%s:%0.3f" %
                                      (pair[0], pair[1])
                                      for pair in valses[i].items()])


def sort_by_low(valses):
    def keyof(vals):
        return vals['SFN'] * vals['SFP']
    ordered = sorted(valses, key=keyof)
    return ordered


def sort_by_best(valses_by_protein):
    ordered_valses_by_protein = {}
    for protein, valses in valses_by_protein.items():
        ordered = sort_by_low(valses)
        ordered_valses_by_protein[protein] = ordered

    return ordered_valses_by_protein


if __name__ == "__main__":
    args = sys.argv[1:]
    main(args)
