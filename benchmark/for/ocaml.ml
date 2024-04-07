let rec testRun = fun i r ->
  if i < 10000000 then
    testRun (1+i) (r+i)
  else
    r;;

Printf.printf "%d" (testRun 0 0);;
