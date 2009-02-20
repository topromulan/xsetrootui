/*
	ui.c

	"xsetrootui"

	2001 Macrowave Brands
*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Scrollbar.h>

#include <stdio.h>
#include <stdlib.h>


/**  GUI Objects  **/

Widget toplevel, form;		/* self-explanatory */
Widget RGB[3];			/* red, green, blue value scrollbars */
Widget Mod[2];			/* x and y mod scrollbars */
Widget rand_button, quit_button;	/* command buttons */
#include "maclogo.h"
Widget mwlogo_space; Pixmap mwlogo_pic; /* label widget for logo pixmap */
Widget FGBGToggle;		/* toggle to adjust foreground or back */
Widget Labels[4];		/* explains scrollbars to the user */

/**               **/


/** GUI Functions **/

#define callbackfn(x) void x (Widget w, XtPointer client, XtPointer call)


 /* callback */
callbackfn(RGB1_scroll);	/* red scroll has changed */
callbackfn(RGB2_scroll);	/* green scroll has changed */
callbackfn(RGB3_scroll);	/* blue scroll has changed */
callbackfn(Mod1_scroll);	/* x mod scroll has changed */
callbackfn(Mod2_scroll);	/* y mod scroll has changed */
callbackfn(FGBGToggle_command);	/* toggled foreground/background */
callbackfn(rand_command);	/* write the change to .xinitrc */
callbackfn(quit_command);	/* self-explanatory */


 /* event */
void redraw_event(Widget w, XtPointer client, XExposeEvent *ev);

 /* internal */
void RGB_update_scrolls();

/**               **/


/**  Application Variables  **/

char FGBGState;			/* 1=foreground 0=background */

int FG[3], 			/* scrollbar callbacks memory */
    BG[3], 			/* values (bargains) into these */
    mod1, 
    mod2;

float FGScrollStates[3],
      BGScrollStates[3];

char Commandbuff[1024];		/* string space for system command */

signed int KnownWidth,		/* avoid duplicate redrawas */
           KnownHeight;
		
/**                         **/


/** Application Subroutines **/
void explainusage();
void rescale_logo(signed int w, signed int h);
void setroot();
int powerof2(int x);
/**                         **/



/******/
/*MAIN*/ /* the user wants to run the application */
/******/ /* do so in a nonstructured oriented fashion */

