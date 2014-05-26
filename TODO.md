TODO
- fonctions outline
- MO, pointeurs de fonction
- �limination des sources inutiles quand des set_val couvrent tout l'espace

RefItem
- operations avec pointeurs
   -> ptr + cst, ptr - cst
   -> ptr - ptr => demande une stabilit� des pointeurs... peut �tre impos� lors de la g�n�ration de code

Int�r�t des variables de taille non connue � la compilation ?
- permet d'allouer sur la pile (exemple: des petits vecteurs comme en C++)
- permet d'�viter des indirections (meilleur pour le cache, peut-�tre pour la vitesse d'ex�cution)
-> on peut s'en passer pour d�marrer

Pb avec les pointeurs:
  a := 10
  b := 10
  print &a == &b -> va renvoyer vrai !!
  Donc (&a).expr() devrait donner la zone m�moire � la base de a
  &(&a) n'existe pas, il faut passer par la cr�ation d'une variable
    -> on pourrait faire d�marrer chaque expr par un id de zone m�moire mais ce n'est pas la vocation des Expr
    -> pointer_on(expr) devrait �tre associ� � un id de zone m�moire
    -> du coup, la r�cup�ration d'un pointeur
    -> pb: @{a:10,b:&c}.b = 15 devrait donner {a:10,b:&10}
    
    -> Prop: les Expr contiennent des Inst, nout et id de zone m�moire
        si on modifie une zone m�moire on fait un subs de toutes les Expr r�f�renc�e... hum
    -> Autre prop: Expr avec id de zone m�moire + snapshot.
         -> pointer_on( expr ) stocke l'id de pointeur (qui ne bouge pas)
         -> si on demande un snapshot (ex pour syscall), on fait un subs avec pointer_on_data
         -> slice( expr, beg, end ) pourrait
            -> ne pas exister c�t� Expr, mais dans une couche suppl�mentaire
            -> ou �tre g�r� avec pointer_on( expr ) + beg
         -> @expr 
         
L'id�e, c'est de g�rer des id de pointeur dans les Expr (les id sont effectivement immutables)

class Ref
    Ptr<Expr> expr;
    Expr      off;
-> &a renvoie pointer_on avec la ref
-> @a doit renvoyer une Ref (a+b renvoie un expr)
-> a[ 16:20 ] cr�e une Ref
-> a[ 16:20 ] = 25 modifie la Ref avec un set_val
-> @( 0x... ) renvoie Ref avec expr = main_mem
-> @phi( c_0, p_0, p_1 ) renvoie 
    -> un Ref avec expr = main_mem (dommage)
    -> ou un RefPhi signifiant que Ref pointe sur un 
--> Rq: a := 10; a += b -> normalement, le pointeur sur `a` a toujours la m�me valeur. Signifie que a est stock�e comme une ref
    -> a + b renvoie une nouvelle Ref, avec expr = *a.expr + *b.expr
    -> syscall( a ) peut utiliser directement l'expr dans la mesure o� le pointeur ne bouge pas. Signifie que le syscall doit ajouter des d�pendances


Probl�me de la gestion des pointeurs
- syscall &a -> copie profonde des expressions
- r := &a + 5 -> r[ -5 ] -> permet de modifier la variable a
  -> r doit �tre un RefAdd( RefPtr( a ), 5 ). L'addition demande un snapshot superficiel, un syscall demande un snapshot profond.
- class T { a := 10; b := Ptr[]() }, t.b = &v
  -> t doit revenir un RefSetVal
- a.b + 10 -> RefAdd( RefSlice(RefSetVal(...)), 10 ) -> 

ATTENTION: Expr ne peut pas devenir une Val : deux Ref diff�rentes peuvent pointer sur la m�me Expr !!!
  -> On ne peut pas faire des Expr qui pointent sur des Ref

a := &10
b := &10
@a = 30 -> ne doit pas modifier b

Couche 1: Expr
  -> non modifiable
Couche 2: Ref 
  -> �galit�: peut passer par les Expr
  -> 

Sujets non explor�s
  - garbage collector pour la pile
  
