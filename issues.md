# Known Issues

The following are some issues you might encounter when running `vagrant` and `virtualbox` on macOS, especially Ventura (13.0).

## Failed to create the host-only adapter

```
§ vagrant up
Bringing machine 'quackns' up with 'virtualbox' provider...
==> quackns: Importing base box 'ubuntu/trusty64'...
==> quackns: Matching MAC address for NAT networking...
==> quackns: Checking if box 'ubuntu/trusty64' version '20190514.0.0' is up to date...
==> quackns: Setting the name of the VM: quackns
==> quackns: Clearing any previously set forwarded ports...
==> quackns: Clearing any previously set network interfaces...
There was an error while executing `VBoxManage`, a CLI used by Vagrant
for controlling VirtualBox. The command and stderr is shown below.

Command: ["hostonlyif", "create"]

Stderr: 0%...
Progress state: NS_ERROR_FAILURE
VBoxManage: error: Failed to create the host-only adapter
VBoxManage: error: VBoxNetAdpCtl: Error while adding new interface: failed to open /dev/vboxnetctl: No such file or directory
VBoxManage: error: Details: code NS_ERROR_FAILURE (0x80004005), component HostNetworkInterfaceWrap, interface IHostNetworkInterface
VBoxManage: error: Context: "RTEXITCODE handleCreate(HandlerArg *)" at line 95 of file VBoxManageHostonly.cpp
```

### Solution

If you are using macOS Ventura, the OS will not load the `kexts` (kernel extensions) used by `VirtualBox 6.1.40` and earlier. Upgrade to `VirtualBox 7.0.2` (or later) on Ventura.

For earlier macOS versions, try the following.
```
sudo kextload -b org.virtualbox.kext.VBoxDrv;
sudo kextload -b org.virtualbox.kext.VBoxNetFlt;
sudo kextload -b org.virtualbox.kext.VBoxNetAdp;
sudo kextload -b org.virtualbox.kext.VBoxUSB;
```

If running the above does not work, try disabling macOS's System Integration Protection (SIP) as follows.
```
§ csrutil status
System Integrity Protection status: enabled.
```
