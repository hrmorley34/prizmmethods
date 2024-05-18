## Screens

- Method selection / search
- Method view
    - TODO: Calls view?
    - Press numbers to highlight/hide line
    - Arrows to scroll
    - TODO: Save state (recent search and stage)


## Rendering

1. Read details e.g. place notation from file; copy into memory.
2. Generate rows.
3. Write visible rows/lines out.


## Method lib

Put names in Unicode NFKD form, then remove non-ASCII.


### File format

Different files for different stages.

Types:
- `uint8` - unsigned 8-bit integer
- `uint16` - unsigned 16-bit integer; **little-endian**
    - `BellBitmask` - `uint16 x`, with `x&(1<<i)` set to include bell `i`
    - `PlaceNotation` - `BellBitmask`, where bells set are those that are *stationary* (i.e. make a place).
- `uint32` - unsigned 32-bit integer; **little-endian**
    - `ptr*` - `uint32` pointer to another location in the file, relative to the start of the file
- `char[]` - null-terminated string

| Offset | Size   | Data/type | Description |
|--------|--------|-----------|-------------|
| `0x00` | `0x04` | `"CCML"`  | Magic word |
| `0x04` | `0x01` | `0x02`    | Version of this file |
| `0x05` | `0x01` | `uint8`   | Stage of this file |
| `0x06` | `0x01` |           | Padding |
| `0x07` | `0x01` | `uint8`   | Depth of pointers: `d` |
| `0x08` | `0x02 * ?` | `ptr*[]` | Pointers to location in the file of the first method beginning with particular characters. |
|        |        | `Method[]` | Methods |

#### Pointers

Characters:
- Special: ` ` or any symbols
- Digit: Any of `0123456789`
- Letters: Each of `ABCDEFGHIJKLMNOPQRSTUVWXYZ`


###### Depth 0

Uses a single pointer to the first method.

###### Depth 1

Uses pointers to Special, Digit, each of Letters.

###### Depth 2

Uses pointers to
- Special
- Digit
- A character in Letters, followed by
  - Special
  - Digit
  - Each character in Letters

etc.

#### Method type

| Offset | Size   | Data/type | Description |
|--------|--------|-----------|-------------|
| `0x00` | `0x02` | `uint16`  | Length of the method data |
| `0x02` | `0x01` | `uint8`   | Length of the method name, excluding null terminator: `n` |
| `0x03` | `0x01 * (n+1)` | `char[]` | Name of the method; null-terminated.
|        | `0x02` | `uint16`  | Length of the place notation: `p` |
|        | `0x02 * p` | `PlaceNotation[]` | The place notation |
|        | `0x02` | `uint16` | The number of leads in a plain course. |
|        | `0x02` | `BellBitmask` | Which bells are hunt bells. |
