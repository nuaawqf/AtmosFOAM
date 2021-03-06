/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2013 OpenFOAM Foundation
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

Application
    advectionFoam

Description
    Solves a transport equation for a passive scalar using explicit leap-frog
    time-stepping or RK2

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "OFstream.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"
    #define dt runTime.deltaT()
    #include "readEnvironmentalProperties.H"
    #include "createFields.H"

    Info<< "\nCalculating advection\n" << endl;

    #include "CourantNo.H"

    while (runTime.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        for (int corr=0; corr < 3; corr++)
        {
            Tf = Tf.oldTime() - 0.5*dt *
            (
                (Uf & fvc::interpolate(fvc::grad(Tf))) + 
                (Uf & fvc::interpolate(fvc::grad(Tf.oldTime())))
            );

	    bf = Tf * gUnitNormal;
	    b = fvc::reconstruct(bf * mesh.magSf());
	    T = b & ghat;
	    Tf = mag(bf) + (1.0 - mag(gUnitNormal))*fvc::interpolate(T, "T_from_b");
        }
        
        Info << " Tf goes from " << min(Tf.internalField()) << " to "
             << max(Tf.internalField()) << endl;
        runTime.write();
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
