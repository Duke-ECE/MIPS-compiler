main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
addi $t0, $zero, 3
sw $t0, -1($fp)
addi $t1, $zero, 5
add $t2, $t0, $t1
sw $t2, -2($fp)
addi $t3, $zero, 0
addi $v0, $t3, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
