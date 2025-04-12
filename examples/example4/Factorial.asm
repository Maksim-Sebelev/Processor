# init args for factorial. we will call 7! (factorial of 7) /
push 7
pop ax

# calling function to calc factorial /
call factorial: 

# print result /
out

# end of programm /
hlt

#
function: factorial
args: ax - num for factorial
entry: last stack element (factorial of ax)
destr: ax, bx
/

factorial:
    # check ax > 1. if ax > 1, than recursive call. else return /
    push ax 
    push 1
    jbe recursive_end:


    # this push for mul in 41 line /
    push ax

    # ax -= 1 / 
    push ax
    push 1
    sub
    pop ax

    # recursive call /
    call factorial:

    # mul for push in 29 line /
    mul


    # swap two last stack element /
    pop ax
    pop bx
    push ax 
    push bx 
    ret 
    
recursive_end:
    # save result in bx /
    pop bx

    # return factorial of 1 (1!) = 1 /
    push 1

    # return result, that is in bx /
    push bx
    ret
