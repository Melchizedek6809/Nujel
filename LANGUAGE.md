## Error Handling
Nujel uses 2 different systems for error handling, depending on the
broad category that the error belongs to:

### Exceptions
*Exceptions* should only be used for **compile time errors**, they are
quite handy to short circuit the reader/compiler and immediatly return
an error message to the programmer. Although you can throw any kind
of value, most code in the standard library by convention returns a
list with a keyword, a human readable string and then maybe the
symbol/region of code that led to the exception.

### Result types
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
