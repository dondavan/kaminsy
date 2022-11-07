#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; fill-column: 80; -*-
"""Bare-bones DNS resolver.
"""


import argparse
import dnslib as dl
from dnslib import dns, server
import idgens
import lrucache as lru
import random as rnd
import socket
import struct
import time


# Capacity of the resolver cache.
CACHE_SZ = 128

# Buffer size.
BUFFER_SZ = 4096


class Cache(lru.LRUCache):
    """Cache of answers."""

    def __init__(self, cap):
        super().__init__(cap)

    def set(self, key, val, ttl):
        """Insert item into cache, overwriting an old value, if it exists."""
        if not ttl:
            return None

        expiry_time = ttl + time.time()
        super().set(key, (expiry_time, val))
        return val

    def get(self, key):
        """Retrieve an item from cache."""
        cached = super().get(key)
        if not cached:
            return None

        current_time = time.time()
        expiry_time, val = cached
        if expiry_time < current_time:
            super().delete(key)
            return None
        return val


class NoCache(lru.LRUCache):
    """Cache-like interface, but performs no caching."""

    def __init__(self, cap):
        super().__init__(cap)

    def set(self, key, val, ttl):
        return val

    def get(self, key):
        """Retrieve an item from cache."""
        return None


class SillyResolver(server.BaseResolver):
    """A bare-bones DNS resolver."""

    def __init__(self, opts):
        self.up_addr = opts.ns_addr
        self.up_port = opts.ns_port
        self.timeout = opts.ns_timeout

        self.__tamper_qid = False
        self.__next_qid = None

        self.__sport = opts.fix_sport

        if opts.fix_qid is not None and opts.fix_qid >= 0:
            self.__tamper_qid = True
            self.__next_qid = lambda: opts.fix_qid

        if opts.lin_qid is not None and opts.lin_qid >= 0:
            self.__tamper_qid = True
            self.__qid_gen = idgens.SeqIDs(200, 200 + opts.lin_qid)
            self.__next_qid = lambda: self.__qid_gen.next()

        if opts.rnd_qid is not None and opts.rnd_qid >= 0:
            self.__tamper_qid = True
            self.__qid_gen = idgens.RndIDs(200, 200 + opts.rnd_qid)
            self.__next_qid = lambda: self.__qid_gen.next()

        if opts.disable_cache:
            self.__cache = NoCache(1)
            self.__addl = NoCache(1)
        else:
            self.__cache = Cache(CACHE_SZ)
            self.__addl = Cache(4)

    @classmethod
    def gen_empty_response(self, req):
        """DNS response indicating that the domain does not exist."""
        res = req.reply()
        res.header.code = getattr(dl.RCODE, 'NXDOMAIN')
        return res

    def prep_proxy_req(self, req):
        """Prepare proxy request by manipulating required fields.
        """
        if not self.__tamper_qid:
            return req

        req = dl.DNSRecord.parse(req.pack())
        req.header.id = self.__next_qid()
        return req

    @classmethod
    def send_and_recv(self, req, dst_addr, dst_port, timeout,
                      src_addr='0.0.0.0', src_port=None):
        """Send and receive data over UDP."""
        # Prepare empty response, in case of a timeout.
        res = SillyResolver.gen_empty_response(req).pack()

        s = None
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.settimeout(timeout)

            if src_port:
                # OK to treat src_port=0 as False.
                s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                s.bind((src_addr, src_port))

            s.sendto(req.pack(), (dst_addr, dst_port))

            beg_time = time.time()
            wait = True
            while wait:
                try:
                    res, srv = s.recvfrom(8192)

                    elapsed = time.time() - beg_time
                    if elapsed / 60 > timeout:
                        wait = False

                    dns_rec = dl.DNSRecord.parse(res)

                    if dns_rec.q.qname != req.q.qname:
                        # Discard answer
                        continue

                    if dns_rec.header.id != req.header.id:
                        # Discard answer
                        continue

                    wait = False
                except Exception as e:
                    print(f"Error: {e}")
            return res
        finally:
            if (s is not None):
                s.close()

    def __parse_addl_rrs(self, qname, auth, addl):
        """Parse authority and additional records, if present."""
        # Perform simple sanity checks.

        auth_recs = {r.rdata.label: r.rname for r in auth
                     if qname.matchSuffix(r.rname)
                     if r.rtype == dl.QTYPE.NS}
        if not auth_recs:
            return

        # We are only interested in glue records of type 'A'.
        for r in (r for r in addl
                  if r.rname in auth_recs
                  if r.rtype == dl.QTYPE.A):
            self.__addl.set(auth_recs[r.rname], r.rdata, r.ttl)

    def find_ns_addr(self, fwd_addr, qname):
        """Check nameserver address for the domain in cache."""
        domain = dl.DNSLabel(qname.label[-2:])
        if domain in self.__addl:
            return str(self.__addl.get(domain))
        return fwd_addr

    def call_up(self, req):
        """Send request upstream and collect the response."""
        qname = req.q.qname
        if qname in self.__cache:
            # Use cached response.
            res = req.reply()
            ans = self.__cache.get(qname)
            if ans:
                res.add_answer(ans)
                # Return only if we have a valid answer in cache.
                return res

        # Fetch new answer from upstream.
        try:
            proxy_req = self.prep_proxy_req(req)
            fwd_addr = self.find_ns_addr(self.up_addr, req.q.qname)

            # OK to treat '__sport=0' as False.
            if self.__sport:
                if self.__sport < 0:
                    self.__sport = rnd.randint(55123, 55124)

                proxy_res = SillyResolver.send_and_recv(proxy_req,
                                                        fwd_addr,
                                                        self.up_port,
                                                        timeout=self.timeout,
                                                        src_port=self.__sport)
            else:
                proxy_res = proxy_req.send(fwd_addr, self.up_port,
                                           timeout=self.timeout)

            res = dl.DNSRecord.parse(proxy_res)

            # Restore query ID, if required.
            if self.__tamper_qid:
                res.header.id = req.header.id

            # Update answer in cache.
            ans = res.a
            self.__cache.set(qname, ans, ans.ttl)

            self.__parse_addl_rrs(req.q.qname, res.auth, res.ar)
        except socket.timeout:
            res = SillyResolver.gen_empty_response(req)
        return res

    def resolve(self, req, handler):
        if handler.protocol != 'udp':
            # Requests over TCP get nothing!
            return SillyResolver.gen_empty_response(req)

        query = req.q
        if query.qtype != dl.QTYPE.A:
            return SillyResolver.gen_empty_response(req)

        return self.call_up(req)


