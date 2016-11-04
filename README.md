# pdf2oac: PDF to Open Annotation Core

A command-line tool to extract annotations from PDFs and convert them to the
[Open Annotation Core Data Model](http://www.openannotation.org/spec/core/).


## Requirements
  * autotools
  * libpoppler
  * librdf


## Building

`./autogen.sh && ./configure && make`


## Usage

`pdf2oac <input_filename>`