int main(int argc, char **argv)
{

	/* check the command line parameters */

	int i;
	
	if(argc > 1) {
		explainusage();
		return 1;
	}

	// XSETMADROOT!

	/* initialize all global variables */

	FG[0] = 0;
	FG[1] = 0;
	FG[2] = 0;

	BG[0] = 0;
	BG[1] = 0;
	BG[2] = 0;

	mod1 = 0;
	mod2 = 0;

	toplevel = XtInitialize("xsetrootui", "xsetrootui", NULL, 0, &argc, argv);

	form = XtCreateManagedWidget("form", formWidgetClass, toplevel,
					NULL, 0);

	FGBGToggle = XtVaCreateManagedWidget(
					"FG/BG", toggleWidgetClass, form,
					XtNwidth, 102,
					XtNlabel, "Background",
					NULL
					);

	RGB[0] = XtVaCreateManagedWidget(
					"Red", scrollbarWidgetClass, form,
					XtNheight, 100, XtNwidth, 30,
					XtNfromVert, FGBGToggle,
					NULL
					);

        RGB[1] = XtVaCreateManagedWidget(
                                        "Green", scrollbarWidgetClass, form,
                                        XtNheight, 100, XtNwidth, 30,
					XtNfromVert, FGBGToggle,
					XtNfromHoriz, RGB[0],
                                        NULL  
                                        );

        RGB[2] = XtVaCreateManagedWidget(
                                        "Blue", scrollbarWidgetClass, form,
                                        XtNheight, 100, XtNwidth, 30,
					XtNfromVert, FGBGToggle,
					XtNfromHoriz, RGB[1],
                                        NULL  
                                        );

        Mod[0] = XtVaCreateManagedWidget(
                                        "H", scrollbarWidgetClass, form,
                                        XtNheight, 100, XtNwidth, 30,
					XtNfromVert, FGBGToggle,
					XtNfromHoriz, RGB[2],
                                        NULL  
                                        );

        Mod[1] = XtVaCreateManagedWidget(
                                        "V", scrollbarWidgetClass, form,
                                        XtNheight, 100, XtNwidth, 30,
					XtNfromVert, FGBGToggle,
					XtNfromHoriz, Mod[0],
                                        NULL  
                                        );

	Labels[0] = XtVaCreateManagedWidget("label r", labelWidgetClass, form,
					XtNwidth, 30,
					XtNlabel, "R",
					XtNfromVert, RGB[0],	
					NULL
					);

	Labels[1] = XtVaCreateManagedWidget("label g", labelWidgetClass, form,
					XtNwidth, 30,
					XtNlabel, "G",
					XtNfromHoriz, Labels[0],
					XtNfromVert, RGB[0],
					NULL
					);

	Labels[2] = XtVaCreateManagedWidget("label b", labelWidgetClass, form,
					XtNwidth, 30,
					XtNlabel, "B",
					XtNfromHoriz, Labels[1],
					XtNfromVert, RGB[0],
					NULL
					);
	
	Labels[3] = XtVaCreateManagedWidget("label mod", labelWidgetClass, form,
					XtNwidth, 66,
					XtNlabel, "mod",
					XtNfromHoriz, Labels[2],
					XtNfromVert, RGB[0],
					NULL
					);

	mwlogo_pic = XCreatePixmapFromBitmapData(
					XtDisplay(toplevel),
					DefaultRootWindow(XtDisplay(toplevel)),
					maclogo_bits,
					maclogo_width, maclogo_height,
					BlackPixelOfScreen(XtScreen(toplevel)),
					WhitePixelOfScreen(XtScreen(toplevel)),
					DefaultDepthOfScreen(XtScreen(toplevel))					);

	mwlogo_space = XtVaCreateManagedWidget("logo", labelWidgetClass, form,
					XtNwidth, maclogo_width,
					XtNheight, maclogo_height,
					XtNbackgroundPixmap, mwlogo_pic,
					XtNlabel, "",
					XtNfromHoriz, Mod[1],
					XtNfromVert, FGBGToggle,
					NULL
					); 

	rand_button = XtVaCreateManagedWidget(
					"rand", commandWidgetClass, form,
					XtNshapeStyle, XawShapeOval,
					XtNcornerRoundPercent, 50,
					XtNwidth, maclogo_width,
					XtNlabel, "random",
					XtNfromVert, mwlogo_space,
					XtNfromHoriz, Mod[1],
					NULL
					);

	quit_button = XtVaCreateManagedWidget(
					"quit", commandWidgetClass, form,
					XtNshapeStyle, XawShapeOval,
					XtNwidth, maclogo_width,
					XtNlabel, "quit",
					XtNfromVert, rand_button,
					XtNfromHoriz, Mod[1],
					NULL
					);

	XtAddCallback(RGB[0], XtNjumpProc, RGB1_scroll, NULL);
        XtAddCallback(RGB[1], XtNjumpProc, RGB2_scroll, NULL);
        XtAddCallback(RGB[2], XtNjumpProc, RGB3_scroll, NULL);
        XtAddCallback(Mod[0], XtNjumpProc, Mod1_scroll, NULL);
        XtAddCallback(Mod[1], XtNjumpProc, Mod2_scroll, NULL);

	XtAddCallback(FGBGToggle, XtNcallback, FGBGToggle_command, NULL);
	XtAddCallback(rand_button, XtNcallback, rand_command, NULL);
        XtAddCallback(quit_button, XtNcallback, quit_command, NULL);

	XtAddEventHandler(mwlogo_space, ExposureMask, FALSE, redraw_event, NULL);

	setroot();

	/* we tried to get ready now try to run the program */

	XtRealizeWidget(toplevel);
	XtMainLoop();

}


callbackfn(RGB1_scroll)
{
	if(FGBGState == 1) /* foreground */ {
		FG[0] = 255 * (*(float *)call);
	} else /* background */ {
		BG[0] = 255 * (*(float *)call);
	}

	setroot();
}

callbackfn(RGB2_scroll)
{
	if(FGBGState == 1) /* foreground */ {
		FG[1] = 255 * (*(float *)call);
	} else /* background */ {
		BG[1] = 255 * (*(float *)call);
	}

	setroot();
}

callbackfn(RGB3_scroll)
{
	if(FGBGState == 1) /* foreground */ {
		FG[2] = 255 * (*(float *)call);
	} else /* background */ {
		BG[2] = 255 * (*(float *)call);
	}

	setroot();
}

callbackfn(Mod1_scroll)
{
	mod1 = 16 * (*(float *)call);
	setroot();
}

callbackfn(Mod2_scroll)                
{
	mod2 = 16 * (*(float *)call);
	setroot();
}

