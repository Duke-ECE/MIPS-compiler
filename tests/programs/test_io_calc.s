main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
input $t0
sw $t0, -1($fp)
input $t1
sw $t1, -2($fp)
lw $t2, -1($fp)
lw $t3, -2($fp)
add $t4, $t2, $t3
sw $t4, -3($fp)
lw $t5, -3($fp)
output $t5
addi $t6, $zero, 0
addi $v0, $t6, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
