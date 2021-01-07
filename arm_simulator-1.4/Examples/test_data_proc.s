.global main
.text

etq: b suite

main:
	@ at the end thus registers must have the same values as before
	mov r0, #1
	mov r1, #1
	mov r2, #2 
	mov r3, #3
	mov r4, #4 
	mov r5, #5
	mov r6, #6 
	mov r7, #7
	mov r8, #8

	@ test processing data instructions
	orr r0, r0, r7
	eor r0, r0, r7
	
	and r6, r6, r6
	
	add r0, r0, #1
	sub r0, r0, #1

	cmp r0, r1
	beq etq
	bgt etq
	
	bic r5, r5, #1
	add r5, r5, #1
	
	rsbs r8, r7, r0
	bmi etq
	
suite:
	mov r8, #1
	lsl r8, r8, #3
	mov r1, r8, lsr #3
	teq r1, r1
	beq fin
	tst r1, r1
	bne fin
	
fin: swi 0x123456
