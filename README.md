# About

The artifacts in this directory will help you demonstrate the *Kaminsky* attack in a custom environment. You will have to use three different VMs, the details of each of which are provided in a separate directory that is named after the role they play.

## Layout

The artifacts of this assignment are organized as follows.

```
artifacts
├── attacker    # Mooncake's VM.
├── quackns     # (recursive) nameserver (to emulate an authoritative NS)
└── victim      # Tribore's VM.
```

Each subdirectory contains an additional `README.md` with more information relevant to that VM or artifacts running in that VM.

## Dependencies

You will need `vagrant` and `virtualbox`, both of which can be freely downloaded and installed in your laptop or desktop.

* Download Vagrant from https://www.vagrantup.com/downloads
* Download VirtualBox from https://www.virtualbox.org/wiki/Downloads

Please also install the vagrant plugins `vagrant-vbguest` and `vagrant-share` as follows.
```
» vagrant plugin install vagrant-vbguest
» vagrant plugin install vagrant-share
```

You should be able to confirm that the plugins were installed properly as shown below.
```
» vagrant plugin list
vagrant-share (2.0.0, global)
vagrant-vbguest (0.30.0, global)
```


## Starting and stopping the VMs

You can start and stop the (headless) VMs as follows. You can learn more about Vagrant [here](https://learn.hashicorp.com/vagrant).

```
# Initialize or start the VM.
» vagrant up

# Power-off the VM.
» vagrant halt

# Destroy the VM (to rebuild it from scratch).
» vagrant destroy
```


## Using the VM

First navigate to the directory containing the Vagrantfile of the VM you want to use. You can then login to the VM (as user `vagrant`) as follows.

```
» vagrant ssh
```

If it, prompts for a password, provide the username.


## Mounting local directory

By default the contents of the VM directory (where the `Vagrantfile` is present) will be mounted inside the VM under the mount point `/vagrant`. You can, therefore, develop your solution using your favorite editor in your local environment, but run the implementation from within the appropriate VM.

If you do not see the local folder (of the host machine) inside the VM, you might have to reboot the VM.

```
# Restart the VM.
» vagrant reload


==> tribore: Machine booted and ready!                                                                     [tribore] GuestAdditions 7.0.2 running --- OK.
==> tribore: Checking for guest additions in VM...
==> tribore: Setting hostname...
==> tribore: Configuring and enabling network interfaces...
==> tribore: Mounting shared folders...
tribore: /vagrant => /Users/.../kaminsky-attack/artifacts/victim
==> tribore: Machine already provisioned. Run `vagrant provision` or use the `--provision`
==> tribore: flag to force provisioning. Provisioners marked to run always will still run.
```


## Network configuration

The VMs are all setup to be on the same network. If you are, however, unable to ping the VMs from one another, ensure that the internal NAT network is setup properly.

```
# If not already configured ...
VBoxManage natnetwork add -t nat-int-network -n 192.168.10.0/24 -e
VBoxManage natnetwork list
```

## Cheating

DO NOT CHEAT. Do **not** attempt to `sniff` packets on this network. The environment in which your solution will be evaluated will be configured to prevent such tricks. In general, do not rely on the weaknesses of your **test** environment, since such weaknesses may not exist in the **grading** environment.
