/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::cubicUpwindFitPolynomial

Description
    Cubic polynomial for upwind biased interpolation fitting.

    Can be used with the UpwindFit scheme to create a cubic surface
    interpolation scheme

\*---------------------------------------------------------------------------*/

#ifndef cubicUpwindCPCFitPolynomial_H
#define cubicUpwindCPCFitPolynomial_H

#include "vector.H"

namespace Foam
{
    class cubicUpwindCPCFitPolynomial
    {
    public:

        static label nTerms(const direction dim)
        {
            return
            (
                dim == 1 ? 4 :
                dim == 2 ? 9 :
                dim == 3 ? 16 : 0
            );
        }

        static void candidates(List<uint32_t>& candidates, const direction dim)
        {
            if (dim == 2)
            {
                const label constant = 1 << 0;
                const label x = 1 << 1;
                const label y = 1 << 2;
                const label xx = 1 << 3;
                const label xy = 1 << 4;
                const label yy = 1 << 5;
                const label xxx = 1 << 6;
                const label xxy = 1 << 7;
                const label xyy = 1 << 8;
                candidates.append(constant | x);
                candidates.append(constant | y);
                candidates.append(constant | x | y);
                candidates.append(constant | x | xx);
                candidates.append(constant | y | yy);
                candidates.append(constant | x | y  | xy);
                candidates.append(constant | x | y  | xx);
                candidates.append(constant | x | y  | yy);
                candidates.append(constant | x | xx | xxx);
                candidates.append(constant | x | y  | xy  | xx);
                candidates.append(constant | x | y  | xy  | yy);
                candidates.append(constant | x | y  | xx  | xxx);
                candidates.append(constant | x | y  | xy  | xx  | xxx);
                candidates.append(constant | x | y  | xy  | xx  | yy);
                candidates.append(constant | x | y  | xy  | xx  | xxy);
                candidates.append(constant | x | y  | xy  | yy  | xyy);
                candidates.append(constant | x | y  | xy  | xx  | yy  | xxy);
                candidates.append(constant | x | y  | xy  | xx  | yy  | xyy);
                candidates.append(constant | x | y  | xy  | xx  | yy  | xxx);
                candidates.append(constant | x | y  | xy  | xx  | xxy | xxx);
                candidates.append(constant | x | y  | xy  | xx  | yy  | xxy | xyy);
                candidates.append(constant | x | y  | xy  | xx  | yy  | xxy | xxx);
                candidates.append(constant | x | y  | xy  | xx  | yy  | xyy | xxx);
                candidates.append(constant | x | y  | xy  | xx  | yy  | xxy | xyy | xxx);
            }
        }

        static void addCoeffs
        (
            scalar* coeffs,
            const vector& d,
            const scalar weight,
            const direction dim
        )
        {
            register label curIdx = 0;

            // degree 0 
            coeffs[curIdx++] = weight;

            // degree 1
            coeffs[curIdx++] = weight*d.x();
            if (dim >= 2) coeffs[curIdx++] = weight*d.y();
            if (dim == 3) coeffs[curIdx++] = weight*d.z();

            // degree 2
            coeffs[curIdx++] = weight*sqr(d.x());
            if (dim >= 2) coeffs[curIdx++] = weight*d.x()*d.y();
            if (dim >= 2) coeffs[curIdx++] = weight*sqr(d.y());
            if (dim == 3) coeffs[curIdx++] = weight*d.x()*d.z();
            if (dim == 3) coeffs[curIdx++] = weight*d.y()*d.z();
            if (dim == 3) coeffs[curIdx++] = weight*sqr(d.z());

            // degree 3
            coeffs[curIdx++] = weight*pow(d.x(),3);
            if (dim >= 2) coeffs[curIdx++] = weight*sqr(d.x())*d.y();
            if (dim >= 2) coeffs[curIdx++] = weight*d.x()*sqr(d.y());
            if (dim == 3)
            {
                coeffs[curIdx++] = weight*d.x()*d.y()*d.z();
                coeffs[curIdx++] = weight*sqr(d.x())*d.z();
                coeffs[curIdx++] = weight*d.x()*sqr(d.z());
            }
        }
    };
}

#endif
