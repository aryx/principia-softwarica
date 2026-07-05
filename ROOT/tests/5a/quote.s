TEXT foo(SB), $0

        /*
         * A bare character constant without '$' is a plain con, which
         * original plan9 5a accepts here as an *absolute-address* memory
         * operand: 'MOVW 'A', R1' loads R1 from address 65 (con -> D_OREG
         * with offset and no base). This lineage dropped that bare-con
         * absolute-addressing form from the gen rule (it kept only the
         * based form 'con(pointer)'), because unbased absolute addressing
         * is an odd, essentially unused plan9 idiom that just confuses the
         * common '$imm vs mem' distinction. Use '$'A'' (below) for the
         * immediate. Kept commented as a reminder of the dropped form.
         */
        /* MOVW 'A', R1 */
        MOVW $'A', R1
        MOVW $''', R2
        MOVW $'\'', R2
