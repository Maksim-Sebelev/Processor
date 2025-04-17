# init args for draw_circle /

push 600
pop ax

push 800
pop bx


push 120
pop cx

# call function to draw a circle with args high (ax) = 600, width (bx) = 800, radius (cx) = 100 /
call draw_circle:

hlt


# ===============================
function: draw_circle - drawing a circle in the sfml window 
in: ax - high
    bx - width
    cx - radius
out: none
destr: ax, bx, cx, dx
================================== /
 
draw_circle:
    push cx
    push cx
    mul
    pop  cx   # in cx square of radius /

    push 0
    pop dx

    cycle:

    call calc_dist_to_centre:
    push fx
    push cx
    ja notINCircle:
    jmp inCircle:

    back_in_cycle:

    pp   dx
    push dx
    push ax
    push bx
    mul

    jb cycle:

    draw ax bx

    ret

#========================/



# ===============
in:    ax - high
       bx - width
       dx - index of array elem
out:   fx
destr: fx
================= / 

calc_dist_to_centre:
    push dx 
    push dx
    push bx
    div
    push bx
    mul
    sub  # x-coordinat of aray elem (y = i%ax = i - (i:ax)*ax) / 

    push bx
    push 2
    div    # x-coordinat of center /

    sub 
    pop  fx
    push fx
    push fx
    mul     # (x - x_center)^2 /



    push dx
    push bx
    div  # y-coordinat of array elem (x = i:ax) /

    push ax
    push 2
    div  # y-coordinat of center /

    sub
    pop  fx
    push fx
    push fx 
    mul     # (y - y_center)^2 /


    add   # (x - x_center)^2 + (y - y_center)^2  /

    pop  fx

    ret

# =========== /


inCircle:
    push 2147483175
    pop [dx]
    jmp back_in_cycle:


notINCircle:
    push 1
    pop [dx]
    jmp back_in_cycle:


