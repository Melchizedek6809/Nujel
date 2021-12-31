## Type System
I greatly prefer static type systems, since then you get more errors
during compile time, and not during runtime where it is a lot harder
to debug. Additionally it makes it a lot easier to generate fast code
since we already know all the types during compile time and allows for
multimethods with no runtime overhead. For those parts where one just
can't know a type ahead of time Nujel should allow for automatic boxing
and unboxing using something like a `mixed` type. Multimethods can also
be extended afterwards, so you could for example implement a custom vector
datatype and expand the primitve operators by writing something similar to:
```
[defun +:vector [a:vector b:vector]
       [vector [+ a:x b:x]
               [+ a:y b:y]
               [+ a:z b:z]]]
```

## Concurrency
Nujel should use promises for concurrency, leveraging the type system and
implicit casts should make for a very comfortable way of doing async:
```
[def α [file/read "a.txt"]]
[def β [file/read "b.txt"]] ; α and β both contain unresolved promise:string values
[cat α β] ; Since [cat] does not support promise:string but values of type string we will now await both promises in parallel
[cat [file/read "a.txt"] [file/read "b.txt"]] ; This works the same, since first we build the complete argument list, and then do any implicit casts
```
That way one never has to manually await a result, the runtime will
automatically await the result on first usage. Additionally Nujel does not make
a difference between an async function and a normal one, they just differ on
the return type.


## Error Handling
Nujel uses 2 different systems for error handling, depending on the
broad category that the error belongs to:

### Compile time: Exceptions
*Exceptions* should only be used for **compile time errors**, they are
quite handy to short circuit the reader/compiler and immediatly return
an error message to the programmer. Although you can throw any kind
of value, most code in the standard library by convention returns a
list with a keyword, a human readable string and then maybe the
symbol/region of code that led to the exception.

### Runtime: Result types
**Runtime errors** should be signaled using the *return value*, either by
return a different type depending on success, or by returning
a pair of values, either way, a simple if statement should be enough to
handle the error:
```
[def file-content [file/read "test.txt"]]
[if file-content
     [display file-content]
	 [display "Error reading file!"]]
```
