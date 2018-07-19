﻿//
// Cross-power Spectra Monitor
//
#include "shm_k5data.inc"
#include <cpgplot.h>
#include <math.h>

#define MAX(a,b) a>b?a:b


int cpg_xspec(
        struct SHM_PARAM *param_ptr,
        float  *freq_ptr,
        float  *spec_ptr	
	)
{
    float xmin, xmax;           // Plot Window Range
    float ymin, ymax;           // Plot Window Range
    float plot_y[MAX_CH_VIEW];  // Y values to plot
    int   ch_index;             // channel index
    int   st_index;             // Index for Sub-Stream
    int   nxwin, nywin;         // Number of Panels in X and Y
    int   nx_index, ny_index;   // Index for Panesl
    int   err_code;             // Error Code
    float xwin_incr, ywin_incr; // Position Increment of Panels
    float x_text, y_text;       // Text Drawing Position
    float peakVal;              // Line peak value to display [dB]
    char  text[256];            // Text to Write

    cpgsch(0.5);
    nxwin = (int)sqrt((double)param_ptr->num_st / 2);
    nywin = (param_ptr->num_st / 2 + nxwin -1)/nxwin;
    xwin_incr = 0.9 / (float)nxwin;
    ywin_incr = 0.9 / (float)nywin;
    for(st_index = 0; st_index < param_ptr->num_st / 2; st_index++){
        nx_index = st_index % nxwin;
        ny_index = st_index / nxwin;

        // PLOT WINDOW
        xmin = freq_ptr[0];
        xmax = freq_ptr[MAX_CH_VIEW - 1];
        ymin = -100.0;
        ymax = 0.0;
        peakVal = -1.0e6;
        for(ch_index = 0; ch_index < MAX_CH_VIEW; ch_index++){
            plot_y[ch_index] = 10.0 * log10(
                sqrt(
                    pow(spec_ptr[(2*st_index)*MAX_CH_VIEW + 2*ch_index], 2.0) + pow(spec_ptr[(2*st_index)*MAX_CH_VIEW + 2*ch_index + 1], 2.0)
                )
            );
        }

        // Peak Search
        for(ch_index = 0.1*MAX_CH_VIEW; ch_index<0.9*MAX_CH_VIEW; ch_index++){
            peakVal = MAX(peakVal, plot_y[ch_index]);
        }
        cpgsvp(
            0.067 + xwin_incr * nx_index, 
            0.067 + xwin_incr * (nx_index + 0.9),
            0.067 + ywin_incr * ny_index, 
            0.067 + ywin_incr * (ny_index + 0.9)
        );
        cpgswin(xmin, xmax, ymin, ymax);

        cpgsci(2);	cpgrect(xmin, xmax, ymin, ymax);
        cpgsci(0);	cpgbox("G", 0.0, 0, "G", 10.0, 0);
        cpgsci(1);	cpgbox("BCNTS", 0.0, 0, "BCNTS", 10.0, 10);
        cpgsci(3);	cpgline(MAX_CH_VIEW, freq_ptr, plot_y);

        //-------- IF number
        x_text = xmin * 0.3 + xmax * 0.7;
        y_text = ymin * 0.1 + ymax * 0.9;
        sprintf(text, "IF=%d Peak%7.2f dB", st_index, peakVal);
        cpgsci(3);
        cpgtext(x_text, y_text, text);
    }

    // UTC
    x_text = xmin * 0.3 + xmax * 0.7;
    y_text = ymin * 0.05 + ymax * 0.95;
    sprintf(text, "%04d %03d %02d:%02d:%02d", param_ptr->year, param_ptr->doy, param_ptr->hour, param_ptr->min, param_ptr->sec);
    cpgsci(1);
    cpgtext(x_text, y_text, text);

    cpgebuf();

    return 0;
}
