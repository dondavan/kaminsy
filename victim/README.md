# About

This directory contains a `Vagrantfile` for instantiating a “quick and dirty” implementation of Tribore's resolver. You have been given this resolver only to test your attack implementation.

## Usage

This resolver implementation accepts various flags to help test your solution as you target each milestone.

```
python3  res/resolv.py -h
usage: resolv.py [-h] [--addr <addr>] [--port <port>] [--no-cache] --ns-ip <ns-addr> [--ns-port <ns-port>]
                 [--ns-timeout <ns-timeout>] [--target-port <ns-port>] [--fix-sport <src-port>]
                 [--fix-qid <dns-query-id>] [--lin-qid <starting-id>]

A bare-bones DNS resolver

optional arguments:
  -h, --help            show this help message and exit
  --addr <addr>, -a <addr>
                        IP address of this resolver
  --port <port>, -p <port>
                        Port number of this resolver
  --no-cache            Disables caching
  --ns-ip <ns-addr>, -n <ns-addr>
                        IP address of upstream nameserver
  --ns-port <ns-port>   Port number of upstream nameserver
  --ns-timeout <ns-timeout>
                        Max. time to wait (in seconds)
  --target-port <ns-port>
                        Port number of upstream nameserver
  --fix-sport <src-port>
                        Set source port for all forwarded requests
  --fix-qid <dns-query-id>
                        Set DNS ID for all forwarded requests
  --lin-qid <starting-id>
                        Use linear DNS IDs for forwarded requests
```


## Caveats

You must not rely on mistakes or bugs in this implementation to make your attacks succeed.
