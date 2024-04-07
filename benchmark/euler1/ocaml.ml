let rec testRun = fun i r ->
  if i < 10000000 then
    testRun (1+i) (if i mod 3 == 0 || i mod 5 == 0 then
        (r+i)
      else
        r)
  else
    r;;

Printf.printf "The sum is: %d" (testRun 0 0);;
