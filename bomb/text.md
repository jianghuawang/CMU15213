### phase1
---
(we can directly find the number of chars via the string length functions and the storing address of the correct string)

hex code:

426f7264

65722072

656c6174

696f6e73

20776974

68204361

6e616461

20686176

65206e65

76657220

6265656e

20626574

7465722e

**Answer**: Border relations with Canada have never been better.


### phase2
---
(we know read_six_numbers split our string into 6 parts, and in the phase_2 function, the function first check if the first number is 1, and then use a while loop to check if the current value is 2 times the previous value )

**Answer**: 1 2 4 8 16 32 


### phase3
---
(from the argument number, we first decide there should be 2 number, and the first one must be larger than 1. The difficulties comes from reading the jump table(the one we learn in switch statement))

**Answer**: 2 707	(there may be other answers out there)


### phase4
---
(just as the last one, we know we should have two numbers after the sscanf function(the comparision betwen 0x2 and %rax), and the first number should be less than 0xe. In the function 4, I am not very sure what the function is doing(it is recursion) but if I have the first number equal to %ecx, I can get out the function with a return value 0, which is exactly what I want. The second number is easy to get(the comparision).)

**Answer**: 7 0


### phase5(This one is harder than all of the previous ones, but it is still easy. First, from the length function and the following comparison, we know the length of the string is 6. And then there is a while loop that do some weird things. I would suggest look at the code after the while loop and then come back. After the while loop, we see that it compares two strings, the one with specific address and one that just store during the while loop. We can find the 6 hexdecimal values stored at the specific address. Then, we can come back to the while loop. `0x4024b0(%rdx)` this instruction is really important, we basically need to find the value of %rdx such that M[0x4024b0+%rdx](retrieve 1 byte each time) will equal to the 6 values we just get in order. And we look above, we find that %edx is just the lower 4 bits value of the char we input, so we open the ascii table, and looking for char that have these hex as the lower 4 bits.)
---
**Answer**: IONEFG

### phase6
---
