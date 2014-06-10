#include "../System/SplittedVec.h"
#include "../System/Stream.h"
#include "TypeChar.h"
#include "Lexer.h"

inline bool is_num     (char c) { return type_char( c ) == TYPE_CHAR_number;   }
inline bool is_letter  (char c) { return type_char( c ) == TYPE_CHAR_letter;   }
inline bool is_operator(char c) { return type_char( c ) == TYPE_CHAR_operator; }

Lexer::Lexer( ErrorList &error_list ) : error_list( error_list ) {
    first_tok.type = Lexem::CR;
    end = &first_tok;
}

void Lexer::parse( const char *beg, const char *src ) {
    approx_line = 0;
    spcr = 0;
    this->beg = beg;
    this->src = src ? src : "inline";
    if ( not beg )
        return;

    //
    for( int i = 0; i < NB_OPERATORS + Lexem::NB_BASE_TYPE; ++i ) {
        first_of_type[ i ] = 0;
        last_of_type [ i ] = 0;
    }

    // limit function
    int lif = STRING___alternative___NUM;
    int enf = STRING_calc_name_NUM - 1;

    // let's go
    s = beg;
    const Lexem *ft = end;
    make_flat_token_list();                     if ( error_list ) return;
    not_in_replacement();                       if ( error_list ) return;
    make_sibling_lists();                       if ( error_list ) return;
    parenthesis_hierarchisation( ft );          if ( error_list ) return;
    make_unit_hierarchy();                      if ( error_list ) return;
    assemble___string_assembly__();             if ( error_list ) return;
    assemble___calc_name__();                   if ( error_list ) return;
    assemble_par___getattr__( ft );             if ( error_list ) return;
    remove_cr();                                if ( error_list ) return;
    assemble_op( lif, enf );                    if ( error_list ) return;
    add_child_to_info( STRING___info___NUM );   if ( error_list ) return;
    add_child_to_info( STRING___infon___NUM );  if ( error_list ) return;
    make_calls_without_par();                   if ( error_list ) return;
    remove_comma_dots();                        if ( error_list ) return;
    assemble_op( 0, lif - 1 );                  if ( error_list ) return;
    update_else_order();                        if ( error_list ) return;
    remove_while_conds();                       if ( error_list ) return;
    set_num_scope();                            if ( error_list ) return;
}

void Lexer::display() {
    if ( first_tok.next )
        display_graph( first_tok.next, "tmp/res.dot", &first_tok );
}

Lexem *Lexer::root() {
    if ( error_list )
        return 0;
    return first_tok.next;
}

Lexem *Lexer::new_tok( int type, const char *beg, unsigned len, int num ) {
    Lexem *t = lexems.push_back();
    t->beg_src = this->beg;
    t->type    = type;
    t->src     = src;
    t->beg     = beg;
    t->len     = len;
    t->num     = num;

    t->approx_line = approx_line;
    t->spcr        = spcr;
    return t;
}

Lexem *Lexer::app_tok( int type, const char *beg, unsigned len, int num ) {
    Lexem *t = new_tok( type, beg, len, num );

    // ordered
    end->next = t;
    t->prev = end;
    t->next = 0;
    end = t;
    return t;
}

void Lexer::write_error( const std::string &msg, const char *beg, const char *pos, const std::string &src, bool due_to_not_ended ) {
    ErrorList::Error &error = error_list.add( msg, false, due_to_not_ended );
    error.ac( beg, pos, src );
    std::cerr << error;
}

void Lexer::read_dollar( const char *old_str, unsigned nb_spaces ) {
    // string before
    app_tok( Lexem::STRING, old_str, s - old_str, nb_spaces );

    // operator __string_assembly__
    app_tok( STRING___string_assembly___NUM, s, 1, get_operators_behavior( STRING___string_assembly___NUM ) );

    // tokens
    const char *e = ++s;
    if ( e[0]=='(' or e[0]=='[' or e[0]=='{' ) { // $(...)
        unsigned cpt = 1;
        while( (++e)[0] ) {
            if ( e[0]==')' or e[0]==']' or e[0]=='}' ) {
                if ( --cpt == 0 ) {
                    ++e;
                    break;
                }
            }
            else if ( e[0]=='(' or e[0]=='[' or e[0]=='{' )
                ++cpt;
        }
        if ( cpt ) {
            write_error( "a ')' is missing, ']' or '}'", beg, s, src );
            return;
        }
    }
    else { // $toto
        while ( (++e)[0] and ( type_char(e[0])==TYPE_CHAR_letter or type_char(e[0])==TYPE_CHAR_number ) ) ;
    }
    read_s<true>( e );

    // operator __string_assembly__
    app_tok( STRING___string_assembly___NUM, s, 1, get_operators_behavior( STRING___string_assembly___NUM ) );
}


