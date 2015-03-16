// name, C++ op, is_oper, return bool, prec
DECL_OP( or_boolean , or , 1, 1, 14 )
DECL_OP( and_boolean, and, 1, 1, 15 )
DECL_OP( add        , +  , 1, 0, 20 )
DECL_OP( sub        , -  , 1, 0, 20 )
DECL_OP( mul        , *  , 1, 0, 21 )
DECL_OP( div        , /  , 1, 0, 21 )

DECL_OP( equ        , == , 1, 1, 17 )
DECL_OP( neq        , != , 1, 1, 17 )
DECL_OP( inf        , <  , 1, 1, 17 )
DECL_OP( sup        , >  , 1, 1, 17 )
DECL_OP( sup_eq     , <= , 1, 1, 17 )
DECL_OP( inf_eq     , >= , 1, 1, 17 )
