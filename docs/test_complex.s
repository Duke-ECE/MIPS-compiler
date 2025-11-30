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
addi $t5, $zero, 2
addi $t6, $zero, 0
LMD_LOOP_0:
beq $t5, $zero, LMD_END_0
add $t6, $t6, $t4
addi $t5, $t5, -1
j LMD_LOOP_0
LMD_END_0:
sw $t6, -4($fp)
addi $v0, $t6, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
