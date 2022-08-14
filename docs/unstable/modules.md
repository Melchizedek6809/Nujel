[^ overview](./../README.md)

# Nujel - module system - **unstable**
The Nujel module system is mostly modeled after ES6 Modules

## Expected changes
The exact forms used for import/export might change, as well as exact error messages/keywords.

## No single/shared namespace
In Nujel there is no single/shared namespace, so there is no way to reference a particular value without importing it first.

## Every symbol needs to be explicitly imported before usage
Of course everything in :core is available by default, but apart from that everything needs to be requested explicitly. This should make upgrading much simpler since modules can provide incompatible functionality under a different symbol.


## Only way to import all symbols from another module is by requesting every export as a map
This is a way of importing everything from a module, without the possibility of inadvertently shadowing symbols