template<bool stop_condition> void Lexer::read_s( const char *limit ) {
    while ( stop_condition == false or s < limit ) {
        switch ( type_char( *s ) ) {
            case TYPE_CHAR_space:
                ++s;
                break;
            case TYPE_CHAR_letter: {
                const char *old_str = s;
                while ( is_num( *( ++s ) ) or is_letter( *s ) );
                unsigned si = s - old_str;
                int num_op = num_operator_le( old_str, si ); // look for an operator
                app_tok( num_op, old_str, si, get_operators_behavior( num_op ) );
                break;
            }
            case TYPE_CHAR_operator: {
                const char *old_str = s;
                while ( is_operator( *( ++s ) ) );
                while ( s != old_str ) {
                    for( unsigned i=s-old_str; ; --i ) {
                        if ( not i ) {
                            write_error( "Unknown operator (or operator set) '" + std::string( old_str, s ) + "'.", beg, old_str, src );
                            return;
                        }
                        int num_op = num_operator_op( old_str, i ); // look for an operator
                        if ( num_op >= 0 ) {
                            app_tok( num_op, old_str, i, get_operators_behavior( num_op ) );
                            old_str += i;
                            /// f <<<< ...
                            if ( num_op == STRING_shift_left_long_str_NUM ) {
                                // get nb_spaces from current line
                                unsigned nb_spaces = 0;
                                const char *b = s;
                                while ( *(--b) and *b!='\n' and *b!='\r' );
                                while ( *(++b)==' ' ) nb_spaces++;

                                // skip spaces after <<<<
                                while ( *old_str == ' ' ) ++old_str;
                                if ( *old_str != '\n' and *old_str != '\r' ) {
                                    write_error( "After a '<<<<' and in the same line, only spaces and carriage returns are allowed.", beg, old_str, src );
                                    return;
                                }
                                old_str += ( *old_str == '\r' );
                                old_str += ( *old_str == '\n' );

                                //
                                s = old_str;
                                while ( true ) {
                                    unsigned new_nb_spaces = 0;
                                    while ( *s==' ' ) { ++s; ++new_nb_spaces; }
                                    if ( new_nb_spaces < nb_spaces + 4 ) {
                                        while ( *s and *s!='\n' and *s!='\r' ) --s; // return to beginning of the line
                                        break;
                                    }
                                    // else
                                    while ( *s and *s!='\n' and *s!='\r' ) ++s; // next line
                                    s += ( *s == '\r' );
                                    s += ( *s == '\n' );
                                }

                                // $...
                                const char *new_s = s;
                                s = old_str;
                                while ( s < new_s ) {
                                    if ( s[0] == '$' and s[-1] != '\\' and s[-1] != '$' and ( type_char(s[1])==TYPE_CHAR_letter or type_char(s[1])==TYPE_CHAR_parenthesis ) ) {
                                        read_dollar( old_str, nb_spaces + 4 );
                                        old_str = s - 1;
                                    }
                                    else
                                        s++;
                                }

                                // app_tok
                                app_tok( Lexem::STRING, old_str, s-old_str, nb_spaces + 4 );

                                // out loop
                                old_str = s;

                            }
                            break;
                        }
                    }
                }
                break;
            }
            case TYPE_CHAR_cr: {
                ++approx_line;
                spcr = 0;
                unsigned num = 0;
                while ( *( ++s ) ) {
                    if ( s[0] == '\t' ) {
                        write_error( "Tabulations are not allowed at the beginning of a line. Please use spaces only.", beg, s, src );
                        num += 3;
                    } else if ( s[0] != ' ' ) {
                        spcr = num;
                        app_tok( Lexem::CR, s, 0, num );
                        break;
                    }
                    ++num;
                }
                break;
            }
            case TYPE_CHAR_ccode: {
                const char *old_str = ++s;
                while ( *s and (*s != '`' or (s[-1] == '\\')) ) ++s;
                app_tok( Lexem::CCODE, old_str, s - old_str, -1 );
                if ( *s ) ++s;
                break;
            }
            case TYPE_CHAR_string: {
                const char *old_str = s;
                while ( *(++s) and (*s != '"' or (s[-1] == '\\' and s[-2] != '\\')) ) {
                    if ( s[0] == '$' and s[-1] != '$' and s[-1] != '\\' and ( type_char(s[1])==TYPE_CHAR_letter or type_char(s[1])==TYPE_CHAR_parenthesis ) ) {
                        read_dollar( old_str + 1 );
                        old_str = --s;
                    }
                }
                //
                if ( not *s )
                    write_error( "a closing '\"' is missing.", beg, old_str, src, true );
                else {
                    app_tok( Lexem::STRING, old_str+1, s-old_str-1, 0 );
                    ++s;
                }
                break;
            }
            case TYPE_CHAR_number: {
                const char *old_str = s;

                // LexemNumber *n = numbers.new_elem(); n->v.init(0);
                if ( s[ 0 ] == '0' and ( s[ 1 ] == 'x' or s[ 1 ] == 'X') ) { // 0xFFE -> hexadecimal
                    for( s += 2; is_num( s[0] == 0 ) or ( s[0]>='a' and s[0]<='f' ) or ( s[0]>='A' and s[0]<='F' ); ++s );
                    app_tok( Lexem::NUMBER, old_str, s - old_str, 0 );
                } else { // decimal number
                    while ( is_num( *( ++s ) ) );
                    if ( s[ 0 ] == '.' and is_num( s[ 1 ] ) )
                        while ( is_num( *( ++s ) ) );
                    if ( s[0] == 'e' or s[0] == 'E' ) { // numbers after [eE]
                        s += ( s[ 1 ] == '-' or s[ 1 ] == '+' );
                        while ( is_num( *( ++s ) ) );
                    }
                    // completion of attributes (unsigned, float, pointer, ...)
                    for( ; s[0] == 'u' or s[0] == 'i' or s[0] == 'p' or s[0] == 'l'; ++s );
                    // else if ( type_char(s[0])==TYPE_CHAR_letter )
                    // error_list->add( "Character '"+std::string(1,*s)+"' is not a known number attribute ('u' for unsigned, 'i' for imaginary ).", s, src );
                    app_tok( Lexem::NUMBER, old_str, s - old_str, 0 );
                }

                break;
            }
            case TYPE_CHAR_parenthesis: {
                if ( s[ 0 ] == '(' and s[ 1 ] == '*' and s[ 2 ] == ')' ) {
                    app_tok( STRING_tensorial_product_NUM, s, 3, get_operators_behavior( STRING_tensorial_product_NUM ) );
                    s += 3;
                } else // (...
                    app_tok( Lexem::PAREN, s++, 1, 0 );
                break;
            }
            case TYPE_CHAR_comment: {
                const char *old_str = s; // used if error
                if ( s[1] == '#' ) { // ## pouet ##
                    for( s += 2; ; ++s ) {
                        if ( not *s ) {
                            write_error( "Comment is not closed ( Ex: ## toto ## ).", beg, old_str, src, true );
                            return;
                        }
                        if ( s[ -1 ]=='#' and s[0]=='#' ) {
                            ++s;
                            break;
                        }
                    }
                } else if ( s[1] == '~' ) { // #~ pouet ~#
                    for( s += 2; ; ++s ) {
                        if ( not *s ) {
                            write_error( "Comment is not closed ( Ex: #~ toto ~# ).", beg, old_str, src, true );
                            return;
                        }
                        if ( s[ -1 ]=='~' and s[ 0 ]=='#' ) {
                            ++s;
                            break;
                        }
                    }
                } else {
                    while( *( ++s ) and s[ 0 ] != '\n' ) ; // looking for a carriage return
                }
                break;
            }
            case TYPE_CHAR_dos_makes_me_sick:
                ++s;
                break;
            case TYPE_CHAR_end:
                return;
            default:
                // error_list->add("Character is not allowed in this context.", s, src );
                ++s;
        }
    }
}


