# JackJmfConsoleTool

Usage : ```JackJmfConsoleTool.exe test_replace.jmf replace TEXTURE_OLD TEXTURE_NEW 1.00 1.00```
Replace texture old name to new name with changes: scaleX, scaleY

Usage : ```JackJmfConsoleTool.exe test_replace.jmf replace_ex TEXTURE_OLD TEXTURE_NEW 2 2 32 32```
Replace texture old name to new name with changes: scaleX, scaleY, shiftX, shiftY

Usage : ```JackJmfConsoleTool.exe test_replace.jmf colorize```
Randomize groups color (visgroups,groups,etc)

Usage : ```JackJmfConsoleTool.exe test_replace.jmf export```
Save .jmf to .map file (Warn: no wad list)

Usage : ```JackJmfConsoleTool.exe test_replace.jmf flip```
Swaps x/y coordinates in .jmf file (make map flipped)

Usage : ```JackJmfConsoleTool.exe test_replace.jmf rotate```
Rotates .jmf map by 90 CW



Now support only jmf 122 and 121 version 


Also support multiple commands like:

```
JackJmfConsoleTool.exe test_replace.jmf replace TEXTURE_OLD TEXTURE_NEW 1.00 1.00 ^
replace TEXTURE_OLD TEXTURE_NEW 1.00 1.00^
replace TEXTURE_OLD TEXTURE_NEW 1.00 1.00^
replace TEXTURE_OLD TEXTURE_NEW 1.00 1.00^
replace TEXTURE_OLD TEXTURE_NEW 1.00 1.00^
```