Important pour la suite:
- layers de variables
  -> Expr/Inst pour du SSA pur (pointer_on renvoie le pointeur d'une variable dans un �tat donn�)
  -> Ref pour des variables non typ�es
      avec gestion des conditions (static cond := Vec[Expr]) ?
      des snapshots (pour les while, les fonctions outline) ?
  -> Var pour des variables typ�es
      Rq: quand on fait break, la couche Ref invalide les instructions suivantes, les destructeurs sont appel�s ensuite dans la couche Var
- Pointeron->copy



Ce qui est mieux que Rust:
- stack closure ne sont pas premi�re classe en Rust alors qu'elles le sont avec Stela
- notations chiante (ex: les closures)
- les m�thodes sont d�finies � l'ext�rieur, alors que �a devrait �tre la premi�re info qui arrive
- generic limit� -> not metaprogrammation


Objets Ref[ T ]
-> id�e de base: un pointeur sur une variable, qui emp�che la variable d'�tre d�truite 
-> oblige � g�rer une table de correspondance c�t� interpr�teur/compilateur, vers les cptr
-> pour �tre conservateur au niveau du compilateur, il faudrait ne d�cr�menter les ref_count que si aucun lien n'est possible vers la variable.
- prop: a basic ptr type (null_ptr) and a basic ref type (null_ref) pour �viter les "add_dep" manuels, mais fonctionner sans Ref[ T ] qui demande des pr�requis (VarArgs, ...)
- autre solution : on catch les appels genre Ref[ T ] pour mettre directement le bon type (pourrait aussi se faire aussi en disant qu'une r�f�rence � une classe, template ou pas, renvoie le type... "Ref" demande Ref sans arguments, alors )

Remarques sur ce qui est connu � la compilation ou pas
- id�e de base : faire un nouveau type force � faire des compilations diff�rentes. Mais on pourrait faire imposer qu'une variable soit connue par exemple avec un "known".
- le known pourrait fonctionner pour retrouver le type d'un MO, ...
- on pourrait aussi accepter des alternatives. Par exemple, cond ? 1 : "a" renvoie une variable avec flag pour dire le type... bof : ralentira tout
- 

Permettre de d�fragmenter la m�moire ?
-> oblige d'interdire de transformer les pointeurs en entiers "basiques"
-> potentiellement lent sur les listes chain�es
-> pourrait �tre utile pour "l'allocateur par taille" qui risque de fragmenter un peu plus 

Probl�me des r�f�rences attrap�es. Ex: def f() ( a := [10 20]; return a[ 0 ] )
-> serait une solution pour �viter les copies de variable
-> permettrait de g�rer compl�tement la dur�e de vie des ref, y compris pour les closures ! 
=> id�e : quand on appelle une fonction, on r�serve de la place pour les variables attrap�es (le f() va r�server de la place pour a := [10 20])
    - pb 1 : dans certains cas, on aura du mal � savoir combien il faut de place.
        - Ex : def f( n ) v := stack_alloc( n ); return v[ 0 ]...
        - solution 1 : s'il y a du stack_alloc, on fait des piles segment�es (e.g. des pointeurs)
        - solution 2 : on rep�re les cas o� on peut trouver les n _avant_ de faire l'allocation. Si pas possible, on refuse de sortir des r�f�rences 
    - pb 2 ; r�cursivivit�. Ex: def f( t ) ( a := [10 t]; return f( a ) )
        - il faudrait augmenter la taille de la pile... de la fonction appelante
        - prop : on rep�re les cas o� la pile de la fonction appelante peut grossir pendant un appel. Dans ce cas, la fonction appel�e est lanc�e dans une nouvelle pile.
        
        
Piles qui �voluent en parall�le
- peut fonctionner comme en go: la r�servation se fait en comparant � de la fin de m�moire r�serv�e.
- rq: on pourrait aussi ne faire qu'une pile, avec une gestion de la fragmentation : on enregistre les causes de fragmentation, et quand on d�roule, on saute le vide...
- autre solution : on fait de l'allocation dynamique quand une pile ne fonctionne plus
    
Faut-il cr�er de nouveaux types � chaque fois qu'on cr�e une fonction ? Si non :
- on aura un syst�me plus dynamique (possible de faire listes de fonctions, ...)
    -> exemple pour du code rentr� de fa�on dynamique
- mieux pour l'occupation m�moire (les infos li�es aux types ne sont jamais effac�es de la m�moire)
- on devra mettre un syst�me embarqu� pour voir si telle ou telle fonction est compil�e, ce qu'elle retourne, etc...
    Exemple : sort( a, f )
        -> pour compiler sort, il faut que le type de retour de f( x, y ) soit connu pour x, y donn�... ce qui ne fonctionne pas avec juste un pointeur sur tok_data
    Sol 1 : sort( a, f! ) pour que la fonction soit compil�e avec un pr�ambule qui teste les valeurs de f pour voir si n�cessaire de compiler qq chose, ... 
        Rq : on pourrait chercher � automatiser la n�cessit� du ! ou pas... mais il faudrait trouver le niveau correct de granularit�.
        Rq : sort( a, f! ) pourrait sortir des types diff�rents fonction du f... du coup on ne peut pas compiler les fonctions qui utilisent sort tant qu'on ne connait pas f.
        
    Autre remarque: les defs sont forc�ment dans des scopes statiques (parce qu'elles peuvent elles-m�mes contenir des scopes statiques).