void Lexer::make_flat_token_list() {
    read_s<false>();
}

static int _not_in_replacement( int type ) {
    switch ( type ) {
    case STRING___in___NUM     : return STRING___not_in___NUM;
    case STRING___inst_of___NUM: return STRING___not_inst_of___NUM;
    default: return 0;
    }
}

void Lexer::not_in_replacement() {
    for( Lexem *i = root(); i; i = i->next ) {
        if ( i->type == STRING_not_boolean_NUM and i->next ) {
            if ( int nr = _not_in_replacement( i->next->type ) ) {
                i->type = nr;
                i->len = ( i->next->beg - i->beg ) + i->next->len;
                i->num = get_operators_behavior( nr );
                if ( i->next->next )
                    i->next->next->prev = i;
                i->next = i->next->next;
            }
        }
    }
}

void Lexer::make_sibling_lists() {
    for( Lexem *t = root(); t; t = t->next ) {
        int type = t->type;

        if ( last_of_type[ type + Lexem::NB_BASE_TYPE ] == NULL ) {
            first_of_type[ type + Lexem::NB_BASE_TYPE ] = t;
            last_of_type [ type + Lexem::NB_BASE_TYPE ] = t;
        } else {
            last_of_type [ type + Lexem::NB_BASE_TYPE ]->sibling = t;
            last_of_type [ type + Lexem::NB_BASE_TYPE ] = t;
        }
    }
}

void Lexer::parenthesis_hierarchisation( const Lexem *ft ) {
    SplittedVec<Lexem *,16> ps_stack;
    for( Lexem *i = ft->next; i; i = i->next ) {
        if ( i->type == Lexem::PAREN ) {
            if ( i->beg[ 0 ] == '(' or i->beg[ 0 ] == '[' or i->beg[ 0 ] == '{' ) // opening
                ps_stack.push_back( i );
            else { // closing parenthesis
                if ( ps_stack.size() == 0 or ps_stack.back()->beg[ 0 ] != ( i->beg[ 0 ]==')' ? '(' : ( i->beg[ 0 ] == ']' ? '[' : '{' ) ) ) {
                    write_error( "Closing '" + std::string( 1, i->beg[ 0 ] )+"' has no correspondance.", beg, i->beg, src );
                    return;
                }
                Lexem *o = ps_stack.back(); // assumed opening correspondance

                o->len += ( i->beg - o->beg );

                ps_stack.pop_back();
                if ( i->prev == o ) { // [nothing]
                    o->next = i->next;
                    if ( i->next ) i->next->prev = o;
                }
                else { // [something ...]
                    if ( o->next ) {
                        o->next->parent = o;
                        o->next->prev = NULL;
                    }
                    o->children[0] = o->next;
                    o->next = i->next;

                    if ( i->next ) i->next->prev = o;
                    if ( i->prev ) i->prev->next = NULL;
                }

            }
        }
        //else if ( i->type==Lexem::CR and ps_stack.size() )
        //    i->num = -1; // for [ x y \n z a ] ?
    }

    for( int i = 0; i < ps_stack.size(); ++i )
        write_error( "Opening '" + std::string( 1, ps_stack[i]->beg[0] ) + "' has no correspondance.", beg, ps_stack[i]->beg, src, true );
}

void Lexer::make_unit_hierarchy() {
    for( Lexem *t = first_of_type[ Lexem::NUMBER + Lexem::NB_BASE_TYPE ]; t; t = t->sibling)
        if ( t->next and t->next->type == Lexem::STRING )
            assemble_right_arg( t );
}

