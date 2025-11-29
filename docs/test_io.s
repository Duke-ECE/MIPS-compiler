main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
L0:
addi $t0, $zero, 1
beq $t0, $zero, L1
input $t1
sw $t1, -1($fp)
lw $t2, -1($fp)
addi $t3, $zero, 10
beq $t2, $t3, LCMP_T_0
addi $t4, $zero, 0
j LCMP_E_0
LCMP_T_0:
addi $t4, $zero, 1
LCMP_E_0:
beq $t4, $zero, L7
addi $t5, $zero, 104
output $t5
j L8
L7:
L8:
j L0
L1:
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
