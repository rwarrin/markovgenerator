@echo off

SET CompilerFlags=/nologo /Z7 /Od /FC /fp:fast
SET LinkerFlags=/incremental:no

IF NOT EXIST build mkdir build

pushd build

cl %CompilerFlags% ..\MarkovGenerator\code\markov_characters.cpp /link %LinkerFlags%
cl %CompilerFlags% ..\MarkovGenerator\code\markov_words.cpp /link %LinkerFlags%

popd
