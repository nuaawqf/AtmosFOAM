// Read in prognostic variables

Info<< "Creating Exner_init\n" << endl;
volScalarField Exner_init
(
    IOobject("Exner_init", runTime.constant(), mesh, IOobject::MUST_READ),
    mesh
);

Info<< "Setting Exner\n" << endl;
volScalarField Exner
(
    IOobject("Exner", runTime.timeName(), mesh, IOobject::NO_READ),
    Exner_init
);

Info<< "Reading thetaf\n" << endl;
surfaceScalarField thetaf
(
    IOobject("thetaf", runTime.timeName(), mesh, IOobject::MUST_READ),
    mesh
);

if (!args.options().found("noInterpolate"))
{
    Info<< "Reading theta\n" << endl;
    const volScalarField theta
    (
        IOobject("theta", runTime.timeName(), mesh, IOobject::MUST_READ),
        mesh
    );

    thetaf = fvc::interpolate(theta);
}

// The flux, including only the gravity term
surfaceScalarField U
(
    IOobject("U", runTime.timeName(), mesh),
    H.Hdiag()*gd
);

// The diffusion coefficiednt for the laplacian
surfaceScalarField gradPcoeff
(
    IOobject("gradPcoeff", runTime.timeName(), mesh),
    H.Hdiag()*Cp*thetaf*H.magd()/mesh.magSf()
);

label groundBC = -1;
label topBC = -1;
forAll(mesh.boundaryMesh(), patchi)
{
    if (mesh.boundaryMesh()[patchi].name() == "top")
    {
        topBC = patchi;
    }
    else if (mesh.boundaryMesh()[patchi].name() == "ground")
    {
        groundBC = patchi;
    }
}
if (groundBC == -1 || topBC == -1)
{
    FatalErrorIn("setExnerBalancedH")
        << " no boundary called top or ground. The boundaries are called "
        << mesh.boundaryMesh().names()
        << exit(FatalError);
}

