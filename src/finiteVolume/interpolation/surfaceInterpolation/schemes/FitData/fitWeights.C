#include "fitWeights.H"

Foam::fitWeights::fitWeights(
    const fitWeights& source
)
:
    cellWeights(source.cellWeights)
{}

Foam::fitWeights::fitWeights
(
    const label cells
)
:
    cellWeights(cells, scalar(1))
{}

void Foam::fitWeights::copyFrom(const scalarList& source)
{
    forAll(source, i)
    {
        cellWeights[i] = source[i];
    }
}

void Foam::fitWeights::setCentralWeight
(
    const scalar weight,
    const bool pureUpwind
)
{
    cellWeights[0] = weight;
    if (!pureUpwind) cellWeights[1] = weight;
}

void Foam::fitWeights::removeDownwindWeight()
{
    cellWeights[1] = 1.0;
}

scalar Foam::fitWeights::downwind() const
{
    return cellWeights[1];
}

scalar& Foam::fitWeights::downwind()
{
    return cellWeights[1];
}

scalar& Foam::fitWeights::operator[](int i)
{
    return cellWeights[i];
}

scalar Foam::fitWeights::operator[](int i) const
{
    return cellWeights[i];
}

Foam::Ostream& Foam::operator<<
(
    Ostream& stream,
    const fitWeights& w
)
{
    stream << w.cellWeights;
    return stream;
}