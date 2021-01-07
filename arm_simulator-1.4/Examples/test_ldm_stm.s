.global main
.text

main:
	mov r0, #0

	mov r1, #1
	mov r2, #2
	mov r3, #3
	mov r4, #4

	stm r0, {r1, r2, r3, r4}
	
	ldm r0, {r5, r6, r7, r8}
	
fin: swi 0x123456
