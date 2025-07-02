; cycle

; init ax (ax = 0)
push 0
pop ax

; print 0 1 2 3 ... 10

cycle:

push ax

out
push ' '
outrc

push 10
pp ax

jb cycle:

; print '\n' (new line)

push '\n'
outrc

; init ax for second (back) cycle (ax = 11)

push 11
pop ax

; print 10 9 ... 2 1 0

back_cycle:

mm ax

push ax

out
push ' '
outrc

push 0

ja back_cycle:


; print '\n' (new line)

push '\n'
outrc

; end of programm

hlt
