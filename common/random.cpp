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

/* $Header: /CounterStrike/RANDOM.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : RAND.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 02/28/96                                                     *
 *                                                                                             *
 *                  Last Update : February 28, 1996 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RandomClass::RandomClass -- Constructor for the random number class.                      *
 *   RandomClass::operator() -- Fetches the next random number in the sequence.                *
 *   RandomClass::operator() -- Ranged random number generator.                                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "random.h"
#ifdef RANDOM_COUNT
#include <stdio.h>
extern long Frame;
#endif

/***********************************************************************************************
 * RandomClass::RandomClass -- Constructor for the random number class.                        *
 *                                                                                             *
 *    This constructor can take an integer as a parameter. This allows the class to be         *
 *    constructed by assigning an integer to an existing object. The compiler creates a        *
 *    temporary with the constructor and then performs a copy constructor operation.           *
 *                                                                                             *
 * INPUT:   seed  -- The optional starting seed value to use.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/27/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
RandomClass::RandomClass(unsigned seed)
    : Seed(seed)
{
#ifdef RANDOM_COUNT
    Count1 = 0;
    Count2 = 0;
#endif
}

/***********************************************************************************************
 * RandomClass::operator() -- Fetches the next random number in the sequence.                  *
 *                                                                                             *
 *    This routine will fetch the next random number in the sequence.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the next random number.                                               *
 *                                                                                             *
 * WARNINGS:   This routine modifies the seed value so that subsequent calls will not return   *
 *             the same value. Take note that this routine only returns 15 bits of             *
 *             random number.                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/27/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int RandomClass::operator()(void)
{
#ifdef RANDOM_COUNT
    Count1++;
    printf("Frame %d: Random Count1:%d Count2:%d (%x)\n", Frame, Count1, Count2, Seed);
#endif

    /*
    **	Transform the seed value into the next number in the sequence.
    */
    Seed = (Seed * MULT_CONSTANT) + ADD_CONSTANT;

    /*
    **	Extract the 'random' bits from the seed and return that value as the
    **	random number result.
    */
    return ((Seed >> THROW_AWAY_BITS) & (~((~0) << SIGNIFICANT_BITS)));
}

/***********************************************************************************************
 * RandomClass::operator() -- Ranged random number generator.                                  *
 *                                                                                             *
 *    This function will return with a random number within the range specified. This replaces *
 *    the functionality of IRandom() in the old library.                                       *
 *                                                                                             *
 * INPUT:   minval   -- The minimum value to return from the function.                         *
 *                                                                                             *
 *          maxval   -- The maximum value to return from the function.                         *
 *                                                                                             *
 * OUTPUT:  Returns with a random number that falls between the minval and maxval (inclusive). *
 *                                                                                             *
 * WARNINGS:   The range specified must fall within the maximum bit significance of the        *
 *             random number algorithm (15 bits), otherwise the value returned will be         *
 *             decidedly non-random.                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/27/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int RandomClass::operator()(int minval, int maxval)
{
#ifdef RANDOM_COUNT
    Count2++;
    printf("Frame %d: Random Count1:%d Count2:%d (%x)\n", Frame, Count1, Count2, Seed);
#endif

    return (Pick_Random_Number(*this, minval, maxval));
}

int Random3Class::Mix1[20] = {
    0x0baa96887, 0x01e17d32c, 0x003bcdc3c, 0x00f33d1b2, 0x076a6491d, 0x0c570d85d, 0x0e382b1e3,
    0x078db4362, 0x07439a9d4, 0x09cea8ac5, 0x089537c5c, 0x02588f55d, 0x0415b5e1d, 0x0216e3d95,
    0x085c662e7, 0x05e8ab368, 0x03ea5cc8c, 0x0d26a0f74, 0x0f3a9222b, 0x048aad7e4,
};

int Random3Class::Mix2[20] = {
    0x04b0f3b58, 0x0e874f0c3, 0x06955c5a6, 0x055a7ca46, 0x04d9a9d86, 0x0fe28a195, 0x0b1ca7865,
    0x06b235751, 0x09a997a61, 0x0aa6e95c8, 0x0aaa98ee1, 0x05af9154c, 0x0fc8e2263, 0x0390f5e8c,
    0x058ffd802, 0x0ac0a5eba, 0x0ac4874f6, 0x0a9df0913, 0x086be4c74, 0x0ed2c123b,
};

Random3Class::Random3Class(unsigned seed1, unsigned seed2)
    : Seed(seed1)
    , Index(seed2)
{
}

int Random3Class::operator()(void)
{
    int loword = Seed;
    int hiword = Index++;
    for (int i = 0; i < 4; i++) {
        int hihold = hiword;
        int temp = hihold ^ Mix1[i];
        int itmpl = temp & 0xffff;
        int itmph = temp >> 16;
        temp = itmpl * itmpl + ~(itmph * itmph);
        temp = (temp >> 16) | (temp << 16);
        hiword = loword ^ ((temp ^ Mix2[i]) + itmpl * itmph);
        loword = hihold;
    }
    return (hiword);
}

int Random3Class::operator()(int minval, int maxval)
{
    return (Pick_Random_Number(*this, minval, maxval));
}
