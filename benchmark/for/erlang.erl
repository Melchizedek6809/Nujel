#!/usr/bin/env escript
-mode(compile).

forLoop(Ret,0) -> Ret;
forLoop(Ret,I) -> forLoop(Ret + I, I - 1).

main([]) ->
    io:format("The result is: ~w~n",[forLoop(0,10000000-1)]),
    0.
