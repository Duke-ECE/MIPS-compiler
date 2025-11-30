_func_main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
sw $s0, 3($sp)
sw $s1, 4($sp)
sw $s2, 5($sp)
sw $s3, 6($sp)
sw $s4, 7($sp)
sw $s5, 8($sp)
sw $s6, 9($sp)
sw $s7, 10($sp)
addi $t0, $zero, 512
sw $t0, -1($fp)
addi $t1, $zero, 4
sw $t1, 0($t0)
lw $t2, -1($fp)
lw $t3, 0($t2)
sw $t3, -2($fp)
lw $t4, -1($fp)
addi $t5, $zero, 1
add $t6, $t4, $t5
addi $t7, $zero, 9
sw $t7, 0($t6)
lw $t8, -1($fp)
addi $t9, $zero, 1
add $s0, $t8, $t9
lw $s1, 0($s0)
sw $s1, -3($fp)
lw $s2, -2($fp)
addi $s3, $zero, 48
add $s4, $s2, $s3
sw $s4, -4($fp)
lw $s5, -3($fp)
addi $s6, $zero, 48
add $s7, $s5, $s6
sw $s7, -5($fp)
lw $t0, -2($fp)
output $t0
lw $t1, -3($fp)
output $t1
L21:
addi $t2, $zero, 1
beq $t2, $zero, L22
j L21
L22:
addi $t3, $zero, 0
addi $v0, $t3, 0
lw $s0, 3($fp)
lw $s1, 4($fp)
lw $s2, 5($fp)
lw $s3, 6($fp)
lw $s4, 7($fp)
lw $s5, 8($fp)
lw $s6, 9($fp)
lw $s7, 10($fp)
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
