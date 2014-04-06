// generated by generation.py
#include "UsualStrings.h"
#include "NstringHash.h"
#include <string.h>
static const char *operators_met_name[] = { ";","label","import","try","catch","transient","virtual","inline","static","property","const","class","def","extends","pertinence","when","else","while","for","if","return","return_n","throw","infon","info","?","",",","=",":=","~=","|=","^=","&=","%=","<<=",">>=","/=","*=","-=","+=","//=","+++=","=>","ref","new","<<<<","<<<","<<",">>","or","and","xor","not","inst_of","in","not_inst_of","not_in","!=","==",">=","<=",">","<","+++","..","...","+","-","","%","*",":",":.","/","//","\\","^","~","'","--","++","","","&","@","$","::","->",".",".?","->?","","" };
static const char *operators_cpp_name[] = { "comma_dot","__label__","__import__","__try__","__catch__","__transient__","__virtual__","__inline__","__static__","__property__","__const__","__class__","__def__","__extends__","__pertinence__","__when__","__else__","__while__","__for__","__if__","__return__","__return_n__","__throw__","__infon__","__info__","__alternative__","comma_in_par","comma","reassign","assign","assign_type","self_or","self_xor","self_and","self_mod","self_shift_left","self_shift_right","self_div","self_mul","self_sub","self_add","self_div_int","self_concatenate","lambda","__ref__","__new__","shift_left_long_str","shift_left_then_endl","shift_left","shift_right","or_boolean","and_boolean","xor_boolean","not_boolean","__inst_of__","__in__","__not_inst_of__","__not_in__","not_equal","equal","superior_equal","inferior_equal","superior","inferior","concatenate","range","triple_dots","add","sub","neg","mod","mul","doubledot","doubledotdot","div","div_int","ml_div","pow","not_bitwise","trans","pre_dec","pre_inc","post_dec","post_inc","pointer_on","pointed_value","calc_name","doubledoubledot","get_attr_ptr","get_attr","get_attr_ask","get_attr_ptr_ask","__string_assembly__","tensorial_product" };
static int         operators_behavior[] = { 0,16,2,2,16,2,2,2,2,2,2,16,16,3,3,3,3,16,16,16,2,16,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,3,3,3,3,3,3,3,2,3,3,3,3,3,3,3,3,3,3,3,3,1,3,6,2,3,3,3,3,3,3,3,3,2,1,2,2,1,1,2,2,2,3,3,3,3,3,3,3 };
const char *get_operators_met_name( int n ) { return operators_met_name[ n ]; }
const char *get_operators_cpp_name( int n ) { return operators_cpp_name[ n ]; }
int         get_operators_behavior( int n ) { return operators_behavior[ n ]; }
int num_operator_le( const char *beg, int len ) {
    static const int tab[] = { -1,51,-1,-1,-1,-1,54,17,-1,-1,-1,-1,-1,55,-1,-1,-1,-1,11,44,-1,-1,-1,-1,8,-1,-1,-1,-1,10,-1,-1,-1,12,-1,-1,-1,-1,-1,-1,24,-1,-1,-1,-1,6,-1,-1,-1,-1,-1,50,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,53,5,-1,-1,-1,-1,57,45,-1,56,2,-1,-1,-1,-1,-1,-1,-1,4,-1,-1,-1,-1,-1,52,22,3,23,-1,-1,19,-1,-1,-1,-1,-1,-1,-1,13,-1,21,-1,18,-1,-1,-1,-1,-1,-1,-1,-1,15,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,-1,-1,16,-1,-1,-1,-1 };
    int n = tab[ nstring_hash( beg, len ) % 168 ];
    if ( n >= 0 and strncmp( beg, operators_met_name[ n ], len ) == 0 )
        return n;
    return -1;
}
int num_operator_op( const char *beg, int len ) {
    static const int tab[] = { -1,-1,48,60,-1,-1,73,41,-1,-1,62,35,-1,31,27,-1,-1,-1,-1,-1,-1,-1,91,-1,-1,-1,-1,79,-1,-1,46,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,37,-1,-1,-1,-1,-1,-1,74,-1,61,49,-1,-1,38,-1,90,63,-1,-1,36,71,-1,-1,-1,-1,-1,34,-1,-1,-1,-1,-1,47,70,-1,-1,-1,-1,65,-1,-1,-1,-1,-1,-1,-1,-1,-1,39,32,-1,25,-1,-1,-1,68,77,29,-1,-1,-1,-1,-1,-1,72,-1,-1,-1,-1,-1,81,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,42,-1,-1,-1,-1,-1,-1,-1,59,-1,-1,-1,40,88,-1,28,-1,-1,-1,67,76,-1,-1,-1,-1,33,-1,-1,-1,-1,-1,-1,84,-1,-1,-1,80,-1,58,-1,87,-1,-1,-1,-1,-1,-1,-1,-1,-1,85,-1,66,30,89,-1,-1,43,-1,64,78,-1,-1,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,86,-1,-1,-1,-1,-1,75,-1 };
    int n = tab[ nstring_hash( beg, len ) % 214 ];
    if ( n >= 0 and strncmp( beg, operators_met_name[ n ], len ) == 0 )
        return n;
    return -1;
}
// generated by generation.py
const char *usual_strings_str[] = {
    "comma_dot",
    "__label__",
    "__import__",
    "__try__",
    "__catch__",
    "__transient__",
    "__virtual__",
    "__inline__",
    "__static__",
    "__property__",
    "__const__",
    "__class__",
    "__def__",
    "__extends__",
    "__pertinence__",
    "__when__",
    "__else__",
    "__while__",
    "__for__",
    "__if__",
    "__return__",
    "__return_n__",
    "__throw__",
    "__infon__",
    "__info__",
    "__alternative__",
    "comma_in_par",
    "comma",
    "reassign",
    "assign",
    "assign_type",
    "self_or",
    "self_xor",
    "self_and",
    "self_mod",
    "self_shift_left",
    "self_shift_right",
    "self_div",
    "self_mul",
    "self_sub",
    "self_add",
    "self_div_int",
    "self_concatenate",
    "lambda",
    "__ref__",
    "__new__",
    "shift_left_long_str",
    "shift_left_then_endl",
    "shift_left",
    "shift_right",
    "or_boolean",
    "and_boolean",
    "xor_boolean",
    "not_boolean",
    "__inst_of__",
    "__in__",
    "__not_inst_of__",
    "__not_in__",
    "not_equal",
    "equal",
    "superior_equal",
    "inferior_equal",
    "superior",
    "inferior",
    "concatenate",
    "range",
    "triple_dots",
    "add",
    "sub",
    "neg",
    "mod",
    "mul",
    "doubledot",
    "doubledotdot",
    "div",
    "div_int",
    "ml_div",
    "pow",
    "not_bitwise",
    "trans",
    "pre_dec",
    "pre_inc",
    "post_dec",
    "post_inc",
    "pointer_on",
    "pointed_value",
    "calc_name",
    "doubledoubledot",
    "get_attr_ptr",
    "get_attr",
    "get_attr_ask",
    "get_attr_ptr_ask",
    "__string_assembly__",
    "tensorial_product",
    "___info",
    "___rand",
    "___syscall",
    "___set_base_size_and_alig",
    "___set_RawRef_dependancy",
    "___reassign_rec",
    "___set_ptr_val",
    "___select_SurdefList",
    "___ptr_size",
    "___ptr_alig",
    "___size_of",
    "___alig_of",
    "___typeof",
    "___address",
    "___block_exec",
    "___get_argc",
    "___get_argv",
    "___apply_LambdaFunc",
    "___inst_of",
    "___not",
    "___log",
    "___ceil",
    "___or",
    "___and",
    "___equ",
    "___neq",
    "___sup",
    "___inf",
    "___sup_eq",
    "___inf_eq",
    "break",
    "continue",
    "abstract",
    "SourceFile",
    "Callable",
    "Class",
    "Def",
    "ClassInst",
    "DefInst",
    "SurdefList",
    "GetSetSopDef",
    "GetSetSopInst",
    "Ptr",
    "Block",
    "getr_ptr",
    "self_ptr",
    "Bool",
    "Void",
    "Error",
    "FreeSlot",
    "self",
    "this",
    "SI8",
    "SI16",
    "SI32",
    "SI64",
    "PI8",
    "PI16",
    "PI32",
    "PI64",
    "FP32",
    "FP64",
    "FP80",
    "PermanentString",
    "true",
    "false",
    "select",
    "chabeha",
    "varargs",
    "init",
    "delete",
    "Vec",
    "Nstring",
    "UntypedPtr",
    "Expr",
    "BegItemDataRefArray",
    "EndItemDataRefArray",
    "BegItemSizeRefArray",
    "EndItemSizeRefArray",
    "RefArray",
    "BegVarargsItem",
    "EndVarargsItem",
    "BegSurdefsItem",
    "EndSurdefsItem",
    "Type",
    "apply",
    "super",
    "super_0",
    "super_1",
    "super_2",
    "super_3",
    "super_4",
    "super_5",
    "super_6",
    "super_7",
    "destroy",
    "LambdaFunc",
    "get_size",
    "name",
    "ext_refs",
    "RawPtr",
    "RawRef",
    "BaseInt",
    "null_ref",
};
int usual_strings_len[] = {
    9,
    9,
    10,
    7,
    9,
    13,
    11,
    10,
    10,
    12,
    9,
    9,
    7,
    11,
    14,
    8,
    8,
    9,
    7,
    6,
    10,
    12,
    9,
    9,
    8,
    15,
    12,
    5,
    8,
    6,
    11,
    7,
    8,
    8,
    8,
    15,
    16,
    8,
    8,
    8,
    8,
    12,
    16,
    6,
    7,
    7,
    19,
    20,
    10,
    11,
    10,
    11,
    11,
    11,
    11,
    6,
    15,
    10,
    9,
    5,
    14,
    14,
    8,
    8,
    11,
    5,
    11,
    3,
    3,
    3,
    3,
    3,
    9,
    12,
    3,
    7,
    6,
    3,
    11,
    5,
    7,
    7,
    8,
    8,
    10,
    13,
    9,
    15,
    12,
    8,
    12,
    16,
    19,
    17,
    7,
    7,
    10,
    25,
    24,
    15,
    14,
    20,
    11,
    11,
    10,
    10,
    9,
    10,
    13,
    11,
    11,
    19,
    10,
    6,
    6,
    7,
    5,
    6,
    6,
    6,
    6,
    6,
    9,
    9,
    5,
    8,
    8,
    10,
    8,
    5,
    3,
    9,
    7,
    10,
    12,
    13,
    3,
    5,
    8,
    8,
    4,
    4,
    5,
    8,
    4,
    4,
    3,
    4,
    4,
    4,
    3,
    4,
    4,
    4,
    4,
    4,
    4,
    15,
    4,
    5,
    6,
    7,
    7,
    4,
    6,
    3,
    7,
    10,
    4,
    19,
    19,
    19,
    19,
    8,
    14,
    14,
    14,
    14,
    4,
    5,
    5,
    7,
    7,
    7,
    7,
    7,
    7,
    7,
    7,
    7,
    10,
    8,
    4,
    8,
    6,
    6,
    7,
    8,
};
int usual_strings_val[] = {
    271673,
    256930,
    277207,
    210481,
    264703,
    396050,
    312365,
    271391,
    268934,
    360531,
    272915,
    263924,
    193717,
    311191,
    457853,
    203034,
    206979,
    264065,
    203437,
    157373,
    281202,
    354237,
    276362,
    265344,
    203044,
    488819,
    348389,
    135603,
    215228,
    174523,
    311368,
    205598,
    209522,
    201756,
    205835,
    484575,
    490852,
    201296,
    208069,
    209745,
    198286,
    349864,
    484962,
    162503,
    195831,
    205750,
    559547,
    573298,
    280084,
    310051,
    278694,
    297900,
    302186,
    301658,
    301042,
    158965,
    491620,
    272623,
    279756,
    141924,
    480470,
    476289,
    221231,
    217050,
    294401,
    134599,
    310065,
    49451,
    55114,
    51246,
    53340,
    56126,
    279450,
    370947,
    53733,
    212095,
    173389,
    56458,
    302763,
    147332,
    201098,
    206992,
    205771,
    210449,
    292253,
    424809,
    259183,
    501557,
    359327,
    210070,
    354782,
    490163,
    551976,
    523393,
    206439,
    198854,
    286512,
    747115,
    671687,
    483544,
    472428,
    570499,
    304401,
    305609,
    280986,
    274668,
    278488,
    275055,
    393254,
    297719,
    302260,
    533531,
    285119,
    173890,
    169449,
    199566,
    139870,
    165708,
    173648,
    166767,
    177355,
    167922,
    280152,
    270719,
    138903,
    218827,
    211564,
    264663,
    198788,
    142682,
    47609,
    275456,
    206243,
    271485,
    343715,
    406588,
    54362,
    136577,
    211369,
    208769,
    72924,
    72086,
    145578,
    196828,
    72714,
    75720,
    34544,
    44233,
    43739,
    44590,
    34295,
    43984,
    43490,
    44341,
    44501,
    45352,
    44858,
    501603,
    77376,
    135689,
    169872,
    198240,
    214326,
    76584,
    174485,
    48650,
    211867,
    289157,
    76885,
    512711,
    514874,
    519018,
    521181,
    203684,
    453047,
    455210,
    465023,
    467186,
    75806,
    150810,
    147893,
    194094,
    194441,
    194788,
    195135,
    195482,
    195829,
    196176,
    196523,
    222386,
    256727,
    219626,
    71199,
    215920,
    173464,
    160845,
    188102,
    208945,
};
int usual_strings_nb_args[] = {
    1,
    2,
    1,
    1,
    2,
    1,
    1,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    1,
    2,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    1,
    1,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    1,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    1,
    2,
    1,
    1,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
};
