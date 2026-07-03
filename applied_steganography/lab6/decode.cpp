#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define DEFAULT_SEG_DURATION 0.4
#define DEFAULT_DELAY_0 0.05
#define DEFAULT_DELAY_1 0.10
#define DEFAULT_ALPHA 0.2

const uint16_t SYNC_WORD = 0xAAAA;

std::vector<int16_t> ReadWav(const std::string& filename, int& sampleRate) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open WAV file");

    char riff[4], wave[4];
    uint32_t riffSize;
    file.read(riff, 4);
    file.read(reinterpret_cast<char*>(&riffSize), 4);
    if (std::string(riff, 4) != "RIFF")
        throw std::runtime_error("Not a RIFF file");

    file.read(wave, 4);
    if (std::string(wave, 4) != "WAVE")
        throw std::runtime_error("Not a WAVE file");

    bool foundFmt = false, foundData = false;
    std::vector<int16_t> samples;
    int sampleRateFound = 0;

    char chunkId[4];
    uint32_t chunkSize;
    uint16_t audioFormat, numChannels, bitsPerSample;
    uint32_t sampleRateTemp, byteRate;
    uint16_t blockAlign;
    size_t numSamples;

    while (!foundData && file) {
        file.read(chunkId, 4);
        if (!file) break;
        file.read(reinterpret_cast<char*>(&chunkSize), 4);
        if (!file) break;

        if (std::string(chunkId, 4) == "fmt ") {
            if (chunkSize < 16)
                throw std::runtime_error("Invalid fmt chunk size");

            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&sampleRateTemp), 4);
            file.read(reinterpret_cast<char*>(&byteRate), 4);
            file.read(reinterpret_cast<char*>(&blockAlign), 2);
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            if (audioFormat != 1)
                throw std::runtime_error("Only PCM format supported");
            if (numChannels != 1)
                throw std::runtime_error("Only mono files supported");
            if (bitsPerSample != 16)
                throw std::runtime_error("Only 16‑bit samples supported");

            sampleRateFound = sampleRateTemp;
            foundFmt = true;

            if (chunkSize > 16) file.seekg(chunkSize - 16, std::ios::cur);
        } else if (std::string(chunkId, 4) == "data") {
            if (!foundFmt)
                throw std::runtime_error("fmt chunk not found before data");
            numSamples = chunkSize / 2;
            samples.resize(numSamples);
            file.read(reinterpret_cast<char*>(samples.data()), chunkSize);
            if (!file) throw std::runtime_error("Error reading data chunk");
            foundData = true;
        } else
            file.seekg(chunkSize, std::ios::cur);
    }

    if (!foundFmt) throw std::runtime_error("fmt chunk not found");
    if (!foundData) throw std::runtime_error("data chunk not found");

    sampleRate = sampleRateFound;
    return samples;
}

double Autocorrelation(const std::vector<double>& segment, const int& lag) {
    if (lag <= 0 || lag >= (int)segment.size()) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < segment.size() - lag; ++i)
        sum += segment[i] * segment[i + lag];
    return sum / (segment.size() - lag);
}

int DetectBit(const std::vector<double>& segment, const double& delay0,
               const double& delay1, const int& sampleRate, const int& idx) {
    int lag0 = static_cast<int>(std::round(delay0 * sampleRate)),
        lag1 = static_cast<int>(std::round(delay1 * sampleRate));
    double corr0 = Autocorrelation(segment, lag0),
           corr1 = Autocorrelation(segment, lag1);
    const double THRESHOLD = 0.005;
    if (std::abs(corr0 - corr1) < THRESHOLD) return -1;
    return (corr0 > corr1) ? 0 : 1;
}

std::string BitsToText(const std::vector<int>& bits) {
    if (bits.size() < 16) return "";
    uint16_t len = 0;
    for (int i = 0; i < 16; ++i) {
        if (bits[i]) len |= (1 << i);
    }
    if (bits.size() < 16 + len * 8) {
        std::cerr << "Warning: not enough bits for claimed length\n";
        return "";
    }
    std::string text;
    text.reserve(len);
    unsigned char ch;
    for (uint16_t i = 0; i < len; ++i) {
        ch = 0;
        for (int b = 0; b < 8; ++b) {
            if (bits[16 + i * 8 + b]) ch |= (1 << b);
        }
        text.push_back(static_cast<char>(ch));
    }
    return text;
}

std::vector<int> ExtractBitsWithSync(const std::vector<int16_t>& samples,
                                        const int& sampleRate,
                                        const double& segDuration,
                                        const double& delay0,
                                        const double& delay1,
                                        const int& maxBits = 1024) {
    int segLen = static_cast<int>(segDuration * sampleRate),
        totalSamples = (int)samples.size(), maxPossible = totalSamples / segLen,
        searchWindow = std::min(maxBits, maxPossible);

    int numSegments, startSample, offset, bit;
    std::vector<int> bits;
    std::vector<double> seg;
    for (int shift = 0; shift < searchWindow; ++shift) {
        numSegments = searchWindow - shift;
        if (numSegments < 32) continue;

        startSample = totalSamples - numSegments * segLen;
        bits.clear();
        bits.reserve(numSegments);
        for (int i = 0; i < numSegments; ++i) {
            offset = startSample + i * segLen;
            seg = std::vector<double>(segLen);
            for (int j = 0; j < segLen; ++j) {
                seg[j] = samples[offset + j] / 32768.0;
            }
            bit = DetectBit(seg, delay0, delay1, sampleRate, i);
            bits.push_back(bit);
        }

        int last_bit = 0;
        for (int i = 0; i < bits.size(); ++i) {
            if (bits[i] == -1) bits[i] = last_bit;
            last_bit = bits[i];
        }

        uint16_t sync = 0;
        for (int i = 0; i < 16; ++i) {
            if (bits[i]) sync |= (1 << i);
        }
        if (sync == SYNC_WORD) {
            std::cout << "Sync found at shift " << shift << ", total bits "
                      << numSegments << "\n";
            std::cout << "First 32 bits after sync: ";
            for (int i = 0; i < 32 && i + 16 < bits.size(); ++i) {
                std::cout << bits[16 + i];
            }
            std::cout << "\n";
            uint16_t len = 0;
            for (int i = 0; i < 16; ++i) {
                if (bits[16 + i]) len |= (1 << i);
            }
            std::cout << "Length from bits: " << len << "\n";
            return std::vector<int>(bits.begin() + 16, bits.end());
        }
    }
    std::cerr << "Sync word not found\n";
    return {};
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " input.wav key [maxBits]\n";
        return 1;
    }

    std::string wavFile = argv[1];
    int maxBits = (argc >= 4) ? std::stoi(argv[3]) : 2048;  // увеличен запас

    try {
        int key = std::stoi(argv[2]), sampleRate;
        std::vector<int16_t> samples = ReadWav(wavFile, sampleRate);
        std::cout << "Audio loaded: " << samples.size() << " samples, "
                  << sampleRate << " Hz\n";

        std::vector<int> bits =
            ExtractBitsWithSync(samples, sampleRate, DEFAULT_SEG_DURATION,
                                   DEFAULT_DELAY_0, DEFAULT_DELAY_1, maxBits);
        if (bits.empty()) {
            std::cout << "Failed to extract bits.\n";
            return 1;
        }

        std::cout << "Extracted " << bits.size() << " bits after sync\n";
        std::string message = BitsToText(bits);
        if (message.empty()) {
            std::cout << "No valid message found.\n";
        } else {
            for (int i = 0; i < message.length(); ++i)
                message[i] = message[i] ^ key;
            std::cout << "Decoded message: \"" << message << "\"\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}