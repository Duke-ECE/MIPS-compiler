_func_test_many_vars:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
addi $t0, $zero, 1
sw $t0, -1($fp)
addi $t1, $zero, 2
sw $t1, -2($fp)
addi $t2, $zero, 3
sw $t2, -3($fp)
addi $t3, $zero, 4
sw $t3, -4($fp)
addi $t4, $zero, 5
sw $t4, -5($fp)
addi $t5, $zero, 6
sw $t5, -6($fp)
addi $t6, $zero, 7
sw $t6, -7($fp)
addi $t7, $zero, 8
sw $t7, -8($fp)
addi $t8, $zero, 9
sw $t8, -9($fp)
addi $t9, $zero, 10
sw $t9, -10($fp)
addi $s0, $zero, 11
sw $s0, -11($fp)
addi $s1, $zero, 12
sw $s1, -12($fp)
addi $s2, $zero, 13
sw $s2, -13($fp)
addi $s3, $zero, 14
sw $s3, -14($fp)
addi $s4, $zero, 15
sw $s4, -15($fp)
addi $s5, $zero, 16
sw $s5, -16($fp)
addi $s6, $zero, 17
sw $s6, -17($fp)
addi $s7, $zero, 18
sw $s7, -18($fp)
addi $t0, $zero, 19
sw $t0, -19($fp)
addi $t1, $zero, 20
sw $t1, -20($fp)
lw $t2, -1($fp)
lw $t3, -2($fp)
add $t4, $t2, $t3
lw $t5, -3($fp)
add $t6, $t4, $t5
lw $t7, -4($fp)
add $t8, $t6, $t7
lw $t9, -5($fp)
add $t0, $t8, $t9
lw $t1, -6($fp)
add $t2, $t0, $t1
lw $t3, -7($fp)
add $t4, $t2, $t3
lw $t5, -8($fp)
add $t6, $t4, $t5
lw $t7, -9($fp)
add $t8, $t6, $t7
lw $t9, -10($fp)
add $t0, $t8, $t9
sw $t0, -21($fp)
lw $t1, -21($fp)
lw $t2, -11($fp)
add $t3, $t1, $t2
lw $t4, -12($fp)
add $t5, $t3, $t4
lw $t6, -13($fp)
add $t7, $t5, $t6
lw $t8, -14($fp)
add $t9, $t7, $t8
lw $t0, -15($fp)
add $t1, $t9, $t0
lw $t2, -16($fp)
add $t3, $t1, $t2
lw $t4, -17($fp)
add $t5, $t3, $t4
lw $t6, -18($fp)
add $t7, $t5, $t6
lw $t8, -19($fp)
add $t9, $t7, $t8
lw $t0, -20($fp)
add $t1, $t9, $t0
sw $t1, -21($fp)
lw $t2, -21($fp)
addi $v0, $t2, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
_func_main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
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
jal _func_test_many_vars
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
sw $t0, -1($fp)
lw $t1, -1($fp)
output $t1
addi $t2, $zero, 0
addi $v0, $t2, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
