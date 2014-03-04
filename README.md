What is Stela ?
===============

Stela is a general purpose programming language dedicated to performance.

Of course, assembly language is the ultimate choice to obtain absolute performance... but not for pratical cases. And of course, C++ is very close to a no compromise approach, with contents that permits to generate very efficient machine code (thanks to the incredible work on compilers), and with constructs like e.g. templates, permitting to obtain genericity, flexibility, ... at no cost at all (in terms of execution speed).

Nevertheless, staying with the later example, the syntax for template meta-programming en C++ is a bit awkward. At least, one can say that if meta-programming is possible with templates, templates are not designed for meta-programming.

Therefore, Stela is (currently an experimental) language with
* the same base behavior than C++ (memory model, ...)
* with an heavy help from the compiler (e.g. to avoid redundancy)
* tools to (dramatically) ease metaprogramming
* a coffeescript/python like syntax
* modern constructs (mixins, ...)
* specific optimization tools

Currently, Stela is a work in progress. Do not expect any example to work properly...

An example
==========

```[python]
class MyClass[ value ]
    def for block
        block value
        for i in [ attr, "b" ]
            block i
    def get_attr
        10

m := MyClass[ sum 0 .. 5 ]()
print map m, x => x + 7 # should give '22 17 b7'
```

A compilation server
====================

To reduce the compilation time, Stela runs as a server with a (permanent) database containing the code for each class or method specialization. (work in progress)

It dramatically reduces the compilation time, permitting to rely heavily on the compile-time passes to help the user.

For example, the return types are found according to the function code and the input parameters, recursively. With a compilation model as the C++ one (with templates, each modification implies recompiling everything), it would be impossible to generalize that.

More generally, it allows to use template everywhere. Actually, every function or method is template. Classes can of course easilly be templated. Of course, meta-programming is still in the game.

Some details on the meta-programming model
==========================================

Meta-programs are programs that are executed during the compilation. It basically helps to develop flexible stuff at no cost. Advanced uses include active libraries (as defined by Veldhuizen in [Active Libraries: Rethinking the roles of compilers and libraries|http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.40.8031])

Within Stela, meta-program can work as with C++, with template (const) parameters and specialization, but meta-program can actually be any code that does not depend on something known only at run time (e.g. random values, user input...).

If you want Stela to execute code during the compilation, you can pass either
* declare variables with `::=`
* send results to template parameters
* or use the `as_a_known_value` function (which basically returns a reference on a `::=` variable)

Example:
```[python]
l := Vec[ FP64 ]()
for c in combinatorial_selection( 2, 5 )
    l << sin sum c
# l will be computed during compilation in the three following cases
t ::= l
A[ l ]( ...)
print as_a_known_value l
```


Tools to simplify the everyday life
===================================

Pertinence and When
-------------------

The rules to decide if a surdefinition is better than an other can be simply overriden by using the keywords `when` and `pertinence`

Example
```[python]
def pow( a, b ) # default pertinence = 0
    return exp b * log( a )
def pow( a, b ) when a.is_diagonalizable # default pertinence = 1 (1 condition)
    ev := eig a
    return ev.P' * pow( ev.D, b ) * ev.P
def pow( a, b ) when always( b == floor( b ) ) and b < 10 pertinence 2 # explicit pertinence
    ev := eig a
    return ev.P' * pow( ev.Vn b ) * ev.P
```

It is also possible to add constraint on class/traits names, as in
```[python]
def pow( a : Mat, b )
    ...
def pow( a, b : [PI32,PI16] )
    ...
```


For loop surdefinition
----------------------

A class can have a method `for`, which takes a block as a parameter.
```[python]
class A
    def for( block )
        block 10
        block "a"
t := 2
for v in A()
    print t * v # will output 20 and aa
```

It allows for an ultra-simplified visitor pattern, where everything is template, and references on external variables are managed by the compiler (like with lambda functions)

Compared to iterators
* sub-type can change (iterator must return only one type), allowing e.g. for heterogeneous lists
* the code is almost always simpler
* execution is almost always faster (state is stored in the program counter without additional variables, ...)
* code passed to `for` methods (as the variable `block` in the previous example) can be analyzed (TODO).

Object of variable sizes (TODO)
-------------------------------

Let say that we want to create an array object with, in memory, the size, followed right after by the data.

In a C code, it is possible to do this manually with `malloc`s or `alloca`s of size `sizeof( int ) + len * sizeof( T )`. But with `struct`s or `class`es, it's not that easy or natural.

Here is an example of how to create a class which size is known during the compilation.

```[python]
class MyVec
    def init( size ) : _size( size )
        ...
    def _nb_T
        _size
    _size := 0
    _data ~= T * _nb_T
```

Dynamic polymorphism (TODO)
---------------------------

Works basically as with C++
```[python]
class A
    virtual def foo( a )
        ...
```
but virtual methods are also template.

It means that the vtable are extensibles (stored as a list of arrays).


Tools for optimization
======================

Auto-tuning (TODO)
------------------

Example
```[python]
def my_test
    exec_time f 0 .. 10000 # representative parameter(s)
def f( range )
    simd_size := TuningParameter( my_test, [2,4,8] )
    # -> simd_size.val is fixed and known at compile time
    for v in simd_loop( range, simd_size.val )
        ...
```
in auto-tuning mode, the compiler generate code for each parameters to be optimized and keep the values that permit to minimize the output of the test functions. Test function can measure total execution time, or other parameters like memory usage, cpu-time, ...

Garbage collection (heap and stack)
===================================

As with C++, there is no garbage collection for the heap. Or at least at a language level.

If one really wants a GC, one can use reflexion, specific pointers (it's possible to surdefine how references on specific objects are transformed to pointer), ... But it's a library concern, not a language one.

Nevertheless, for the stack(s), there is a garbage collector. But it's managed statically by the compiler.

Actually, for standard cases, it allows for obtaining exactly the same behavior than the C++ (the stacks are managed the same way). The point here is that Stela encourages the use of references from the stack (to avoid wastefull copies, transient wrapper classes, ...), which leads to break the simple LIFO rules for variables in the stack. (work in progress)

In this example
```[python]
def f
    a := ...
    return a
b := ref f()
```
`b` will be a reference on the variable `a` normally created in the stack of `f` during the call to `f` (if not inlined of course). Actually, in the caller function, `f()` will ask for the compiler to create a free slot to store `a` such as `a` will be stored in the parent stack (an implicit pointer containing the free slot adress is send to `f`).

It is possible to have class that store managed references of the stack, as in
```[python]
class MyWrapper
    c ~= Ref[ Orig ]
def f
    o := Orig() # on the stack
    MyWrapper( o )
```
classes in this case cannot be created in the heap.

This is a work in progress. All the implementation details have not been defined.


TODO
====

Actually a lot of things.

Some experiments have been made with an interpreter. It has allowed for testing of the syntax, the helpfullness of such or such construct, notably with big programs (where C++ shines in front of e.g. python and javascript)... which lead now to something that can considered as convenient

Currently a compiler is under heavy development. The goal now is to validate that the execution speed are as expected. Currently, for testing purpose, it generate basic C++ code.



