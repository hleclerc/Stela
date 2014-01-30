import sys, string

hash_table = [ 83,263,151,199,619,311,347,19,601,43,239,523,463,599,223,29 ]

Needlarg    = 1
Needrarg    = 2
Need2args   = 3
MayNeedlarg = 4;
MayNeedrarg = 8;
Needrrargs  = 16

#
class LexemOperator:
    def __init__( self, a, b, c = 0, d = -1 ):
        self.met_name = a
        self.cpp_name = b
        self.behavior = c
        self.group    = d
        self.num      = -1
    def of_type( self, ty ):
        if len( self.met_name ) == 0:
            return False
        op = self.met_name[ 0 ] in string.letters
        return op ^ ( ty == "op" )
    def nb_args( self ):
        if ( self.behavior == Need2args ) or ( self.behavior == Needrrargs ):
            return 2
        return 1

#~  ~#
class Operators:
    def __init__( self ):
        O = LexemOperator

        self.lst = [
            O( ";"          , "comma_dot"                               ),

            #
            O( "label"      , "__label__"             , Needrrargs , 0  ),
            O( "import"     , "__import__"            , Needrarg   , 0  ),

            O( "try"        , "__try__"               , Needrarg   , 0  ),
            O( "catch"      , "__catch__"             , Needrrargs , 0  ),

            O( "transient"  , "__transient__"         , Needrarg   , 0  ),
            O( "virtual"    , "__virtual__"           , Needrarg   , 0  ),
            O( "inline"     , "__inline__"            , Needrarg   , 0  ),
            O( "static"     , "__static__"            , Needrarg   , 0  ),
            O( "property"   , "__property__"          , Needrarg   , 0  ),
            O( "const"      , "__const__"             , Needrarg   , 0  ),

            #
            O( "class"      , "__class__"             , Needrrargs , 1  ),
            O( "def"        , "__def__"               , Needrrargs , 1  ),

            O( "extends"    , "__extends__"           , Need2args  , 2  ),
            O( "pertinence" , "__pertinence__"        , Need2args  , 2  ),
            O( "when"       , "__when__"              , Need2args  , 2  ),

            #
            O( "else"       , "__else__"              , Need2args  , 4  ),

            #
            O( "while"      , "__while__"             , Needrrargs , 5  ),
            O( "for"        , "__for__"               , Needrrargs , 5  ),
            O( "if"         , "__if__"                , Needrrargs , 5  ),

            #
            O( "return"     , "__return__"            , Needrarg   ,  6 ),
            O( "return_n"   , "__return_n__"          , Needrrargs ,  6 ),
            O( "throw"      , "__throw__"             , Needrarg   ,  6 ),

            #
            O( "infon"      , "__infon__"             , Needrarg   ,  7 ),
            O( "info"       , "__info__"              , Needrarg   ,  7 ),

            # ------------------------------------------------------------
            # limit auto function call (without parenthesis)
            # ------------------------------------------------------------

            #
            O( "?"          , "__alternative__"       , Needrarg   ,  8 ),

            #
            O( ""           , "comma_in_par"          , Need2args  ,  9 ),
            O( ","          , "comma"                 , Need2args  ,  9 ),

            #
            O( "="          , "reassign"              , Need2args  , 10 ),
            O( ":="         , "assign"                , Need2args  , 10 ),
            O( "~="         , "assign_type"           , Need2args  , 10 ),
            O( "|="         , "self_or"               , Need2args  , 10 ),
            O( "^="         , "self_xor"              , Need2args  , 10 ),
            O( "&="         , "self_and"              , Need2args  , 10 ),
            O( "%="         , "self_mod"              , Need2args  , 10 ),
            O( "<<="        , "self_shift_left"       , Need2args  , 10 ),
            O( ">>="        , "self_shift_right"      , Need2args  , 10 ),
            O( "/="         , "self_div"              , Need2args  , 10 ),
            O( "*="         , "self_mul"              , Need2args  , 10 ),
            O( "-="         , "self_sub"              , Need2args  , 10 ),
            O( "+="         , "self_add"              , Need2args  , 10 ),
            O( "//="        , "self_div_int"          , Need2args  , 10 ),
            O( "+++="       , "self_concatenate"      , Need2args  , 10 ),

            #
            O( "=>"         , "lambda"                , Need2args  , 11 ),
            O( "ref"        , "__ref__"               , Needrarg   , 11 ),
            O( "new"        , "__new__"               , Needrarg   , 11 ),

            #
            O( "<<<<"       , "shift_left_long_str"   , Need2args  , 12 ),
            O( "<<<"        , "shift_left_then_endl"  , Need2args  , 12 ),
            O( "<<"         , "shift_left"            , Need2args  , 12 ),
            O( ">>"         , "shift_right"           , Need2args  , 12 ),


            #
            O( "or"         , "or_boolean"            , Need2args  , 14 ),
            O( "and"        , "and_boolean"           , Need2args  , 14 ),
            O( "xor"        , "xor_boolean"           , Need2args  , 14 ),

            #
            O( "not"        , "not_boolean"           , Needrarg   , 15 ),

            #
            O( "inst_of"    , "__inst_of__"           , Need2args  , 16 ),
            O( "in"         , "__in__"                , Need2args  , 16 ),
            O( "not_inst_of", "__not_inst_of__"       , Need2args  , 16 ),
            O( "not_in"     , "__not_in__"            , Need2args  , 16 ),

            #
            O( "!="         , "not_equal"             , Need2args  , 17 ),
            O( "=="         , "equal"                 , Need2args  , 17 ),
            O( ">="         , "superior_equal"        , Need2args  , 17 ),
            O( "<="         , "inferior_equal"        , Need2args  , 17 ),
            O( ">"          , "superior"              , Need2args  , 17 ),
            O( "<"          , "inferior"              , Need2args  , 17 ),

            O( "+++"        , "concatenate"           , Need2args  , 18 ),

            O( ".."         , "range"                 , Need2args  , 19 ),
            O( "..."        , "triple_dots"           , Needlarg   , 19 ),

            O( "+"          , "add"                   , Need2args  , 20 ),
            O( "-"          , "sub"                   , MayNeedlarg+Needrarg, 20 ),
            O( ""           , "neg"                   , Needrarg   , 20 ),

            O( "%"          , "mod"                   , Need2args  , 21 ),
            O( "*"          , "mul"                   , Need2args  , 21 ),
            O( ":"          , "doubledot"             , Need2args  , 21 ),
            O( ":."         , "doubledotdot"          , Need2args  , 21 ),
            O( "/"          , "div"                   , Need2args  , 21 ),
            O( "//"         , "div_int"               , Need2args  , 21 ),
            O( "\\"         , "ml_div"                , Need2args  , 21 ),

            O( "^"          , "pow"                   , Need2args  , 22 ),

            O( "~"          , "not_bitwise"           , Needrarg   , 23 ),

            O( "'"          , "trans"                 , Needlarg   , 24 ),

            O( "--"         , "pre_dec"               , Needrarg   , 25 ),
            O( "++"         , "pre_inc"               , Needrarg   , 25 ),
            O( ""           , "post_dec"              , Needlarg   , 25 ),
            O( ""           , "post_inc"              , Needlarg   , 25 ),

            O( "&"          , "pointer_on"            , Needrarg   , 26 ),
            O( "@"          , "pointed_value"         , Needrarg   , 26 ),
            O( "$"          , "calc_name"             , Needrarg   , 26 ),

            O( "::"         , "doubledoubledot"       , Need2args  , 27 ),
            O( "->"         , "get_attr_ptr"          , Need2args  , 27 ),
            O( "."          , "get_attr"              , Need2args  , 27 ),
            O( ".?"         , "get_attr_ask"          , Need2args  , 27 ),
            O( "->?"        , "get_attr_ptr_ask"      , Need2args  , 27 ),

            O( ""           , "__string_assembly__"   , Need2args  , 28 ),
            O( ""           , "tensorial_product"     , Need2args  , 29 ),
            
        ]
        
        n = 0
        for l in self.lst:
            l.num = n
            n += 1

