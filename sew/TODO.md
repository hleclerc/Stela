� quoi servent les types ?
* actuellement � stocker Class *orig + les param�tres. Si le type des attributs ne d�pend pas du contexte, on peut les stocker dans le Type *

Gestion des registres
* contraintes sur les points d'arriv�e et de d�part ()
* contraintes sur la non cohabitation de Rx_{n} et Rx_{m} si n != m (avec n et m des instructions)
* prop : une liste constraints_to_solve, qu'on peut traiter off-line
    * Inst *, int num

Attention:
class A
  def f
    class B
       def g
          self... # <- quand on parse B, on demande � catcher le self de f alors que c'est inutile

          
Prop pour les classes de taille variable

class A
    size := 0
    data ~= ___rep_type PI8, sb
    def sb
       size
    def init( s ) : size( s + 1 )

-> l'initialisation se fait... sans conna�tre la taille.
    Prop 1: cst() de taille inconnue puis on assigne en calculant les offsets au fur et � mesure
    On compl�te la taille pendant la g�n�ration de code

Prop pour parse/find des m�thodes:
* on met syst�matiquement self dans les variables catch�es...
* mais lorsqu'on parse les m�thodes, met 0 dedans
* quand on vient les chercher, on compl�te la donn�e

Remarque: 
    toto := 10
    class A
        def f # -> la variable f fait r�f�rence � l'instance de A
        def g
        ... := toto # -> catche toto
    -> on ne peut a priori pas stocker f dans le type...
        Rq: le type d�pend de Class, qui d�pend bien des variables catch�es
        !! les variables catch�es peuvent �tre de diff�rents type lors d'appels diff�rents avec les m�me types
            Exemple :
                a ~= [ SI32, SI64 ][ val ]
                class A
                    def foo
                        a # 
        
!!! reassign devrait demander � mettre le r�sultat dans l'espace qui existe d�j�
  a = b + c # l'espace m�moire pour b + c doit lui �tre propos�
  def f a, b; return a + b; c = f A, B # dans le cas de matrices, �a implique des copies
    -> prop 1:
        def f a, b
            if return_target isa Matrix
                return_target.resize ...
                return_target[ ... ] = ...
        A = f ... # -> return_target = A
        c := f(..) + ... # -> return_target = void
        
        signifie que tous les appels se font avec Expr& en param�tre
        
    -> prop 2: �valuation paresseuse
            
  def f a, b; return a + b; c := f A, B # on doit pouvoir rep�rer le cas en comptant les refs
    
� faire

* destruction des objets
* garbage collection
* reassign, copy, ... avec les types
* factorisation des op�rations similaires


a := Ptr[Item]( 0 )
while ...
   a = Item( a, 123 )
   
Dans le while, on augmente la taille ext�rieure de la pile (la pile ext�rieure peut �tre g�r�e par un pointeur dans la pile)
On d�truit cette partie l� d'un bloc quand la variable n'est plus accessible

Rq: si on sauve un pointeur de la pile vers une zone g�r�e par le tas
  -> � tester

Pb philosophique : toutes les instructions agissent sur des zones m�moire
- on pourrait avoir add( room, a, b ) qui serait un set_val( room, add( a, b ) -> � ce moment l�, autant faire un self_add( room, b )
- comment dans ce cas rep�rer que deux instructions sont �gales ? Avec simplify

Autre solution : 
- un arbre de manipulation de zones m�moires en premier niveau
- un arbre SSA "pur" pour la g�n�ration de code.                               

-> syscall &p devient ssa
    (assign room_xx) -dep-> syscall room_xx -dep-> (les assign suivant)
-> une Var, c'est une position en m�moire + la valeur � cette position
-> un pointeur 
    
Pour les conditions :
- l'id�e du assign_to (instructions conditionnelles) est elle vraiment SSA ?

Pb en cours : le conditions apparaissent comme des d�pendances dans les blocks alors qu'une fois mises dans les blocks, on n'en a plus besoin
  -> prop 1 : on remplace les inp par des instructions bidon
  -> prop 2 : les blocks sont repr�sent�s pas des instructions

  
  
  
  
  
Concours scientifique -> pas de risque d'atteinte.
- droit � 20% pour le concours scientifique

- SARL -> 
- SAS -> soci�t� pas action simplifi�
- 
- 





