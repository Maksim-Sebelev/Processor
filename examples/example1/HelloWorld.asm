#=== hello world ===/


#=== pushing chars for "Hello, World!" string in reverse order (because it's stack) ===/

push '\n'  pp cx
push '!'   pp cx
push 'd'   pp cx
push 'l'   pp cx
push 'r'   pp cx
push 'o'   pp cx
push 'W'   pp cx
push '\_'  pp cx
push ','   pp cx
push 'o'   pp cx
push 'l'   pp cx
push 'l'   pp cx
push 'e'   pp cx
push 'H'   pp cx


#=== printing chars in cycle ====/

cycle:

outrc

pp  ax
push ax
push cx

jb cycle:

#==== end of programm ====/

hlt
