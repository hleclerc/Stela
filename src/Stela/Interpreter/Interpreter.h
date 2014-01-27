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

    // architecture information
    int  ptr_size() const;
    int  ptr_alig() const;

    // error messages
    ErrorList::Error &make_error( String msg, const Expr *sf = 0, int off = 0, Scope *sc = 0, bool warn = false );
    void              disp_error( String msg, const Expr *sf = 0, int off = 0, Scope *sc = 0, bool warn = false );

    // methods for sourcefiles
    bool              already_imported( String filename );

    SfInfo           &sf_info_of( const Expr *sf ); ///< ref on a buffer for sourcefile data
    int               glo_nstr( const Expr *sf, int n ); ///< global string id for string n in sourcefile sf
    const PI8        *tok_data_of( const Expr *sf );

    // methods for Type variables
    Var               type_of( const Var &var ) const; ///< a variable to represent var.type
    TypeInfo         *type_info( const Expr &type );

    Var              *type_for( ClassInfo &class_info ); ///< type for class_info without template arguments
    Var              *type_for( ClassInfo &class_info, Var *parm_0 ); ///< type for class_info with 1 template argument
    Var              *type_for( ClassInfo &class_info, Vec<Var *> parm_l ); ///< type for class_info with template arguments

    /// callable
    CallableInfo     *callable_info( const Expr &ce );
    ClassInfo        &class_info( const Var &class_var ); ///<
    ClassInfo        &class_info( const Expr &cg );



    // helpers
    bool              equal( Var a, Var b );
    Var               constified( Var &var ); ///< make a copy if referenced several times
    bool              isa_ptr_int( const Var &var ) const;
    Expr              cst_ptr( SI64 val );

    // references
    Expr              ref_expr_on( const Var &var );
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

    // basic types (not parameterized)
    #define DECL_BT( T ) \
        Var type_##T; \
        bool isa_##T( const Var &var ) const { return var.type == type_##T.data; }
    #include "DeclBaseClass.h"
    #undef DECL_BT

    // basic classes
    #define DECL_BT( T ) Var class_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    // parameterized classes
    #define DECL_BT( T ) Var class_##T; bool isa_##T( const Var &var ) const;
    #include "DeclParmClass.h"
    #undef DECL_BT

    // std variables
    Var                          error_var;
    Var                          void_var;

    const BaseType              *bt_ST;

    // attributes
    SplittedVec<ToDel *,16>      obj_to_delete;
    SplittedVec<Expr,8>          sourcefiles; ///< contains Cst( data )
    Vec<String>                  inc_paths;

    ErrorList                   &error_list;
    Scope                       *main_scope;

    // context
    std::map<Expr        ,CallableInfo *> callable_info_map; ///< pointer to either ClassInfo or DefInfo
    std::map<Expr        ,ClassInfo     > class_info_map;
    std::map<Expr        ,DefInfo       > def_info_map;
    std::map<Expr        ,TypeInfo     *> type_info_map;
    std::map<Expr        ,SfInfo        > sf_info_map;

    std::map<const PRef *,VarRef        > var_refs; ///< references
};

extern NstrCor glob_nstr_cor;

#endif // INTERPRETER_H
