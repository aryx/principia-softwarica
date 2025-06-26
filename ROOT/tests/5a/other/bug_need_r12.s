/*
When doing MOVW $hello(SB), R2 in _main below
the generated code could be either
   ADD $<offset_hello>, R12, R2
 or
   MOVW $<offset_pool>(R15), R2

depending on whether $<offset_hello> can be an "immrot", that is
if it can fit in the 12 bits reserved for immediate constant.
Those 12 bits are made of 8 bits for the constant and 4 bits
for a rotation. See http://alisdair.mcdiarmid.org/arm-immediate-value-encoding/

If the offset can be an "immrot", then the first case is used, but
this assumes R12 has already been set appropriately!

The code below, which assumes BIG is set to 0 in 5l, will make
offset_hello an immrot and so will cause a fault if
R12 has not been set before.
        
To do that we must have an extra global before hello so that
the address of hello is not equal to bdata. Indeed, in that
case 5l has a special case and use the second form anyway
(to bootstrap setR12). 
*/


TEXT _main(SB), $20
        MOVW $hello(SB), R2
        MOVW $'W', R1        
        MOVB R1, 6(R2)
loop:
        B loop

                      
/* fake will have hash which puts it after, so we use faker */
GLOBL   faker(SB), $64
DATA    faker+0(SB)/6, $"Faker "        
                        
GLOBL   hello(SB), $12
DATA    hello+0(SB)/6, $"Hello "