void Lexer::remove_cr() {
    // remove CR at beginnings
    for( Lexem *t = first_of_type[ Lexem::CR + Lexem::NB_BASE_TYPE ]; t; t = t->sibling ) {
        if ( not t->prev ) {
            int nch = t->parent->children[ 1 ] == t;
            t->parent->children[ nch ] = t->next;
            t->next->parent = t->parent;
            t->next->prev = 0;
            t->next = 0;
        }
    }

    // update preceded_by_a_cr
    for( Lexem *t = first_of_type[ Lexem::CR + Lexem::NB_BASE_TYPE ]; t; t = t->sibling ) {
        if ( t->next ) {
            t->next->nb_preceding_cr += t->nb_preceding_cr + 1;
            t->next->preceded_by_a_cr = true;
        }
    }

    // remove unused CR ( void lines or CR with same num )
    for( Lexem *t = first_of_type[ Lexem::CR + Lexem::NB_BASE_TYPE ]; t; t = t->sibling ) {
        if ( t->next and t->next->type == Lexem::CR ) {
            t->num = -2;
            if ( t->prev )  t->prev->next = t->next;
            if ( t->next )  t->next->prev = t->prev;
        }
    }
    // then, we remove \n in a+\nb, a\n+b and so on...
    for( Lexem *t = first_of_type[ Lexem::CR + Lexem::NB_BASE_TYPE ]; t; t = t->sibling ) {
        if ( t->num >= -1 ) {
            if ( need_rarg(t->prev) and t->prev->children[0]==NULL and t->prev->type!=STRING___if___NUM and t->prev->type!=STRING___else___NUM and t->prev->type!=STRING___try___NUM ) {
                t->num = -2;
                t->prev->next = t->next;
                if ( t->next ) t->next->prev = t->prev;
            }
            else if ( need_larg(t->next) and t->next->children[0]==NULL and t->next->type!=STRING___if___NUM and t->next->type!=STRING___else___NUM ) {
                t->num = -2;
                t->next->prev = t->prev;
                if ( t->prev ) t->prev->next = t->next;
            }
        }
    }

    // after what, we make blocks
    int old_nb_spaces = 0;
    int nb_pending_spaces = 0;
    int pending_spaces_num[ 256 ];
    Lexem *pending_spaces_prev[ 256 ];
    for( Lexem *t = first_of_type[ Lexem::CR + Lexem::NB_BASE_TYPE ]; t; t = t->sibling ) {
        int tmp = t->num;
        if ( tmp >= 0 ) {
            if ( tmp == old_nb_spaces ) {
                if ( t->prev )  t->prev->next = t->next;
                if ( t->next )  t->next->prev = t->prev;
            }
            else if ( tmp > old_nb_spaces ) {
                t->type = Lexem::BLOCK;

                if ( t->next ) {
                    t->children[0] = t->next;
                    if ( t->next ) {
                        t->next->parent = t;
                        t->next->prev = NULL;
                    }
                    t->next = NULL;
                    pending_spaces_num[ nb_pending_spaces ] = old_nb_spaces;
                    pending_spaces_prev[ nb_pending_spaces ] = t;
                    nb_pending_spaces += 1;
                    ASSERT( nb_pending_spaces < 256, "TODO : growing list" );
                }
                else { // a block with 0 instructions is not very interesting
                    if ( t->prev ) t->prev->next = t->next;
                    if ( t->next ) t->next->prev = t->prev;
                }
            }
            else {
                nb_pending_spaces -= 1;
                while ( tmp != pending_spaces_num[nb_pending_spaces] ) {
                    nb_pending_spaces -= 1;
                    if ( t->next and nb_pending_spaces < 0 ) {
                        write_error( "columns are not aligned.", beg, t->beg, src );
                        return;
                    }
                }
                if ( t->prev )
                    t->prev->next = NULL;
                if ( pending_spaces_prev[ nb_pending_spaces ] )
                    pending_spaces_prev[ nb_pending_spaces ]->next = t->next;
                if ( t->next )
                    t->next->prev = pending_spaces_prev[ nb_pending_spaces ];
            }
            old_nb_spaces = tmp;
        }
    }
}

void Lexer::assemble___calc_name__() {
    for(Lexem *o = first_of_type[ STRING_calc_name_NUM + Lexem::NB_BASE_TYPE ]; o; o=o->sibling)
        assemble_right_arg( o );
}

void Lexer::assemble___string_assembly__() {
    for(Lexem *o = first_of_type[ STRING___string_assembly___NUM + Lexem::NB_BASE_TYPE ]; o; o=o->sibling)
        assemble_2_args( o, true, true );
}

bool elligible_token_for_par(const Lexem *pr) { // return true if in something(), something should become a child of ()
    return ( pr and
            ( pr->type < 0 or
                ( pr->type==Lexem::APPLY or
                  pr->type==Lexem::SELECT  or
                  pr->type==Lexem::CHANGE_BEHAVIOR  or
                  pr->type==STRING_get_attr_ptr_NUM or
                  pr->type==STRING_get_attr_NUM or
                  pr->type==STRING_get_attr_ptr_ask_NUM or
                  pr->type==STRING_get_attr_ask_NUM or
                  pr->type==STRING_doubledoubledot_NUM or
                  pr->type==STRING_self_NUM or
                  pr->type==STRING_calc_name_NUM
                )
            )
            and pr->type != Lexem::CCODE
            and pr->type != Lexem::CR
           );
}

