.code

Glitch PROC

mov r8,en
mov r9,rsp
pushfq
or dword ptr[rsp],10100h
popfq
jmp rcx
en:
ret


Glitch ENDP

end