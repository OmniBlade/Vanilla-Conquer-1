//
// Copyright 2020 Electronic Arts Inc.
//
// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection

/* $Header: /CounterStrike/RANDOM.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : RANDOM.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 02/27/96                                                     *
 *                                                                                             *
 *                  Last Update : February 27, 1996 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef RANDOM_H
#define RANDOM_H

/**********************************************************************
** Enable this define to turn on the random # counters.  This must not
** be defined for the release version, or saved games won't work!
*/
//#define RANDOM_COUNT

/*
**	This class functions like a 'magic' int value that returns a random number
**	every time it is read. To set the "random seed" for this, just assign a number
**	to the object (just as you would if it were a true 'int' value). Take note that although
**	this class will return an 'int', the actual significance of the random number is
**	limited to 15 bits (0..32767).
*/
class RandomClass
{
public:
    RandomClass(unsigned seed = 0);

    operator int(void)
    {
        return (operator()());
    };
    int operator()(void);
    int operator()(int minval, int maxval);

    enum
    {
        SIGNIFICANT_BITS = 15 // Random number bit significance.
    };

    unsigned int Seed;

#ifdef RANDOM_COUNT
    unsigned int Count1;
    unsigned int Count2;
#endif

protected:
    /*
    **	Internal working constants that are used to generate the next
    **	random number.
    */
    enum
    {
        MULT_CONSTANT = 0x41C64E6D, // K multiplier value.
        ADD_CONSTANT = 0x00003039,  // K additive value.
        THROW_AWAY_BITS = 10        // Low bits to throw away.
    };
};

class Random3Class
{
public:
    Random3Class(unsigned seed1 = 0, unsigned seed2 = 0);

    operator int(void)
    {
        return (operator()());
    };
    int operator()(void);
    int operator()(int minval, int maxval);

    enum
    {
        SIGNIFICANT_BITS = 32
    };

protected:
    static int Mix1[20];
    static int Mix2[20];
    int Seed;
    int Index;
};

template <class T> int Pick_Random_Number(T& generator, int minval, int maxval)
{
    /*
    **	Test for shortcut case where the range is null and thus
    **	the number to return is actually implicit from the
    **	parameters.
    */
    if (minval == maxval) {
        return (minval);
    }

    /*
    **	Ensure that the min and max range values are in proper order.
    */
    if (minval > maxval) {
        int temp = minval;
        minval = maxval;
        maxval = temp;
    }

    /*
    **	Find the highest bit that fits within the magnitude of the
    **	range of random numbers desired. Notice that the scan is
    **	limited to the range of significant bits returned by the
    **	random number algorithm.
    */
    int magnitude = maxval - minval;
    int highbit = T::SIGNIFICANT_BITS - 1;
    while ((magnitude & (1 << highbit)) == 0 && highbit > 0) {
        highbit--;
    }

    /*
    **	Create a full bit mask pattern that has all bits set that just
    **	barely covers the magnitude of the number range desired.
    */
    int mask = ~((~0L) << (highbit + 1));
    int pick = magnitude + 1;

    /*
    **	Keep picking random numbers until it fits within the magnitude desired.
    */
    while (pick > magnitude) {
        pick = generator() & mask;
    }

    /*
    **	Finally, bias the random number pick to the start of the range
    **	requested.
    */
    return (pick + minval);
}

#endif
