#=== hello world ===/



#=== pushing chars for "Hello, World!" string in reverse order (because it's stack) ===/

push 10
push 33
push 100
push 108
push 114
push 111
push 87
push 32
push 44
push 111
push 108
push 108
push 101
push 72


#=== printing chars in cycle ====/

arman:

outrc

push 1
push ax
add
pop ax
push ax
push ax
pop [ax+100]
push 14

ja arman:

#==== end of programm ====/

hlt
