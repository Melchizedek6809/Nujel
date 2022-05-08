#!/usr/bin/env python
def testCalc():
  ret = 0
  for i in range(10_000_000):
    ret = ret + i
  return ret

print('The result is: ', testCalc())
