#!/usr/bin/env python

import collections
import threading, subprocess
import glob
import sys, os, stat
import datetime

# parameter ranges. We do every combination...!
g_BetaThicknesses = [5.0]

# These are in % of beta_thickness; min = beta - 10%, max = beta + 10% etc
# ie, 25%, 40%
g_ThicknessRangeRatios = [0.15, 0.25, 0.35, 0.4, 0.5]

# In Angstroms, or rather, the same units as that of the image itself.
g_Sigmas = [3.0]

g_RelativeSeedDensityThresholds = [0.4, 0.5, 0.7]

g_Falloffs = [0.7, 0.8, 0.9]

g_Separations = [0.5]

# This is automatic, now; we use 2 x FeatureSigma (on each side, for 4x sigma total) for 95% coverage
#g_GSupports = [13, 31]

#UndergradLabMachineDescs = [
## 4s are all Intels, 2s are AMD. All are 64bit.
#("cecrops", 4),
#("centaur", 4),
#("cerberus", 4),
#("charger", 2),
#("charon", 4),
#("charybdis", 4),
#("chimaera", 4),
#("cobra", 2),
#("corsa", 2),
#("countach", 2),
#("dart", 2),
#("duster", 2),
#("edsel", 2),
#("elcamino", 2),
#("eclipse", 2),
#("gorgon", 2),
#("gryphyn", 4),
#("hydra", 4),
#("judge", 2),
#("ladon", 4),
#("minotaur", 4),
#("mustang", 2),
#("nova", 2),
#("siren", 4),
#("typhoeus", 4),
#("vantage", 2),
#("viggen", 2)
#]
#
#
#class MachineState(object):
#    def __init__(self, _name, _num_procs):
#        self.name = _name
#        self.num_procs = _num_procs
#        self.num_procs_busy = 0
#
#
#    def note_use(self):
#        self.num_procs_busy += 1
#        assert self.num_procs_busy <= self.procs_free_for_use()
#
#
#    def note_freed(self):
#        assert 1 <= self.num_procs_busy
#        self.num_procs_busy -= 1
#
#
#    def procs_free_for_use(self):
#        # Be nice; let real user have one entirely free CPU even though
#        # we nice +19 anyway
#        return self.num_procs - 1
#
#
#    def has_free_proc(self):
#        return self.num_procs_busy < self.procs_free_for_use()
#
#
## We really want a nice, lazy generator here...
#def next_machine(machines):
#    for machine in machines:
#        if machine.has_free_proc():
#            return machine.name
#
## (Name, processors)
#g_MachineDescs = UndergradLabMachineDescs;


class ThreadQueue(object):
    def __init__(self, numThreads):
        self.q = collections.deque()
        # total work put in
        self.size_original_queue = 0
        # total work pulled out
        self.size_original_done = 0
        self.sem = threading.Semaphore(0)
        self.mutex = threading.Lock()
        self.n_threads = numThreads
        self.thread_objs = []
        for i in range(self.n_threads):
            self.thread_objs.append(threading.Thread(None, runThread, None, [self]))

    def enqueue(self, work):
        self.mutex.acquire()
        work = self.q.append(work)
        self.size_original_queue += 1
        self.mutex.release()
        self.sem.release()

    def dequeue(self):
        self.sem.acquire()
        self.mutex.acquire()
        work = self.q.popleft()
        # &&& it's not really done at this point, just easier this way
        self.size_original_done += 1
        percent_done = 100.0 * float(self.size_original_done) / float(self.size_original_queue)
        self.mutex.release()
        return (work, percent_done)

    def start(self):
        for i in range(self.n_threads):
            self.thread_objs[i].start()

    def wait_for_threads_to_finish(self):
        for i in range(self.n_threads):
            # not inelegant (reaps in fixed order)
            self.thread_objs[i].join()


def spawnWork(fname, beta_thickness, thickness_flex, falloff, rel_density, sigma,
              separation):
    # ssh user@xyz.cs.nmsu.edu nice +19 ./find-sheets
    cmd_line_parts = ["./find-sheets",
#                      "--FinalSnapshot=0",
                      "--BetaThickness=%0.3f" % beta_thickness,
                      "--BetaThickRangeRatio=%0.3f" % thickness_flex,
                      "--RelativeSeedDensityThreshold=%0.3f" % rel_density,
#                      "--GaussianSupportSize=%2.2d" % gaussian_support,
                      "--SeedDensityFalloff=%0.3f" % falloff,
                      "--SigmaOfFeatureGaussian=%0.3f" % sigma,
                      "--RequiredNewPointSeparation=%0.3f" % separation,
                      "--OutputDir=output",
#                      "--MaxPoints=0",
                      fname]

    print "running: ", ' '.join(cmd_line_parts)

    subprocess.call(cmd_line_parts)


# (was for debugging)
def fakeSpawnWork(fname, falloff, sigma):
    print "pretending to execute gabble %s, %f, %f" % (fname, falloff, sigma)
    subprocess.call(["sleep", "1"])


# What each 'thread' runs
def runThread(tq):
    while True:
        (work, percent_done) = tq.dequeue()
        if work is None:
            exit()
            # we're done; we'll be collected
        else:
            (fname, beta_thickness, thickness_flex, falloff, rel_density, sigma, separation) = work
            print datetime.datetime.now(), "now at: ~%0.2f%%" % percent_done
            spawnWork(fname, beta_thickness, thickness_flex, falloff,
                      rel_density, sigma, separation)


def queue_up_parallel_work(tq, n_threads, mrc_files):
    for fname in mrc_files:
        for beta_thickness in g_BetaThicknesses:
            for thickness_flex in g_ThicknessRangeRatios:
                for falloff in g_Falloffs:
                    for rel_density in g_RelativeSeedDensityThresholds:
                        for sigma in g_Sigmas:
                            for separation in g_Separations:
                                tq.enqueue((fname, beta_thickness, thickness_flex,
                                            falloff, rel_density, sigma, separation))

    # let threads know they're done
    for i in range(n_threads):
        tq.enqueue(None)


def main(args):
    # Apply to all .mrc files in current directory
    if not len(args):
        args.append('*.mrc')

    volume_files = []
    for spec in args:
        volume_files.extend(glob.glob(spec))

    print "Files [%s]: " % ", ".join(volume_files)

    # 50MB
    LargeSize = 50000000
    small_files = [f for f in volume_files
                   if os.stat(f)[stat.ST_SIZE] < LargeSize]
    large_files = [f for f in volume_files
                   if os.stat(f)[stat.ST_SIZE] >= LargeSize]

    print "%d large files" % len(large_files)
    print "%d smaller files" % len(small_files)

    t3q = ThreadQueue(3)
    queue_up_parallel_work(t3q, 3, large_files)
    t3q.start()
    t3q.wait_for_threads_to_finish()

    num_processors = 4
    print "using %d processors" % num_processors
    t4q = ThreadQueue(num_processors)

    queue_up_parallel_work(t4q, num_processors, small_files)
    t4q.start()
    t4q.wait_for_threads_to_finish()


if __name__ == "__main__":
    # Give it any number of file specs, anything that a shell would
    # recognize eg
    #   ./feeder.py mrc-input/*.mrc
    main(sys.argv[1:])
