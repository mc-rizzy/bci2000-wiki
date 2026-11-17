////////////////////////////////////////////////////////////////////////////////
// $Id: LCRandomGenerator.h 9610 2026-11-10 12:47:47Z mellinger $
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates a Linear Congruential Pseudo Random
//   number generator.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef TINY_LC_RANDOM_GENERATOR_H
#define TINY_LC_RANDOM_GENERATOR_H

#include <cstdint>
#include <string>
#include <random>
#include <algorithm>

namespace Tiny
{

class LCRandomGenerator
{
  public:
    typedef uint32_t SeedType;
    typedef uint32_t NumberType;

    LCRandomGenerator() : mSeed(DefaultSeed())
    {
    }
    LCRandomGenerator(SeedType s) : mSeed(s)
    {
    }
    SeedType DefaultSeed();
    // Properties
    SeedType Seed() const
    {
        return mSeed;
    }
    LCRandomGenerator &SetSeed(SeedType s)
    {
        mSeed = s;
        return *this;
    }

    static constexpr NumberType RandMax()
    {
        return NumberFromSeed(~SeedType(0));
    }
    // A random integer between 0 and including RandMax().
    NumberType Random();
    // A random integer between 0 and (N-1).
    NumberType Random(NumberType N)
    {
        return (Random() * N) / (RandMax() + 1);
    }

    // STL functor interface: operator() returns a random integer between 0 and (N-1).
    template <typename Int> Int operator()(Int inN)
    {
        return Random(inN);
    }

    // STL Uniform Random Bit Generator interface: operator() returns a random integer between 0 and RandMax().
    typedef NumberType result_type;
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return RandMax(); }
    result_type operator()() { return Random(); }

    struct Uniform01
    {
        static double InverseCPDF(double x)
        {
            return x;
        }
    };
    struct Normal
    {
        static double InverseCPDF(double);
    };
    // Use as LCRandomGenerator::RandomValue<Normal>()
    // or LCRandomGenerator::RandomValue<Uniform01>().
    template <class Dist> double RandomValue()
    {
        return Dist::InverseCPDF(Random() * 1.0 / RandMax());
    }

    // We need to provide our own Shuffle() function because std::shuffle() 
    // is implementation dependent with results differing between compilers/platforms.
    template<class RandomIt> void Shuffle(RandomIt first, RandomIt last)
    {
        // adapted from example implementation at cppreference.com
        typedef typename std::iterator_traits<RandomIt>::difference_type diff_t;

        for (diff_t i = last - first - 1; i > 0; --i)
        {
            std::swap(first[i], first[(*this)(i + 1)]);
        }
    }

    std::string RandomName(size_t inLength);
    char RandomCharacter(int (*)(int) = &::isalpha);

  private:
    static constexpr NumberType NumberFromSeed(SeedType inSeed)
    {
        return inSeed >> 16; // Ignore lower bits which have bad periodicity.
    }

    SeedType mSeed;
};

} // namespace Tiny

using Tiny::LCRandomGenerator;

#endif // TINY_LC_RANDOM_GENERATOR_H
