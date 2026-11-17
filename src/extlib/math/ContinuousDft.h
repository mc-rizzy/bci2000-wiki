////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A continuous DFT with an exponential window.
//   Operation of individual bins is similar to a 1st-order digital low-pass filter.
//   At each step, the output is multiplied with a complex-valued constant
//   instead of a real-valued one, before the input is added:
//   $$ output_{i+1} = output_i * c_{\alpha f} + input_i $$.
//   The constant consists of bin width $\alpha$, center frequency $f$, and a
//   normalization factor as follows:
//   $$ c_{\alpha f} = \alpha e^{-\alpha - i 2\pi f} $$.
//   Here, $\alpha$ and $f$ are given in terms of normalized frequency, and
//   are both $>-1/2$ and $<1/2$.
// 
//   Note that for real-valued input, output at negative frequencies will
//   match the complex conjugate of the output at positive frequencies, and
//   thus need not be computed in this case.
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
#ifndef CONTINUOUS_DFT_H
#define CONTINUOUS_DFT_H

#include "Debugging.h"
#include "NumericConstants.h"
#include "GenericSignal.h"
#include <complex>
#include <vector>
#include <algorithm>

namespace ContinuousDft
{
    class Config
    {
    public:
        static Config CreateConstantQ(int binCount)
        {
            const double f0 = 1.0 / binCount;
            const double factor = std::pow(0.5 / f0, 1.0 / binCount);
            double f = f0;
            Config cfg;
            cfg.mBinProps.resize(binCount);
            for (int i = 0; i < binCount; ++i) {
                auto& bin = cfg.mBinProps[i];
                bin.f = f;
                bin.alpha = std::sqrt(f);
                f *= factor;
                bin.alpha *= std::sqrt(f);
                bin.alpha /= 4;
            }
            return cfg;
        }

        static Config CreateLinear(int binCount)
        {
            double alpha = 0.5 / binCount;
            Config cfg;
            cfg.mBinProps.resize(binCount);
            for (int i = 0; i < binCount; ++i) {
                auto& bin = cfg.mBinProps[i];
                bin.alpha = alpha;
                bin.f = i * alpha;
            }
            return cfg;
        }

        void Flip()
        {
            std::reverse(mBinProps.begin(), mBinProps.end());
        }

        int BinCount() const
        {
            return mBinProps.size();
        }

        const double& Width(int i) const
        {
            Assert(i >= 0 && i < mBinProps.size());
            return mBinProps[i].alpha;
        }

        const double& Frequency(int i) const
        {
            Assert(i >= 0 && i < mBinProps.size());
            return mBinProps[i].f;
        }

    private:
        struct BinProps { double f = 0, alpha = 0; };
        std::vector<BinProps> mBinProps;
    };

    template <typename Real> class Bin
    {
    public:
        Bin() = default;
        ~Bin() = default;
        Bin(const Bin&) = default;
        Bin& operator=(const Bin&) = default;

        Bin(Real alpha, Real f)
        {
            Assert(alpha >= 0);
            Assert(f >= 0 && f <= 0.5);
            std::complex<Real> exponent(-alpha, 2 * Pi<Real>() * f);
            mTimePropagator = std::exp(exponent);
            // The norm factor is chosen such that power is proportional to bin width.
            // Test: white noise should be evenly distributed over bins.
            mNormFactor = std::sqrt(alpha);
            Reset();
        }
        Bin& Reset()
        {
            mOutput = 0;
            return *this;
        }
        Bin& Observe(Real input)
        {
            mOutput *= mTimePropagator;
            mOutput += input * mNormFactor;
            return *this;
        }
        const std::complex<Real>& Output() const
        {
            return mOutput;
        }
        bool NanStalled() const
        {
            return std::isnan(mOutput.real()) || std::isnan(mOutput.imag());
        }
    private:
        std::complex<Real> mTimePropagator = 1, mNormFactor = 1, mOutput = 0;
    };

