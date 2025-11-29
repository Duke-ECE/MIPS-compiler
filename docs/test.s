main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
addi $t0, $zero, 3
sw $t0, -1($fp)
lw $t1, -1($fp)
addi $t2, $zero, 5
add $t3, $t1, $t2
sw $t3, -2($fp)
addi $t4, $zero, 0
addi $v0, $t4, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra