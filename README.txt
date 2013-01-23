OpenMP RedBlue Simulation

Parallel version requires a compiler with OpenMP support.

What is the RedBlue simulation?

A bunch of red and blue cells inhabit a board. Each turn, the red cells
move one cell to the right if they can, and then the blue cells move 
down one cell. If either are at the end of the board they roll over to the
other side.

Cells of the same colour move simultaneously, so for example if a row looked like:

RRRR_

Then after they move it will look like:

RRR_R

Only the last red cell has moved.

Timing relies on OpenMP. If compiled without OpenMP, time will always display -1 second.