    template <typename Real> class Bank
    {
    public:
        Bank() = default;
        ~Bank() = default;
        Bank(const Bank&) = default;
        Bank& operator=(const Bank&) = default;

        Bank(const Config& inConfig)
        {
            mBins.reserve(inConfig.BinCount());
            for (int i = 0; i < inConfig.BinCount(); ++i) {
                Bin<Real> bin(inConfig.Width(i), inConfig.Frequency(i));
                mBins.push_back(bin);
            }
        }

        int BinCount() const
        {
            return mBins.size();
        }

        Bank& Reset()
        {
            for (auto& bin : mBins)
                bin.Reset();
            return *this;
        }
        Bank& Observe(Real input)
        {
            for (auto& bin : mBins)
                bin.Observe(input);
            return *this;
        }
        const std::complex<Real>& Output(int i) const
        {
            Assert(i >= 0 && i < mBins.size());
            return mBins[i].Output();
        }
        bool NanStalled() const
        {
            for (auto& bin : mBins)
                if (bin.NanStalled())
                    return true;
            return false;
        }
        Bank& ClearNans()
        {
            for (auto& bin : mBins)
                if (bin.NanStalled())
                    bin.Reset();
            return *this;
        }

    private:
        std::vector<Bin<Real>> mBins;
    };

    template <typename Real> class SignalObserver
    {
    public:
        SignalObserver() = default;
        ~SignalObserver() = default;
        SignalObserver(const SignalObserver&) = default;
        SignalObserver& operator=(const SignalObserver&) = default;

        SignalObserver(int channels, const Config& config)
        {
            mBanks = std::vector<Bank<Real>>(channels, Bank<Real>(config));
            mChannelsActive = std::vector<uint8_t>(channels, true);
        }

        void SetOutputDecimation(int d)
        {
            Assert(d > 0);
            mOutputDecimation = d;
            mSampleCount = 0;
        }

        int OutputDecimation() const
        {
            return mOutputDecimation;
        }

        void Clear()
        {
            mBanks.clear();
            mSampleCount = 0;
        }

        bool Empty() const
        {
            return mBanks.empty();
        }

        int BankCount() const
        {
            return mBanks.size();
        }

        void Observe(const Real* pChannelData)
        {
            for (int i = 0; i < mBanks.size(); ++i)
                mBanks[i].Observe(pChannelData[i]);
        }
        void SetChannelActive(int i, bool b = true)
        {
            Assert(i >= 0 && i < mChannelsActive.size());
            mChannelsActive[i] = b;
        }
        void GetOutput(Real* pSpectrum) const
        {
            if (mBanks.empty())
                return;
            Assert(mBanks.size() == mChannelsActive.size());

            int bins = mBanks.front().BinCount();
            for (int bin = 0; bin < bins; ++bin) {
                Real value = 0;
                int count = 0;
                for (int channel = 0; channel < mBanks.size(); ++channel) {
                    if (mChannelsActive[channel]) {
                        ++count;
                        // std::norm() is L2 norm (squared magnitude)
                        value += std::norm(mBanks[channel].Output(bin));
                    }
                }
                if (count > 0) {
                    pSpectrum[bin] = value / count;
                }
                else {
                    pSpectrum[bin] = 0;
                }
            }
        }

        typedef void(*OutputHandler)(const GenericSignal&, void*);
        void Process(const GenericSignal& input, OutputHandler onOutput, void* pAuxData)
        {
            Assert(input.Channels() == mBanks.size());
            if (mBanks.empty())
                return;

            GenericSignal output(mBanks[0].BinCount(), 1);
            auto pInput = input.ConstData();
            std::vector<Real> observations(input.Channels());
            auto pOutput = output.MutableData(GenericSignal::intentOverwriteContents);
            std::vector<Real> spectrum(output.Channels());
            for (int el = 0; el < input.Elements(); ++el) {

                for (int ch = 0; ch < input.Channels(); ++ch) {
                    observations[ch] = pInput[input.LinearIndex(ch, el)];
                }
                Observe(observations.data());
                ++mSampleCount %= mOutputDecimation;
                if (mSampleCount == mOutputDecimation - 1 && onOutput) {
                    GetOutput(spectrum.data());
                    for (int bin = 0; bin < output.Channels(); ++bin) {
                        pOutput[bin] = spectrum[bin];
                    }
                    onOutput(output, pAuxData);
                    for (auto& bank : mBanks) {
                        bank.ClearNans();
                    }
                }
            }
        }

    private:
        std::vector<Bank<Real>> mBanks;
        // use std::vector<uint8_t> rather than possibly slow std::vector<bool>
        std::vector<uint8_t> mChannelsActive;
        int mOutputDecimation = 1;
        uint32_t mSampleCount = 0;
    };

}

#endif // CONTINUOUS_DFT_H