/* Adjust whether RGB scrolls effecting foreground or back */
callbackfn(FGBGToggle_command)
{

	/* this is the only place FGBGState gets set */
	XtVaGetValues(w, XtNstate, &FGBGState, NULL);
	/* the value we getting is already been toggled by the widget */

	if(FGBGState == 1) /* foreground */ {

		/* save the state of the RGB scrollers */
		XtVaGetValues(RGB[0], XtNtopOfThumb, &BGScrollStates[0], NULL);
                XtVaGetValues(RGB[1], XtNtopOfThumb, &BGScrollStates[1], NULL);
                XtVaGetValues(RGB[2], XtNtopOfThumb, &BGScrollStates[2], NULL);

		/* change the toggles text */
		XtVaSetValues(w, XtNlabel, "Foreground", NULL);

	} else /* background */ {

                /* save the state of the RGB scrollers */
                XtVaGetValues(RGB[0], XtNtopOfThumb, &FGScrollStates[0], NULL);
                XtVaGetValues(RGB[1], XtNtopOfThumb, &FGScrollStates[1], NULL);
                XtVaGetValues(RGB[2], XtNtopOfThumb, &FGScrollStates[2], NULL);

		/* change the toggles text */
		XtVaSetValues(w, XtNlabel, "Background", NULL);

	}

	/* update the scrollbars */
	RGB_update_scrolls();

}

/* user is not happy with own selection and wants a one click solution */
callbackfn(rand_command)
{
	float mod_float_convert;

#define myrnd(x) (int) (  ( ((x * 1.0)*rand()) / (RAND_MAX) )  )


	FG[0] = myrnd(255); FG[1] = myrnd(255); FG[2] = myrnd(255);
  	BG[0] = myrnd(255); BG[1] = myrnd(255); BG[2] = myrnd(255);
	mod1 = myrnd(16); mod2 = myrnd(16);

	FGScrollStates[0] = (float) FG[0] / 256.0;
	FGScrollStates[1] = (float) FG[1] / 256.0;
	FGScrollStates[2] = (float) FG[2] / 256.0;
	BGScrollStates[0] = (float) BG[0] / 256.0;
	BGScrollStates[1] = (float) BG[1] / 256.0;
	BGScrollStates[2] = (float) BG[2] / 256.0;

	/* manually updates the mod1 and mod2 bars */
	mod_float_convert = (float) mod1 / 16.0;

                XtVaSetValues(Mod[0],
                        XtNtopOfThumb, *(XtArgVal*)&mod_float_convert,
                        NULL
                        );

	mod_float_convert = (float) mod2 / 16.0;

                XtVaSetValues(Mod[1],
                        XtNtopOfThumb, *(XtArgVal*)&mod_float_convert,
                        NULL
                        );


	/* update the rest with this function */
	RGB_update_scrolls();

	setroot();
}

/* no explanation needed */
callbackfn(quit_command)
{
	exit(0);
}

/* an expose event has occured, possibly indicating a resize;
   if a resize occured i need to execute the biggest waste of
   time function to rescale the macrowave logo */
void redraw_event(Widget w, XtPointer client, XExposeEvent *ev)
{
	signed int height=0, width=0;

	printf("redraw event\n");

	/* if the window has been resized, rescale the macrowave logo */
	XtVaGetValues(mwlogo_space, XtNwidth, &width, NULL);
	XtVaGetValues(mwlogo_space, XtNheight, &height, NULL);

	/* i'll know because i saved KnownWidth and Height last time */
	if((width == KnownWidth) && (height == KnownHeight)) 
		/* no need to rescale for this! */ {
		char *temporarilywastedmemory="A MACROWAVE BRAND";
	} else /* change */ {
		rescale_logo(width, height);
	}
	
	/* update for the current sizes */
	KnownWidth = width;
	KnownHeight = height;
}


void RGB_update_scrolls()
{
        if(FGBGState == 1) /* foreground */ {

        	/* adjust those cute RGB scrolls */
        	XtVaSetValues(RGB[0],
                	XtNtopOfThumb, *(XtArgVal*)&FGScrollStates[0],
                	NULL
                	);

        	XtVaSetValues(RGB[1],
                	XtNtopOfThumb, *(XtArgVal*)&FGScrollStates[1],
                	NULL
                	);

        	XtVaSetValues(RGB[2],
                	XtNtopOfThumb, *(XtArgVal*)&FGScrollStates[2],
                	NULL
                	);

	} else 	/* background */ {
                /* adjust those cute RGB scrolls */
                XtVaSetValues(RGB[0],
                        XtNtopOfThumb, *(XtArgVal*)&BGScrollStates[0],
                        NULL
                        );

                XtVaSetValues(RGB[1],
                        XtNtopOfThumb, *(XtArgVal*)&BGScrollStates[1],
                        NULL
                        );

                XtVaSetValues(RGB[2],
                        XtNtopOfThumb, *(XtArgVal*)&BGScrollStates[2],
                        NULL
                        );
	}

}



void explainusage(void)
{
}

/* this function tries to improve the Macrowave brand credibility
   by rescaling the macrowave logo Pixmap. other brands logos
   in this situation may tile or crop */
void rescale_logo(signed int w, signed int h)
{
	int x, y, /* for-loop counters to walk through the new dimensions */
	    rowsize, /* how much bytes in a single row */
            size, /* how much bytes required for the new pixmap */
	    orig_rowsize; 			

	unsigned char *mem; /* temporary buffer to render new pixmap in */

	float wconversion,  /* "the inverse ratios of old-to-new size" */
		hconversion,
		running_x=0, /* the ratios are added to running totals */
		running_y=0; /* that walk across the original map */

        /* calculate the bytes size of a row */
        rowsize = (w / 8); if ( (w % 8) ) rowsize++;        
        orig_rowsize = (maclogo_width / 8) + 1;

	//size = (w * h) / 8;
	size = rowsize * h;

	printf("%d by %d\n", w, h);

	/* calculate the conversion figures */
	wconversion = (float)maclogo_width / (float)w;
	hconversion = (float)maclogo_height / (float)h;

	/* allocate the memory */
	mem=(char *)malloc(size);

	/*rowsize = (w / 8); if ( (w % 8) ) rowsize++;
	orig_rowsize = (maclogo_width / 8) + 1;
	*/

	/*...................................................... 
	.	walk through the x & y dimension of the new bitmap
	.	using the running total to estimate pixel values from
	.	the original logo 
	............................................................*/

	for(y = 0; y < h; y++)
	{
		for(x = 0; x < w; x++)
		{
			int orig_byte_index, /* array byte current bit is in */
		   	    scale_byte_index,
		  	    orig_bit_index,  /* number of the bit */
 			    scale_bit_index,
			    orig_bit_value,  /* check if pixel is set or no */
  			    scale_bit_mask,  /* if so scale_bit_index bit will
						be binary OR'd with byte */
		  	    orig_x, orig_y;  /* integer conversion/round off
						of running totals */

			/* figure xy of the closest pixel in the original */
			orig_x = running_x;
				if((running_x - (float)orig_x) > 0.5) orig_x++;
			orig_y = running_y;
				if((running_y - (float)orig_y) > 0.5) orig_y++;
			
			/* figure the array bytes pixel lands in */
			scale_byte_index = (y * rowsize) + (x / 8);
			orig_byte_index = (orig_y * orig_rowsize) + (orig_x / 8);
			/* and the bits */
			scale_bit_index = (x % 8);
			orig_bit_index = (orig_x % 8);

			/* binary or to find out if original pixel was set */
			orig_bit_value = maclogo_bits[orig_byte_index] & powerof2(orig_bit_index);
			if(orig_bit_value) /* yes */ {
				scale_bit_mask = powerof2(scale_bit_index);
			} else /* no */ {
				scale_bit_mask = 0;
			}

			/*if it's the first bit, zero the byte*/
			if( ! scale_bit_index )
			{
				mem[scale_byte_index] = (unsigned char)0;
			}

			mem[scale_byte_index] |= (unsigned char)scale_bit_mask;

			/* proportionately increment the running */
			running_x += wconversion;
		}

		running_y += hconversion;
		/* horrible row is over */
		running_x = 0;
	}

	/* make a pixmap from our calculations, update logo, dicard memory */
	mwlogo_pic = XCreatePixmapFromBitmapData(
					XtDisplay(toplevel),
					DefaultRootWindow(XtDisplay(toplevel)),
					mem,
					w, h,
					BlackPixelOfScreen(XtScreen(toplevel)),
					WhitePixelOfScreen(XtScreen(toplevel)),
					DefaultDepthOfScreen(XtScreen(toplevel))					);

	XtVaSetValues(mwlogo_space, XtNbackgroundPixmap, mwlogo_pic , 0);

	/* you are free, mem! */
	free(mem);

}

void setroot(void)
{
	sprintf(Commandbuff, "xsetroot -mod %d %d -bg \"#%02x%02x%02x\" -fg \"#%02x%02x%02x\"", mod1, mod2, BG[0], BG[1], BG[2], FG[0], FG[1], FG[2]);

	system(Commandbuff);

//	printf("%s\n", Commandbuff);

}

int powerof2(int x)
{
	int i, r=2;

	if(x==0)
		return 1;

	for(i=1; i<x; i++)
		r *= 2;

	return r;
}




