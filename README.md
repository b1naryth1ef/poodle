# poodle

poodle is a small Go wrapper around the Epic Games version of the oodle2 network compression library. This wrapper is required because the original libraries are only shipped as static libraries (compiled under mscvc), which Go cannot link against. Additionally this wrapper only passes C-allocated memory into the Oodle library to avoid some internal library issues with non-aligned memory.
