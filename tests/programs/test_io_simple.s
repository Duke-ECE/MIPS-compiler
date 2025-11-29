main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
input $t0
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
