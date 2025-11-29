main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
L0:
addi $t0, $zero, 1
beq $t0, $zero, L1
addi $t1, $zero, 104
output $t1
addi $t2, $zero, 101
output $t2
addi $t3, $zero, 108
output $t3
addi $t4, $zero, 108
output $t4
addi $t5, $zero, 111
output $t5
j L0
L1:
addi $t6, $zero, 0
addi $v0, $t6, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
