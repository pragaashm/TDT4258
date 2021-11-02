// TDT4258 Low-Level Programming
//		Lab assignment 1
// Case-insensitive Palindrome Finder

.global _start

.section .text

_start:
	ldr r0, =input // load pointer
	
	ldr r9, =output1 // pointer to output1
	ldr r10, =output2 // pointer to output2
	
	mov r1,r0 // copy r0 to r1
	
	
check_input:
	ldrb r2,[r1],#1 // add r1 to r2, with an increment of 1
	
	cmp r2, #'a' // comparing element to the ASCII-value of 'a'
	bhs upper_string // moves to next branch if r2 is equals or higher than 'a'
	
	cmp r2, #0 // checks if it is empty
	bne check_input // loop to check_input if not
	
	sub r1,r1,#2 // decrement by 2
	bl check_palindrome // branch to check if palindrome when ready
	
upper_string:
	sub r2,r2,#32 // if element is lowercase, subtract by 32 to convert to uppercase
	strb r2,[r1,#-1] // return to memory
	
	b check_input // loop back
	
check_palindrome:
	ldrb r3,[r0] // loading byte r0 to r3
	ldrb r4,[r1] // loading byte r1 to r4
	
	cmp r3, #32 //checking if r3[] is blank space
	beq check1 // if equal, branch to check1
	
	cmp r4, #32 // checking if r4[] is blank space
	beq check2 // if equal, branch to check2
	
	cmp r3, r4 // comparing beginning and end of the string
	bne palindrome_not_found // if the first and last element !=, palindrome not detected
	
	cmp r0,r1 // comparing addresses
	beq palindrome_found // if same, palindrome detected
	
	add r2,r0,#1 // add r0 to r2 with an increment by 1
	cmp r2,r1 // comparing addresses
	beq palindrome_found // if ==, palindrome detected.
	
	//Moving towards the middle of the string
	add r0,r0,#1 // increasing counter
	sub r1,r1,#1 // decreasing counter
	b check_palindrome // looping back to the beginning

check1:
	cmp r0, r1 //checking if r0 < r1 to avoid skip
	bhs check2 //if higher or same, check if r1 > r0 in check2
	
	add r0,r0,#1 //else, increase counter
	b check_palindrome //return to check_palindrome

check2:
	cmp r1, r0 //checking if r1 > r0,
	bls palindrome_found //if lower or same as well, palindrome detected
	
	sub r1,r1,#1 //else, decrease counter
	b check_palindrome //return to check_palindrome
	
palindrome_found:
	// Switch on only the 5 rightmost LEDs
	ldr r0, =0xFF200000 //initialize LEDs with base address
	mov r1, #0x1f //copy the value of the 5 rightmost LEDs
	str r1,[r0] //write to LEDs
	
	// Write 'Palindrome detected' to UART
	ldrb r0, [r9] //load output1-byte to r0
	
	cmp r0,#0 //null-check
	beq exit //end if null
	
	bl jtag_on //send r0 to UART
	add r9,r9,#1 //increase counter
	
	b palindrome_found //loop back
	
palindrome_not_found:
	// Switch on only the 5 leftmost LEDs
	ldr r0, =0xFF200000 //initialize LEDs with base address
	mov r1, #0xfe0 //copy the value of the 5 leftmost LEDs
	str r1,[r0] //write to LEDs
	
	// Write 'Not a palindrome' to UART
	ldrb r0,[r10] //load output2-byte to r0
	
	cmp r0,#0 //null-check
	beq exit //end if null
	
	bl jtag_on //send r0 to UART
	add r10,r10,#1 //increase counter
	
	b palindrome_not_found //loop back
	
jtag_on:
	ldr r1, =0xFF201000 //initialize with base address
	ldr r2, [r1, #4] //read control register
	
	str r0,[r1] //write character
	bx lr //return to calling process
	
exit:
	// Branch here for exit
	b exit
	

.section .data
.align
	// Test strings for input
	
	input: .asciz "Was it a cat I saw"
	//input: .asciz "l E ve l mem l e VE lug gul E ve l mem l e VE l"
	//input: .asciz "level"
	//input: .asciz "82448"
    //input: .asciz "KayAk"
    //input: .asciz "step on no pets"
    //input: .asciz "Never odd or even"
	
	// Output strings
	output1: .string "Palindrome detected.\n"
	output2: .string "Not a palindrome.\n"


.end


