; x64 /Gh /GH hooks of the call trace (port/provisional/calltrace.cpp). The compiler calls _penter / _pexit at a function's entry / before its
; return with an unknown stack alignment (leaf functions: before any frame), so the stub realigns, saves the volatile integer registers
; and xmm0-5 (rax / xmm0 carry the return value at _pexit) and calls ct_enter / ct_exit(return address, saved registers, xmm block).
; Saved-register block (u64 words from rbp-64): 0 r11, 1 r10, 2 r9, 3 r8, 4 rdx, 5 rcx, 6 rax, 7 rflags; xmm block: xmm0-5, 16 bytes each.
extrn ct_enter:proc
extrn ct_exit:proc
.code
_penter proc
  push rbp
  mov rbp, rsp
  pushfq
  push rax
  push rcx
  push rdx
  push r8
  push r9
  push r10
  push r11
  sub rsp, 128
  and rsp, -16
  movdqu [rsp+32], xmm0
  movdqu [rsp+48], xmm1
  movdqu [rsp+64], xmm2
  movdqu [rsp+80], xmm3
  movdqu [rsp+96], xmm4
  movdqu [rsp+112], xmm5
  mov rcx, [rbp+8]
  lea rdx, [rbp-64]
  lea r8, [rsp+32]
  call ct_enter
  movdqu xmm0, [rsp+32]
  movdqu xmm1, [rsp+48]
  movdqu xmm2, [rsp+64]
  movdqu xmm3, [rsp+80]
  movdqu xmm4, [rsp+96]
  movdqu xmm5, [rsp+112]
  lea rsp, [rbp-64]
  pop r11
  pop r10
  pop r9
  pop r8
  pop rdx
  pop rcx
  pop rax
  popfq
  pop rbp
  ret
_penter endp
_pexit proc
  push rbp
  mov rbp, rsp
  pushfq
  push rax
  push rcx
  push rdx
  push r8
  push r9
  push r10
  push r11
  sub rsp, 128
  and rsp, -16
  movdqu [rsp+32], xmm0
  movdqu [rsp+48], xmm1
  movdqu [rsp+64], xmm2
  movdqu [rsp+80], xmm3
  movdqu [rsp+96], xmm4
  movdqu [rsp+112], xmm5
  mov rcx, [rbp+8]
  lea rdx, [rbp-64]
  lea r8, [rsp+32]
  call ct_exit
  movdqu xmm0, [rsp+32]
  movdqu xmm1, [rsp+48]
  movdqu xmm2, [rsp+64]
  movdqu xmm3, [rsp+80]
  movdqu xmm4, [rsp+96]
  movdqu xmm5, [rsp+112]
  lea rsp, [rbp-64]
  pop r11
  pop r10
  pop r9
  pop r8
  pop rdx
  pop rcx
  pop rax
  popfq
  pop rbp
  ret
_pexit endp
end
