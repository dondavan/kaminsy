# About

This directory contains a `Vagrantfile` for instantiating a VM for _Mooncake_ (i.e., the attacker). Use this VM to develop your solution for cache poisoning the resolver running on the VM of _Tribore_ (i.e., the victim).

## Rules

1. The solution that you submit **must** be written by you.
    * Do not copy solutions from others.
    * Do not discuss implementation-specific details with others.

2. The solution must be written in C.

3. The solution only includes your attack implementation. You may optionally provide a `Makefile` script to compile your solution or install additional dependencies.
    * If your solution depends on third-party libraries, please list all of them, one per line, in a file called `deps.txt`.
    * You MUST obtain prior approval for these dependencies to avoid being penalized later.

4. If the code does not compile or run, you will get a zero.

5. Ensure that your solution is named simply as `attack` so that our test scripts can run it simply as follows.
    ```
    » ./attack <param-1> <param-2> ...
    ```

5. In a separate file named `ID.txt`, state your full name, student number, and email address (with each item in its own line).

6. Bundle your solution files, along with the `Makefile` and ID file as a Gzip'd tarball (with extension `tar.gz`).

7. Ensure that your solution accepts parameters strictly in the format shown below. You will get a zero for the assignment if your solution does not comply.

## Solution Format

1. The solution **must** be named `attack`, and it must provide an option `-p` with which our test scripts can specify the setting we are trying to test. The option `-p 1`, for instance, indicates that we are testing “part 1” of the assignment.
    ```
    » ./attack -p <part-number> <... other args ...>
    ```

2. For solving the first part, we will test your solution by providing the port number of the victim resolver as well as the query ID it will use as command-line parameters. The output should simply be the domain name and its (spoofed) IP address.
    ```
    » ./attack -p 1 <resolver-port> <query-ID>

    » ./attack -p 1 53053 12345
    vunet.vu.nl. 1.2.3.4
    ```

2. For the second part, we will provide on the command line the resolver port and the range (i.e., beginning and end) of query IDs it may use. The output should simply be the domain name and its (spoofed) IP address.
    ```
    » ./attack -p 2 <resolver-port> <beginning-ID> <end-ID>

    » ./attack -p 2 53053 1000 2000
    vunet.vu.nl. 1.2.3.4
    ```

3. For the third part, in addition to the parameters in the last part, we will also provide a domain name. The output should simply be a list of 3 domain names and their (spoofed) IP addresses. To ensure that all subdomains under the poisoned zone return a spoofed answer, you will have to poison the victim's cache such that any query for subdomains under the poison domain go to a nameserver that is under your control. For this purpose we have provided you with a `bind` nameserver in your own VM, which will return `1.2.3.4` for any query. You will not need to change the configuration of this nameserver.
    ```
    » ./attack -p 3 <resolver-port> <beginning-ID> <end-ID> <domain>

    » ./attack -p 3 53053 1000 2000 vu.nl.

    vunet.vu.nl. 1.2.3.4
    web.vu.nl. 1.2.3.4
    nonexistent.vu.nl. 1.2.3.4
    ```

4. For the last part, we will also mention (in addition to the parameters above) the range of port numbers that the resolver may use. The output should simply be a list of 3 domain names and their (spoofed) IP addresses.
    ```
    » ./attack -p 4 <beginning-port> <beginning-ID> <end-ID> <domain> <end-port>

    » ./attack -p 4 53053 1000 2000 vu.nl. 53153

    vunet.vu.nl. 1.2.3.4
    web.vu.nl. 1.2.3.4
    nonexistent.vu.nl. 1.2.3.4
    ```
