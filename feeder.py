#!/usr/bin/env python

import collections
import threading, subprocess
import glob
import sys, os, stat
import datetime

# parameter ranges. We do every combination...!
g_BetaThicknesses = [5.0, 5.5, 6.0]

# These are in % of beta_thickness; min = beta - 10%, max = beta + 10% etc
# ie, 25%, 30%
g_ThicknessFlexes = [0.25, 0.3]

# ie, at what proportion of
g_ThicknessFalloffs = [0.5, 0.6, 0.65, 0.7]

# In Angstroms, or rather, the same units as that of the image itself.
g_Sigmas = [3.0, 4.5]


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


class SerialQueue(object):
    def __init__(self):
        self.q = collections.deque()
        # total work put in
        self.size_original_queue = 0
        # total work pulled out
        self.size_original_done = 0

    def enqueue(self, work):
        work = self.q.append(work)
        self.size_original_queue += 1

    def dequeue(self):
        work = self.q.popleft()
        # &&& it's not really done at this point, just easier this way
        self.size_original_done += 1
        percent_done = 100.0 * float(self.size_original_done) / float(self.size_original_queue)
        return (work, percent_done)

    def start(self):
        for work in self.q:
            (fname, beta_thickness, thickness_flex, falloff, sigma) = work
            spawnWork(fname, beta_thickness, thickness_flex, falloff, sigma)


def spawnWork(fname, beta_thickness, thickness_flex, falloff, sigma):
    cmd_line_parts = ["./find-sheets",
#                      "--FinalSnapshot=0",
                      "--BetaThickness=%0.3f" % beta_thickness,
                      "--BetaThicknessFlex=%0.3f" % thickness_flex,
                      # Let these two sigmas be the same for now
                      "--SigmaOfDerivativeGaussian=%0.3f" % sigma,
                      "--SigmaOfFeatureGaussian=%0.3f" % sigma,
                      "--SeedDensityFalloff=%0.3f" % falloff,
                      "--RequiredNewPointSeparation=0.5",
                      "--BetaPointFakeDensity=0.5",
                      "--OutputDir=output",
#                      "--MaxPoints=0",
                      fname]

    print "running: ", ' '.join(cmd_line_parts)

    subprocess.call(cmd_line_parts)


# (for debugging)
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
            (fname, beta_thickness, thickness_flex, falloff, sigma) = work
            print datetime.datetime.now(), "now at: ~%0.2f" % percent_done
            spawnWork(fname, beta_thickness, thickness_flex, falloff, sigma)


def queue_up_parallel_work(tq, n_threads, mrc_files):
    for fname in mrc_files:
        for beta_thickness in g_BetaThicknesses:
            for thickness_flex in g_ThicknessFlexes:
                for falloff in g_ThicknessFalloffs:
                    for sigma in g_Sigmas:
                        tq.enqueue((fname, beta_thickness, thickness_flex, falloff, sigma))

    # let threads know they're done
    for i in range(n_threads):
        tq.enqueue(None)


# We do larger files serially because they consume so much memory that
# having 4 of them (say) in memory at a time can choke even an 8GB machine.
def queue_up_serial_work(sq, serial_files):
    for fpath in serial_files:
        for beta_thickness in g_BetaThicknesses:
            for thickness_flex in g_ThicknessFlexes:
                for falloff in g_ThicknessFalloffs:
                    for sigma in g_Sigmas:
                        sq.enqueue((fpath, beta_thickness, thickness_flex, falloff, sigma))
        # Don't need a 'time to stop' entry for serial


def main(args):
    # Apply to all .mrc files in current directory
    if not len(args):
        args.append('*.mrc')

    volume_files = []
    for spec in args:
        volume_files.extend(glob.glob(spec))

    print "Files [%s]: " % ", ".join(volume_files)

    # 10MB
    LargeSize = 10000000
    small_files = [f for f in volume_files if os.stat(f)[stat.ST_SIZE] < LargeSize]
    large_files = [f for f in volume_files if os.stat(f)[stat.ST_SIZE] >= LargeSize]

    print "%d large, serial files" % len(large_files)
    print "%d smaller, parallel files" % len(small_files)

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
    main(sys.argv[1:])
