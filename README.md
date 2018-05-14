# SuperDCA

## Get the code
```
git clone --recursive https://github.com/santeripuranen/SuperDCA.git
```
See how to compile [here](SuperDCA/README.md#building-superdca).

## About

SuperDCA is a tool for global [Direct Coupling Analysis (DCA)](https://en.wikipedia.org/wiki/Direct_coupling_analysis) of input genome alignments. Specifically, it implements a variant of the [pseudolikelihood maximization DCA (plmDCA)](https://doi.org/10.1016/j.jcp.2014.07.024), with emphasis on optimizations that enable its use on a genome scale (DCA has so far typically been used on much smaller protein sequence alignments). Here, DCA may be used to discover co-evolving pairs of loci. 

## What's in this repository

* [SuperDCA](SuperDCA) contains the actual source code along with compile instructions.
* [binaries](binaries) contains precompiled SuperDCA binaries.
* [MGen_2018_Tables_S1-S3](MGen_2018_Tables_S1-S3) contains SuperDCA couplings output for *S pneumoniae* genome data sets used in our primary reference.
* [ranking_scripts](ranking_scripts) contains a post-processing script that can be used on SuperDCA couplings output.

## Basic usage

Use `SuperDCA -h` or `SuperDCA --help` to get a list of available command line options.

To run SuperDCA with default settings use:
```
SuperDCA -v <name of input genome alignment file>
```
where the input alignment should be in [FASTA format](https://en.wikipedia.org/wiki/FASTA_format).

This will parse the alignment, apply default filtering rules, run the plmDCA inference algorithm and output coupling values.

Default filtering will extract loci with *more than* 1 allele (not counting gaps), *at least* 1% minor allele frequency and *at most* 15% gap frequency. The filtering criteria can be changed with the `--maf-threshold` and `--gap-threshold` command line options, or disabled completely with the `--no-filter-alignment` flag.

The main output file (*.out*) of SuperDCA contains a white space delimited, unsorted list of coupling values and pairs of position indices (using *1-based indexing* by default) relative to the columns in the input alignment.

## Cite

SuperDCA was developed as part of an academic research project. Please cite (all three, if possible):

* Santeri Puranen, Maiju Pesonen, Johan Pensar, Ying Ying Xu, John A. Lees, Stephen D. Bentley, Nicholas J. Croucher and Jukka Corander. SuperDCA for genome-wide epistasis analysis. *Microbial Genomics* 2018;4, [DOI 10.1099/mgen.0.000184](https://doi.org/10.1099/mgen.0.000184)

* Marcin J. Skwark, Nicholas J. Croucher, Santeri Puranen, Claire Chewapreecha, Maiju Pesonen, Ying Ying Xu, Paul Turner, Simon R. Harris, Stephen B. Beres, James M. Musser, Julian Parkhill, Stephen D. Bentley, Erik Aurell, Jukka Corander. Interacting networks of resistance, virulence and core machinery genes identified by genome-wide epistasis analysis. *PLOS Genetics* 2017, [DOI 10.1371/journal.pgen.1006508](https://doi.org/10.1371/journal.pgen.1006508)

* https://github.com/santeripuranen/SuperDCA

