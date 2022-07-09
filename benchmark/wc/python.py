#!/usr/bin/env python
def wordCount(file):
  file = open(file, 'r')
  nl = 0
  nw = 0
  nc = 0
  inWord = False
  while 1:
    # read by character
    char = file.read(1)
    if not char:
      break
    nc = nc + 1
    if(char == " "):
      inWord = False
    elif(char == "\n"):
      inWord = False
      nl = nl + 1
    else:
      if (not inWord):
        nw = nw + 1
      inWord = True
  file.close()
  print('Lines: {}\nWords: {}\nCharacters: {}'.format(nl,nw,nc))

wordCount('benchmark/bib.txt')