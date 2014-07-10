#include "../System/UsualStrings.h"
#include "../System/SplittedVec.h"
#include "../System/ErrorList.h"
#include "../System/RaiiSave.h"
#include "../System/Assert.h"
#include "../Ir/Numbers.h"
#include "../Met/Lexem.h"
#include "Ast_Primitive.h"
#include "Ast_Continue.h"
#include "Ast_Variable.h"
#include "Ast_GetAttr.h"
#include "Ast_Number.h"
#include "Ast_String.h"
#include "Ast_NdList.h"
#include "Ast_Assign.h"
#include "Ast_Select.h"
#include "Ast_ChBeBa.h"
#include "Ast_Import.h"
#include "Ast_Return.h"
#include "Ast_Class.h"
#include "Ast_Apply.h"
#include "Ast_While.h"
#include "Ast_Block.h"
#include "Ast_Break.h"
#include "Ast_Void.h"
#include "Ast_Def.h"
#include "Ast_New.h"
#include "Ast_For.h"
#include "Ast_If.h"

#include "IrWriter.h"

Ast::Ast( int off ) : _off( off ) {
}

Ast::~Ast() {
}

void Ast::write_to( IrWriter *aw ) const {
    int old_delayed_parse_size = aw->delayed_parse.size();

    // write inst
    aw->data << _tok_number();
    aw->data << _off;
    _get_info( aw );

    // write instruction needed for inst
    while ( aw->delayed_parse.size() > old_delayed_parse_size ) {
        IrWriter::DelayedParse dp = aw->delayed_parse.pop_back_val();
        *dp.offset = aw->data.size() - dp.old_size;

        dp.l->write_to( aw );
    }
}

void Ast::_get_info( IrWriter *aw ) const {
}

void Ast::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
}


struct AstMaker {
    AstMaker() {
        static_inst = false;
        const_inst = false;
    }

    Ast *make_ast_variable( const Lexem *l ) {
        if ( l->eq( "break"    ) ) return new Ast_Break   ( l->off() );
        if ( l->eq( "continue" ) ) return new Ast_Continue( l->off() );
        if ( l->eq( "true"     ) ) return new Ast_Number  ( l->off(), true  );
        if ( l->eq( "false"    ) ) return new Ast_Number  ( l->off(), false );
        return new Ast_Variable( l->off(), String( l->beg, l->beg + l->len ) );
    }

    Ast *make_ast_number( const Lexem *l ) {
        Ast_Number *res = new Ast_Number( l->off() );

        int e = l->len - 1;
        while ( e >= 0 ) {
            if ( l->beg[ e ] == 'l' ) { res->l = true; --e; continue; }
            if ( l->beg[ e ] == 'p' ) { res->p = true; --e; continue; }
            break;
        }

        res->str = String( l->beg, l->beg + e + 1 );

        return res;
    }

    Ast *make_ast_string( const Lexem *l ) {
        return new Ast_String( l->off(), String( l->beg, l->beg + l->len ) );
    }

    Ast *make_ast_block( const Lexem *l ) {
        while ( l->type == Lexem::BLOCK and not l->next )
            l = l->children[ 0 ];

        Ast_Block *a = new Ast_Block( l->off() );
        for( ; l; l = l->next )
            if ( Ast *r = make_ast_single( l ) )
                a->lst << r;
        return a;
    }

    static int dim_nb_cr( const Lexem *l, bool has_cd ) {
        if ( has_cd )
            return l->nb_preceding_comma_dot ? 1 : l->nb_preceding_cr + ( l->nb_preceding_cr != 0 );
        return l->nb_preceding_cr;
    }