def hash_string( s ):
    res = 0
    cpt = 0
    for i in s:
        res += ord( i ) * hash_table[ cpt % len( hash_table ) ]
        cpt += 1
    return res

# return True if hash(operators) % n appears several times
def hash_collision( n, operators, ty ):
    l = []
    for o in operators.lst:
        if o.of_type( ty ):
            h = hash_string( o.met_name ) % n
            if h in l:
                return True
            l.append( h )
    return False

def make_num_operator( h_file, c_file, operators, ty ):
    n = 1
    while hash_collision( n, operators, ty ):
        n += 1
    #
    lst = [ "-1" for i in range( n ) ]
    cpt = 0
    for o in operators.lst:
        if o.of_type( ty ):
            lst[ hash_string( o.met_name ) % n ] = str( cpt )
        cpt += 1
    #
    print >> h_file, "int num_operator_" + ty + "( const char *beg, int len );"
    print >> c_file, "int num_operator_" + ty + "( const char *beg, int len ) {"
    print >> c_file, "    static const int tab[] = { " + string.join( lst, "," ) + " };"
    print >> c_file, "    int n = tab[ nstring_hash( beg, len ) % " + str( n ) + " ];"
    print >> c_file, "    if ( n >= 0 and strncmp( beg, operators_met_name[ n ], len ) == 0 )"
    print >> c_file, "        return n;"
    print >> c_file, "    return -1;"
    print >> c_file, "}"


