gprof ./gabble gmon.out > gabble.gproffed.txt
/big/common/gprof2dot.py -s gabble.gproffed.txt > gabble.gprof.dot
dot -Tpdf gabble.gprof.dot -ogabble.gprof.pdf
kpdf gabble.gprof.pdf
