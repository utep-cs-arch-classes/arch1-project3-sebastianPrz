	
	.text
	.global switch_interrupt_handler

	.extern p2sw_read
	.extern buzzer_set_period
	.extern f1CarAdvance
	.extern SW1	;not using it
	.extern SW2	;not using it
	.extern SW3
	.extern SW4
	.extern ml0
	.extern fieldFence
	.extern movement

switch_interrupt_handler:
	sub #6, r1
	mov.b #0, 0(r1)		;init p2val
	call #p2sw_read
	mov.b r12, 0(r1)	;p2val

	mov.b #0, 1(r1)		;sw1_down / not using it
	mov.b #0, 2(r1)		;sw2_down / not using it
	mov.b #0, 3(r1)		;sw3_down
	mov.b #0, 4(r1)		;sw4_down
	
	and &SW3, 0(r1)		;sw3_down
	cmp #1, 0(r1)		;if it is not true or not 1
	jz switch3
	mov.b #1, 3(r1)		;it it was true or 1
	jmp sw3			;then do the sound

switch3:
	mov.b #0, 3(r1)		;if it was 0 then make sw3_down = 0

sw3:
	mov #800, r12		;this will be the sound
	call buzzer_set_period	;call the method using that parameter
	mov #-10, &movement	;make the movement be -10
	mov &ml0, r12		;put the layer as the parameter for f1CarAdvance
	mov &fieldFence, r13	;put fieldFence as parameter for f1CarAdvance
	call f1CarAdvance	;call f1CarAdvance
	
	and &SW4, 0(r1)		;sw4_down
	cmp #1, 0(r1)		;if and(&SW4 and p2val)
	jz switch4		;if it is false or 0
	mov.b #1, 4(r1)		;if it was true or not 0
	jmp sw4

switch4:
	mov.b #0, 4(r1)

sw4:
	mov #1000, r12		;mov as parameter for the sound
	call buzzer_set_period	;call method using that parameter
	mov #10, &movement	;make the movement positive 10
	mov &ml0, r12		;put the layer of the car as parameter
	mov &fieldFence, r13	;put fieldFence as parameter
	call f1CarAdvance	;call f1CarAdvance using those parameters

	add #6, r1		;give back the previously subtracted slots
	pop r0			;return
