Important pour la suite:
- layers de variables
  -> Expr/Inst pour du SSA pur (pointer_on renvoie le pointeur d'une variable dans un �tat donn�)
  -> Ref pour des variables non typ�es
      avec gestion des conditions (static cond := Vec[Expr]) ?
      des snapshots (pour les while, les fonctions outline) ?
  -> Var pour des variables typ�es
      Rq: quand on fait break, la couche Ref invalide les instructions suivantes, les destructeurs sont appel�s ensuite dans la couche Var
-


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




