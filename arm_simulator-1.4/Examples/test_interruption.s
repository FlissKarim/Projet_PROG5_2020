.global main
.text

swi_handler:
	mov r1, #59
	movs pc, lr

irq_handler:
	mov r1, #60
	movs pc, lr

main:
	@ put swi_handler on interrupt vector (0x8)
    mov r3, #0x8
    ldr r2, =swi_handler
    str r2, [r3]    

	@ put irq on interrupt vector (0x18)
    mov r3, #0x18
    ldr r2, =irq_handler
    str r2, [r3]   
    
    @ call the swi interrupt
	swi 0x8

fin: swi 0x123456
