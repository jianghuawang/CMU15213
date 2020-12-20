
cl3.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 a8 dc 61 55 	mov    $0x5561dca8,%rdi
   7:	48 b8 35 39 62 39 39 	movabs $0x6166373939623935,%rax
   e:	37 66 61 
  11:	48 89 07             	mov    %rax,(%rdi)
  14:	c6 47 08 00          	movb   $0x0,0x8(%rdi)
  18:	68 fa 18 40 00       	pushq  $0x4018fa
  1d:	c3                   	retq   
