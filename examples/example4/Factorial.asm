





    push 5
pop ax

call factorial:

push bx
out

hlt


#
arg: ax - num for factorial
ret: bx - factorial of ax
/

factorial:
    push 1
    pop bx
    call factorial_help:
    ret


factorial_help:
    push ax
    push 1
    je end:

    # ax * bx /
    push ax
    push bx
    mul
    pop bx


    # ax = ax - 1 /
    push ax
    push 1
    sub

    pop ax

    # recursive call /
    call factorial_help:

    end:
    ret