///
void Lexer::assemble_par___getattr__( const Lexem *ft ) {
    SplittedVec<Lexem *,256> st;
    st.push_back( ft->next );
    while ( st.size() ) { // while stack is not empty
        Lexem *t = st.back();
        st.pop_back();
        for( ; t; t = t->next ) {
            if ( t->children[0] )
                st.push_back( t->children[0] );
            if ( t->children[1] )
                st.push_back( t->children[1] );

            if ( t->type == Lexem::PAREN and elligible_token_for_par(t->prev) ) {
                Lexem *pr = t->prev, *pa = pr->parent, *pp = pr->prev;

                t->preceded_by_a_cr = pr->preceded_by_a_cr;

                if ( pa ) {
                    if ( pa->children[ 0 ] == pr )
                        pa->children[ 0 ] = t;
                    else
                        pa->children[ 1 ] = t;
                    t->parent = pa;
                    pr->parent = NULL;
                }
                if ( pp )
                    pp->next = t;
                t->prev = pp;

                if ( t->beg[ 0 ] == '(' )      t->type = Lexem::APPLY;
                else if ( t->beg[ 0 ] == '[' ) t->type = Lexem::SELECT;
                else                           t->type = Lexem::CHANGE_BEHAVIOR;

                t->children[ 1 ] = t->children[ 0 ];
                t->children[ 0 ] = pr;
                pr->parent = t;
                pr->next = NULL;
                pr->prev = NULL;
            }
            else if ( ( t->type==STRING_get_attr_NUM or t->type==STRING_get_attr_ptr_NUM or
                        t->type==STRING_get_attr_ask_NUM or t->type==STRING_get_attr_ptr_ask_NUM or
                        t->type==STRING_doubledoubledot_NUM ) and elligible_token_for_par( t->prev ) ) {
                assemble_2_args( t, true, true );
                if ( t->children[1] )
                    st.push_back( t->children[ 1 ] );
            }
        }
    }
}

bool Lexer::assemble_2_args( Lexem *o, int need_left, int need_right ) {
    if ( need_right and o->next==NULL ) {
        write_error( "Operator " + std::string( o->beg, o->beg + o->len ) + " needs a right expression.", beg, o->beg, src, is_in_main_block( o ) );
        return false;
    }
    if ( need_left and o->prev==NULL ) {
        write_error( "Operator " + std::string( o->beg , o->beg + o->len ) + " needs a left expression.", beg, o->beg, src );
        return false;
    }

    // lifting of cr information
    o->preceded_by_a_cr = o->prev->preceded_by_a_cr;

    // prev
    if ( o->prev->parent ) {
        o->parent = o->prev->parent;
        if ( o->prev->parent->children[0] == o->prev )
            o->prev->parent->children[0] = o;
        else
            o->prev->parent->children[1] = o;
    }
    if ( o->prev->prev )
        o->prev->prev->next = o;

    o->nb_preceding_comma_dot = o->prev->nb_preceding_comma_dot;
    o->nb_preceding_cr        = o->prev->nb_preceding_cr;
    o->prev->nb_preceding_comma_dot = 0;
    o->prev->nb_preceding_cr        = 0;

    o->children[0] = o->prev;
    o->children[0]->parent = o;
    o->prev = o->prev->prev;

    o->children[0]->next = NULL;
    o->children[0]->prev = NULL;

    // next
    if ( o->next->next )
        o->next->next->prev = o;

    o->children[1] = o->next;
    o->children[1]->parent = o;
    o->next = o->next->next;

    o->children[1]->prev = NULL;
    o->children[1]->next = NULL;

    return true;
}

bool Lexer::assemble_right_arg(Lexem *o) {
    if ( o->next == NULL ) {
        write_error( "Operator " + std::string( o->beg, o->beg + o->len ) + " needs a right expression.", beg, o->beg, src, is_in_main_block( o ) );
        return false;
    }
    if ( o->next->next )
        o->next->next->prev = o;

    o->children[0] = o->next;
    o->children[0]->parent = o;
    o->next = o->next->next;

    o->children[0]->prev = NULL;
    o->children[0]->next = NULL;
    return true;
}

bool Lexer::assemble_left_arg(Lexem *o) {
    if ( o->prev == NULL ) {
        write_error( "Operator " + std::string( o->beg, o->beg + o->len ) + " needs a left expression.", beg, o->beg, src );
        return false;
    }
    // lifting of cr information
    o->preceded_by_a_cr = o->prev->preceded_by_a_cr;

    if ( o->prev->parent ) {
        o->parent = o->prev->parent;
        if (o->prev->parent->children[0]==o->prev)
            o->prev->parent->children[0] = o;
        else
            o->prev->parent->children[1] = o;
    }
    if ( o->prev->prev )
        o->prev->prev->next = o;

    o->nb_preceding_comma_dot = o->prev->nb_preceding_comma_dot;
    o->nb_preceding_cr        = o->prev->nb_preceding_cr;
    o->prev->nb_preceding_comma_dot = 0;
    o->prev->nb_preceding_cr        = 0;

    o->children[ 0 ] = o->prev;
    o->children[ 0 ]->parent = o;
    o->prev = o->prev->prev;

    o->children[ 0 ]->next = NULL;
    o->children[ 0 ]->prev = NULL;
    return true;
}

