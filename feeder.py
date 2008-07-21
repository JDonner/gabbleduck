#!/usr/bin/env python

import collections
import threading
import subprocess


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

    def wait_to_finish(self):
        for i in range(self.n_threads):
            # a bit inelegant, but we don't care
            self.thread_objs[i].join()


def spawnWork(fname, falloff, sigma):
    subprocess.call(["gabble", "--SeedDensityFalloff=%s --FinalSnapshot=0 --SigmaOfGaussian=%s --MaxPoints=10000 %s" %
                     (falloff, sigma, fname)])


def fakeSpawnWork(fname, falloff, sigma):
    print "executing gabble %s, %f, %f" % (fname, falloff, sigma)
    subprocess.call(["sleep", "10"])


def runThread(tq):
    while True:
        work = tq.dequeue()
        if work is None:
            break
            # we're done; we'll be collected
        else:
            (fname, falloff, sigma) = work
            fakeSpawnWork(fname, falloff, sigma)


def queue_up_work(tq, n_threads):
    for fname in ["1AGW.mrc", "1BVP.mrc", "1CID.mrc"]:
        for falloff in [0.1, 0.3, 0.5, 0.7, 0.9, 1.0]:
            for sigma in [1.0, 3.0, 5.0, 7.0, 9.0, 11.0]:
                tq.enqueue((fname, falloff, sigma))

    # let threads know they're done
    for i in range(n_threads):
        tq.enqueue(None)


def main():
    tq = ThreadQueue(4)
    tq.start()
    queue_up_work(tq, 4)
    tq.wait_to_finish()


if __name__ == "__main__":
    main()
