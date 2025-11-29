fibonacci:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
sw $t0, -1($fp)
lw $t1, -1($fp)
addi $t2, $zero, 2
bgt $t2, $t1, LCMP_T_0
addi $t3, $zero, 0
j LCMP_E_0
LCMP_T_0:
addi $t3, $zero, 1
LCMP_E_0:
beq $t3, $zero, L3
lw $t4, -1($fp)
addi $v0, $t4, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
j L4
L3:
L4:
lw $t5, -1($fp)
addi $t6, $zero, 1
sub $t7, $t5, $t6
addi $a0, $t7, 0
# Save caller-saved
addi $sp, $sp, -10
sw $t0, 1($sp)
sw $t1, 2($sp)
sw $t2, 3($sp)
sw $t3, 4($sp)
sw $t4, 5($sp)
sw $t5, 6($sp)
sw $t6, 7($sp)
sw $t7, 8($sp)
sw $t8, 9($sp)
sw $t9, 10($sp)
jal fibonacci
lw $t0, 1($sp)
lw $t1, 2($sp)
lw $t2, 3($sp)
lw $t3, 4($sp)
lw $t4, 5($sp)
lw $t5, 6($sp)
lw $t6, 7($sp)
lw $t7, 8($sp)
lw $t8, 9($sp)
lw $t9, 10($sp)
addi $sp, $sp, 10
addi $t8, $v0, 0
lw $t9, -1($fp)
addi $t0, $zero, 2
sub $t0, $t9, $t0
addi $a0, $t0, 0
# Save caller-saved
addi $sp, $sp, -10
sw $t0, 1($sp)
sw $t1, 2($sp)
sw $t2, 3($sp)
sw $t3, 4($sp)
sw $t4, 5($sp)
sw $t5, 6($sp)
sw $t6, 7($sp)
sw $t7, 8($sp)
sw $t8, 9($sp)
sw $t9, 10($sp)
jal fibonacci
lw $t0, 1($sp)
lw $t1, 2($sp)
lw $t2, 3($sp)
lw $t3, 4($sp)
lw $t4, 5($sp)
lw $t5, 6($sp)
lw $t6, 7($sp)
lw $t7, 8($sp)
lw $t8, 9($sp)
lw $t9, 10($sp)
addi $sp, $sp, 10
addi $t0, $v0, 0
add $t0, $t8, $t0
addi $v0, $t0, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
input $t0
sw $t0, -1($fp)
lw $t0, -1($fp)
addi $a0, $t0, 0
# Save caller-saved
addi $sp, $sp, -10
sw $t0, 1($sp)
sw $t1, 2($sp)
sw $t2, 3($sp)
sw $t3, 4($sp)
sw $t4, 5($sp)
sw $t5, 6($sp)
sw $t6, 7($sp)
sw $t7, 8($sp)
sw $t8, 9($sp)
sw $t9, 10($sp)
jal fibonacci
lw $t0, 1($sp)
lw $t1, 2($sp)
lw $t2, 3($sp)
lw $t3, 4($sp)
lw $t4, 5($sp)
lw $t5, 6($sp)
lw $t6, 7($sp)
lw $t7, 8($sp)
lw $t8, 9($sp)
lw $t9, 10($sp)
addi $sp, $sp, 10
addi $t0, $v0, 0
sw $t0, -2($fp)
lw $t0, -2($fp)
output $t0
addi $t0, $zero, 0
addi $v0, $t0, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
