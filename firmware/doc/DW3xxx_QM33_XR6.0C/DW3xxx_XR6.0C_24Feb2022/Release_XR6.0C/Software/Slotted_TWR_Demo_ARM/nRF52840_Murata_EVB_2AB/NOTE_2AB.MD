**Note.**

Murata 2AB module is based on the DW3120 AoA chip, this module does have a built-in AoA antenna.

1. The characteristics of the RF track and antenna the type connected to 2AB module will affect its performance.
2. The 2AB Module is normally used in a 2-port AOA antenna configuration.
3. The pdoa2path_diff_ch5/9() LUT table should be characterized to give an optimum performance for the specific Antenna.
4. If the design uses 1-port non-AOA antenna, then the second port of the 2AB module should be terminated via 50Ohm resistor and the configuration of PDOA_M3 should be changed to PDOA_M0.



/Signed

17-Feb-2022