class Opts(dict):
    """Dictionary of options with lookup."""

    def __getattr__(self, attr):
        return self[attr]


def main(args):
    if (args.fix_qid is not None) and (args.lin_qid is not None):
        raise ValueError(f"Must set either `fix_qid` or `lin_qid`")

    opts = Opts(**{arg: getattr(args, arg) for arg in vars(args)})

    resolver = SillyResolver(opts)
    proxy = server.DNSServer(resolver, address=args.addr, port=args.port,
                             handler=server.DNSHandler)
    proxy.start_thread()

    while proxy.isAlive():
        try:
            time.sleep(1)
        except KeyboardInterrupt:
            break


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="A bare-bones DNS resolver")
    parser.add_argument('--addr', '-a', dest='addr',
                        metavar='<addr>',
                        type=str,
                        default='',
                        help='IP address of this resolver')
    parser.add_argument('--port', '-p', dest='port',
                        metavar='<port>',
                        type=int,
                        default=53,
                        help='Port number of this resolver')
    parser.add_argument('--no-cache', dest='disable_cache',
                        action='store_true',
                        help='Disables caching')
    parser.add_argument('--ns-ip', '-n', dest='ns_addr',
                        metavar='<ns-addr>',
                        type=str,
                        required=True,
                        help='IP address of upstream nameserver')
    parser.add_argument('--ns-port', dest='ns_port',
                        metavar='<ns-port>',
                        type=int,
                        default=53,
                        help='Port number of upstream nameserver')
    parser.add_argument('--ns-timeout', dest='ns_timeout',
                        metavar='<ns-timeout>',
                        type=int,
                        default=2,
                        help='Max. time to wait (in seconds)')
    parser.add_argument('--fix-sport', dest='fix_sport',
                        metavar='<src-port>',
                        type=int,
                        help='Set source port for all forwarded requests')
    parser.add_argument('--fix-qid', dest='fix_qid',
                        metavar='<dns-query-id>',
                        type=int,
                        help='Set DNS ID for all forwarded requests')
    parser.add_argument('--lin-qid', dest='lin_qid',
                        metavar='<starting-id>',
                        type=int,
                        help='Use linear DNS IDs for forwarded requests')
    parser.add_argument('--rnd-qid', dest='rnd_qid',
                        metavar='<starting-id>',
                        type=int,
                        help='Use random DNS IDs for forwarded requests')
    main(parser.parse_args())