    void output_list( Ast_NdList *res, const Lexem *l, int nb_dim, bool has_cd ) {
        // ... , ...
        if ( nb_dim == 1 ) {
            SplittedVec<const Lexem *,8> ch;
            get_children_of_type( l, STRING_comma_NUM, ch );

            res->sizes << ch.size();
            for( int i = 0; i < ch.size(); ++i )
                res->lst << make_ast_single( ch[ i ] );
        } else {
            // get nb_values
            int nb_values = 1;
            for( const Lexem *t = l->next; t and dim_nb_cr( t, has_cd ) < nb_dim; t = t->next )
                nb_values += dim_nb_cr( t, has_cd ) == nb_dim - 1;
            res->sizes << nb_values;

            // output children values
            for( const Lexem *t = l; t and dim_nb_cr( t, has_cd ) < nb_dim; t = t->next ) {
                if ( dim_nb_cr( t, has_cd ) == nb_dim - 1 ) {
                    output_list( res, l, nb_dim - 1, has_cd );
                    l = t;
                }
            }
            output_list( res, l, nb_dim - 1, has_cd );
        }
    }


    Ast *make_ast_paren( const Lexem *l ) {
        // []
        if ( *l->beg == '[' ) {
            l = l->children[ 0 ];
            while ( l and l->type == Lexem::BLOCK )
                l = l->children[ 0 ];

            // get nb_dim
            int nb_dim = 0;
            bool has_cd = false; // has comma dot
            if ( l ) {
                for( const Lexem *t = l->next; t; t = t->next ) {
                    if ( t->nb_preceding_comma_dot )
                        has_cd = true;
                    else
                        nb_dim = std::max( nb_dim, t->nb_preceding_cr );
                }
            }
            nb_dim += 1 + has_cd;

            //
            Ast_NdList *res = new Ast_NdList( l->off() );
            res->nb_dim = nb_dim;
            output_list( res, l, nb_dim, has_cd );
            return res;
        }

        // else
        return make_ast_block( l->children[ 0 ] );
    }

    Ast *make_ast_assign( const Lexem *l, bool type ) {
        Ast_Assign *res = new Ast_Assign( l->off() );

        // LHS
        res->name = String( l->children[ 0 ]->beg, l->children[ 0 ]->end() );

        // ref
        const Lexem *c = l->children[ 1 ];
        res->ref = c->type == STRING___ref___NUM;
        if ( res->ref )
            c = c->children[ 0 ];

        // RHS
        res->val  = make_ast_single( c );

        // flags
        res->stat = static_inst;
        res->cons = const_inst;
        res->type = type;

        return res;
    }

    static int num_primitive( const String &str ) {
        #define DECL_IR_TOK( PN ) if ( str == "___" #PN ) return IR_TOK_##PN;
        #include "../Ir/Decl_Primitives.h"
        #include "../Ir/Decl_Operations.h"
        #undef DECL_IR_TOK

        return -1;
    }

    Ast *make_ast_apply( const Lexem *l, Ast_Call *res ) {
        // get nb_..._children and ch list
        SplittedVec<const Lexem *,16> ch;
        int nb_unnamed_children = 0, nb_named_children = 0;
        if ( l->children[ 1 ] ) {
            get_children_of_type( next_if_CR( l->children[ 1 ] ), STRING_comma_NUM, ch );
            for( ; nb_unnamed_children < ch.size() and ch[ nb_unnamed_children ]->type != STRING_reassign_NUM; ++nb_unnamed_children );
            for( int i=nb_unnamed_children; i < ch.size(); ++i, ++nb_named_children ) {
                if ( ch[ i ]->type != STRING_reassign_NUM )
                    return add_error( "after a named argument, all arguments must be named", ch[ i ] );
            }
        }

        // particular case : assertion
        const Lexem *f = l->children[ 0 ] ? l->children[ 0 ] : l;
        if ( nb_unnamed_children == 2 and nb_named_children == 0 and f->type == Lexem::VARIABLE and f->len == 6 and strncmp( f->beg, "assert", 6 ) == 0 ) {
            TODO;
            // -> make an if
        }

        // break n
        if ( f->eq( "break" ) ) {
            if ( ch.size() > 1 )
                return add_error( "break accepts at most 1 parameter", f, res );

            delete res;

            Ast_Break *nres = new Ast_Break( f->off() );
            if ( ch.size() == 1 ) {
                if ( not ch[ 0 ]->is_an_int() )
                    return add_error( "break expects a known integer value", f, res );
                nres->n = ch[ 0 ]->to_int();
            }
            return nres;
        }


        // primitive ?
        if ( f->type == Lexem::VARIABLE ) {
            int np = num_primitive( f->str() );
            if ( np >= 0 ) {
                if ( nb_named_children )
                    return add_error( "a primitive do not accept named children", f, res );
                delete res;

                Ast_Primitive *nres = new Ast_Primitive( f->off(), np );
                for( int i = 0; i < nb_unnamed_children; ++i )
                    nres->args << make_ast_single( ch[ i ] );
                return nres;
            }
        }

        // function
        res->f = make_ast_single( f );

        // unnamed arguments
        for( int i = 0; i < nb_unnamed_children; ++i )
            res->args << make_ast_single( ch[ i ] );

        // named arguments
        for( int i = 0; i < nb_named_children; ++i ) {
            const Lexem *n = ch[ nb_unnamed_children + i ]->children[ 0 ];
            res->names << String( n->beg, n->beg + n->len );
            res->args << make_ast_single( ch[ nb_unnamed_children + i ]->children[ 1 ] );
        }

        return res;
    }

