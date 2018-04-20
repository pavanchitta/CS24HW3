.globl rl_decode

#============================================================================
# rl_decode:  decode RLE-encoded input into a malloc'd buffer
#
# Author:  Ben Bitdiddle (those guys are lucky I said I'd do this before
#          going on my vacation. They never appreciate my work ethic!)
#
# Arguments to rl_decode are in these registers (following the System V AMD64
# ABI calling convention):
#
#      %rdi = pointer to buffer containing run-length encoded input data
#
#      %esi = int length of the run-length-encoded data in the buffer
#
#      %rdx = OUTPUT pointer to where the length of the decoded result
#             should be stored
#
# Return-value in %rax is the pointer to the malloc'd buffer containing
# the decoded data.
#
rl_decode:
        # No need for a frame pointer - we don't need a stack frame.

        # Save rbx since it is callee-save, and we definitely use it a lot.
        push    %rbx

        # First, figure out how much space is required to decode the data.
        # We do this by summing up the counts, which are in the even memory
        # locations.

        mov     $0, %ecx                  # %ecx = loop variable (int)
        mov     $0, %ebx                  # %ebx = size required (also int)
        mov     $0, %r11                  # Set %rll to 0 so we can use %r11d

        # Find-space while-loop starts here...
        cmp     %esi, %ecx
        jge     find_space_done

find_space_loop:

        mov     (%rdi, %rcx), %r8b         # First move count into %r8b
        movzx   %r8b, %r11d                # Zero-extend %r8b into %r11d
        add     %r11d, %ebx                # Add %r11d to %ebx, the total count
        add     $2, %rcx                   # forward to the next count, by
                                           # incrementing by 2!

        cmp     %esi, %ecx
        jl      find_space_loop

find_space_done:

        # Allocate memory for the decoded data using malloc.
        # Number of bytes required goes in rdi, which we were using before.
        # Pointer to allocated memory will be returned in %rax.
        # We also have to save %rsi, and %rdx, since these are caller save.
        push    %rdi
        push    %rsi
        push    %rdx
        mov     %rbx, %rdi         # Number of bytes to allocate...
        call    malloc
        pop     %rdx
        pop     %rsi
        pop     %rdi


        # Write the length of the decoded output to the output-variable
        mov     %ebx, (%rdx)      # store computed size into this location

        # Now, decode the data from the input buffer into the output buffer.
        mov     $0, %ecx          # Loop variable again (int)
        xor     %r10, %r10        # Index in output buffer


        # First comparison of decode while-loop here...
        cmp     %esi, %ecx
        jge     decode_done

decode_loop:
        # Pull out the next [count][value] pair from the encoded data.
        mov     (%rdi, %rcx), %bh        # bh is the count of repetitions
        mov     1(%rdi, %rcx), %bl       # bl is the value to repeat

write_loop:
        mov     %bl, (%rax, %r10)
        inc     %r10                     # Need to increment %r10 to next index
        dec     %bh
        jnz     write_loop

        add     $2, %ecx

        cmp     %esi, %ecx
        jl      decode_loop

decode_done:
        # Restore callee-save registers.
        pop     %rbx

        # No stack frame to clean up.
        ret
