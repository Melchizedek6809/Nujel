#!/usr/bin/env python
def testCalc():
  ret = 0
  for i in range(10000000):
    if ((i % 3) == 0) or ((i % 5) == 0):
      ret = ret + i
  return ret

print('The sum is: {}'.format(testCalc()))
