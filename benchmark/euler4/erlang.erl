#!/usr/bin/env escript
-mode(compile).

reverseNum(A,Ret) ->
    if (A < 1) ->
            Ret;
       true ->
            reverseNum( A div 10, (Ret * 10) + (A rem 10))
    end.

palindrome(A) ->
    A == reverseNum(A,0).

ifPalindrome(A,Ret) ->
    case palindrome(A) of
        true ->
            max(Ret,A);
        false ->
            Ret
    end.

startSearch(A,B,Ret) ->
    if (A >= 1000) ->
            Ret;
       (B >= 1000) ->
            startSearch(A+1,0,Ret);
       true ->
            startSearch(A,B+1, ifPalindrome(A*B,Ret))
    end.

main([]) ->
    io:format("The biggest product of 2 3-digit numbers that is a palindrome is: ~w~n",[startSearch(0,0,0)]),
    0.
