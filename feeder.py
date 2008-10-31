#!/usr/bin/env python

import collections
import threading
import subprocess
import glob


class ThreadQueue(object):
    def __init__(self, numThreads):
        self.q = collections.deque()
        self.sem = threading.Semaphore(0)
        self.mutex = threading.Lock()
        self.n_threads = numThreads
        self.thread_objs = []
        for i in range(self.n_threads):
            self.thread_objs.append(threading.Thread(None, runThread, None, [self]))

    def dequeue(self):
        self.sem.acquire()
        self.mutex.acquire()
        work = self.q.popleft()
        self.mutex.release()
        return work

    def enqueue(self, work):
        self.mutex.acquire()
        work = self.q.append(work)
        self.mutex.release()
        self.sem.release()

    def start(self):
        for i in range(self.n_threads):
            self.thread_objs[i].start()

    def wait_for_threads_to_finish(self):
        for i in range(self.n_threads):
            # not inelegant (reaps in fixed order)
            self.thread_objs[i].join()


def spawnWork(fname, falloff, sigma):
    cmd_line_parts = ["./find-sheets",
                      "--FinalSnapshot=0",
                      "--SigmaOfGaussian=%f" % sigma,
                      "--SeedDensityFalloff=%f" % falloff,
                      "--RequiredNewPointSeparation=0.25",
                      "--MaxPoints=0",
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
        work = tq.dequeue()
        if work is None:
            exit()
            # we're done; we'll be collected
        else:
            (fname, falloff, sigma) = work
            spawnWork(fname, falloff, sigma)


def queue_up_work(tq, n_threads, mrc_files):
    for fname in mrc_files:
        for falloff in [0.1, 0.3, 0.5, 0.7, 0.9, 1.0]:
            # 9 & 11 are not very good
            for sigma in [1.0, 2.0, 3.0, 4.0, 5.0, 7.0, 9.0]:
                tq.enqueue((fname, falloff, sigma))

    # let threads know they're done
    for i in range(n_threads):
        tq.enqueue(None)


def main():
    num_processors = 4
    tq = ThreadQueue(num_processors)
    tq.start()
    # Apply to all .mrc files in current directory
    mrc_files = glob.glob('*.mrc')
    print "using %d processors" % num_processors
    print "Files: ", ", ".join(mrc_files)
    queue_up_work(tq, num_processors, mrc_files)
    tq.wait_for_threads_to_finish()


if __name__ == "__main__":
    main()
