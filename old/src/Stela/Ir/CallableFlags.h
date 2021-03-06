#ifndef Stela_Ir_CallableFlags_H
#define Stela_Ir_CallableFlags_H

enum CallableFlags {
    IR_SELF_AS_ARG = 1,
    IR_VARARGS = 2,
    IR_HAS_CONDITION = 4,
    IR_HAS_RETURN_TYPE = 8,
    IR_HAS_COMPUTED_PERT = 16,
    IR_ABSTRACT = 32,
    IR_NEG_PERT = 64,
    IR_IS_A_GET = 128,
    IR_IS_A_SET = 256,
    IR_IS_A_SOP = 512
};

#endif // Stela_Ir_CallableFlags_H
