//
// Set up for PGPLOT
//
#include "shm_k5data.inc"
#include <cpgplot.h>
#include <math.h>


int cpg_setup(char *xaxis_labal)
{
    int err_code; // Error Code

    //
    // OPEN PGPLOT DEVICE
    //
    // Color definishon
    cpgscrn(0, "DarkSlateGrey", &err_code);
    cpgscrn(1, "White", &err_code);
    cpgscrn(2, "SlateGrey", &err_code);
    cpgscrn(3, "Yellow", &err_code);
    cpgscrn(4, "Cyan", &err_code);
    cpgeras();

    cpgsvp(0.0, 1.0, 0.0, 1.0);
    cpgswin(0.0, 1.0, 0.0, 1.0);
    cpgsci(1);

    cpgsch(1.0);
    cpgptxt(0.5, 0.975, 0.0, 0.5, "PolariS");
    cpgptxt(0.5, 0.025, 0.0, 0.5, xaxis_labal);
    cpgptxt(0.025, 0.5, 90.0, 0.5, "Relative Power [dB]");

    return(0);
}
