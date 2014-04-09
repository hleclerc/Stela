#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "../System/SplittedVec.h"
#include "../System/ErrorList.h"
#include "../System/AutoPtr.h"
#include "../System/Stream.h"
#include "../System/Math.h"
#include "../System/Vec.h"
#include "../System/S.h"
#include "ClassInfo.h"
#include "DefInfo.h"
#include "NstrCor.h"
#include "SfInfo.h"
#include "Var.h"

class CallableInfo;
class BaseType;
class Scope;
class ToDel;
class Expr;

/**
  Gestion des types
    - proposition: on se base sur les expression Class et Type pour trouver CallInfo et TypeInfo


*/
class Interpreter {
public:
    Interpreter( ErrorList &error_list );
    ~Interpreter();

    void import( String filename );
    Vec<ConstPtr<Inst> > get_outputs(); ///< output instructions

    void add_inc_path( String path );

    // error messages
    ErrorList::Error &make_error( String msg, const Expr &sf = Expr(), int off = 0, Scope *sc = 0, bool warn = false );
    void              disp_error( String msg, const Expr &sf = Expr(), int off = 0, Scope *sc = 0, bool warn = false );

    // methods for sourcefiles
    bool              already_imported( String filename );

    int               glo_nstr( const Expr &sf, int n ); ///< global string id for string n in sourcefile sf
    SfInfo           *sf_info( const Expr &sf ); ///< ref on a buffer for sourcefile data

    // methods for Type variables
    Var               type_of( const Var &var ) const; ///< a variable to represent var.type
    TypeInfo         *type_info( const Expr &type );

    Var              *type_for( ClassInfo *class_info ); ///< type for class_info without template arguments
    Var              *type_for( ClassInfo *class_info, Var *parm_0 ); ///< type for class_info with 1 template argument
    Var              *type_for( ClassInfo *class_info, Var **parm_l ); ///< type for class_info with template arguments

    /// callable
    CallableInfo     *callable_info( const Expr &callable_ptr );
    ClassInfo        *class_info( const Expr &class_ptr, bool crea = true );
    DefInfo          *def_info( const Expr &def_ptr, bool crea = true );

    ClassInfo        *class_info( const Var &class_var ); ///< helper


    // helpers
    bool              equal( Var a, Var b );
    bool              isa_ptr_int( const Var &var ) const;
    Expr              cst_ptr( SI64 val );
    Var               make_varargs_var( const Vec<Var> &uv_args, const Vec<Var> &nv_args, const Vec<int> &nv_name );
    Var               ext_method( const Var &var ); ///< return var if var is a function with self as arg
    Var               make_Callable( const Vec<Var> &lst, Var self );
    Var               update_Callable( Var surdef_list, Var varargs );
    Var               copied_or_constified( const Var &var );

    Var              *_make_varargs_type( const Vec<Var> &uv_args, const Vec<Var> &nv_args, const Vec<int> &nv_name, int off );
    void              _update_base_type_from_class_expr( Var type, Expr class_expr );
    Var               _get_type_var( ClassInfo *class_info );

    // references
    bool              is_of_class( const Var &var, const Var &class_ ) const;

    // conversion
    template<class T>
    bool conv( T &res, const Var &var ) {
        if ( const PI8 *data = var.cst_data() ) {
            #define DECL_BT( U ) \
                if ( isa_##U( var ) ) \
                    return ::conv( res, *reinterpret_cast<const U *>( data ) );
            #include "../Inst/DeclArytTypes.h"
            #undef DECL_BT
        }
        return false;
    }

    int to_bool( Var val, const Expr &sf, const PI8 *tok ); ///< -1 means unknown, 0 means false, 1 means true

    struct VarRef {
        VarRef() : cpt( -1 ) {}
        Var var;
        int cpt;
    };
    
    // base types and class
    #define DECL_BT( T ) \
        bool isa( const Var &var, S<T> ) const { return isa_##T( var ); } \
        Var *type_for( S<T> ) { return &type_##T; }
    #include "../Inst/DeclArytTypes.h"
    #undef DECL_BT

    const BaseType *bt_of( const Var &var ) const;
    Var *type_for( const BaseType *bt );

    // basic types (not parameterized)
    #define DECL_BT( T ) \
        Var type_##T; \
        bool isa_##T( const Var &var ) const { return var.type == type_##T.data; }
    #include "DeclBaseClass.h"
    #undef DECL_BT

    // basic classes
    #define DECL_BT( T ) \
        Var class_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    // parameterized classes
    #define DECL_BT( T ) \
        Var class_##T; \
        bool isa_##T( const Var &var ) const;
    #include "DeclParmClass.h"
    #undef DECL_BT

    bool isa_POD( const Var &var ) const;


    // std variables
    Var                          error_var;
    Var                          void_var;

    Var                         *type_ST;
    const BaseType              *bt_ST;

    // attributes
    SplittedVec<ToDel *,16>      obj_to_delete;
    SplittedVec<Expr,8>          sourcefiles; ///< contains Cst( data ) for each opened sourcefile
    Vec<String>                  inc_paths;

    ErrorList                   &error_list;
    Scope                       *main_scope;

    NstrCor                      glob_nstr_cor; ///< string <-> global int number

    // maps
    std::map<Expr,CallableInfo *> callable_info_map; ///< pointer to either ClassInfo or DefInfo
    std::map<Expr,ClassInfo    *> class_info_map;
    std::map<Expr,DefInfo      *> def_info_map;
    std::map<Expr,TypeInfo     *> type_info_map;
    std::map<Expr,SfInfo        > sf_info_map;
};

extern Interpreter *ip; ///< global variable made for convenience... starts at 0

#endif // INTERPRETER_H
