# SuperDCA

## Get the code
```
git clone --recursive https://github.com/santeripuranen/SuperDCA.git
```

## About

SuperDCA is a tool for global direct couplings analysis (DCA) of input genome alignments.

## What's in this repository

* [SuperDCA](SuperDCA) contains the actual source code along with compile instructions.
* [binaries](binaries) contains precompiled SuperDCA binaries.
* [pneumo_couplings](pneumo_couplings) contains SuperDCA couplings output for *S pneumoniae* genome data sets.
* [ranking_scripts](ranking_scripts) contains a post-processing script that can be used on SuperDCA couplings output.

## Basic usage

Use `SuperDCA -h` or `SuperDCA --help` to get a list of available command line options.

To run SuperDCA with default settings use:
```
SuperDCA -v <name of input genome alignment file>
```
where the input alignment should be in [FASTA format](https://en.wikipedia.org/wiki/FASTA_format).

The main output file (*.out*) of SuperDCA contains a white space delimited, unsorted list of coupling values and pairs of position indices (using *1-based indexing* by default) relative to the columns in the input alignment.

## Cite

SuperDCA was developed as part of an academic research project. Please cite:
```
https://github.com/santeripuranen/SuperDCA/
```
