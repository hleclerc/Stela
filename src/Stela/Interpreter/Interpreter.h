#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "../System/SplittedVec.h"
#include "../System/ErrorList.h"
#include "../System/Stream.h"
#include "../System/Math.h"
#include "../System/Vec.h"
#include "../System/S.h"
#include "NstrCor.h"
#include "SfInfo.h"
#include "Var.h"

class Scope;
class ToDel;

/**
*/
class Interpreter {
public:
    Interpreter( ErrorList &error_list );
    ~Interpreter();

    void import( String filename );
    void add_inc_path( String path );
    void set_argc( int argc );
    void set_argv( char **argv );
    int  ptr_size() const;
    int  ptr_alig() const;

protected:
    friend class Scope;

    ErrorList::Error &make_error( String msg, const Var *sf = 0, int off = 0, Scope *sc = 0, bool warn = false );
    void              disp_error( String msg, const Var *sf = 0, int off = 0, Scope *sc = 0, bool warn = false );
    int               glo_nstr( const Var *sf, int n );

    bool              already_imported( String filename );
    const PI8        *tok_data_of( const Var *sf );
    SfInfo           &sf_info_of( const Var *sf );
    Var               type_of( const Var &var );

    // conversion
    template<class T>
    bool conv( T &res, const Var &var ) {
        if ( const PI8 *data = var.cst_data() ) {
            #define DECL_BT( U ) \
                if ( isa_##U( var ) ) \
                    return ::conv( res, *reinterpret_cast<const U *>( data ) );
            #include "DeclArytTypes.h"
            #undef DECL_BT
        }
        return false;
    }

    
    // base types and class
    #define DECL_BT( T ) \
        bool isa( const Var &var, S<T> ) const { return isa_##T( var ); } \
        Var *type_for( S<T> ) { return &type_##T; }
    #include "DeclArytTypes.h"
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
    #define DECL_BT( T ) Var class_##T;
    #include "DeclParmClass.h"
    #undef DECL_BT

    // std variables
    Var                          error_var;
    Var                          void_var;

    // attributes
    SplittedVec<ToDel *,16>      obj_to_delete;
    SplittedVec<Var,8>           sourcefiles;
    Vec<String>                  inc_paths;
    ErrorList                   &error_list;
    Scope                       *main_scope;
    char                       **argv;
    int                          argc;

    // context
    std::map<const PI8 *,SfInfo> sf_info_map;
};

extern NstrCor glob_nstr_cor;

#endif // INTERPRETER_H