    /// @see parse_callable
    struct Argument {
        const Lexem *name;
        const Lexem *default_value;
        const Lexem *type_constraint;
    };

    /// != get_children_of_type because it takes left and right arguments
    static unsigned nb_child_and( const Lexem *t ) {
        if ( t->type != STRING___and___NUM ) {
            if ( ( t->type == Lexem::PAREN or t->type == Lexem::BLOCK ) and t->children[ 0 ] )
                return nb_child_and( t->children[ 0 ] );
            return 1;
        }
        return nb_child_and( t->children[ 0 ] ) + nb_child_and( t->children[ 1 ] );
    }


    static bool is_a_method( const Lexem *t ) {
        while ( t and not t->parent )
            t = t->prev;
        while ( t and t->parent->type == Lexem::BLOCK )
            t = t->parent;
        return t and t->parent->type == STRING___class___NUM;
    }


    Ast *make_ast_callable( const Lexem *t, bool def ) {
        // parsed data
        SplittedVec<const Lexem *,2> inheritance, starts_with;
        SplittedVec<Argument,8> arguments;
        bool self_as_arg, varargs, abstract;
        int default_pertinence_num, default_pertinence_den;
        const Lexem *return_type;
        const Lexem *pertinence;
        const Lexem *condition;
        const Lexem *block;
        const Lexem *name;

        // block
        block = t->children[ 1 ]->type == Lexem::PAREN or t->children[ 1 ]->type == Lexem::BLOCK ?
                    t->children[ 1 ]->children[ 0 ] : t->children[ 1 ];

        // beginning of arguments
        const Lexem *c = t->children[ 0 ];

        pertinence  = 0;
        condition   = 0;
        return_type = 0;
        while ( true ) {
            if ( c->type == STRING___extends___NUM     ) { get_children_of_type( c->children[ 1 ], STRING_comma_NUM, inheritance ); c = c->children[ 0 ]; continue; }
            if ( c->type == STRING___starts_with___NUM ) { get_children_of_type( c->children[ 1 ], STRING_comma_NUM, starts_with ); c = c->children[ 0 ]; continue; }
            if ( c->type == STRING___pertinence___NUM  ) { pertinence  = c->children[ 1 ]; c = c->children[ 0 ]; continue; }
            if ( c->type == STRING___when___NUM        ) { condition   = c->children[ 1 ]; c = c->children[ 0 ]; continue; }
            if ( c->type == STRING_doubledot_NUM       ) { return_type = c->children[ 1 ]; c = c->children[ 0 ]; continue; }
            break;
        }

        // name and arguments
        if ( c->type == Lexem::APPLY or c->type == Lexem::SELECT ) {
            name = c->children[ 0 ];

            SplittedVec<const Lexem *,8> tl;
            get_children_of_type( c->children[ 1 ], STRING_comma_NUM, tl );

            bool mandatory_default_val = false;
            for( int i = 0; i < tl.size(); ++i ) {
                const Lexem *t = tl[ i ];
                if ( not t )
                    continue;

                Argument *arg = arguments.push_back();

                // look for a = ... ( default_value )
                if ( t->type == STRING_reassign_NUM ) {
                    Lexem *dv = t->children[ 1 ];
                    if ( dv->type == STRING_doubledot_NUM )
                        return add_error( "Syntax for argument with type constraint and default value is 'name_arg : typectraint = default_value' (in this specific order).", dv );
                    arg->default_value = dv;
                    mandatory_default_val = true;
                    t = t->children[ 0 ];
                } else if ( mandatory_default_val ) {
                    if ( not t->eq( "varargs" ) )
                        return add_error( "After one argument with default value, all arguments must have default values.", t );
                } else
                    arg->default_value = 0;

                // look for a : Toto
                if ( t->type == STRING_doubledot_NUM ) {
                    arg->type_constraint = t->children[ 1 ];
                    t = t->children[ 0 ];
                } else
                    arg->type_constraint = 0;

                // name
                arg->name = t;
            }
        } else {
            name = c;
        }

        // check that name is correct
        if ( name->children[ 1 ] or ( name->children[ 0 ] and name->children[ 0 ]->type != Lexem::PAREN ) )
            return add_error( "syntax error.", name );

        // default_pertinence
        if ( pertinence ) {
            if ( pertinence->type == Lexem::NUMBER ) {
                pertinence->to_rat( default_pertinence_num, default_pertinence_den );
                pertinence = NULL;
            } else if ( pertinence->type == STRING_sub_NUM and pertinence->children[ 1 ] == 0 and pertinence->children[ 0 ]->type == Lexem::NUMBER ) {
                pertinence->children[0]->to_rat( default_pertinence_num, default_pertinence_den );
                default_pertinence_num *= -1;
                pertinence = NULL;
            } else {
                default_pertinence_num = 0;
                default_pertinence_den = 1;
            }
        } else {
            default_pertinence_num = condition ? nb_child_and( condition ) : 0;
            for( int i = 0; i < arguments.size(); ++i )
                default_pertinence_num += bool( arguments[i].type_constraint );
            default_pertinence_num += is_a_method( t );
            default_pertinence_den = 1;
        }

        // self_as_arg, varargs
        self_as_arg = ( arguments.size() and arguments[ 0 ].name->eq( "self" ) );
        if ( self_as_arg )
            arguments.erase( 0 );
        varargs = ( arguments.size() and arguments.back().name->type==Lexem::VARIABLE and
                   arguments.back().name->len == 7 and strncmp( arguments.back(). name->beg, "varargs", 7 ) == 0 );
        if ( varargs )
            arguments.pop_back();

        // nb_default_args
        int nb_default_args = 0;
        for( int i = 0; i < arguments.size(); ++i )
            nb_default_args += arguments[ i ].default_value != 0;


        // basic assertions
        for( int i = 0; i < arguments.size(); ++i ) {
            Argument &arg = arguments[ i ];
            if ( arg.name->eq( "self" ) )
                add_error( "'self' as non first argument name of a def is weird.", arg.name );
            //if ( arg.type_constraint and arg.type_constraint->type != Lexem::VARIABLE )
            //    add_error( "type constraints must be simple variables (no composition).", arg.type_constraint );
        }

        // abstract
        abstract = ( block->type == STRING_abstract_NUM );

        // get / set / ...
        const char *get_beg = 0; int get_len = 0;
        const char *set_beg = 0; int set_len = 0;
        const char *sop_beg = 0; int sop_len = 0;
        if ( def ) {
            if ( name->type == Lexem::VARIABLE and name->begin_with( "get_" ) ) { get_beg = name->beg + 4; get_len = name->len - 4; }
            if ( name->type == Lexem::VARIABLE and name->begin_with( "set_" ) ) { set_beg = name->beg + 4; set_len = name->len - 4; }
            if ( name->type == Lexem::VARIABLE and name->begin_with( "sop_" ) ) { sop_beg = name->beg + 4; sop_len = name->len - 4; }
        }


        // output --------------------------------------------------------------
        Ast_Callable *res;
        if ( def )
            res = new Ast_Def  ( t->off() );
        else
            res = new Ast_Class( t->off() );

        res->name = String( name->beg, name->end() );

        res->self_as_arg  = self_as_arg;
        res->varargs      = varargs;
        res->abstract     = abstract;
        res->pertinence   = make_ast_single( pertinence  );
        res->condition    = make_ast_single( condition   );
        res->def_pert_num = default_pertinence_num;
        res->def_pert_den = default_pertinence_den;

        for( int i = 0; i < arguments.size(); ++i ) {
            // arg name
            if ( arguments[ i ].name->type == Lexem::VARIABLE )
                res->arguments << arguments[ i ].name->str();
            else
                TODO;

            // nb_constraints + constraints
            Vec<String> &c = *res->arg_constraints.push_back();
            if ( arguments[ i ].type_constraint ) {
                if ( arguments[ i ].type_constraint->type == Lexem::VARIABLE ) {
                    c << arguments[ i ].type_constraint->str();
                } else if ( arguments[ i ].type_constraint->type == Lexem::PAREN ) {
                    SplittedVec<const Lexem *,8> ch;
                    get_leaves( ch, arguments[ i ].type_constraint );
                    for( int j = 0; j < ch.size(); ++j ) {
                        if ( ch[ j ]->type == Lexem::VARIABLE )
                            c << ch[ j ]->str();
                        else
                            return add_error( "constraints must be simple variables", ch[ j ], res );
                    }
                } else
                    return add_error( "constraints must be simple variables", arguments[ i ].type_constraint, res );
            }
        }

        for( int i = 0; i < arguments.size(); ++i )
            if ( arguments[ i ].default_value )
                res->default_values << make_ast_single( arguments[ i ].default_value );

        // block
        res->block = make_ast_block( block );

        // specific cases
        if ( def ) {
            Ast_Def *rd = static_cast<Ast_Def *>( res );

            // return type
            rd->return_type = make_ast_single( return_type );

            if ( name->eq( "init" ) ) {
                for( int i = 0; i < starts_with.size(); ++i ) {
                    Ast_Def::StartsWith_Item *item = rd->starts_with.push_back();

                    // get args
                    SplittedVec<const Lexem *,16> ch;
                    int nb_unnamed_children = 0;
                    int nb_named_children = 0;
                    const Lexem *l = starts_with[ i ];

                    if ( l->children[ 1 ] ) {
                        get_children_of_type( next_if_CR( l->children[ 1 ] ), STRING_comma_NUM, ch );
                        for( ; nb_unnamed_children < ch.size() and ch[ nb_unnamed_children ]->type != STRING_reassign_NUM; ++nb_unnamed_children )
                            ;
                        for( int i = nb_unnamed_children; i < ch.size(); ++i, ++nb_named_children )
                            if ( ch[ i ]->type != STRING_reassign_NUM )
                                return add_error( "after a named argument, all arguments must be named", ch[ i ], res );
                    }

                    // attribute
                    const Lexem *f = l->children[ 0 ] ? l->children[ 0 ] : l;
                    if ( f->type != Lexem::VARIABLE )
                        return add_error( "expecting attribute name", ch[ i ], res );
                    item->attr = f->str();

                    // unnamed arguments
                    for( int i = 0; i < nb_unnamed_children; ++i )
                        item->args << make_ast_single( ch[ i ] );

                    // named arguments
                    for( int i = 0; i < nb_named_children; ++i ) {
                        item->names << ch[ nb_unnamed_children + i ]->children[ 0 ]->str();
                        item->args  << make_ast_single( ch[ nb_unnamed_children + i ]->children[ 1 ] );
                    }
                }
            }

            // get set sop
            rd->get = String( get_beg, get_beg + get_len );
            rd->set = String( set_beg, set_beg + set_len );
            rd->sop = String( sop_beg, sop_beg + sop_len );
        } else {
            Ast_Class *rd = static_cast<Ast_Class *>( res );

            // extends
            for( int i = 0; i < inheritance.size(); ++i )
                rd->inheritance << make_ast_single( inheritance[ i ] );
        }

        return res;
    }

