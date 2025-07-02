; init args for factorial. we will call 7! (factorial of 7)
push 7 ; you can type here any number
pop ax

push ax ; this push for print result

; calling function to calc factorial
call factorial: 

; print 'n! = ...'

pop ax ; save result in ax

outr

push '!'
outrc

push ' '
outrc

push '='
outrc

push ' '
outrc

; print answer
push ax
outr

; print '\n' for new line
push '\n'
outrc

; end of programm
hlt


; ===============================
; function: factorial
; args: ax - num for factorial
; entry: last stack element (factorial of ax)
; destr: ax, bx
;================================

factorial:
    jmp check_arg:

factorial_helper:
    ; check ax > 1. if ax > 1, than recursive call. else return
    push ax
    push 1

    jbe end_of_recursive_factorial_call:

    ; this push for mul in 41 line
    push ax

    ; ax -= 1 
    push ax
    push 1
    sub
    pop ax

    ; recursive call
    call factorial_helper:

    ; mul for push in 29 line
    mul

    ; swap two last stack element
    pop ax
    pop bx
    push ax 
    push bx 
    ret
    
end_of_recursive_factorial_call:
    ; save result in bx
    pop bx

    ; return factorial of 1 (1!) = 1
    push 1

    ; return result, that is in bx
    push bx
    ret


check_arg:
    push ax
    push -1
    ja factorial_helper:

error:
    call print_err_msg:
    hlt

print_err_msg:
    push 'a'  outrc
    push 'r'  outrc
    push 'g'  outrc
    push ' '  outrc
    push 'm'  outrc
    push 'u'  outrc
    push 's'  outrc
    push 't'  outrc
    push ' '  outrc
    push 'b'  outrc
    push 'e'  outrc
    push ' '  outrc
    push '>'  outrc
    push '='  outrc
    push ' '  outrc
    push '0'  outrc
    push '\n' outrc
    push 'y'  outrc
    push 'o'  outrc
    push 'u'  outrc
    push 'r'  outrc
    push ' '  outrc
    push 'a'  outrc
    push 'r'  outrc
    push 'g'  outrc
    push ' '  outrc
    push '='  outrc
    push ' '  outrc
    push ax   outr
    push '\n' outrc

    ret
