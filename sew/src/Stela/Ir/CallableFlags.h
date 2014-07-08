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
    IR_IS_A_SOP = 512,

    // for CatchedVar::type
    IN_CATCHED_VARS = 0,
    IN_STATIC_SCOPE = 1,
    IN_LOCAL_SCOPE  = 2,
    IN_SELF         = 3,
    IN_STATIC_ATTR  = 4,
    IN_LOCAL_ATTR   = 5,

    // for Callable::Attribute
    CALLABLE_ATTR_TYPE      = 1,
    CALLABLE_ATTR_STATIC    = 2,
    CALLABLE_ATTR_BASE_SIZE = 4,
    CALLABLE_ATTR_BASE_ALIG = 8
};

#endif // Stela_Ir_CallableFlags_H
