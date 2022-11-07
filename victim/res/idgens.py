1#!/usr/bin/env python
# -*- mode: python; coding: utf-8; fill-column: 80; -*-
"""Classes to generating query or transaction IDs.
"""

import threading
import random


class SeqIDs(object):
    """Generate sequential IDs starting from an initial number.

    NOTE: Values will repeat after exhausting the sequence."""

    def __init__(self, start=0, limit=2**16, step=1):
        self.__start = start
        self.__curr = start
        self.__step = step
        self.__limit = limit
        self.__count = 0
        self.__lock = threading.Lock()

    def next(self):
        with self.__lock:
            value = self.__curr
            self.__curr = (self.__curr + self.__step) % self.__limit
            if self.__curr < self.__start:
                self.__curr = self.__start
            self.__count += 1
        return value

    def count(self):
        return self.__count

    def __iter__(self):
        while True:
            yield self.next()


class RndIDs(object):
    """Random IDs from an specified range of IDs.

    NOTE: Values will _only_ repeat after exhausting the sequence.
    """

    def __init__(self, start, limit=2**16):
        self.__curr = 0
        self.__range = range(start, limit, 1)
        self.__total = limit-start
        self.__mark = self.__total // 4
        self.__ids = random.sample(self.__range, limit-start)
        self.__count = 0
        self.__lock = threading.Lock()

    def next(self):
        """Retrieve next random item from the sequence."""
        with self.__lock:
            if self.__curr >= self.__total:
                # Shuffle some items in the beginning of the sequence to move
                # them to the end.
                new_part = random.sample(self.__ids[:self.__mark], self.__mark)
                old_part = self.__ids[self.__mark:]
                self.__ids = old_part + new_part
                self.__curr -= self.__mark

            value = self.__ids[self.__curr]
            self.__curr += 1
            self.__count += 1
        return value

    def __iter__(self):
        while True:
            yield self.next()
