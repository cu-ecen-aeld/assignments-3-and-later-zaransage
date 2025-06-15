# Faulty Oops Assessent

## Dana Marble
## MSEE ESL2

### Assignment 7:

Below, we can see that the dev/faulty binary is loaded properly in the qemu environment.

```bash

root@qemuarm64:~# ls /dev/faulty
/dev/faulty

```

We run the basic command into the driver as per the instructions.

```bash
root@qemuarm64:~# echo “hello_world” > /dev/faulty
[  137.054090] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
[  137.057762] Mem abort info:
[  137.057933]   ESR = 0x0000000096000045
[  137.058374]   EC = 0x25: DABT (current EL), IL = 32 bits
[  137.058719]   SET = 0, FnV = 0
[  137.058926]   EA = 0, S1PTW = 0
[  137.059171]   FSC = 0x05: level 1 translation fault
[  137.059510] Data abort info:
[  137.059700]   ISV = 0, ISS = 0x00000045
[  137.059990]   CM = 0, WnR = 1
[  137.060333] user pgtable: 4k pages, 39-bit VAs, pgdp=00000000437ba000
[  137.060798] [0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
[  137.062712] Internal error: Oops: 0000000096000045 [#1] PREEMPT SMP
[  137.064214] Modules linked in: scull(O) hello(O) faulty(O)
[  137.066677] CPU: 0 PID: 422 Comm: sh Tainted: G           O      5.15.164-yocto-standard #1
[  137.068367] Hardware name: linux,dummy-virt (DT)
[  137.069845] pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[  137.071147] pc : faulty_write+0x18/0x20 [faulty]
[  137.074443] lr : vfs_write+0xf8/0x29c
[  137.079037] sp : ffffffc0096e3d80
[  137.079273] x29: ffffffc0096e3d80 x28: ffffff8002051b80 x27: 0000000000000000
[  137.079690] x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
[  137.079936] x23: 0000000000000000 x22: ffffffc0096e3dc0 x21: 00000055750e6ba0
[  137.080174] x20: ffffff8003605d00 x19: 0000000000000012 x18: 0000000000000000
[  137.080408] x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
[  137.080665] x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
[  137.080904] x11: 0000000000000000 x10: 0000000000000000 x9 : ffffffc00826a7ac
[  137.081165] x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
[  137.081788] x5 : 0000000000000001 x4 : ffffffc000b80000 x3 : ffffffc0096e3dc0
[  137.082538] x2 : 0000000000000012 x1 : 0000000000000000 x0 : 0000000000000000
[  137.084127] Call trace:
[  137.084619]  faulty_write+0x18/0x20 [faulty]
[  137.085256]  ksys_write+0x74/0x10c
[  137.085981]  __arm64_sys_write+0x24/0x30
[  137.086486]  invoke_syscall+0x5c/0x130
[  137.086850]  el0_svc_common.constprop.0+0x4c/0x100
[  137.087691]  do_el0_svc+0x4c/0xb4
[  137.088336]  el0_svc+0x28/0x80
[  137.088535]  el0t_64_sync_handler+0xa4/0x130
[  137.092509]  el0t_64_sync+0x1a0/0x1a4
[  137.092797] Code: d2800001 d2800000 d503233f d50323bf (b900003f)
[  137.094504] ---[ end trace d7b1d138ceda2720 ]---
Segmentation fault

Poky (Yocto Project Reference Distro) 4.0.21 qemuarm64 /dev/ttyAMA0

qemuarm64 login:

```

### Key indicators:

Here it says we are to dereference a null pointer, which of course, is not possible.

```bash

[  137.054090] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000

```


### Next indicators:

The 18th byte in the file errored out. The greater the number, the later in the code we might expect to find this error.

```bash

 faulty_write+0x18/0x20 [faulty]

```

### Lastly:

x0 and x1 suggest the number of parameters for the function and the length of the inputs.
All zeros suggests these are null values.

X2 suggests the length of the data in the function.

```bash

[  137.082538] x2 : 0000000000000012 x1 : 0000000000000000 x0 : 0000000000000000

```