struct Expr {
    Ptr<Inst> inst;
    int nout;
};
struct Inst_PointerOn {
    Var var; // pour le code généré,
};
struct Type {
    TypeParser *parser;
};

struct RefItem {
};
struct RefItem_Expr : RefItem {
    Expr expr;
};
struct RefItem_Slice : RefItem {
};
struct RefItem_Phi : RefItem {
};

struct Ref {
    RefItem *ref_item;
    Type    *type;
    int      flags;
};

struct Var {
    Ptr<Ref> ref;
    int      flags;
};

/**
Syscall
  a := 10
  p := &a
  syscall &a // syscall( p.snapshot() ) -> ???????
  a = 20 // va remplacer l'expr par 20
  a + 30 //

Question des reassign

Question des destructeurs
  -> a appeler quand des Ref ne sont plus _accessibles_
  -> nécessite un Type *... pquoi pas un attribut TypeParser *parser


Autre solution: les instructions ne sont que des opérations en mémoire,
sans simplification quand il s'agit
  a := 10 # e_0 = set_val( room_0( 32 ), cst( 10 ) )
  a = 20  # e_1 = set_val( set_val( room_0( 32 ), cst( 10 ) ), cst( 20 ) )
    -> toujours possible de récupérer l'adresse initiale
    -> Expr::operator=( expr )
  a + 30 # set_val( room_1( 32 ), e_1.simplified() + cst( 30 ) ) => peut se simplifier.
         # simplified() élimine les set_val
  syscall &a # pointer_on( set_val( room_0( 32 ), cst( 20 ) ) )

  &a #
  &a + 5 # add( pointer_on( room_0( 32 ) ), 5 )
  @( &a + 5 ) # pointed_
              # prop: les Room contiennent une Ref pour dire où ça en est
              # si on fait @( &a + 5 ) = 10, on @( &a + 5 ).ref() renvoie
              #  la ref de room_0() avec un offset

  ( if c a else b ) = ...
             # phi( set_val(r0...), set_val(r1...) ) si on dit que phi renvoie des ref
             # .ptr() renvoie phi( pointer_on(r0), pointer_on(r1) )
             # .ref() renvoie RefItem_Phi

La prise de tête vient du fait que &a == &b avec ( a := 10; b := 10 )

*/







