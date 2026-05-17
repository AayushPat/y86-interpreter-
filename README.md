# Y86 Mini-ELF Interpreter

A command-line tool for parsing, disassembling, and executing Y86-64 Mini-ELF binaries, built in C across a semester for CS 261. Each of the four parts of the project corresponded to a distinct stage of how a real processor actually handles a program — from reading it off disk all the way through executing instructions one cycle at a time.

## Usage

```
./y86 <option(s)> <mini-elf-file>
```

| Flag | Description |
|------|-------------|
| `-H` | Show the Mini-ELF header |
| `-s` | Show program headers |
| `-m` | Show memory contents (brief) |
| `-M` | Show memory contents (full) |
| `-a` | Show all with brief memory |
| `-f` | Show all with full memory |
| `-d` | Disassemble code segments |
| `-D` | Disassemble data segments |
| `-e` | Execute program |
| `-E` | Execute program in trace mode |
| `-h` | Display usage |

## Build

```bash
gcc -o y86 main.c p1-check.c p2-load.c p3-disas.c p4-interp.c
```

---

## How It Works — Following the Von Neumann Cycle

The project was structured so that each part built directly on the last, and together they trace the full path a program takes from binary file to running process. That maps pretty cleanly onto the Von Neumann architecture model.

### Part 1 — Validating the Binary (`p1-check.c`)

Before anything can run, the system has to verify that what it's looking at is actually a valid executable. This part reads the Mini-ELF header from a binary file and checks the magic number (`0x00464c45`) to confirm the file is a legitimate Mini-ELF. It also extracts metadata like the entry point address, number of program headers, and locations of the symbol and string tables.

This is the equivalent of the OS loader doing a sanity check before it hands a binary off to the CPU. Without this step nothing downstream is trustworthy — if the magic number is wrong, the whole tool exits immediately.

### Part 2 — Loading into Memory (`p2-load.c`)

Once the header is verified, the program headers describe where each segment (code, data, stack, heap) lives in the file and where it should be mapped in virtual memory. This part reads each program header (also magic-number-checked with `0xDEADBEEF`), then copies each segment from the file into the right offset in a flat 4096-byte simulated memory array using `fread` directly into `memory[p_vaddr]`.

This is the memory part of Von Neumann — before any instruction can execute, the program has to exist in memory. I also wrote `dump_memory` here, which prints memory contents in a hex dump format with proper 16-byte row alignment, and `dump_phdrs` which decodes the segment type and permission flags (R/W/X) from their numeric values.

### Part 3 — Disassembly and Fetch (`p3-disas.c`)

This part implements the **fetch** stage of the processor pipeline. The `fetch` function reads the byte at the current program counter, splits it into a 4-bit `icode` (what kind of instruction) and 4-bit `ifun` (which variant), then reads additional bytes for register specifiers and an 8-byte immediate/address value (`valC`) if the instruction needs them. It also computes `valP`, the address of the next instruction.

A lot of the logic here is validation — making sure register fields that should be `NOREG` actually are, that memory accesses won't go out of bounds, and that `ifun` values are in range for each instruction type. Invalid instructions set the CPU status to `INS` and halt cleanly rather than corrupting state.

The `disassemble` function then takes a fetched instruction struct and prints it in Y86 assembly syntax — handling all 14 instruction types including the conditional move and jump variants. `disassemble_code`, `disassemble_data`, and `disassemble_rodata` walk through a memory segment and format it like a proper disassembly listing with addresses and raw hex bytes alongside the mnemonics.

### Part 4 — Decode, Execute, Memory, Writeback (`p4-interp.c`)

This is where the processor actually runs. The execution loop in `main.c` calls three functions per cycle that implement the remaining pipeline stages:

**`decode_execute`** reads source register values into `valA` and `valB`, performs the operation (arithmetic, logic, address calculation, condition code evaluation), and returns the result as `valE`. For `OPQ` instructions this is where the zero, sign, and overflow flags get set. For jumps and conditional moves, this is where the condition codes are checked against the `ifun` to set the `cnd` signal.

**`memory_wb_pc`** handles everything that touches memory or writes back to registers — pushing/popping the stack, reading/writing for `mrmovq`/`rmmovq`, saving return addresses for `call`, and restoring the PC for `ret`. It also advances the program counter, either to `valP` (next instruction) or to `valC.dest` for taken jumps and calls. Bounds checks before every memory write prevent out-of-bounds accesses from silently corrupting state.

The interpreter also handles `IOTRAP` instructions, a custom extension for I/O — character and decimal input/output buffered through a global output buffer that gets flushed explicitly. Trace mode (`-E`) prints the disassembled instruction and full CPU register state after every single cycle, which was extremely useful for debugging.

---

## What I Learned

This project made a lot of things from lecture concrete. Parsing a binary format by hand makes it obvious why magic numbers and structured headers exist. Implementing `fetch` myself made the instruction encoding (icode/ifun split, optional register byte, optional 8-byte immediate) something I actually understand rather than just memorized. Writing the condition code logic for overflow detection in signed arithmetic required thinking carefully about two's complement in a way that just using `+` in a higher-level language never forces you to. And stepping through programs in trace mode — watching the PC, registers, and flags change one instruction at a time — made the Von Neumann cycle feel like a real mechanical thing rather than a diagram in a textbook.
