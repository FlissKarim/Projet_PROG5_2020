.global main
.text

undefined_instruction:
	mov r1, #38
	movs pc, lr

prefetch_abort:
	@ fixing the problem ...
	mov r1, #99
	subs pc, lr, #4

data_abort:
	@ fixing the problem ...
	
	@ r1 value: sub 8 from lr to re-execute the aborted instruction, sub 4 from lr to pass to the next instruction
	mov r1, #8
	subs pc, lr, r1
	
swi_handler:
	mov r1, #59
	movs pc, lr

irq_handler:
	mov r1, #60
	movs pc, lr

fiq_handler:
	mov r1, #61
	movs pc, lr

main:
	@ put undefined_instruction on interrupt vector (0x4)
    mov r3, #0x4
    ldr r2, =undefined_instruction
    str r2, [r3]
    
	@ put swi_handler on interrupt vector (0x8)
    mov r3, #0x8
    ldr r2, =swi_handler
    str r2, [r3] 

	@ put prefetch_abort on interrupt vector (0xC)
    mov r3, #0xC
    ldr r2, =prefetch_abort
    str r2, [r3]

	@ put data_abort on interrupt vector (0x10)
    mov r3, #0x10
    ldr r2, =data_abort
    str r2, [r3]
        
	@ put irq_handler on interrupt vector (0x18)
    mov r3, #0x18
    ldr r2, =irq_handler
    str r2, [r3]   

	@ put fiq_handler on interrupt vector (0x1C)
    mov r3, #0x1C
    ldr r2, =fiq_handler
    str r2, [r3] 
       
    @ call the swi_handler with the special instruction
	swi 0x8

fin: swi 0x123456
