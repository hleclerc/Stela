#include "../System/UsualStrings.h"
#include "../Ir/CallableFlags.h"
#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Lexem.h"

#define PARSING( T ) // std::cout << "Parsing " #T << std::endl

IrWriter::IrWriter( ErrorList &error_list ) : error_list( error_list ) {
    static_inst = false;
    const_inst = false;
}

void IrWriter::parse( const Lexem *root ) {
    parse_lexem_and_siblings( root );
    int_reduction();
}

ST IrWriter::size_of_binary_data() {
    ST res = 2;
    for( auto iter : nstrings )
        res += BinStreamWriter::size_needed_for( iter.first.size() ) + iter.first.size();
    return res + 1 + data.size();
}

void IrWriter::copy_binary_data_to( PI8 *ptr ) {
    // list of size + strings (not 0 ended)
    Vec<std::string> strings( Size(), nstrings.size() );
    for( auto iter : nstrings )
        strings[ iter.second ] = iter.first;

    for( int i = 0; i < strings.size(); ++i ) {
        // size
        ST s = strings[ i ].size();
        BinStreamWriter::write_val_to( ptr, s );
        // data
        memcpy( ptr, strings[ i ].c_str(), s );
        ptr += s;
    }
    *(ptr++) = 0;

    // tok data
    data.copy_to( ptr );
}

void IrWriter::parse_lexem_and_siblings( const Lexem *l ) {
    int old_delayed_parse_size = delayed_parse.size();

    // parse instructions
    if ( l ) {
        if ( l->next ) { // -> block
            data << IR_TOK_BLOCK;
            push_offset( l );
            for( ; l; l = l->next )
                push_delayed_parse( l, false );
            // end of the block (offset = 0)
            data << 0;
        } else // -> single instruction
            parse_lexem( l );
    } else {
        data << IR_TOK_VOID;
        push_offset( l );
    }

    // delayed parse
    while ( delayed_parse.size() > old_delayed_parse_size ) {
        DelayedParse dp = delayed_parse.pop_back_val();
        *dp.offset = data.size() - dp.old_size;

        if ( dp.want_siblings )
            parse_lexem_and_siblings( dp.l );
        else
            parse_lexem( dp.l );
    }
}

void IrWriter::push_delayed_parse( const Lexem *l, bool want_siblings ) {
    int_to_reduce << IntToReduce{ data.size(), IntToReduce::OFFSET };

    if ( l ) {
        DelayedParse *dp = delayed_parse.push_back();
        dp->want_siblings = want_siblings;
        dp->old_size = data.size();
        dp->offset = data.room_for();
        dp->l = l;

        *dp->offset = 0;
    } else {
        OffsetType *o = data.room_for();
        *o = 0;
    }
}

void IrWriter::parse_lexem( const Lexem *l ) {
    switch( l->type )  {
    case Lexem::VARIABLE            : PARSING( Variable ); return parse_variable        ( l );
    case Lexem::NUMBER              : PARSING( Number   ); return parse_number          ( l );
    case Lexem::STRING              : PARSING( String   ); return parse_string          ( l );
    case Lexem::CR                  : PARSING( Cr       ); return;                  
    case Lexem::PAREN               : PARSING( Paren    ); return parse_paren           ( l ); // parse_lexem_and_siblings( l->children[ 0 ], false );
    case Lexem::CCODE               : PARSING( CCode    ); TODO;
    case Lexem::BLOCK               : PARSING( Block    ); TODO;
    case Lexem::NONE                : PARSING( None     ); return;
    case Lexem::APPLY               : PARSING( Apply    ); return parse_apply           ( l, IR_TOK_APPLY  );
    case Lexem::SELECT              : PARSING( Select   ); return parse_apply           ( l, IR_TOK_SELECT );
    case Lexem::CHANGE_BEHAVIOR     : PARSING( ChBeHa   ); return parse_apply           ( l, IR_TOK_CHBEBA );
    case STRING_assign_NUM          : PARSING( Assign   ); return parse_assign          ( l );
    case STRING_assign_type_NUM     : PARSING( AssType  ); return parse_assign          ( l, true );
    case STRING_reassign_NUM        : PARSING( Reassign ); return parse_reassign        ( l );
    case STRING___def___NUM         : PARSING( Def      ); return parse_def             ( l );
    case STRING___class___NUM       : PARSING( Class    ); return parse_class           ( l );
    case STRING___if___NUM          : PARSING( If       ); return parse_if              ( l );
    case STRING___else___NUM        : PARSING( Else     ); ERROR( "weird" );
    case STRING___while___NUM       : PARSING( While    ); return parse_while           ( l );
    case STRING___return___NUM      : PARSING( Return   ); return parse_return          ( l );
    case STRING_get_attr_NUM        : PARSING( GetAttr  ); return parse_get_attr        ( l, IR_TOK_GET_ATTR         );
    case STRING_get_attr_ptr_NUM    : PARSING( GetAttr  ); return parse_get_attr        ( l, IR_TOK_GET_ATTR_PTR     );
    case STRING_get_attr_ask_NUM    : PARSING( GetAttr  ); return parse_get_attr        ( l, IR_TOK_GET_ATTR_ASK     );
    case STRING_get_attr_ptr_ask_NUM: PARSING( GetAttr  ); return parse_get_attr        ( l, IR_TOK_GET_ATTR_PTR_ASK );
    case STRING_doubledoubledot_NUM : PARSING( DDDot    ); return parse_get_attr        ( l, IR_TOK_GET_ATTR_PA      );
    case STRING___static___NUM      : PARSING( Static   ); return parse_static          ( l );
    case STRING___const___NUM       : PARSING( Const    ); return parse_const           ( l );
    case STRING___for___NUM         : PARSING( For      ); return parse_for             ( l );
    case STRING___import___NUM      : PARSING( Import   ); return parse_import          ( l );
    case STRING___new___NUM         : PARSING( New      ); return parse_apply           ( l->children[ 0 ], IR_TOK_NEW );
    case STRING_lambda_NUM          : PARSING( New      ); return parse_lambda          ( l );
    case STRING___and___NUM         : PARSING( New      ); return parse_and             ( l );
    case STRING___or___NUM          : PARSING( New      ); return parse_or              ( l );
    default:
        PARSING( CallOp );
        return parse_call_op( l );
    }
}

