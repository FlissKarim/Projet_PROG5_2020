.global main
.text

main:
	@ test passed if at the end r5, r6, r7, r8 will have the same values as r1, r2, r3, r4
	mov r1, #1
	mov r2, #2
	mov r3, #3
	mov r4, #4

	@@@ 1 & 2 do the same thing @@@
	
	@ 1)
	@ start from the address r0, is the base registre
	stmia r0, {r1, r2, r3, r4}	
	ldmia r0, {r5, r6, r7, r8}
	
	@ 2)
	@ start from the address = 12 - 16 (16 is number of registre between brackets * 4) + 4
	mov r0, #12
	stmda r0, {r1, r2, r3, r4}	
	ldmda r0, {r5, r6, r7, r8}
	
fin: swi 0x123456