    Ast *make_ast_if( const Lexem *l ) {
        Ast_If *res = new Ast_If( l->off() );
        res->cond = make_ast_single( l->children[ 0 ] );

        if ( l->children[ 1 ]->type == STRING___else___NUM ) {
            res->ok = make_ast_block( l->children[ 1 ]->children[ 0 ] );
            res->ko = make_ast_block( l->children[ 1 ]->children[ 1 ] );
        } else
            res->ok = make_ast_block( l->children[ 1 ] );

        return res;
    }

    Ast *make_ast_while( const Lexem *l ) {
        Ast_While *res = new Ast_While( l->off() );

        if ( l->children[ 0 ]->type == STRING___else___NUM ) {
            res->ok = make_ast_block( l->children[ 0 ]->children[ 0 ] );
            res->ko = make_ast_block( l->children[ 0 ]->children[ 1 ] );
        } else
            res->ok = make_ast_block( l->children[ 0 ] );

        return res;
    }

    Ast *make_ast_return( const Lexem *l ) {
        Ast_Return *res = new Ast_Return( l->off() );
        res->val = make_ast_single( l->children[ 0 ] );
        return res;
    }

    Ast *make_ast_get_attr( const Lexem *l, bool ptr, bool ask, bool ddo ) {
        Ast_GetAttr *res = new Ast_GetAttr( l->off() );
        res->name = l->children[ 1 ]->str();
        res->obj = make_ast_single( l->children[ 0 ] );
        res->ptr = ptr;
        res->ask = ask;
        res->ddo = ddo;
        return res;
    }