void IrWriter::int_reduction() {
    if ( int_to_reduce.size() == 0 )
        return;

    // copy of data
    Vec<PI8> dcp( Size(), data.size() );
    data.copy_to( dcp.ptr() );

    // get needed size for each int to reduce
    Vec<int> needed_size( Size(), int_to_reduce.size(), 8 );

    while ( true ) {
        // since offset are >= 0, one iteration should be enough
        bool changed = false;
        for( ST i = int_to_reduce.size() - 1; i >= 0; --i ) {
            const IntToReduce &itr = int_to_reduce[ i ];
            if ( itr.type == IntToReduce::OFFSET ) {
                // get "real" offset
                while ( true ) {
                    ST off = *reinterpret_cast<const PI64 *>( dcp.ptr() + itr.pos ), lim = itr.pos + off;
                    for( int j = i; j < int_to_reduce.size() and int_to_reduce[ j ].pos < lim; ++j )
                        off -= 8 - needed_size[ j ];
                    int ns = BinStreamWriter::size_needed_for( off );
                    if ( needed_size[ i ] != ns ) {
                        needed_size[ i ] = ns;
                        changed = true;
                    } else
                        break;
                }
            } else
                TODO;
        }
        if ( not changed )
            break;
    }

    //
    data.clear();
    data.write( dcp.ptr(), int_to_reduce[ 0 ].pos );
    for( ST i = 0; i < int_to_reduce.size(); ++i ) {
        const IntToReduce &itr = int_to_reduce[ i ];
        if ( itr.type == IntToReduce::OFFSET ) {
            SI64 off = *reinterpret_cast<const SI64 *>( dcp.ptr() + itr.pos ), lim = itr.pos + off;
            for( int j = i; j < int_to_reduce.size() and int_to_reduce[ j ].pos < lim; ++j )
                off -= 8 - needed_size[ j ];

            data << off;
            int a = itr.pos + 8;
            int b = i + 1 < int_to_reduce.size() ? int_to_reduce[ i + 1 ].pos : dcp.size();
            data.write( dcp.ptr() + a, b - a );
        }
    }
}

