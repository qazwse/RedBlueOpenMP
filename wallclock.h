/* wallclock.h for CIS*3090

B. Gardner, 24-Oct-11

#include or copy/paste the following functions into your program.
They implement Pilot's PI_StartTime(), PI_EndTime() style of timing.

This version gets accurate wall clock time from an OpenMP library function,
rather than clock() which returns CPU time.  To call it from a non-OpenMP
program it is necessary to adjust a property:

Visual Studio:
 Configuration Properties > C/C++ > Language [Intel C/C++] > OpenMP Support:
	"Generate Sequential Code"

Command line compilation:
 Add option /Qopenmp_stubs to the icl command

If you don't do the above, you will get a linker error.

If this is an OpenMP program, you should NOT do the above since you will
already have "Generate Parallel Code" or /Qopenmp.

Call StartTime() to start timing (duh), then when you want to finish timing,
print the seconds returned by EndTime() using "%lf" format.
*/

#include <omp.h>

// helper function, don't call this directly
static double TimeHelper( int reset )
{
	static double start = 0.0;
	if ( reset ) {
		start = omp_get_wtime();
		return 0.0;
	}
	return (double)(omp_get_wtime()-start);
}

// StartTime resets timer
// EndTime returns no. of wal lclock seconds elapsed since StartTime
void StartTime()
{
	TimeHelper( 1 ); // reset timer
}

double EndTime()
{
	return TimeHelper( 0 );
}