/// take a,x. Should x be a right value for a tuple ?
bool tok_elligible_maylr( const Lexem *o, const Lexem *orig ) {
    return o and not (
            o->type == Lexem::CR or
            o->type == STRING___pertinence___NUM or
            o->type == STRING___info___NUM or
            o->type == STRING___return___NUM or
            o->type == STRING___in___NUM or
            o->type == STRING_comma_NUM or
            o->type == STRING_comma_dot_NUM or

            ( orig->type == STRING_sub_NUM and (
                o->type == STRING_assign_NUM or
                o->type == STRING_reassign_NUM or
                o->type == STRING_inferior_NUM or
                o->type == STRING_superior_NUM or
                o->type == STRING_inferior_equal_NUM or
                o->type == STRING_superior_equal_NUM or
                o->type == STRING_equal_NUM or
                o->type == STRING_not_equal_NUM or
                o->type == STRING_shift_left_NUM or
                o->type == STRING_shift_left_then_endl_NUM or

                o->type == STRING_self_or_NUM or
                o->type == STRING_self_xor_NUM or
                o->type == STRING_self_and_NUM or
                o->type == STRING_self_mod_NUM or
                o->type == STRING_self_shift_left_NUM or
                o->type == STRING_self_shift_right_NUM or
                o->type == STRING_self_div_NUM or
                o->type == STRING_self_mul_NUM or
                o->type == STRING_self_sub_NUM or
                o->type == STRING_self_add_NUM or
                o->type == STRING_self_div_int_NUM or
                o->type == STRING_self_concatenate_NUM
            ) )
    );
}
/// transform next and prev to children if necessary
void Lexer::assemble_op( int mi, int ma ) {
    for( int num_op = ma; num_op >= mi; --num_op ) {
        for( Lexem *o = first_of_type[ num_op + Lexem::NB_BASE_TYPE ]; o; o = o->sibling ) {
            int behavior = o->num;
            bool need_left = behavior & 1;
            bool need_right = behavior & 2;
            bool may_need_left = behavior & 4;
            bool may_need_right = behavior & 8;
            bool need_right_right = behavior & 16;


            // both
            if (
                ( need_left  or ( may_need_left  and tok_elligible_maylr( o->prev, o ) ) ) and
                ( need_right or ( may_need_right and tok_elligible_maylr( o->next, o ) ) )
               ) {
                assemble_2_args( o, need_left, need_right );
            }
            // only right
            else if ( need_left == 0 and need_right ) {
                assemble_right_arg( o );
            }
            // only left
            else if ( need_left and need_right == 0 ) {
                assemble_left_arg( o );
            }
            // need two tokens at the right
            else if ( need_right_right ) {
                if ( ! o->next ) {
                    write_error( "Operator needs a right expression.", beg, o->beg, src, is_in_main_block( o ) );
                    return;
                }

                if ( o->next->next == 0 or ( o->approx_line != o->next->next->approx_line and o->spcr >= o->next->next->spcr ) ) {
                    const char *pass = "void";
                    Lexem *p = new_tok( Lexem::VARIABLE, pass, 4, 0 );
                    p->beg_src = pass;
                    p->src = "internal";
                    p->approx_line = o->approx_line;
                    p->spcr = o->spcr;

                    Lexem *m = o->next;
                    if ( Lexem *n = o->next->next ) {
                        n->prev = p;
                        p->next = n;
                    }
                    m->next = p;
                    p->prev = m;
                }

                if ( ! o->next->next ) {
                    write_error( "Operator needs TWO right expressions.", beg, o->beg, src, is_in_main_block( o ) );
                    return;
                }

                if ( o->next->next->next )
                    o->next->next->next->prev = o;

                o->children[ 0 ] = o->next;
                o->children[ 0 ]->parent = o;

                o->children[ 1 ] = o->next->next;
                o->children[ 1 ]->parent = o;
                o->next = o->next->next->next;

                o->children[ 0 ]->prev = NULL;
                o->children[ 0 ]->next = NULL;
                o->children[ 1 ]->prev = NULL;
                o->children[ 1 ]->next = NULL;
            }

        }
    }
}

void Lexer::add_child_to_info( int stn ) {
    for( Lexem *t = first_of_type[ stn + Lexem::NB_BASE_TYPE ]; t; t = t->sibling ) {
        // get str
        if ( not t->children[ 0 ] ) continue;
        const Lexem *l = leftmost_child( t->children[ 0 ] );
        const Lexem *r = rightmost_child( t->children[ 0 ] );

        const char *b = l->beg - ( l->type == Lexem::STRING );
        unsigned si = ( r->beg - b ) + r->len + ( r->type == Lexem::STRING );

        Lexem *res = app_tok( Lexem::STRING, b, si, 0 );
        res->prev = NULL;
        res->next = NULL;
        res->sibling = NULL;
        t->children[ 1 ] = res;
        res->parent = t;
    }
}

static void set_prev_chro_rec( Lexem *&prev, Lexem *t ) {
    t->prev_chro = prev;
    prev = t;

    if ( t->children[ 0 ] ) set_prev_chro_rec( prev, t->children[ 0 ] );
    if ( t->children[ 1 ] ) set_prev_chro_rec( prev, t->children[ 1 ] );
    if ( t->next          ) set_prev_chro_rec( prev, t->next          );
}

static Lexem *applier_if_no_par( Lexem *l ) {
    switch ( l->type ) {
    case Lexem::APPLY:
    case Lexem::PAREN:
    case Lexem::SELECT:
    case Lexem::CHANGE_BEHAVIOR:
    case STRING_get_attr_NUM:
    case STRING_get_attr_ptr_NUM:
    case STRING_get_attr_ask_NUM:
    case STRING_get_attr_ptr_ask_NUM:
        return l;
    }
    if ( l->children[ 1 ] )
        return applier_if_no_par( l->children[ 1 ] );
    if ( l->children[ 0 ] )
        return applier_if_no_par( l->children[ 0 ] );
    return l;
}

