main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
addi $t0, $zero, 10
sw $t0, -1($fp)
addi $t1, $zero, 20
sw $t1, -2($fp)
lw $t2, -1($fp)
lw $t3, -2($fp)
add $t4, $t2, $t3
sw $t4, -3($fp)
lw $t5, -3($fp)
addi $t6, $zero, 2
addi $t7, $zero, 0
LMD_LOOP_0:
beq $t6, $zero, LMD_END_0
add $t7, $t7, $t5
addi $t6, $t6, -1
j LMD_LOOP_0
LMD_END_0:
sw $t7, -4($fp)
lw $t8, -4($fp)
addi $v0, $t8, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
