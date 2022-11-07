# About

This directory contains a `Vagrantfile` for instantiating a simple recursive nameserver (`bind`). You will **not** need to tamper with the resources in this VM. You could, however, interact with this nameserver to learn more about DNS and resolvers. 


## Interacting with `bind`

* You can start and stop, reload (i.e., restart with an updated configuration), and check the status of the nameserver as follows.
    ```
    vagrant@quackns:~$ sudo service bind9 start
    * Starting domain name service... bind9                             [ OK ]
    
    vagrant@quackns:~$ sudo service bind9 status
    * bind9 is running
    
    vagrant@quackns:~$ sudo service bind9 stop
    * Stopping domain name service... bind9                             [ OK ]
    
    vagrant@quackns:~$ sudo service bind9 status
    * bind9 is not running
    
    vagrant@quackns:~$ sudo service bind9 start
    * Starting domain name service... bind9                             [ OK ]
    
    vagrant@quackns:~$ sudo service bind9 reload
    * Reloading domain name service... bind9                            [ OK ]
    ```

* If required, you can flush the nameserver's cache without rebooting it as follows.

    ```
    vagrant@quackns:~$ sudo rndc flush
    ```


* You can manually resolve a hostname using this nameserver as follows.

    ```
    vagrant@quackns:~$ dig @192.168.10.30 -t A vunet.vu.nl

    ; <<>> DiG 9.9.5-3ubuntu0.19-Ubuntu <<>> @192.168.10.30 -t A vunet.vu.nl
    ; (1 server found)
    ;; global options: +cmd
    ;; Got answer:
    ;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 42418
    ;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 13, ADDITIONAL: 1

    ;; OPT PSEUDOSECTION:
    ; EDNS: version: 0, flags:; udp: 4096
    ;; QUESTION SECTION:
    ;vunet.vu.nl.			IN	A

    ;; ANSWER SECTION:
    vunet.vu.nl.		267	IN	A	130.37.253.159

    ;; AUTHORITY SECTION:
    .			38480	IN	NS	g.root-servers.net.
    .			38480	IN	NS	k.root-servers.net.
    .			38480	IN	NS	i.root-servers.net.
    .			38480	IN	NS	b.root-servers.net.
    .			38480	IN	NS	c.root-servers.net.
    .			38480	IN	NS	d.root-servers.net.
    .			38480	IN	NS	m.root-servers.net.
    .			38480	IN	NS	f.root-servers.net.
    .			38480	IN	NS	h.root-servers.net.
    .			38480	IN	NS	e.root-servers.net.
    .			38480	IN	NS	l.root-servers.net.
    .			38480	IN	NS	j.root-servers.net.
    .			38480	IN	NS	a.root-servers.net.

    ;; Query time: 5 msec
    ;; SERVER: 192.168.10.30#53(192.168.10.30)
    ;; WHEN: Sun Oct 24 18:29:21 UTC 2021
    ;; MSG SIZE  rcvd: 267
    ```
