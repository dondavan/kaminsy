#!/usr/bin/env python
# -*- mode: python; coding: utf-8; fill-column: 80; -*-
#
# lrucache.py
# Created by Balakrishnan Chandrasekaran on 2015-05-02 06:33 -0400.
# Copyright (c) 2014 Balakrishnan Chandrasekaran <balakrishnan.c@gmail.com>.
#

"""
lrucache.py
Implements a dictionary that uses LRU-algorithm for cache evictions.
"""

__author__  = 'Balakrishnan Chandrasekaran <balakrishnan.c@gmail.com>'
__version__ = '1.0'
__license__ = 'MIT'


from collections import namedtuple


class _Node:
    """Node in a double-linked list.
    """

    __slots__ = ('k', 'v', 'lt', 'rt')

    def __init__(self, k, v, lt=None, rt=None):
        """Initialize a Node of a doubly-linked list.
        """
        self.k = k
        self.v = v
        self.lt = None
        self.rt = None


class _DoublyLinkedList:
    """Simple doubly-linked list.
    FIXME: Use weak references and auto-update on reference!
    """

    __slots__ = ('h', 't')

    def __init__(self):
        """Initialize doubly-linked list.
        """
        self.h = None
        self.t = None

    def remove(self, item):
        """Remove an item from the doubly-linked list in O(1) time.
        """
        if item.lt:
            item.lt.rt = item.rt
        if item.rt:
            item.rt.lt = item.lt
        if item == self.h:
            self.h = item.rt
        if item == self.t:
            self.t = item.lt
        item.lt = None
        item.rt = None

    def insert(self, item):
        """Insert an item at the head of the list in O(1) time.
        """
        if self.h:
            self.h.lt = item
        item.rt = self.h
        self.h = item
        if not self.t:
            self.t = item

    def update(self, item):
        """Move an item to the head of the list in O(1) time.
        """
        self.remove(item)
        self.insert(item)

    def pop(self):
        """Pop the tail of the doubly-linked list in O(1) time.
        """
        t = self.t
        self.remove(self.t)
        return t


class LRUCache(dict):
    """Simple implementation of an LRU-cache.
    """

    def __init__(self, cap, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.cap = cap
        self._d = _DoublyLinkedList()

    def get(self, key):
        """Get item from cache, if it exists, and track its access."""
        try:
            _val = super().__getitem__(key)
            self._d.update(_val)
            return _val.v
        except KeyError:
            return None

    def set(self, key, val):
        """Set item in cache, replacing it if it already exists."""
        if key not in self and len(self) == self.cap:
            _pop = self._d.pop()
            del self[_pop.k]
        _val = _Node(key, val)
        self._d.insert(_val)
        return super().__setitem__(key, _val)

    def delete(self, key):
        """Evict item from cache."""
        try:
            _val = super().__getitem__(key)
            self._d.remove(_val)
            return _val
        except KeyError:
            return None

    def items(self):
        """Retrieve the cache items in no specific order."""
        items = super().items
        return ((key, cval.v) for (key, cval) in items())
