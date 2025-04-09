#======= cycle =======/

#=== init ax (ax = 0) ===/
push 0
pop ax


#=== print 0 1 2 3 ... 10 ===/

cycle:
push ax
out
push 1
add
pop ax
push ax
push 10
jae cycle:

#=== init ax for second (back) cycle (ax = 11) ===/

push 11
pop ax

#=== print 10 9 ... 2 1 0 ===/

back_cycle:
push ax
push 1
sub
out
pop ax
push ax
push 0
jb back_cycle:


#=== end of programm ===/
hlt
