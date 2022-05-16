#!/usr/bin/env escript
-mode(compile).

forLoop(Ret,0) -> Ret;
forLoop(Ret,I) ->
    if (I rem 3) == 0 ->
            forLoop(Ret + I, I - 1);
       (I rem 5) == 0 ->
            forLoop(Ret + I, I - 1);
       true ->
            forLoop(Ret, I - 1)
    end.

main([]) ->
    io:format("The sum is: ~w~n",[forLoop(0,10000000-1)]),
    0.