static bool potential_call_lhs( Lexem *l ) {
    return l->type > STRING___info___NUM or l->type < 0;
}

static bool potential_call_rhs( Lexem *l ) {
    return l->type > STRING___info___NUM or (
           l->type < 0 and
           l->type != Lexem::BLOCK );
}

void Lexer::make_calls_without_par() {
    if ( not root() )
        return;
    if ( first_tok.next )
        first_tok.next->preceded_by_a_cr = true;

    Lexem *ulc = 0;
    set_prev_chro_rec( ulc, root() );

    for( Lexem *l = ulc; l; ) {
        if ( l->preceded_by_a_cr == false and l->prev and potential_call_lhs( l->prev ) and potential_call_rhs( l ) ) {
            // display_graph( root() );
            Lexem *p = l->prev;
            Lexem *f = applier_if_no_par( p ); // ex: f a + b

            // remove l from the graph
            p->next = l->next;
            if ( l->next ) l->next->prev = p;

            // new APPLY Lexem
            Lexem *c = new_tok( Lexem::APPLY, f->beg, 0, 0 );
            c->preceded_by_a_cr = l->prev->preceded_by_a_cr;
            c->prev_chro = l->prev_chro;

            // replace f by c
            c->parent = f->parent;
            c->prev = f->prev;
            c->next = f->next;
            if ( Lexem *pf = f->parent ) pf->children[ pf->children[ 1 ] == f ] = c;
            if ( Lexem *pr = f->prev   ) pr->next = c;
            if ( Lexem *ne = f->next   ) ne->prev = c;

            // and set f as a child of c
            c->children[ 0 ] = f;
            f->parent = c;
            f->prev = 0;
            f->next = 0;

            //
            c->children[ 1 ] = l;
            l->parent = c;
            l->next = 0;
            l->prev = 0;

            //
            l = c;
        } else
            l = l->prev_chro;
    }
}

static bool acts_like_an_if( const Lexem *l ) {
    return ( l->type == STRING___if___NUM or l->type == STRING___while___NUM );
}

typedef SplittedVec<Lexem *,8> SL;

static void get_if_data( Lexer *le, const char *beg, const char *src, Lexem *t, SL &ch, SL &if_lexems, SL &el_lexems ) {
    // -> STRING___if___NUM, STRING___while___NUM, ...
    if ( acts_like_an_if( t ) ) { // end of recursion
        if_lexems  << t;
        // conditions << t->children[ 0 ];
        ch         << t->children[ 1 ];
        return;
    }

    // -> STRING___else___NUM
    if ( t->type == STRING___else___NUM ) {
        el_lexems << t;

        // recursion
        if ( acts_like_an_if( t->children[ 0 ] ) or t->children[ 0 ]->type == STRING___else___NUM )
            get_if_data( le, beg, src, t->children[ 0 ], ch, if_lexems, el_lexems );

        //
        if ( acts_like_an_if( t->children[ 1 ] ) )
            get_if_data( le, beg, src, t->children[ 1 ], ch, if_lexems, el_lexems );
        else
            ch << t->children[ 1 ];

        return;
    }

    // -> ??
    le->write_error( "Operator should be an 'else'", beg, t->beg, src );
}

static bool from_a_bracket_block( const Lexem *l ) {
    return false;
}

void Lexer::remove_comma_dots() {
    for( Lexem *l = first_of_type[ STRING_comma_dot_NUM + Lexem::NB_BASE_TYPE ]; l; l = l->sibling ) {
        if ( l->parent or not l->prev or not l->next ) {
            write_error( "Misplaced ;", beg, l->beg, src );
        } else if ( not from_a_bracket_block( l->parent ) ) {
            ++l->next->nb_preceding_comma_dot;
            l->next->prev = l->prev;
            l->prev->next = l->next;
        }
    }
}

void Lexer::update_else_order() {
    for( Lexem *lex_else = first_of_type[ STRING___else___NUM + Lexem::NB_BASE_TYPE ]; lex_else; lex_else = lex_else->sibling ) {
        if ( lex_else->parent and lex_else->parent->type == STRING___else___NUM )
            continue;

        SL ch, if_lexems, el_lexems;
        get_if_data( this, beg, src, lex_else, ch, if_lexems, el_lexems );
        Lexem *ple = lex_else->parent;

        for( int i = 0; i < if_lexems.size(); ++i ) {
            if ( i < el_lexems.size() ) {
                Lexem *el = el_lexems[ i ], *il = if_lexems[ i ];
                il->children[ 1 ] = el;
                el->parent = il;

                el->children[ 0 ] = ch[ i ];
                ch[ i ]->parent = el;

                if ( i + 1 < if_lexems.size() ) {
                    el->children[ 1 ] = if_lexems[ i + 1 ];
                    if_lexems[ i + 1 ]->parent = el;
                } else if ( i + 1 < ch.size() ) {
                    el->children[ 1 ] = ch[ i + 1 ];
                    ch[ i + 1 ]->parent = el;
                }
            }
        }

        if ( if_lexems.size() == 0 ) {
            write_error( "lonely else", beg, lex_else->beg, src );
            continue;
        }

        Lexem *lex_if = if_lexems[ 0 ];
        lex_if->prev = lex_else->prev; if ( lex_else->prev ) lex_else->prev->next = lex_if; lex_else->prev = 0;
        lex_if->next = lex_else->next; if ( lex_else->next ) lex_else->next->prev = lex_if; lex_else->next = 0;
        lex_if->parent = ple; if ( ple ) ple->children[ ple->children[ 1 ] == lex_else ] = lex_if;
    }
}

