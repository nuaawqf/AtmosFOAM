/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2009 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Application
    initMoistFoam

Description
    with inital uniform theta_e and uniform r_t
    Initialise moistFoam, the compressible atmospheric solver with moisture

\*---------------------------------------------------------------------------*/

#include "HodgeOps.H"
#include "fvCFD.H"
#include "moistThermo.H"
#include "mathematicalConstants.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"
    #include "orthogonalBoundaries.H"
    #include "readEnvironmentalProperties.H"
    #include "readThermoProperties.H"
    #include "readThermoPropertiesMoist.H"
    #include "readInitProps.H"
    HodgeOps H(mesh);
    surfaceScalarField gd("gd", g & H.delta());
    #define dt runTime.deltaT()
    #include "createFields.H"
    static scalar piby2 = 0.5*constant::mathematical::pi;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    bool innerConverged = false;
    bool outerConverged = false;
    scalar topBCval = Exner.boundaryField()[topBC][0];
    for
    (
        label iter = 0;
        iter < maxIters && !(innerConverged && outerConverged);
        iter++
    )
    {
        Info << "Outer iteration " << iter << endl;
        
        // Inner iterations with fixed top boundary
        innerConverged = false;
        for(label BCiter = 0; BCiter < BCiters && !innerConverged; BCiter++)
        {
            Info << "Outer " << iter << " inner " << BCiter << endl;
            gradPcoeff = gradPCoeff(theta, rv, rl, Cp, epsilon);
            fvScalarMatrix ExnerEqn
            (
                fvc::div(U)
              - fvm::laplacian(gradPcoeff, Exner)
            );
            innerConverged
                = ExnerEqn.solve(mesh.solver(Exner.name())).nIterations() == 0;

            // More inner iterations for moisture variables
            p = pFromExner(Exner, kappa, pRef);
            for(label it = 0; it < 4; it++)
            {
                // Temperature from thetae
                TfromThetae(T, thetae0, p, Lv, rv, rt0, pRef, epsilon,Cp,Cpl,R);
                // Latent heat and saturation vapour pressure from T
                Lv == latentHeat(T, Lv0, T0, Cpl, Cpv);

                // Calculate saturation vapour pressure and set rv to be saturated
                es == pSat(T, pSat0, Lv0, Rv, T0);
                rvs == epsilon*es/(p-es);
                // Update rv with under-relaxation
                rv == max
                (
                    min((1-underRelax)*rv + underRelax*rvs, rt0),
                    scalar(0)
                );
                
                condenseRate = (rv - rvs)/(dt*(1 + sqr(Lv)*rvs/(Cp*Rv*sqr(T))));
                Info << "condenseRate goes from "  << min(condenseRate).value() 
                     << " to " << max(condenseRate).value() << endl;
            }
            rl == max(rt0 - rv, scalar(0));
        
            // Update theta
            theta == T/Exner;
        }
        scalar maxGroundExner = max(Exner.boundaryField()[groundBC]);
        outerConverged = (mag(1-maxGroundExner)< BCtol);

        // modify the top boundary condition
        Info << "Exner ground value = " << maxGroundExner
             << "  ground value minus one = "
             << maxGroundExner-1
             << " Exner top BC going from  = " << topBCval << " to ";

        topBCval = (1-boundaryRelaxation)*topBCval
                 + boundaryRelaxation*Exner.boundaryField()[topBC][0]
                       /maxGroundExner;
        topBCval = min(max(topBCval, scalar(0)), scalar(1));
        Info << topBCval << endl;
        Exner.boundaryFieldRef()[topBC] = topBCval;
        
        Info << "thata goes from " << min(theta).value()
             << " to " << max(theta).value() << " rl from "
             << min(rl).value() << " to " << max(rl).value() 
             << " rv from " << min(rv).value() << " to " << max(rv).value()
             << endl;
    }

    // Reference profiles of theta, rv and rl for calculating differences
    volScalarField thetaRef
    (
        IOobject("thetaRef", runTime.constant(), mesh), theta
    );
    volScalarField rvRef(IOobject("rvRef", runTime.constant(), mesh), rv);
    volScalarField rlRef(IOobject("rlRef", runTime.constant(), mesh), rl);
    thetaRef.write();
    rvRef.write();
    rlRef.write();

    Info << "Adding bouyancy perturbation and re-calculating rv and rl" << endl;
    // Cell where bouancy perturbation is maximum
    label cellMax = 0;
    forAll(theta, celli)
    {
        scalar L = Foam::sqrt
        (
            sqr((mesh.C()[celli].x() - bubbleCentre.x())/bubbleRadii.x())
          + sqr((mesh.C()[celli].y() - bubbleCentre.y())/bubbleRadii.y())
          + sqr((mesh.C()[celli].z() - bubbleCentre.z())/bubbleRadii.z())
        );

        if (L < 1)
        {
            thetaScale[celli] += thetaPrime*sqr(Foam::cos(piby2*L));
            if (thetaScale[celli] > thetaScale[cellMax])
            {
                cellMax = celli;
            }
        }
    }
    // Interate so that new theta, rv and rl are in balance
    volScalarField thetaRho0 = theta*(1+rv/epsilon)/(1+rt0);
    for(label iter = 0; iter < 5; iter++)
    {
        theta == thetaRho0*(1 + rt0)/(1+rv/epsilon)*thetaScale;
        T == theta*Exner;
        es == pSat(T, pSat0, Lv0, Rv, T0);
        rvs == epsilon*es/(p-es);
        rv == max(min(rvs, rt0), scalar(0));
        Info << "Bouyancy perturbation " << iter << " theta at centre = "
             << theta[cellMax] << nl;
    }
    rl == max(rt0 - rv, scalar(0));

    theta.write();
    rv.write();
    rl.write();
    
    // Additional diagnositcs for de-bugging
    p = pFromExner(Exner, kappa, pRef);
    p.write();
    T.write();

    // Change the top boundary type to be fixedFluxBuoyantExner
    wordList ExnerBCs = Exner.boundaryField().types();
    ExnerBCs[topBC] = "fixedFluxBuoyantExnerMoist";
    volScalarField ExnerNew
    (
        IOobject("Exner", runTime.timeName(), mesh, IOobject::NO_READ),
        Exner,
        ExnerBCs
    );
    ExnerNew.correctBoundaryConditions();
    ExnerNew.write();
    
    // Check thetae
    volScalarField thetae = thetaeFromPrimitive
    (
        T, p, Lv, rv, rt0, pRef, epsilon, Cp, Cpl, R
    );
    thetae.write();

    return 0;
}


// ************************************************************************* //