static int num_primitive( const Lexem *l ) {
    #define DECL_IR_TOK( PN ) \
    if ( l->len == 3 + strlen( #PN ) and strncmp( "___" #PN, l->beg, l->len ) == 0 ) { \
        return IR_TOK_##PN; \
    }
    #include "../Ir/Decl_Primitives.h"
    #include "../Ir/Decl_Operations.h"
    #undef DECL_IR_TOK

    return -1;
}


void IrWriter::parse_apply( const Lexem *l, PI8 tok ) {
     // get nb_..._children and ch list
     SplittedVec<const Lexem *,16> ch;
     int nb_unnamed_children = 0, nb_named_children = 0;
     if ( l->children[ 1 ] ) {
         get_children_of_type( next_if_CR( l->children[ 1 ] ), STRING_comma_NUM, ch );
         for( ; nb_unnamed_children < ch.size() and ch[ nb_unnamed_children ]->type != STRING_reassign_NUM; ++nb_unnamed_children );
         for( int i=nb_unnamed_children; i < ch.size(); ++i, ++nb_named_children ) {
             if ( ch[ i ]->type != STRING_reassign_NUM ) {
                 add_error( "after a named argument, all arguments must be named", ch[ i ] );
                 return;
             }
         }
     }

     // particular case : assertion
     const Lexem *f = l->children[ 0 ] ? l->children[ 0 ] : l;
     if ( nb_unnamed_children == 2 and nb_named_children == 0 and f->type == Lexem::VARIABLE and f->len == 6 and strncmp( f->beg, "assert", 6 ) == 0 ) {
         TODO;
         //        // cond
         //        push_type_and_offsets( ch[0], Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1);
         //        push_tok_variable( ch[0], STRING_not_boolean_NUM );
         //        push_tok( ch[0], true ); // cond
         //        push_tok_apply( ch[0], 1, true );

         //        // if
         //        unsigned os = bin.size();
         //        push_type_and_offsets( ch[0], Tok::get_IF_OR_WHILE_type(false), 0 );
         //        bin.binary_write<unsigned>( 0 ); // unsigned *offset_if_fail
         //        unsigned *offset_if_not_executed = bin.binary_write<unsigned>( 0 );

         //        //
         //        push_tok_variable( ch[1], STRING_throw_failed_assertion_NUM );
         //        push_tok( ch[1], true );
         //        push_tok_apply( ch[1], 1, false );

         //        // end if
         //        push_type_and_offsets( ch[0], Tok::END_OF_AN_IF_BLOCK, 0 );
         //        *offset_if_not_executed = bin.size() - os;
         //        return;
     }

     // break n
     if ( tok == IR_TOK_APPLY and f->eq( "break" ) ) {
         if ( ch.size() > 1 )
             return add_error( "break accept at most 1 parameter", f );
         data << IR_TOK_BREAK;
         push_offset( f );
         int n = 1;
         if ( ch.size() == 1 ) {
             if ( not ch[ 0 ]->is_an_int() )
                 return add_error( "break expects a known integer value", f );
             n = ch[ 0 ]->to_int();
         }
         data << n; // nb of loop to break
     }

     // primitive ?
     int np = tok == IR_TOK_APPLY ? num_primitive( f ) : -1;
     if ( np >= 0 ) {
         data << np;
         push_offset( l );

         data << nb_unnamed_children;
         for( int i = 0; i < nb_unnamed_children; ++i )
             push_delayed_parse( ch[ i ] );

         if ( nb_named_children )
             add_error( "primitives do not take named arguments", f );

     } else {
         // instruction
         data << tok;
         push_offset( l );

         // function
         push_delayed_parse( f );

         // unnamed arguments
         data << nb_unnamed_children;
         for( int i = 0; i < nb_unnamed_children; ++i )
             push_delayed_parse( ch[ i ] );

         // named arguments
         data << nb_named_children;
         for( int i = 0; i < nb_named_children; ++i ) {
             push_nstring( ch[ nb_unnamed_children + i ]->children[ 0 ] );
             push_delayed_parse( ch[ nb_unnamed_children + i ]->children[ 1 ] );
         }

     }
}

void IrWriter::parse_number( const Lexem *l ) {
     if ( l->len and l->beg[ l->len - 1 ] == 'l' ) {
         data << IR_TOK_SI64;
         push_offset( l );
         data << l->to_PI64();
         return;
     }

     //
     if ( l->len and l->beg[ l->len - 1 ] == 'p' ) {
         data << IR_TOK_PTR;
         push_offset( l );
         data << l->to_PI64();
         return;
     }

     //
     if ( l->is_an_int() ) {
         data << IR_TOK_SI32;
         push_offset( l );
         data << l->to_int();
         return;
     }

     // else
     TODO;
}

void IrWriter::parse_string( const Lexem *l ) {
     std::string r;
     for( int i = 0; i < l->len; ++i ) {
         if ( l->beg[ i ] == '\\' and i + 1 < l->len ) {
             if ( l->beg[ i + 1 ] == '\\' ) { r += '\\'; ++i; continue; }
             if ( l->beg[ i + 1 ] ==  'n' ) { r += '\n'; ++i; continue; }
             if ( l->beg[ i + 1 ] ==  'r' ) { r += '\r'; ++i; continue; }
             if ( l->beg[ i + 1 ] ==  '"' ) { r +=  '"'; ++i; continue; }
         }
         r += l->beg[ i ];
     }
     //
     data << IR_TOK_STRING;
     push_offset( l );
     data << r.size();
     data.write( r.data(), r.size() );

}

static int dim_nb_cr( const Lexem *l, bool has_cd ) {
    if ( has_cd )
        return l->nb_preceding_comma_dot ? 1 : l->nb_preceding_cr + ( l->nb_preceding_cr != 0 );
    return l->nb_preceding_cr;
}

void IrWriter::output_list( const Lexem *l, int nb_dim, bool has_cd ) {
    // ... , ...
    if ( nb_dim == 1 ) {
        SplittedVec<const Lexem *,8> ch;
        get_children_of_type( l, STRING_comma_NUM, ch );

        data << ch.size();
        for( int i = 0; i < ch.size(); ++i )
            push_delayed_parse( ch[ i ], false );
    } else {
        // get nb_values
        int nb_values = 1;
        for( const Lexem *t = l->next; t and dim_nb_cr( t, has_cd ) < nb_dim; t = t->next )
            nb_values += dim_nb_cr( t, has_cd ) == nb_dim - 1;
        data << nb_values;

        // output children values
        for( const Lexem *t = l; t and dim_nb_cr( t, has_cd ) < nb_dim; t = t->next ) {
            if ( dim_nb_cr( t, has_cd ) == nb_dim - 1 ) {
                output_list( l, nb_dim - 1, has_cd );
                l = t;
            }
        }
        output_list( l, nb_dim - 1, has_cd );
    }
}

void IrWriter::parse_paren( const Lexem *l ) {
    // []
    if ( *l->beg == '[' ) {
        l = l->children[ 0 ];
        while ( l and l->type == Lexem::BLOCK )
            l = l->children[ 0 ];

        // get nb_dim
        int nb_dim = 0;
        bool has_cd = false;
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
        data << IR_TOK_LIST;
        push_offset( l );
        data << nb_dim;
        output_list( l, nb_dim, has_cd );
        return;
    }

    // else
    return parse_lexem_and_siblings( l->children[ 0 ] );
}

void IrWriter::parse_variable( const Lexem *l ) {
    if ( l->eq( "true" ) ) {
        data << IR_TOK_TRUE;
        push_offset( l );
    } else if ( l->eq( "false" ) ) {
        data << IR_TOK_FALSE;
        push_offset( l );
    //} else if ( l->eq( "self" ) ) {
    //    data << IR_TOK_SELF;
    //    push_offset( l );
    } else if ( l->eq( "this" ) ) {
        data << IR_TOK_THIS;
        push_offset( l );
    } else if ( l->eq( "continue" ) ) {
        data << IR_TOK_CONTINUE;
        push_offset( l );
        data << 1; // nb of loop to continue
    } else if ( l->eq( "break" ) ) {
        data << IR_TOK_BREAK;
        push_offset( l );
        data << 1; // nb of loop to break
    } else if ( l->eq( "null_ref" ) ) {
        data << IR_TOK_NULL_REF;
        push_offset( l );
    } else {
        Vec<CatchedVar> cvl;
        find_needed_var( cvl, l );
        if ( cvl.size() ) {
            int r = 0;
            bool surdef = false;
            for( CatchedVar cv : cvl ) {
                if ( cv.l->num_scope ) {
                    surdef |= cv.surdef;
                    ++r;
                }
            }
            if ( surdef ) {
                data << IR_TOK_VAR_SET;
                push_offset( l );
                data << r;
                push_nstring( l );
            }
            for( CatchedVar cv : cvl ) {
                if ( cv.l->num_scope ) {
                    if ( cv.s >= 0 ) {
                        // -> in catched vars of cv.l
                        data << IR_TOK_VAR_IN_CATCHED_VARS;
                        if ( not surdef )
                            push_offset( l );
                        data << cv.s;
                    } else {
                        // -> in local or static scope
                        if ( cv.l->scope_type & Lexem::SCOPE_TYPE_CLASS )
                            TODO;
                        data << ( cv.l->scope_type & Lexem::SCOPE_TYPE_STATIC ? IR_TOK_VAR_IN_STATIC_SCOPE : IR_TOK_VAR_IN_LOCAL_SCOPE );
                        if ( not surdef )
                            push_offset( l );
                        data << l->num_scope - cv.l->num_scope;
                        data << cv.l->num_in_scope;
                    }
                    if ( not surdef )
                        return;
                }
            }
            if ( surdef )
                return;
        }

        data << PI8( IR_TOK_VAR );
        push_offset( l );
        push_nstring( l );
    }
}

void IrWriter::parse_assign( const Lexem *l, bool assign_type ) {
     const Lexem *c = l->children[ 1 ];
     bool ref = c->type == STRING___ref___NUM;
     if ( ref )
         c = c->children[ 0 ];

     // LHS
     if ( c->num_scope ) {
         data << IR_TOK_PUSH_IN_SCOPE;
         push_offset( l );
     } else {
         data << IR_TOK_ASSIGN;
         push_offset( l );
         push_nstring( l->children[ 0 ] );
     }

     data << ref         * IR_ASSIGN_REF +
             static_inst * IR_ASSIGN_STATIC +
             const_inst  * IR_ASSIGN_CONST +
             assign_type * IR_ASSIGN_TYPE;

     // RHS
     push_delayed_parse( c );
}

void IrWriter::parse_reassign( const Lexem *l ) {
    data << IR_TOK_REASSIGN;
    push_offset( l );
    push_delayed_parse( l->children[ 0 ] );
    push_delayed_parse( l->children[ 1 ] );
}

void IrWriter::parse_def( const Lexem *l ) {
    parse_callable( l, IR_TOK_DEF );
}

void IrWriter::parse_class( const Lexem *l ) {
    parse_callable( l, IR_TOK_CLASS );
}

void IrWriter::parse_if( const Lexem *l ) {
    data << IR_TOK_IF;
    push_offset( l );

    push_delayed_parse( child_if_block( l->children[ 0 ] ) ); // condition
    if ( l->children[ 1 ]->type == STRING___else___NUM ) {
        push_delayed_parse( child_if_block( l->children[ 1 ]->children[ 0 ] ) ); // ok
        push_delayed_parse( child_if_block( l->children[ 1 ]->children[ 1 ] ) ); // ko
    } else {
        push_delayed_parse( child_if_block( l->children[ 1 ] ) ); // ok
        push_delayed_parse(                 0                  ); // ko
    }
}

void IrWriter::parse_while( const Lexem *l ) {
    data << IR_TOK_WHILE;
    push_offset( l );

    if ( l->children[ 0 ]->type == STRING___else___NUM ) {
        push_delayed_parse( child_if_block( l->children[ 0 ]->children[ 0 ] ) ); // ok
        push_delayed_parse( child_if_block( l->children[ 0 ]->children[ 1 ] ) ); // ko
    } else {
        push_delayed_parse( child_if_block( l->children[ 0 ] ) ); // ok
        push_delayed_parse(                                  0 ); // ko
    }
}

void IrWriter::parse_return( const Lexem *l ) {
    data << IR_TOK_RETURN;
    push_offset( l );
    push_delayed_parse( l->children[ 0 ] );
}

void IrWriter::parse_get_attr( const Lexem *l, PI8 tok ) {
    data << tok;
    push_offset( l );
    push_delayed_parse( l->children[ 0 ] );
    push_nstring( l->children[ 1 ] );
}

void IrWriter::parse_static( const Lexem *l ) {
    bool o = static_inst;
    static_inst = true;
    parse_lexem_and_siblings( l->children[ 0 ] );
    static_inst = o;
}

void IrWriter::parse_const( const Lexem *l ) {
    bool o = const_inst;
    const_inst = true;
    parse_lexem_and_siblings( l->children[ 0 ] );
    const_inst = o;
}

void IrWriter::parse_call_op( const Lexem *l ) {
    // hum, bad particular case due to Lexer badness
    if ( l->type == STRING_break_NUM ) {
        data << IR_TOK_BREAK;
        push_offset( l );
        data << 1; // nb of loop to break
        return;
    }


    // operation( a, b )
    data << IR_TOK_APPLY;
    push_offset( l );

    // offset to operation
    int_to_reduce << IntToReduce{ data.size(), IntToReduce::OFFSET };
    ST old = data.size();
    PI64 *off = data.room_for();

    // unnamed children
    data << 1 + bool( l->children[ 1 ] );
    push_delayed_parse( l->children[ 0 ] );
    if ( l->children[ 1 ] )
        push_delayed_parse( l->children[ 1 ] );

    // named children;
    data << 0;

    // operation
    *off = data.size() - old;
    data << PI8( IR_TOK_VAR );
    push_offset( l );
    push_nstring( nstring( get_operators_cpp_name( l->type ) ) );
}

void IrWriter::parse_for( const Lexem *l ) {
    if ( l->children[ 0 ]->type != STRING___in___NUM ) {
        add_error( "syntax of 'for' is 'for ... in ... ...'.", l->children[ 0 ] );
        return;
    }

    // names
    SplittedVec<const Lexem *,8> names;
    get_children_of_type( l->children[ 0 ]->children[ 0 ], STRING_comma_NUM, names );

    SplittedVec<const Lexem *,8> ch;
    get_children_of_type( l->children[ 0 ]->children[ 1 ], STRING_comma_NUM, ch );

    // catched vars
    std::map<String,CatchedVarWithNum> &catched_vars = catched[ l ];
    get_needed_var_rec( catched_vars, l->children[ 1 ], l->num_scope );

    //
    data << IR_TOK_FOR;
    push_offset( l );

    data << names.size();
    for( int i = 0; i < names.size(); ++i ) {
        if ( names[ i ]->type != Lexem::VARIABLE )
            TODO;
        push_nstring( names[ i ] );
    }

    data << ch.size();
    for( int i = 0; i < ch.size(); ++i )
        push_delayed_parse( ch[ i ] );

    out_catched_vars( catched_vars, l->num_scope );

    push_delayed_parse( child_if_block( l->children[ 1 ] ) );
}

void IrWriter::out_catched_vars( std::map<String,CatchedVarWithNum> &catched_vars, int num_scope ) {
    data << catched_vars.size();
    for( auto it : catched_vars ) {
        Vec<CatchedVar> &cl = it.second.cv;
        if ( cl[ 0 ].surdef ) {
            TODO;
        } else {
            CatchedVar cv = cl[ 0 ];
            if ( cv.s >= 0 ) { // in catched vars of a parent callable
                data << PI8( IN_CATCHED_VARS );
                data << cv.s;
            } else if ( cv.s == -2 ) { // self
                data << PI8( IN_SELF );
            } else {
                if ( cv.l->attribute ) {
                    data << PI8( cv.l->scope_type & Lexem::SCOPE_TYPE_STATIC ? IN_STATIC_ATTR : IN_LOCAL_ATTR );
                    data << 0;
                    data << cv.l->num_in_scope;
                } else {
                    data << PI8( cv.l->scope_type & Lexem::SCOPE_TYPE_STATIC ? IN_STATIC_SCOPE : IN_LOCAL_SCOPE );
                    data << num_scope - cv.l->num_scope; // nb parents
                    data << cv.l->num_in_scope;
                }
            }
        }
    }
}

void IrWriter::parse_import( const Lexem *l ) {
    data << IR_TOK_IMPORT;
    push_offset( l );

    data << l->children[ 0 ]->len;
    data.write( l->children[ 0 ]->beg, l->children[ 0 ]->len );
}

void IrWriter::parse_lambda( const Lexem *l ) {
    data << IR_TOK_LAMBDA;
    push_offset( l );

    // names
    SplittedVec<const Lexem *,8> ch;
    get_children_of_type( l->children[ 0 ], STRING_comma_NUM, ch );
    data << ch.size();
    for( int i = 0; i < ch.size(); ++i )
        push_nstring( ch[ i ] );

    // code
    push_delayed_parse( l->children[ 1 ] );
}

void IrWriter::parse_and( const Lexem *l ) {
    data << IR_TOK_AND;
    push_offset( l );
    push_delayed_parse( l->children[ 0 ] );
    push_delayed_parse( l->children[ 1 ] );
}

void IrWriter::parse_or( const Lexem *l ) {
    data << IR_TOK_OR;
    push_offset( l );
    push_delayed_parse( l->children[ 0 ] );
    push_delayed_parse( l->children[ 1 ] );
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

void IrWriter::find_needed_var( Vec<CatchedVar> &cl, const Lexem *v ) {
    const Lexem *b = v;
    if ( b->prev )
        b = b->prev;
    else {
        b = b->parent;
        if ( b )
            while ( b->next )
                b = b->next;
    }

    for( ; b; ) {
        const Lexem *a = b;
        while ( true ) {
            if ( a->type == STRING___static___NUM ) { a = a->children[ 0 ]; continue; }
            if ( a->type == STRING___const___NUM  ) { a = a->children[ 0 ]; continue; }
            break;
        }

        if ( a->type == STRING_assign_NUM or a->type == STRING_assign_type_NUM ) {
            // foo := ...
            if ( a->children[ 0 ]->same_str( v->beg, v->len ) )
                cl << CatchedVar{ a->children[ 0 ], -1, false };
        } else if ( a->type == STRING___def___NUM or a->type == STRING___class___NUM ) {
            // def foo / class foo
            const Lexem *c = a->children[ 0 ];
            while ( true ) {
                if ( c->type == STRING___extends___NUM    ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___starts_with___NUM ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___pertinence___NUM ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___when___NUM       ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING_doubledot_NUM      ) { c = c->children[ 0 ]; continue; }
                break;
            }
            if ( c->type == Lexem::APPLY or c->type == Lexem::SELECT )
                c = c->children[ 0 ];

            // name of the callable itself ?
            if ( c->same_str( v->beg, v->len ) )
                cl << CatchedVar{ c, -1, true };
        }

        // in args or catched vars of a for ?
        if ( b->parent and b->parent->type == STRING___for___NUM ) {
            const Lexem *t = b->parent, *c = t->children[ 0 ];

            // in args ?
            SplittedVec<const Lexem *,8> tl;
            get_children_of_type( c->children[ 0 ], STRING_comma_NUM, tl );
            for( int i = 0; i < tl.size(); ++i ) {
                const Lexem *t = tl[ i ];
                if ( t->same_str( v->beg, v->len ) )
                    cl << CatchedVar{ t, -1, false };
            }

            // in catched vars ?
            auto it = catched[ t ].find( String( v->beg, v->beg + v->len ) );
            if ( it != catched[ t ].end() )
                cl << CatchedVar{ t, it->second.num, false };

            // looking for self
            if ( v->eq( "self" ) )
                cl << CatchedVar{ t, -2, false };
        }

        // in args or catched vars of a callable ?
        if ( b->parent and ( b->parent->type == STRING___def___NUM or b->parent->type == STRING___class___NUM ) ) {
            const Lexem *t = b->parent, *c = t->children[ 0 ];

            while ( true ) {
                if ( c->type == STRING___extends___NUM    ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___starts_with___NUM ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___pertinence___NUM ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___when___NUM       ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING_doubledot_NUM      ) { c = c->children[ 0 ]; continue; }
                break;
            }

            // in args ?
            if ( c->type == Lexem::APPLY or c->type == Lexem::SELECT ) {
                SplittedVec<const Lexem *,8> tl;
                get_children_of_type( c->children[ 1 ], STRING_comma_NUM, tl );
                for( int i = 0; i < tl.size(); ++i ) {
                    const Lexem *t = tl[ i ];
                    if ( t->type == STRING_reassign_NUM  ) t = t->children[ 0 ];
                    if ( t->type == STRING_doubledot_NUM ) t = t->children[ 0 ];

                    if ( t->same_str( v->beg, v->len ) )
                        cl << CatchedVar{ t, -1, false };
                }
                c = c->children[ 0 ];
            }

            // in catched vars ?
            auto it = catched[ t ].find( String( v->beg, v->beg + v->len ) );
            if ( it != catched[ t ].end() )
                cl << CatchedVar{ t, it->second.num, false };

            // looking for self, in "def ..." that is a method ?
            if ( v->eq( "self" ) and is_a_method( t ) )
                cl << CatchedVar{ t, -2, false };
        }

        if ( b->prev )
            b = b->prev;
        else {
            b = b->parent;
            if ( b )
                while ( b->next )
                    b = b->next;
        }
    }
}

void IrWriter::get_needed_var_rec( std::map<String,CatchedVarWithNum> &vars, const Lexem *b, int onp ) {
    for( ; b; b = b->next ) {
        if ( b->type == STRING_assign_NUM or b->type == STRING_assign_type_NUM ) {
            if ( b->children[ 1 ] )
                get_needed_var_rec( vars, b->children[ 1 ], onp );
        } else if ( b->type == STRING___def___NUM or b->type == STRING___class___NUM ) {
            const Lexem *c = b->children[ 0 ];
            while ( true ) {
                if ( c->type == STRING___extends___NUM    ) { get_needed_var_rec( vars, c->children[ 1 ], onp ); c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___starts_with___NUM ) { get_needed_var_rec( vars, c->children[ 1 ], onp ); c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___pertinence___NUM ) { get_needed_var_rec( vars, c->children[ 1 ], onp ); c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___when___NUM       ) { get_needed_var_rec( vars, c->children[ 1 ], onp ); c = c->children[ 0 ]; continue; }
                if ( c->type == STRING_doubledot_NUM      ) { get_needed_var_rec( vars, c->children[ 1 ], onp ); c = c->children[ 0 ]; continue; }
                break;
            }

            if ( c->type == Lexem::APPLY or c->type == Lexem::SELECT ) {
                SplittedVec<const Lexem *,8> tl;
                get_children_of_type( c->children[ 1 ], STRING_comma_NUM, tl );
                for( int i = 0; i < tl.size(); ++i ) {
                    const Lexem *t = tl[ i ];
                    if ( t->type == STRING_reassign_NUM  ) { get_needed_var_rec( vars, t->children[ 1 ], onp ); t = t->children[ 0 ]; }
                    if ( t->type == STRING_doubledot_NUM ) { get_needed_var_rec( vars, t->children[ 1 ], onp ); t = t->children[ 0 ]; }
                }
            }

            get_needed_var_rec( vars, b->children[ 1 ], onp );
        } else if ( b->type == Lexem::VARIABLE ) {
            Vec<CatchedVar> cl;
            find_needed_var( cl, b );
            if ( cl.size() ) {
                if ( cl[ 0 ].l->num_scope and cl[ 0 ].l->num_scope <= onp ) {
                    String str( b->beg, b->beg + b->len );
                    if ( not vars.count( str ) ) {
                        int os = vars.size();
                        vars[ str ] = CatchedVarWithNum{ cl, os };
                    }
                }
            }
        } else {
            if ( b->children[ 0 ] ) get_needed_var_rec( vars, b->children[ 0 ], onp );
            if ( b->children[ 1 ] ) get_needed_var_rec( vars, b->children[ 1 ], onp );
        }
    }
}

/**
 *   (c)def +-> base_size_... +-> extends    +-> pertinence +-> when       +-> :         +-> apply     +-> name
 *          |                 |              |              |              |             |             |-> args
 *          |                 |              |              |              |             |-> return_type
 *          |                 |              |              |              |-> condition
 *          |                 |              |              +-> pert value
 *          |                 |              +-> inheritance
 *          |                 +-> block
 *          +-> block
 */
void IrWriter::parse_callable( const Lexem *t, PI8 token_type ) {
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
                if ( dv->type == STRING_doubledot_NUM ) {
                    add_error( "Syntax for argument with type constraint and default value is 'name_arg : typectraint = default_value' (in this specific order).", dv );
                    return;
                }
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
    if ( name->children[ 1 ] or ( name->children[ 0 ] and name->children[ 0 ]->type != Lexem::PAREN ) ) {
        add_error( "syntax error.", name );
        return;
    }

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
    if ( token_type == IR_TOK_DEF ) {
        if ( name->type == Lexem::VARIABLE and name->begin_with( "get_" ) ) { get_beg = name->beg + 4; get_len = name->len - 4; }
        if ( name->type == Lexem::VARIABLE and name->begin_with( "set_" ) ) { set_beg = name->beg + 4; set_len = name->len - 4; }
        if ( name->type == Lexem::VARIABLE and name->begin_with( "sop_" ) ) { sop_beg = name->beg + 4; sop_len = name->len - 4; }
    }

    // variables to catch
    std::map<String,CatchedVarWithNum> &catched_vars = catched[ t ];
    get_needed_var_rec( catched_vars, block, name->num_scope );

    if ( is_a_method( t ) ) {
        if ( not catched_vars.count( "self" ) ) {
            CatchedVarWithNum &cn = catched_vars[ "self" ];
            cn.num = catched_vars.size() - 1;
            cn.cv = CatchedVar{ 0, -2, false };
        }
    }

    // output --------------------------------------------------------------
    data << token_type;
    push_offset( t );

    push_nstring( name );
    data << bool( self_as_arg ) * IR_SELF_AS_ARG +
            bool( varargs     ) * IR_VARARGS +
            bool( abstract    ) * IR_ABSTRACT +
            bool( return_type ) * IR_HAS_RETURN_TYPE +
            bool( pertinence  ) * IR_HAS_COMPUTED_PERT +
            bool( condition   ) * IR_HAS_CONDITION +
            bool( default_pertinence_num < 0 ) * IR_NEG_PERT +
            bool( get_len ) * IR_IS_A_GET +
            bool( set_len ) * IR_IS_A_SET +
            bool( sop_len ) * IR_IS_A_SOP;
    data << arguments.size();
    data << nb_default_args;
    if ( not pertinence ) {
        data << abs( default_pertinence_num );
        data << default_pertinence_den;
    }
    for( int i = 0; i < arguments.size(); ++i ) {
        if ( arguments[ i ].name->type == Lexem::VARIABLE )
            push_nstring( arguments[ i ].name );
        else
            TODO;

        // nb_constraints + constraints
        if ( arguments[ i ].type_constraint ) {
            if ( arguments[ i ].type_constraint->type == Lexem::VARIABLE ) {
                data << 1;
                push_nstring( arguments[ i ].type_constraint );
            } else if ( arguments[ i ].type_constraint->type == Lexem::PAREN ) {
                SplittedVec<const Lexem *,8> ch;
                get_leaves( ch, arguments[ i ].type_constraint );
                data << ch.size();
                for( int j = 0; j < ch.size(); ++j ) {
                    if ( ch[ j ]->type == Lexem::VARIABLE )
                        push_nstring( ch[ j ] );
                    else
                        return add_error( "constraints must be simple variables", ch[ j ] );
                }
            } else
                TODO;
        } else
            data << 0;
    }
    for( int i = 0; i < arguments.size(); ++i )
        if ( arguments[ i ].default_value )
            push_delayed_parse( arguments[ i ].default_value );
    if ( pertinence )
        push_delayed_parse( pertinence );
    if ( condition )
        push_delayed_parse( condition );

    // catched var data
    out_catched_vars( catched_vars, t->num_scope );

    //
    if ( token_type == IR_TOK_DEF ) {
        // block
        push_delayed_parse( block );

        // return type
        if ( return_type )
            push_delayed_parse( return_type );

        if ( name->eq( "init" ) ) {
            data << starts_with.size();

            for( int i = 0; i < starts_with.size(); ++i ) {
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
                            return add_error( "after a named argument, all arguments must be named", ch[ i ] );
                }

                const Lexem *f = l->children[ 0 ] ? l->children[ 0 ] : l;

                // argument
                if ( f->type != Lexem::VARIABLE )
                    return add_error( "expecting attribute name", ch[ i ] );
                push_nstring( f );

                // unnamed arguments
                data << nb_unnamed_children;
                for( int i = 0; i < nb_unnamed_children; ++i )
                    push_delayed_parse( ch[ i ] );

                // named arguments
                data << nb_named_children;
                for( int i = 0; i < nb_named_children; ++i ) {
                    push_nstring( ch[ nb_unnamed_children + i ]->children[ 0 ] );
                    push_delayed_parse( ch[ nb_unnamed_children + i ]->children[ 1 ] );
                }
            }
        }

        // get set sop
        if ( get_len ) push_nstring( nstring( get_beg, get_beg + get_len ) );
        if ( set_len ) push_nstring( nstring( set_beg, set_beg + set_len ) );
        if ( sop_len ) push_nstring( nstring( sop_beg, sop_beg + sop_len ) );
    } else {
        // extends
        data << inheritance.size();
        for( int i = 0; i < inheritance.size(); ++i )
            push_delayed_parse( inheritance[ i ] );

        // methods
        Vec<std::pair<int,const Lexem *> > methods;
        for( const Lexem *l = block; l; l = l->next ) {
            while ( true ) {
                if ( l->type == STRING___virtual___NUM ) { l = l->children[ 0 ]; continue; }
                if ( l->type == STRING___static___NUM  ) { l = l->children[ 0 ]; continue; }
                break;
            }

            if ( l->type == STRING___def___NUM ) {
                const Lexem *c = l->children[ 0 ];
                while ( true ) {
                    if ( c->type == STRING___extends___NUM    ) { c = c->children[ 0 ]; continue; }
                    if ( c->type == STRING___starts_with___NUM ) { c = c->children[ 0 ]; continue; }
                    if ( c->type == STRING___pertinence___NUM ) { c = c->children[ 0 ]; continue; }
                    if ( c->type == STRING___when___NUM       ) { c = c->children[ 0 ]; continue; }
                    if ( c->type == STRING_doubledot_NUM      ) { c = c->children[ 0 ]; continue; }
                    break;
                }
                if ( c->type == Lexem::APPLY or c->type == Lexem::SELECT )
                    c = c->children[ 0 ];

                methods << std::make_pair( nstring( c->beg, c->beg + c->len ), l );
            }
        }

        data << methods.size();
        for( std::pair<int,const Lexem *> attr : methods ) {
            push_nstring( attr.first );
            push_delayed_parse( attr.second, false );
        }

        // attributes
        struct Attr {
            int type;
            int name;
            const Lexem *code;
        };
        Vec<Attr> attributes;
        for( const Lexem *l = block; l; l = l->next ) {
            bool stat = false;
            while ( true ) {
                if ( l->type == STRING___virtual___NUM ) { l = l->children[ 0 ]; continue; }
                if ( l->type == STRING___static___NUM  ) { l = l->children[ 0 ]; stat = true; continue; }
                break;
            }

            switch ( l->type ) {
            case STRING_assign_NUM:
                attributes << Attr{ CALLABLE_ATTR_STATIC * stat, nstring( l->children[ 0 ]->beg, l->children[ 0 ]->beg + l->children[ 0 ]->len ), l->children[ 1 ] };
                break;
            case STRING_assign_type_NUM:
                attributes << Attr{ CALLABLE_ATTR_STATIC * stat + CALLABLE_ATTR_TYPE, nstring( l->children[ 0 ]->beg, l->children[ 0 ]->beg + l->children[ 0 ]->len ), l->children[ 1 ] };
                break;
            case STRING___class___NUM:
                // PRINT( *l->children[ 0 ] );
                TODO;
                break;
            case Lexem::APPLY:
                if ( l->children[ 0 ]->eq( "___set_base_size" ) ) {
                    attributes << Attr{ CALLABLE_ATTR_BASE_SIZE, 0, l->children[ 1 ] };
                    break;
                }
                if ( l->children[ 0 ]->eq( "___set_base_alig" ) ) {
                    attributes << Attr{ CALLABLE_ATTR_BASE_ALIG, 0, l->children[ 1 ] };
                    break;
                }
                add_error( "unexpected token (class block has a limited set of acceptable tokens)", l );
                break;
            case STRING___def___NUM:
                break;
            default:
                PRINT( l );
                add_error( "unexpected token (class block has a limited set of acceptable tokens)", l );
                break;
            }
        }

        data << attributes.size();
        for( Attr &attr : attributes  ) {
            data << attr.type;
            push_nstring( attr.name );
            push_delayed_parse( attr.code );
        }
    }
}

void IrWriter::add_error( String msg, const Lexem *l ) {
    ErrorList::Error &e = error_list.add( msg ).ac( l->beg_src, l->beg, l->src );
    std::cerr << e;
}

int IrWriter::nstring( const char *beg, const char *end ) {
    std::string str( beg, end );
    ASSERT( str.size(), "should not append" );
    auto iter = nstrings.find( str );
    if ( iter != nstrings.end() )
        return iter->second;
    int res = nstrings.size();
    nstrings[ str ] = res;
    return res;
}

int IrWriter::nstring( const char *beg ) {
    return nstring( beg, beg + strlen( beg ) );
}

int IrWriter::nstring( const Lexem *l ) {
    return nstring( l->beg, l->beg + l->len );
}

void IrWriter::push_nstring( const Lexem *l ) {
    push_nstring( nstring( l ) );
}


void IrWriter::push_nstring( int nstring_num ) {
    data << nstring_num;
}


void IrWriter::push_offset( const Lexem *l ) {
    data << ( l ? l->beg - l->beg_src : 0 );
}
