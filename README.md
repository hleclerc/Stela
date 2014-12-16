What is Stela ?
===============

Stela is a general purpose programming language, designed for performance, or to be more accurate, designed to simplify the design of flexible and performant programs.

Of course, C++ is currently a great match for these goals, thanks to the incredible work on compilers and constructs like e.g. templates, permitting to obtain genericity, flexibility at no cost at all (at least in terms of execution speed).

Nevertheless, staying with the later example, the syntax for C++ template meta-programming is not really convenient. At least, one can say that if meta-programming is possible with templates, templates do not really seem to be designed for meta-programming.

Stela is an experimental language with the following goals
* steal all the things that make C++(11) such a great language (memory model, templates, ...)
* a coffeescript/python like syntax
* a compilation working at a function level, with a server, to dramatically reduce compilation time, in order to let the user specify only the things the compiler cannot guess_
    - return types, type of declared variables,
    - constexpr functions or not, pure or not, ...
    - ...
* tools to ease metaprogramming
* modern constructs (mixins, ...)
* and specific optimization tools

Currently, Stela is a work in progress.

An example
==========

```python
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

To reduce the compilation time, Stela runs as a server with a database containing the code for each class or method specialization.

It significantly reduces the compilation time, giving the opportunity to rely heavily on the compiler to help the user.

To take an example, the return types are found according to function code and input parameters (using the code of the definition). With a compilation model as the standard C++ one, it would be impossible to generalize that with non prohibitive compilation passes.

More generally, *it allows to use template everywhere*. Actually, unless explicitely specified, functions and methods are template. Classes can easilly be templated (as shown in the above example).


Some details on the meta-programming model
==========================================

Meta-programs are programs that are executed during the compilation. It basically helps to develop flexible stuff at no cost. Advanced uses include for example active libraries (as defined by Veldhuizen in [Active Libraries: Rethinking the roles of compilers and libraries](http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.40.8031)) which provide impressive tools to bind speed, flexibility and expressiveness.

Within Stela, meta-program can work as with C++, with template (const) parameters and specialization, but *meta-program can actually be any code provided that does not depend on something known only at run time* (e.g. random values, user input...).

If you want Stela to execute code during the compilation, you can either
* declare variables with `::=` (`a ::= b` means that `a` will be equal to `b`, and the type *and* the value will be known at compile time)
* send results to template class parameters (e.g. MyClass[ expr ])
* or use the `as_a_known_value` function (which basically returns a reference on a `::=` variable)

Example:
```python
l := Vec[ FP64 ]()
for c in combinatorial_selection( 2, 5 )
    l << sin sum c
# l will be computed during compilation in the three following cases:
t ::= l
A[ l ]( ... )
print as_a_known_value l
```


Tools to simplify the everyday life
===================================

Pertinence and When
-------------------

The generic rules to decide if a surdefinition is better than an other can be simply overriden by using the keywords `when` and `pertinence`

Example
```python
def pow( a, b ) # -> default pertinence = 0
    return exp b * log( a )
def pow( a, b ) when a.is_diagonalizable # default pertinence = 1 (because there is 1 condition)
    ev := eig a
    return ev.P' * pow( ev.D, b ) * ev.P
def pow( a, b ) when always( b == floor( b ) ) and b < 10 pertinence 2 # explicit pertinence
    ev := eig a
    return ev.P' * pow( ev.Vn b ) * ev.P
```

It is also possible to add constraints on class/traits names, as in
```python
def pow( a : Mat, b )
    ...
def pow( a, b : [PI32,PI16] )
    ...
```


For loop surdefinition
----------------------

A class can have a method `for`, which takes a block as a parameter.
```python
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

Object of variable sizes
------------------------

Let say that we want to create an array object with, in memory, the size, followed right after by the data.

In a C code, it is possible to do this manually with `malloc`s or `alloca`s of size `sizeof( int ) + len * sizeof( T )`. But with `struct`s or `class`es, it's not that easy or natural.

Here is an example of how to create a class with size not known during the compilation.

```python
class MyVec
    size := CUnsigned # compressed unsigned
    data := repeat PI8, n = size
```

Dynamic polymorphism
--------------------

Works basically as with C++
```python
class A
    virtual def foo( a )
        ...
```
but virtual methods are also template.

It means that the vtable are extensibles (stored as a list of arrays).


Tools for optimization
======================

Auto-tuning
-----------

Example
```python
def my_test
    exec_time f 0 .. 10000 # representative parameter(s)
def f( range )
    simd_size := TuningParameter( my_test, [2,4,8] )
    # -> simd_size.val is fixed and known at compile time
    for v in simd_loop( range, simd_size.val )
        ...
```
in auto-tuning mode, the compiler generate code for each parameters to be optimized and keep the values that permit to minimize the output value of the test functions. Test function can measure total execution time, or other parameters like memory usage, cpu-time, ...

Garbage collection (heap and stack)
===================================

As with C++, there is no garbage collection for the heap. Or at least at the language level.

If one really wants a GC, one can use reflexion, specific pointers (it's possible to surdefine how references on specific objects are transformed to pointer), ... But it's a library concern, not a language one.

Nevertheless, for the stack(s), there is a garbage collector. But it's managed statically by the compiler.

Actually, for standard cases, it allows for obtaining exactly the same behavior than the C++ (the stacks are managed the same way). The point here is that Stela encourages the use of references from the stack (to avoid wastefull copies, transient wrapper classes, ...), which leads to break the simple LIFO rules for variables in the stack. Beware netherveless, it is a work in progress, is works only for simple cases (and the compiler shouts if the case is too "complicated").

In this example
```python
def f
    a := ...
    return a
b := ref f()
```
`b` will be a reference on the variable `a` normally created in the stack of `f` during the call to `f` (if not inlined of course). Actually, in the caller function, `f()` will ask for the compiler to create a free slot to store `a` such as `a` will be stored in the parent stack (an implicit pointer containing the free slot adress is send to `f`).

It is possible to have class that store managed references of the stack, as in
```python
class MyWrapper
    c ~= Ref[ Orig ]
def f
    o := Orig() # on the stack
    MyWrapper( o )
```
classes in this case cannot be created in the heap.

This is a work in progress.


TODO
====

Actually a lot of things.

Some experiments have been made with an interpreter. It has allowed for testing of the syntax, the helpfullness of such or such construct, notably with big programs (where C++ shines in front of e.g. python and javascript)... which leads now to something that has considered as convenient by us.

Currently a compiler is under heavy development. The goal now is to validate that the execution speeds are as expected. Currently, for testing purpose, it generate C++ code.



