# FM-7 Tape Image Tool
## by CaptainYS (http://www.ysflight.com)
---

# Introduction
This program makes a .T77 tape image from 44.1KHz WAV sampling of FM-7 data (program) tape.

The program filters the input WAV so that the high and low peaks become flat.  Then analyze the wave to extract data bytes that FM-7 BIOS recognizes.  Finally the data bytes are saved to .T77 tape-image format that FM-7 emulators can read.