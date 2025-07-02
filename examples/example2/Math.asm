; calc (11 + 4) * 3 : 5 - 4

push 11
push 4 
add       ; 11 + 4

push 3
mul       ; (11 + 4) * 3

push 5
div       ; ((11 + 4) * 3) / 5 

push 4
sub       ; (((11 + 4) * 3) / 5) - 4

out       ; print result

push '\n' 
outrc

hlt       ; end of programm
