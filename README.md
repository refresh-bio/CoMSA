# CoMSA

[![GitHub downloads](https://img.shields.io/github/downloads/refresh-bio/comsa/total.svg?style=flag&label=GitHub%20downloads)](https://github.com/refresh-bio/CoMSA/releases)

Compressor of Multiple Sequence Alignments (CoMSA) is a tool to represent a family of aligned protein sequences (or a collection of families) in a highly compressed form. As an input it takes the FASTA (single aligned family) of Stockholm (collection of families) files.
Pfam v. 31.0 Stockholm file of size 41.6 GB can be compressed to as 1.74 GB (compare to 5.6 GB for gzip). CoMSA is also significantly faster in compression than gzip. More details can be found in our paper pointed below.

## Installation and configuration

CoMSA comes with a set of precompiled binaries for Windows and Linux. They can be found under *Releases* tab.

The software can be also built from the sources distributed as:
* Visual Studio 2017 solution for Windows,
* MAKE project (G++ 6.2 required) for Linux.

## Usage

CoMSA can compress and decompress both FASTA files with aligned protein sequences as well as the protein families stored in Stockholm files (used in Pfam database).

### Usage

`CoMSA <mode> [options] <in_file> <out_file>`

`Parameters:`

`   mode       - Fc (Fasta compress), Fd (Fasta decompress), Sc (Stockholm compress), Sd (Stockholm decompress)`

`   in_file    - name of input file`

`   out_file   - name of output file`

`Options:`

`   -w <width> - wrap sequences in FASTA file to given length (only for Fd mode); default: 0 (no wrapping)`

`   -f         - turn on fast variant (MTF in place of WFC)`

  
Examples:

* `CoMSA Sc sample.stockholm sample.smsac`

Compresses `sample.stockholm` file.

* `CoMSA Sc sample.stockholm.gz sample.smsac`

Compresses `sample.stockholm.gz` file.

* `CoMSA Sd sample.smsac sample.stockholm.dec`

Decompresses `sample.stockholm` file.

* `CoMSA Fc pf00005.26.fasta pf00005.26.msac`

Compresses `pf00005.26.fasta` file.

* `CoMSA Fd -w 60 pf00005.26.msac pf00005.26.fasta.dec`

Decompresses `pf00005.26.fasta` file and wrap the protein data to 60 columns.


## Citing

<a href="https://doi.org/10.1093/bioinformatics/bty619">Deorowicz, S., Walczyszyn, J., Debudaj-Grabysz (2018) CoMSA: Compression of multiple sequence alignment files, Bioinformatics 35, 227&ndash;234 (2018), doi: 10.1093/bioinformatics/bty619</a>