void Lexer::remove_while_conds() {
    for( Lexem *lex = first_of_type[ STRING___while___NUM + Lexem::NB_BASE_TYPE ]; lex; lex = lex->sibling ) {
        Lexem *n = new_tok( STRING_not_boolean_NUM, lex->beg, 0, 0 );
        n->children[ 0 ] = lex->children[ 0 ];
        n->children[ 0 ]->parent = n;

        Lexem *c = new_tok( STRING___if___NUM, lex->beg, 0, 0 );
        c->children[ 0 ] = n;
        c->children[ 0 ]->parent = c;

        Lexem *r = new_tok( STRING_break_NUM, lex->beg, 0, 0 );
        c->children[ 1 ] = r;
        c->children[ 1 ]->parent = c;

        lex->children[ 0 ] = c;
        c->parent = lex;

        Lexem *b = child_if_block( lex->children[ 1 ] );
        c->next = b;
        b->prev = c;
        b->parent = 0;

        lex->children[ 1 ] = 0;
    }
}

void Lexer::set_num_scope() {
    int nsd = 0, nss = 0;
    set_num_scope( root(), 0, nsd, nss, Lexem::SCOPE_TYPE_STATIC );
}

void Lexer::set_num_scope( Lexem *b, int np, int &nsd, int &nss, int scope_type ) {
    for( ; b; b = b->next ) {
        b->num_scope = np;
        b->scope_type = scope_type;

        if ( b->type == STRING_assign_NUM or b->type == STRING_assign_type_NUM ) {
            b->children[ 0 ]->num_in_scope = ( scope_type & Lexem::SCOPE_TYPE_STATIC ) ? nss++ : nsd++;
            b->children[ 0 ]->scope_type = scope_type;
            b->children[ 0 ]->num_scope = np;

            set_num_scope( b->children[ 1 ], np, nsd, nsd, 0 );
        } else if ( b->type == STRING___if___NUM ) {
            int oss, osd;
            set_num_scope( b->children[ 0 ], np + 1, oss = 0, osd = 0, 0 );
            if ( b->children[ 1 ]->type == STRING___else___NUM ) {
                set_num_scope( b->children[ 1 ]->children[ 0 ], np + 2, oss = 0, osd = 0, 0 ); // ok
                set_num_scope( b->children[ 1 ]->children[ 1 ], np + 2, oss = 0, osd = 0, 0 ); // ko
            } else
                set_num_scope( b->children[ 1 ], np + 2, oss = 0, osd = 0, 0 ); // ok
        } else if ( b->type == STRING___while___NUM ) {
            int oss, osd;
            if ( b->children[ 0 ]->type == STRING___else___NUM ) {
                set_num_scope( b->children[ 0 ]->children[ 0 ], np + 2, oss = 0, osd = 0, 0 ); // ok
                set_num_scope( b->children[ 0 ]->children[ 1 ], np + 2, oss = 0, osd = 0, 0 ); // ko
            } else
                set_num_scope( b->children[ 0 ], np + 2, oss = 0, osd = 0, 0 ); // ok
        } else if ( b->type == STRING___def___NUM or b->type == STRING___class___NUM ) {
            Lexem *c = b->children[ 0 ];
            while ( true ) {
                if ( c->type == STRING___extends___NUM    ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___pertinence___NUM ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING___when___NUM       ) { c = c->children[ 0 ]; continue; }
                if ( c->type == STRING_doubledot_NUM      ) { c = c->children[ 0 ]; continue; }
                break;
            }

            int oss = 0, osd = 0;
            if ( c->type == Lexem::APPLY or c->type == Lexem::SELECT ) {
                // name
                c->children[ 0 ]->num_in_scope = nss++; // static
                c->children[ 0 ]->scope_type = scope_type | Lexem::SCOPE_TYPE_STATIC;
                c->children[ 0 ]->num_scope = np;

                // args
                SplittedVec<Lexem *,8> tl;
                get_children_of_type( c->children[ 1 ], STRING_comma_NUM, tl );
                for( int i = 0; i < tl.size(); ++i ) {
                    Lexem *t = tl[ i ];
                    if ( t->type == STRING_reassign_NUM  ) t = t->children[ 0 ];
                    if ( t->type == STRING_doubledot_NUM ) t = t->children[ 0 ];

                    t->num_scope = np + 1;
                    t->num_in_scope = osd++;
                    t->scope_type = 0;
                }
            } else {
                // name
                c->num_in_scope = nss++;
                c->scope_type = scope_type | Lexem::SCOPE_TYPE_STATIC;
                c->num_scope = np;
            }

            set_num_scope( b->children[ 1 ], np + 1, osd, oss, ( b->type == STRING___class___NUM ? Lexem::SCOPE_TYPE_CLASS : 0 ) ); // block
        } else if ( b->type == STRING___static___NUM ) {
            set_num_scope( b->children[ 0 ], np, nsd, nss, scope_type | Lexem::SCOPE_TYPE_STATIC );
        } else {
            set_num_scope( b->children[ 0 ], np, nsd, nss, scope_type );
            set_num_scope( b->children[ 1 ], np, nsd, nss, scope_type );
        }
    }
}


