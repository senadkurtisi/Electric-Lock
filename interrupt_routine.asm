;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; @brief Interrupt routines
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

					.cdecls C,LIST,"msp430.h"       ; Include device header file

					.ref	pressed

; PORT2 ISR
					.text
P2ISR				mov.w	#0xfff,R9				; debounce time
debounce:			dec		R9						; debounce
					jnz		debounce
					bit.b 	#BIT4,&P2IFG			; check if P2.4 triggered isrr
					jz		isr_exit
					bit.b   #BIT4,&P2IN				; check if button is still pressed
           			jnz     isr_exit				; if it is not pressed, exit isr
					bis.b	#BIT0,pressed			; set the button pressed indicator
isr_exit:			bic.b	#BIT4,&P2IFG			; clear P2.4 interrupt flag
					reti

; Interrupt vector
					.sect	.int42
					.short	P2ISR

					.end
