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
output $t2
j L0
L1:
addi $t3, $zero, 0
addi $v0, $t3, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