    Ast *make_ast_static( const Lexem *l ) {
        RaiiSave<bool> o( static_inst, true );
        return make_ast_single( l->children[ 0 ] );
    }

    Ast *make_ast_const( const Lexem *l ) {
        RaiiSave<bool> o( const_inst, true );
        return make_ast_single( l->children[ 0 ] );
    }

    Ast *make_ast_for( const Lexem *l ) {
        if ( l->children[ 0 ]->type != STRING___in___NUM )
            return add_error( "syntax of 'for' is 'for ... in ... ...'.", l->children[ 0 ] );

        // names
        SplittedVec<const Lexem *,8> names;
        get_children_of_type( l->children[ 0 ]->children[ 0 ], STRING_comma_NUM, names );

        SplittedVec<const Lexem *,8> ch;
        get_children_of_type( l->children[ 0 ]->children[ 1 ], STRING_comma_NUM, ch );

        //
        Ast_For *res = new Ast_For( l->off() );

        for( int i = 0; i < names.size(); ++i ) {
            if ( names[ i ]->type != Lexem::VARIABLE )
                return add_error( "expecting a name", names[ i ], res );
            res->names << names[ i ]->str();
        }

        for( int i = 0; i < ch.size(); ++i )
            res->objects << make_ast_single( ch[ i ] );

        res->block = make_ast_block( l->children[ 1 ] );

        return res;
    }

