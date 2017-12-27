# MSAC

## Installation and configuration

MSAC comes with a set of precompiled binaries for Windows and Linux. They can be found under *Releases* tab.

The software can be also built from the sources distributed as:
* Visual Studio 2017 solution for Windows,
* MAKE project (G++ 6.2 required) for Linux.

## Usage

MSAC can compress and decompress both FASTA files with aligned protein sequences as well as the protein families stored in Stockholm files (used in Pfam database).

### Usage

`msac <mode> [options] <in_file> <out_file>`
`Parameters:`
`   mode       - Fc (Fasta compress), Fd (Fasta decompress), Sc (Stockholm compress), Sd (Stockholm decompress)`
`   in_file    - name of input file`
`   out_file   - name of output file`
`Options:`
`   -w <width> - wrap sequences in FASTA file to given length (only for Fd mode); default: 0 (no wrapping)`
`   -f         - turn on fast variant (MTF in place of WFC)`
  
Examples:

* `msac Sc sample.stockholm sample.smsac`

Compresses `sample.stockholm` file.

* `msac Sd sample.smsac sample.stockholm.dec`

Decompresses `sample.stockholm` file.

* `msac Fc pf00005.26.fasta pf00005.26.msac`

Compresses `pf00005.26.fasta` file.

* `msac Fd -w 60 pf00005.26.msac pf00005.26.fasta.dec`

Decompresses `pf00005.26.fasta` file and wrap the protein data to 60 columns.


## Citing

[Deorowicz, S., Walczyszyn, J., Debudaj-Grabysz, A., Gudyœ, A., Grabowski, S. (2017) MSAC: Compression of multiple sequence alignment files, bioRxiv]() 
DOI:


 