# init args for factorial /

push 7 
pop ax 

call factorial: 

out
hlt

#
args: ax - num for factorial
entry: last stack element (factorial of ax)
destr: ax, bx
/

factorial:
    # check ax > 1. if ax > 1, than recursive call. else return /
    push ax 
    push 1
    jbe recursive_end:


    push ax

    # ax -= 1 / 
    push ax
    push 1
    sub
    pop ax

    # recursive call /
    call factorial:

    mul

    pop ax
    pop bx
    push ax 
    push bx 
    ret 
    
recursive_end: 
    pop bx
    push 1
    push bx
    ret
