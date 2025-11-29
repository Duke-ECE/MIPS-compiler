_func_main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
sw $s0, 3($sp)
sw $s1, 4($sp)
sw $s2, 5($sp)
addi $t0, $zero, 5
sw $t0, -1($fp)
addi $t1, $zero, 3
sw $t1, -2($fp)
lw $t2, -1($fp)
lw $t3, -2($fp)
addi $a0, $t2, 0
addi $a1, $t3, 0
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
jal _func_add
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
addi $t4, $v0, 0
sw $t4, -3($fp)
output $t5
L6:
addi $t6, $zero, 1
beq $t6, $zero, L7
j L6
L7:
addi $t7, $zero, 0
addi $v0, $t7, 0
lw $s0, 3($fp)
lw $s1, 4($fp)
lw $s2, 5($fp)
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
_func_add:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
sw $s0, 3($sp)
sw $s1, 4($sp)
sw $s2, 5($sp)
sw $a0, -1($fp)
sw $a1, -2($fp)
lw $t8, -1($fp)
lw $t9, -2($fp)
add $s0, $t8, $t9
addi $s1, $zero, 48
add $s2, $s0, $s1
sw $s2, -3($fp)
addi $v0, $s2, 0
lw $s0, 3($fp)
lw $s1, 4($fp)
lw $s2, 5($fp)
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
