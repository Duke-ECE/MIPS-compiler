main:
addi $r1, $r0, 3
addi $r2, $r1, 0
addi $r3, $r0, 5
add $r4, $r2, $r3
addi $r5, $r4, 0
L3:
addi $r6, $r0, 0
bgt $r5, $r6, LCMP_T_0
addi $r7, $r0, 0
j LCMP_E_0
LCMP_T_0:
addi $r7, $r0, 1
LCMP_E_0:
beq $r7, $r0, L4
addi $r8, $r0, 1
sub $r9, $r5, $r8
addi $r5, $r9, 0
j L3
L4:
addi $r10, $r0, 0
jr $r10