    Ast *make_ast_import( const Lexem *l ) {
        SplittedVec<const Lexem *,8> ch;
        get_children_of_type( l->children[ 0 ], STRING_comma_NUM, ch );

        Ast_Import *res = new Ast_Import( l->off() );
        for( int i = 0; i < ch.size(); ++i )
            res->files << ch[ i ]->str();

        return res;
    }

    Ast *make_ast_lambda( const Lexem *l ) {
        TODO;
        return 0;
    }

    Ast *make_ast_and( const Lexem *l ) {
        TODO;
        return 0;
    }

    Ast *make_ast_or( const Lexem *l ) {
        TODO;
        return 0;
    }

    Ast *make_ast_break( const Lexem *l ) {
        return new Ast_Break( l->off() );
    }

    Ast *make_ast_call_op( const Lexem *l ) {
        Ast_Apply *res = new Ast_Apply( l->off() );
        res->f = new Ast_Variable( l->off(), get_operators_cpp_name( l->type ) );
        for( int i = 0; i < 2; ++i )
            if ( l->children[ i ] )
                res->args << make_ast_single( l->children[ i ] );
        return res;
    }



    /// may return 0
    Ast *make_ast_single( const Lexem *l ) {
        if ( not l )
            return 0;
        switch( l->type )  {
        case Lexem::VARIABLE            : return make_ast_variable( l );
        case Lexem::NUMBER              : return make_ast_number  ( l );
        case Lexem::STRING              : return make_ast_string  ( l );
        case Lexem::CR                  : return 0;
        case Lexem::PAREN               : return make_ast_paren   ( l ); // case 1: [], else simple block
        case Lexem::CCODE               : return add_error( "todo: ``", l );
        case Lexem::BLOCK               : return make_ast_block   ( l );
        case Lexem::NONE                : return 0;
        case Lexem::APPLY               : return make_ast_apply   ( l, new Ast_Apply ( l->off() ) );
        case Lexem::SELECT              : return make_ast_apply   ( l, new Ast_Select( l->off() ) );
        case Lexem::CHANGE_BEHAVIOR     : return make_ast_apply   ( l, new Ast_ChBeBa( l->off() ) );
        case STRING___new___NUM         : return make_ast_apply   ( l->children[ 0 ], new Ast_New( l->off() ) );
        case STRING_assign_NUM          : return make_ast_assign  ( l, false );
        case STRING_assign_type_NUM     : return make_ast_assign  ( l, true  );
        case STRING___def___NUM         : return make_ast_callable( l, true  );
        case STRING___class___NUM       : return make_ast_callable( l, false );
        case STRING___if___NUM          : return make_ast_if      ( l );
        case STRING___while___NUM       : return make_ast_while   ( l );
        case STRING___return___NUM      : return make_ast_return  ( l );
        case STRING_get_attr_NUM        : return make_ast_get_attr( l, 0, 0, 0 );
        case STRING_get_attr_ptr_NUM    : return make_ast_get_attr( l, 1, 0, 0 );
        case STRING_get_attr_ask_NUM    : return make_ast_get_attr( l, 0, 1, 0 );
        case STRING_get_attr_ptr_ask_NUM: return make_ast_get_attr( l, 1, 1, 0 );
        case STRING_doubledoubledot_NUM : return make_ast_get_attr( l, 0, 0, 1 );
        case STRING___static___NUM      : return make_ast_static  ( l );
        case STRING___const___NUM       : return make_ast_const   ( l );
        case STRING___for___NUM         : return make_ast_for     ( l );
        case STRING___import___NUM      : return make_ast_import  ( l );
        case STRING_lambda_NUM          : return make_ast_lambda  ( l );
        case STRING___and___NUM         : return make_ast_and     ( l );
        case STRING___or___NUM          : return make_ast_or      ( l );
        case STRING___else___NUM        : return make_ast_or      ( l );
        case STRING_break_NUM           : return make_ast_break   ( l );
        default:
            return make_ast_call_op( l );
        }
    }

    Ast *add_error( String msg, const Lexem *l, Ast *to_del = 0 ) {
        if ( to_del )
            delete to_del;
        ErrorList::Error &e = error_list->add( msg ).ac( l->beg_src, l->beg, l->src );
        std::cerr << e;
        return 0;
    }

    ErrorList *error_list;
    bool static_inst;
    bool const_inst;
};

Ast *make_ast( ErrorList &e, const Lexem *l, bool sibling ) {
    AstMaker am;
    am.error_list = &e;

    if ( sibling and l and l->next )
        return am.make_ast_block( l );
    if ( Ast *r = am.make_ast_single( l ) )
        return r;
    return new Ast_Void( l ? l->off() : 0 );
}