def make_usual_string_info( h_file, c_file, usual_strings ):
    print >> c_file, "// generated by generation.py"
    print >> h_file, "// generated by generation.py"
    print >> h_file, "#define NB_USUAL_STRINGS ", len( usual_strings )
    
    cpt = 0
    for o in usual_strings:
        print >> h_file, "#define STRING_" + o + "_NUM " + str( cpt )
        cpt += 1
        
    # str usual
    print >> h_file, "extern const char *usual_strings_str[];"
    print >> c_file, "const char *usual_strings_str[] = {"
    for o in usual_strings: print >> c_file, '    "' + o + '",'
    print >> c_file, "};"
    
    print >> h_file, "extern int usual_strings_len[];"
    print >> c_file, "int usual_strings_len[] = {"
    for o in usual_strings: print >> c_file, '    ' + str( len( o ) ) + ','
    print >> c_file, "};"
    
    print >> h_file, "extern int usual_strings_val[];"
    print >> c_file, "int usual_strings_val[] = {"
    for o in usual_strings: print >> c_file, '    ' + str( hash_string( o ) ) + ','
    print >> c_file, "};"


def make_usual_strings():
    h_file = file( "src/Stela/System/UsualStrings.h"  , "w" )
    c_file = file( "src/Stela/System/UsualStrings.cpp", "w" )
    print >> c_file, "// generated by generation.py"
    print >> c_file, "#include \"UsualStrings.h\""
    print >> c_file, "#include \"NstringHash.h\""
    print >> c_file, "#include <string.h>"

    # usual_strings preparation
    usual_strings = []
    for o in operators.lst:
        usual_strings.append( o.cpp_name )

    print >> h_file, "// generated by bin/generation.py"
    print >> h_file, "#ifndef USUAL_STRINGS"
    print >> h_file, "#define USUAL_STRINGS"
    print >> h_file, "#define NB_PRIMITIVES", len( operations ) + len( base_primitives )
    print >> h_file, "#define FIRST_PRIM_ID", len( usual_strings )
    for s in base_primitives + operations:
        if not ( s in usual_strings ):
            usual_strings.append( "___" + s )
    # print >> h_file, "inline int get_primitive_id( int num_nstring ) { return num_nstring * ( num_nstring >= " + str( len( operators.lst ) ) + " and num_nstring < " + str( len( usual_strings ) ) + " ); }"

    for s in std_strings:
        if not ( s in usual_strings ):
            usual_strings.append( s )

    # operators_met_name and cpp_name
    def cm( met_name ):
        if met_name == "\\":
            return "\\\\"
        return met_name
    print >> h_file, "#define NB_OPERATORS", len( operators.lst )
    print >> h_file, "const char *get_operators_met_name( int n );"
    print >> h_file, "const char *get_operators_cpp_name( int n );"
    print >> h_file, "int         get_operators_behavior( int n );"

    print >> c_file, "static const char *operators_met_name[] = { " + string.join( [ '"' + cm( o.met_name ) + '"' for o in operators.lst ], "," ) + " };"
    print >> c_file, "static const char *operators_cpp_name[] = { " + string.join( [ '"' + cm( o.cpp_name ) + '"' for o in operators.lst ], "," ) + " };"
    print >> c_file, "static int         operators_behavior[] = { " + string.join( [      str( o.behavior )       for o in operators.lst ], "," ) + " };"
    print >> c_file, "const char *get_operators_met_name( int n ) { return operators_met_name[ n ]; }"
    print >> c_file, "const char *get_operators_cpp_name( int n ) { return operators_cpp_name[ n ]; }"
    print >> c_file, "int         get_operators_behavior( int n ) { return operators_behavior[ n ]; }"

    # 
    make_num_operator( h_file, c_file, operators, "le" )
    make_num_operator( h_file, c_file, operators, "op" )
    
    #
    make_usual_string_info( h_file, c_file, usual_strings )

    # nb args needed
    print >> h_file, "extern int usual_strings_nb_args[];"
    print >> c_file, "int usual_strings_nb_args[] = {"
    for o in operators.lst: print >> c_file, '    ' + str( o.nb_args() ) + ','
    print >> c_file, "};"
    
    print >> h_file, "#endif // USUAL_STRINGS"
    
