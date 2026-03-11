#include <xc.inc>
    psect	asm_data,global,class=BANK0,space=1,delta=1,noexec

btemp: ds 1;

 psect   myfunc,local,class=CODE,delta=2 ; PIC10/12/16
; psect   portfunc,local,class=CODE,reloc=2 ; PIC18
    global _setPin
     //set proc
_setPin:
   // stack_auto x,1
    //alloc_stack
    movwf btemp
    movlw 0x0C
    btfsS btemp,3 
    goto skip_set  
    movlw 0x0E
skip_set:
    movwf FSR1L
    clrf FSR1H
    lslf btemp,W
    andlw 0x0F
    //restore_stack
    //BANKSEL (PORTA)
    BRW
    bsf INDF1,0
    return
    bsf INDF1,1
    return
    bsf INDF1,2
    return
    bsf INDF1,3
    return
    bsf INDF1,4
    return
    bsf INDF1,5
    return
    bsf INDF1,6
    return
    bsf INDF1,7
    return
    
       //clr proc
    global _clrPin
_clrPin:
    nop
    //stack_auto x,1
    //alloc_stack
    movwf btemp
    movlw 0x0C
    btfsS btemp,3 
    goto skip_clr  
    movlw 0x0E
skip_clr:
    movwf FSR1L
    clrf FSR1H
    lslf btemp,W
    andlw 0x0F
    //restore_stack
    //BANKSEL (PORTA)
    BRW
    bcf INDF1,0
    return
    bcf INDF1,1
    return
    bcf INDF1,2
    return
    bcf INDF1,3
    return
    bcf INDF1,4
    return
    bcf INDF1,5
    return
    bcf INDF1,6
    return
    bcf INDF1,7
    return
   
    global _getVal
_getVal:
    nop
    //stack_auto x,1
    //alloc_stack
    movwf btemp
    movlw 0x0C
    btfsS btemp,3 
    goto skip_get  
    movlw 0x0E
skip_get:
    movwf FSR1L
    clrf FSR1H
    lslf btemp,F
    lslf btemp,W
    andlw 0x1F
    //restore_stack
    //BANKSEL (PORTA) 
    BRW
    nop
    btfss INDF1,0
    retlw 0
    retlw 1
    nop
    btfss INDF1,1
    retlw 0
    retlw 1 
    nop
    btfss INDF1,2
    retlw 0
    retlw 1 
    nop
    btfss INDF1,3
    retlw 0
    retlw 1
    nop
    btfss INDF1,4
    retlw 0
    retlw 1 
    nop
    btfss INDF1,5
    retlw 0
    retlw 1
    nop
    btfss INDF1,6
    retlw 0
    retlw 1
    nop
    btfss INDF1,7
    retlw 0
    retlw 1
    
   //clrTris proc
    global _clrTris
_clrTris:
    nop
    //stack_auto x,1
    //alloc_stack
    movwf btemp
    movlw 0x8C
    btfsS btemp,3 
    goto skip_tris  
    movlw 0x8E
skip_tris:
    movwf FSR1L
    clrf FSR1H
    lslf btemp,W
    andlw 0x0F
    //restore_stack
    //BANKSEL (PORTA)
    BRW
    bcf INDF1,0
    return
    bcf INDF1,1
    return
    bcf INDF1,2
    return
    bcf INDF1,3
    return
    bcf INDF1,4
    return
    bcf INDF1,5
    return
    bcf INDF1,6
    return
    bcf INDF1,7
    return
    //num_pos proc
    global _num_pos
_num_pos:
    nop
    andlw 0x07
    BRW
    retlw 1
    retlw 2
    retlw 4
    retlw 8
    retlw 16
    retlw 32
    retlw 64
    retlw 128
    return