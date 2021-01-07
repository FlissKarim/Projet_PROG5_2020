.global main
.text
a:
    add r2, r2, #1
    bal a1
b:
    add r3, r3, #1
    bal b1
c:
    add r4, r4, #1
    bal c1
d:
    add r5, r5, #1
    bal d1
e:
    add r6, r6, #1  
    bal e1
f:
    add r7, r7, #1
    bal f1
g:
    add r8, r8, #1
    bal g1
h:
    add r9, r9, #1
    bal h1



main:
    mov r0, #0x7FFFFFFF
    mov r1, #0
    add r0, r0, #10
    beq a
a1:  
    bhs b
b1:
    bmi c
c1:
    bvc d
d1:

    sub r1, r1, #1
    beq e   
e1:
    bcs f
f1:
    bmi g
g1:
    bvc h
h1:
    
    swi 0x123456