def make_hash_string():
    h_file = file( "src/Stela/System/NstringHash.h", "w" )
    print >> h_file, "// generated by bin/generation.py"
    print >> h_file, "#ifndef NSTRINGHASH_H"
    print >> h_file, "#define NSTRINGHASH_H"
    print >> h_file, ""
    print >> h_file, "inline unsigned nstring_hash( const char *beg, int len ) {"
    print >> h_file, "    static const unsigned m[] = { " + string.join( [ str( x ) for x in hash_table ], "," ) + " };"
    print >> h_file, "    "
    print >> h_file, "    unsigned res = 0;"
    print >> h_file, "    for( int i = 0; i < len; ++i )"
    print >> h_file, "        res += beg[ i ] * m[ i % " + str( len( hash_table ) ) + " ];"
    print >> h_file, "    return res;"
    print >> h_file, "}"
    print >> h_file, ""
    print >> h_file, "#endif // NSTRINGHASH_H"

def make_ir_data():
    h_file = file( "src/Stela/Ir/Decl_Base.h", "w" )
    print >> h_file, "// generated by bin/generation.py"
    for x in base_token_types:
        print >> h_file, "DECL_IR_TOK( " + x + " )"
        
    h_file = file( "src/Stela/Ir/Decl_UnaryOperations.h", "w" )
    print >> h_file, "// generated by bin/generation.py"
    for x in unary_operations:
        print >> h_file, "DECL_IR_TOK( " + x + " )"
        
    h_file = file( "src/Stela/Ir/Decl_BinaryOperations.h", "w" )
    print >> h_file, "// generated by bin/generation.py"
    for x in binary_operations:
        print >> h_file, "DECL_IR_TOK( " + x + " )"
        
    h_file = file( "src/Stela/Ir/Decl_Operations.h", "w" )
    print >> h_file, "// generated by bin/generation.py"
    print >> h_file, "#include \"Decl_UnaryOperations.h\""
    print >> h_file, "#include \"Decl_BinaryOperations.h\""

    h_file = file( "src/Stela/Ir/Decl_Primitives.h", "w" )
    print >> h_file, "// generated by bin/generation.py"
    for x in base_primitives:
        print >> h_file, "DECL_IR_TOK( " + x + " )"

    h_file = file( "src/Stela/Ir/Decl.h", "w" )
    print >> h_file, "// generated by bin/generation.py"
    print >> h_file, "#include \"Decl_Base.h\""
    print >> h_file, "#include \"Decl_Primitives.h\""
    print >> h_file, "#include \"Decl_UnaryOperations.h\""
    print >> h_file, "#include \"Decl_BinaryOperations.h\""

    # numberq
    h_file = file( "src/Stela/Ir/Numbers.h", "w" )
    print >> h_file, "// generated by bin/generation.py"
    print >> h_file, "#include \"../System/TypeConfig.h\""
    cpt = 0
    for x in base_token_types + base_primitives + operations:
        print >> h_file, "#define IR_TOK_" + x + "  PI8( " + str( cpt ) + " )"
        cpt += 1

