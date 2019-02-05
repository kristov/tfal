#include "stdint.h"
#include "stdio.h"

/*

# Types

0x01 uint8
0x02 int8
0x03 uint16
0x04 int16
0x05 uint32
0x06 int32
0x07 uint64
0x08 int64
0x09 float32
0x0a float64
0x0b float96
0x0c structure

ltypes are:

  symbol     - A symbol (data, or a "value").
  expression - An expression that can be evaluated. A list of expressions or atoms.
  atom       - A reference to a symbol.
  utf8       - A chunk of text in utf8.

A scalar symbol:

  [structure:
    [uint8:stype]
    [(type):data]
    [uint8:ident_ascii]
  ]

Symbol types can be:

  scope     0x01
  builtin   0x02
  function  0x03
  constant  0x04

An atom:

  [structure:
    [uint32:location] - A list of indexes to locate the symbol.
    [uint8:ident_ascii]
  ]

An expression:

  [structure:expressions]

If a structure begins with a uint8 type it is a symbol. If it begins with a uint32 type it is an atom, or a reference to a symbol. If it begins with a structure it is an expression.

The structure of:

  (+ 1 (- 3 x))

First some global builtins:

  [0x01 [[0x1a 0x02 0x2b 0x2e2e2e][0x1a 0x03 0x2d 0x2e2e2e]] 0x6d61696e 0x2e2e2e]

  [structure:
    [uint8:0x01]               ; a scope
    [structure:
      [structure:              ; a scalar in-place, not a reference atom
        [uint8:0x02]           ; builtin
        [uint8:2]              ; plus opcode
        [uint8:"+"]            ; called "+"
        [utf8:"..."]           ; documentation
      [structure:
        [uint8:0x02]           ; builtin
        [uint8:3]              ; minus opcode
        [uint8:"-"]            ; called "-"
        [utf8:"..."]           ; documentation
      ]
    ]
    [uint8:"main"]             ; called "main"
    [utf8:"..."]               ; documentation of main scope
  ]

Then the expression:

  [structure:                  ; expression
    [structure:
      [uint32:0x00000000]      ; location of "+" builtin
    ]
    [structure:                ; also a scalar in-place because constant
      [uint8:0x04]             ; constant
      [uint8:0x01]             ; the value
    ]
    [structure:
      [structure:
        [structure:
          [uint32:0x00000000]  ; builtin
        ]
        [structure:
          [uint8:0x04]         ; constant
          [uint8:0x03]         ; the value
        ]
        [structure:
          [uint32:0x00000000]  ; scalar reference (current scope)
          [uint8:"x"]          ; here it is called "x"
        ]
      ]
    ]
  ]

Byte size: 60

Data layout (NEW STYLE):

       ___________ Type (uint8, int8, etc)
      /  /
  00000000
  \___\___________ Count of following length bytes


  00010001 00000010 xxxxxxxx xxxxxxxx

Data type: uint8
Count of length bytes: 1
Length byte: 2
Read: (2 * sizeof(uint8))

Data layout (OLD STYLE):

    ____________________ Header byte
   /      / ____________ Data bytes start
  /      / /
  00000000 xxxxxxxx xxxxxxxx ....
     \\__\________________________ Data type
      \
       \______ If 1 tells interpreter to look for a length sequence after the
               header

The smallest possible block of data is two bytes:

  00000001 xxxxxxxx

Meaning a 8 bit unsigned integer unpacked (1 value). Because the length
sequence bit is not set the data packet is assumed to be sizeof(uint8) long.

Two packed unsigned integers:

  00010001 00000010 xxxxxxxx xxxxxxxx

The header bit indicates that there is a length description after the header.
The length descriptor is two, meaning there are two bytes in the block.

Two 16 bit signed integers:

  00010100 00000010 xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
           \\_____\___ 2
            \
             \________ The "more bytes" flag is not set

Five thousand 8 bit signed integers packed, first the header:

  00010010

Binary value for 5000 is:

  00010011 10001000

Broken into 7 bit chunks:

  00 0100111 0001000

Reversed in order and packed with "more bytes" flag:

  10001000 00100111
  \\     \ \\_____\____ The high part
   \\     \ \__________ The "more bytes" flag not set
    \\_____\___________ The low part
     \_________________ The "more bytes" flag is set

The 00 is discarded.

*/

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

void test_decode() {
    uint8_t addr = 0;
    uint8_t data[] = {
        0x14,
        0x88,
        0x27
    };

    uint8_t head = data[addr];
    addr++;

    uint8_t type = (head & 0x0f);
    fprintf(stderr, "type: %d\n", type);

    uint8_t tmp = 0;
    uint32_t len = 1;

    if (head & 0x10) {
        uint8_t pos = 0;
        len = 0;

        tmp = data[addr];
        while ((tmp & 0x80)) {
            len |= ((tmp & 0x7f) << pos);
            pos += 7;
            addr++;
            tmp = data[addr];
        }
        len |= ((tmp & 0x7f) << pos);
    }
    fprintf(stderr, "len: %d\n", len);
}

void test_encode() {
    uint32_t len = 5000;
    uint8_t type = 4;

    uint8_t data[] = {
        0x00,
        0x00,
        0x00
    };
    data[0] = type;

    if (len > 1) {
        data[0] |= 0x10;
    }

    uint8_t bytes = 1;
    if (len > 127) {
        bytes = 2;
    }
    if (len > 16383) {
        bytes = 3;
    }
    if (len > 2097151) {
        bytes = 4;
    }
    if (len > 268435455) {
        bytes = 5;
    }

    uint8_t i = 0;
    uint8_t tmp = 0;
    for (i = 0; i < bytes; i++) {
        tmp = (0x7f & (len >> (i * 7)));
        if (i < (bytes - 1)) {
            tmp = (tmp | (1 << 7));
        }
        data[1 + i] = tmp;
    }

    fprintf(stderr, "0x%02x\n", data[0]);
    fprintf(stderr, "0x%02x\n", data[1]);
    fprintf(stderr, "0x%02x\n", data[2]);
}

int main(int argc, char** argv) {
    test_decode();
    test_encode();
    return 0;
}
