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
    Vec<ConstPtr<Inst> > get_outputs();

    void add_inc_path( String path );

    void set_argc( int argc );
    void set_argv( char **argv );

    int  ptr_size() const;
    int  ptr_alig() const;

    //
    ErrorList::Error &make_error( String msg, const PI8 *sf = 0, int off = 0, Scope *sc = 0, bool warn = false );
    void              disp_error( String msg, const PI8 *sf = 0, int off = 0, Scope *sc = 0, bool warn = false );

    // methods for sourcefiles
    bool              already_imported( String filename );
    const PI8        *tok_data_of( const PI8 *sf );

    SfInfo           &sf_info_of( const PI8 *sf );

    int               glo_nstr( const PI8 *sf, int n );

    // methods for Type variables
    Var               type_of( const Var &var ) const;
    ClassInfo        &class_info( const Var &class_var );
    ClassInfo        &class_info( const Expr &cg );
    TypeInfo         *type_info( const Expr &type );
    CallableInfo     *callable_info( Expr ce );

    Var              *type_for( ClassInfo &class_info );
    Var              *type_for( ClassInfo &class_info, Var *parm_0 );
    Var              *type_for( ClassInfo &class_info, Vec<Var *> parm_l );


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
    char                       **argv;
    int                          argc;

    // context
    typedef AutoPtr<CallableInfo> PCAL;
    std::map<Expr        ,PCAL      > callable_info_map; ///<
    std::map<Expr        ,ClassInfo > class_info_map;
    std::map<Expr        ,TypeInfo *> type_info_map;
    std::map<const PI8  *,SfInfo    > sf_info_map;
    std::map<const PRef *,VarRef    > var_refs;
};

extern NstrCor glob_nstr_cor;

#endif // INTERPRETER_H