def make_primitive_inc():
    pass
    #h_file = file( "src/Stela/VirtualMachines/VirtualMachine/Defines/Primitives.h", "w" )
    #print >> h_file, "// generated by bin/generation.py"
    #for x in base_primitives:
        #print >> h_file, "DECL_PRIM(", x, ")"
    #print >> h_file, '#define DECL_OP( X ) DECL_PRIM( X )'
    #print >> h_file, '#include "../../../Operations/Defines/All.h"'
    #print >> h_file, '#undef DECL_OP'

    #u_file = file( "src/Stela/Operations/Defines/Unary.h", "w" )
    #print >> u_file, "// generated by bin/generation.py"
    #for x in unary_operations:
        #print >> u_file, "DECL_OP(", x, ")"

    ##print >> u_file, "#define DECL_BT( TT ) DECL_OP( Conv_##TT )"
    ##print >> u_file, "#include \"../../PrimTypes/Defines/Arithmetic.h\""
    ##print >> u_file, "#undef DECL_BT"

    #u_file = file( "src/Stela/Operations/Defines/Binary.h", "w" )
    #print >> u_file, "// generated by bin/generation.py"
    #for x in binary_operations:
        #print >> u_file, "DECL_OP(", x, ")"

# ---------------------------------------------------------------------------
operators = Operators()

unary_operations = string.split( """
    Not
    Log
    Ceil
    Conv
""" )

binary_operations = string.split( """
    Or
    And

    Add
    Sub
    Mul
    Div
    Pow
    Mod

    Equ
    Neq
    Sup
    Inf
    SupEq
    InfEq
""" )

operations = unary_operations + binary_operations

base_primitives = string.split( """
    info
    rand
    syscall
    set_base_size_and_alig
    set_RawRef_dependancy
    reassign_rec
    set_ptr_val
    select_SurdefList
    ptr_size
    ptr_alig
    size_of
    alig_of
    typeof
    address
    pointed_value
    block_exec
    get_argc
    get_argv
    apply_LambdaFunc
    inst_of
""" ) #

std_strings = string.split( """
    break continue abstract
    SourceFile Callable Class Def ClassInst DefInst SurdefList
    GetSetSopDef GetSetSopInst Ptr Block getr_ptr self_ptr
    Bool Void Error FreeSlot self this
    SI8 SI16 SI32 SI64 PI8 PI16 PI32 PI64 FP32 FP64 FP80
    PermanentString true false select chabeha
    varargs init delete Vec Nstring UntypedPtr Expr
    BegItemDataRefArray
    EndItemDataRefArray
    BegItemSizeRefArray
    EndItemSizeRefArray
    RefArray
    BegVarargsItem
    EndVarargsItem
    BegSurdefsItem
    EndSurdefsItem
    Type
    apply
    super
    super_0 super_1 super_2 super_3
    super_4 super_5 super_6 super_7
    destroy
    LambdaFunc
    get_size
    name
    ext_refs
    RawPtr
    RawRef
    BaseInt
    null_ref
""" )

base_token_types = string.split( """
    BLOCK
    DEF
    CLASS
    RETURN
    APPLY
    SELECT
    CHBEBA
    SI32
    PI32
    SI64
    PI64
    PTR
    STRING
    VAR
    ASSIGN
    REASSIGN
    GET_ATTR
    GET_ATTR_PTR
    GET_ATTR_ASK
    GET_ATTR_PTR_ASK
    GET_ATTR_PA
    IF
    WHILE
    BREAK
    CONTINUE
    FALSE
    TRUE
    VOID
    SELF
    THIS
    FOR
    IMPORT
    NEW
    LIST
    LAMBDA
    NULL_REF
""" )

make_hash_string()
make_usual_strings()
make_ir_data()
make_primitive_inc()




