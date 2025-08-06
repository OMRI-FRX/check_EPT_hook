extern ALvmIdtHandler : proc

trap_frame struct
  ; general-purpose registers
  $rax qword ?
  $rcx qword ?
  $rdx qword ?
  $rbx qword ?
  $rbp qword ?
  $rsi qword ?
  $rdi qword ?
  $r8  qword ?
  $r9  qword ?
  $r10 qword ?
  $r11 qword ?
  $r12 qword ?
  $r13 qword ?
  $r14 qword ?
  $r15 qword ?

  ; interrupt vector
  $vector qword ?

  ; _MACHINE_FRAME
  $error  qword ?
  $rip    qword ?
  $cs     qword ?
  $rflags qword ?
  $rsp    qword ?
  $ss     qword ?
trap_frame ends

.code

; pushes error code to stack
IDT_HANDLER macro  proc_name:req,code:req
proc_name proc
  ; allocate space for the trap_frame structure (minus the size of the
  ; _MACHINE_FRAME, error code, and interrupt vector)
  sub rsp, 78h

  ; general-purpose registers
  mov trap_frame.$rax[rsp], rax
  mov trap_frame.$rcx[rsp], rcx
  mov trap_frame.$rdx[rsp], rdx
  mov trap_frame.$rbx[rsp], rbx
  mov trap_frame.$rbp[rsp], rbp
  mov trap_frame.$rsi[rsp], rsi
  mov trap_frame.$rdi[rsp], rdi
  mov trap_frame.$r8[rsp],  r8
  mov trap_frame.$r9[rsp],  r9
  mov trap_frame.$r10[rsp], r10
  mov trap_frame.$r11[rsp], r11
  mov trap_frame.$r12[rsp], r12
  mov trap_frame.$r13[rsp], r13
  mov trap_frame.$r14[rsp], r14
  mov trap_frame.$r15[rsp], r15

  ; first argument is the trap frame
  mov rcx, rsp

  ; call handle_host_interrupt
  sub rsp, 20h
  call ALvmIdtHandler
  add rsp, 20h

  mov trap_frame.$vector[rsp],rax ;把获取到的地址放在向量里
 ; int 3
  ; general-purpose registers
  mov rax, trap_frame.$rax[rsp]
  mov rcx, trap_frame.$rcx[rsp]
  mov rdx, trap_frame.$rdx[rsp]
  mov rbx, trap_frame.$rbx[rsp]
  mov rbp, trap_frame.$rbp[rsp]
  mov rsi, trap_frame.$rsi[rsp]
  mov rdi, trap_frame.$rdi[rsp]
  mov r8,  trap_frame.$r8[rsp]
  mov r9,  trap_frame.$r9[rsp]
  mov r10, trap_frame.$r10[rsp]
  mov r11, trap_frame.$r11[rsp]
  mov r12, trap_frame.$r12[rsp]
  mov r13, trap_frame.$r13[rsp]
  mov r14, trap_frame.$r14[rsp]
  mov r15, trap_frame.$r15[rsp]

  ; free the trap_frame
  add rsp, 78h
  ; pop the interrupt vector
  add rsp, 8
  ; pop the error code
  add rsp, 8
  ;int 3	
  iretq
proc_name endp
endm



; pushes error code to stack
DEFINE_ISR macro interrupt_vector:req, proc_name:req
proc_name proc
  ; interrupt vector is stored right before the machine frame
  push interrupt_vector

  jmp ALvmIdtHandler_asm_yes
proc_name endp
endm

; doesn't push error code to stack
DEFINE_ISR_NO_ERROR macro interrupt_vector:req, proc_name:req
proc_name proc
  ; push a dummy error code onto the stack
  push 0

  ; interrupt vector is stored right before the machine frame
  push interrupt_vector

  jmp ALvmIdtHandler_asm_no
proc_name endp
endm

IDT_HANDLER		ALvmIdtHandler_asm_no	,8
IDT_HANDLER		ALvmIdtHandler_asm_yes	,0



DEFINE_ISR_NO_ERROR  0, interrupt_handler_00_DE      ;;/** Divide-by-Zero-Error Exception (Vector 0) */    
DEFINE_ISR_NO_ERROR  1, interrupt_handler_01_DB      ;;/** Debug Exception (Vector 1) */    
DEFINE_ISR_NO_ERROR  2, interrupt_handler_02_NMI     ;;/** Non-Maskable-Interrupt Exception (Vector 2) */    
DEFINE_ISR_NO_ERROR  3, interrupt_handler_03_BP      ;;/** Breakpoint Exception (Vector 3) */    
DEFINE_ISR_NO_ERROR  4, interrupt_handler_04_OF      ;;/** Overflow Exception (Vector 4) */    
DEFINE_ISR_NO_ERROR  5, interrupt_handler_05_BR      ;;/** Bound-Range Exception (Vector 5) */    
DEFINE_ISR_NO_ERROR  6, interrupt_handler_06_UD      ;;/** Invalid-Opcode Exception (Vector 6) */    
DEFINE_ISR_NO_ERROR  7, interrupt_handler_07_NM      ;;/** Device-Not-Available Exception (Vector 7) */    
DEFINE_ISR           8, interrupt_handler_08_DF      ;;/** Double-Fault Exception (Vector 8) */    
					            				
DEFINE_ISR			10, interrupt_handler_10_TS      ;;/** Invalid-TSS Exception (Vector 10) */    
DEFINE_ISR          11, interrupt_handler_11_NP      ;;/** Segment-Not-Present Exception (Vector 11) */    
DEFINE_ISR          12, interrupt_handler_12_SS      ;;/** Stack Exception (Vector 12) */    
DEFINE_ISR          13, interrupt_handler_13_GP      ;;/** General-Protection Exception (Vector 13) */    
DEFINE_ISR          14, interrupt_handler_14_PF      ;;/** Page-Fault Exception (Vector 14) */    
						               			
DEFINE_ISR_NO_ERROR 16, interrupt_handler_16_MF      ;;/** x87 Floating-Point Exception-Pending (Vector 16) */    
DEFINE_ISR          17, interrupt_handler_17_AC      ;;/** Alignment-Check Exception (Vector 17) */    
DEFINE_ISR_NO_ERROR 18, interrupt_handler_18_MC      ;;/** Machine-Check Exception (Vector 18) */    
DEFINE_ISR_NO_ERROR 19, interrupt_handler_19_XF      ;;/** SIMD Floating-Point Exception (Vector 19) */    
DEFINE_ISR_NO_ERROR 20, interrupt_handler_20_VE      ;;EPT violations This exception can occur only on processors 
;;														that support the 1-setting of the “EPT-violation #VE” VM-execution control.
DEFINE_ISR          21, interrupt_handler_21_CP      ;;/** Control-Protection Exception (Vector 21) */  



end