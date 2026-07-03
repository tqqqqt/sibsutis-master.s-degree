#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using Complex = std::complex<double>;

#define DEFAULT_SEG_DURATION 0.4
#define DEFAULT_DELAY_0 0.05
#define DEFAULT_DELAY_1 0.10
#define DEFAULT_ALPHA 0.2

const uint16_t SYNC_WORD = 0xAAAA;  // 1010101010101010

#pragma pack(push, 1)
struct BitmapFileHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BitmapInfoHeader {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)

void FFT(std::vector<Complex>& a, const bool& invert) {
    int n = (int)a.size();
    if (n == 1) return;

    int bit = 0;
    for (int i = 1, j = 0; i < n; ++i) {
        bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    double ang = 0;
    Complex wlen, w, u, v;
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * M_PI / len * (invert ? -1 : 1);
        wlen = Complex(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            w = Complex(1);
            for (int j = 0; j < len / 2; ++j) {
                u = a[i + j];
                v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (!invert) return;
    for (Complex& x : a) x /= n;
}

void ReadBmp(const std::string& filename, std::vector<uint8_t>& pixels,
             int& width, int& height) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open BMP file");

    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;

    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    if (fileHeader.bfType != 0x4D42) throw std::runtime_error("Not a BMP file");
    if (infoHeader.biBitCount != 8)
        throw std::runtime_error("Only 8‑bit grayscale BMP is supported");
    if (infoHeader.biCompression != 0)
        throw std::runtime_error("Compressed BMP not supported");

    width = infoHeader.biWidth;
    height = std::abs(infoHeader.biHeight);
    bool topDown = (infoHeader.biHeight < 0);

    std::vector<uint8_t> palette(256 * 4), grayPalette(256);
    file.read(reinterpret_cast<char*>(palette.data()), 256 * 4);
    for (int i = 0; i < 256; ++i) {
        grayPalette[i] = palette[i * 4 + 2];
    }

    int rowSize = (width + 3) & ~3;
    std::vector<uint8_t> rowBuffer(rowSize);

    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    pixels.resize(width * height);

    for (int y = 0; y < height; ++y) {
        int fileRow = topDown ? y : (height - 1 - y);
        file.read(reinterpret_cast<char*>(rowBuffer.data()), rowSize);
        for (int x = 0; x < width; ++x) {
            uint8_t index = rowBuffer[x];
            pixels[y * width + x] = grayPalette[index];
        }
    }
    file.close();
}

void WriteWav(const std::string& filename, const std::vector<int16_t>& samples,
              const int& sample_rate) {
    std::ofstream f(filename, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot create WAV file");

    uint32_t data_size = static_cast<uint32_t>(samples.size() * 2),
             total_size = 4 + 8 + 16 + 8 + data_size;

    uint32_t fmt_size = 16;
    uint16_t audio_format = 1, num_channels = 1;
    uint32_t sample_rate_ = sample_rate,
             byte_rate = sample_rate * num_channels * 2;
    uint16_t block_align = num_channels * 2, bits_per_sample = 16;

    f.write("RIFF", 4);
    f.write(reinterpret_cast<const char*>(&total_size), 4);
    f.write("WAVE", 4);
    f.write("fmt ", 4);
    f.write(reinterpret_cast<const char*>(&fmt_size), 4);
    f.write(reinterpret_cast<const char*>(&audio_format), 2);
    f.write(reinterpret_cast<const char*>(&num_channels), 2);
    f.write(reinterpret_cast<const char*>(&sample_rate_), 4);
    f.write(reinterpret_cast<const char*>(&byte_rate), 4);
    f.write(reinterpret_cast<const char*>(&block_align), 2);
    f.write(reinterpret_cast<const char*>(&bits_per_sample), 2);
    f.write("data", 4);
    f.write(reinterpret_cast<const char*>(&data_size), 4);
    f.write(reinterpret_cast<const char*>(samples.data()), data_size);

    f.close();
}

void GenerateSignal(const std::vector<uint8_t>& pixels, int& width, int& height,
                    std::vector<int16_t>& output_samples,
                    const int& sample_rate = 44100, const int& nfft = 1024) {
    int hop = nfft / 2, num_frames = width, max_bins = nfft / 2 + 1;
    if (height > max_bins) {
        std::cerr << "Warning: image height (" << height
                  << ") exceeds available frequency bins (" << max_bins
                  << "). Truncating the top of the image.\n";
        height = max_bins;
    }

    int output_len = (num_frames - 1) * hop + nfft;
    std::vector<double> output(output_len, 0.0), window(nfft);
    for (int i = 0; i < nfft; ++i)
        window[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (nfft - 1)));

    const double amp_factor = 1.0 / 255.0;

    int k;
    double amplitude;
    uint8_t pixel;
    for (int t = 0; t < num_frames; ++t) {
        std::vector<Complex> spectrum(nfft, Complex(0.0, 0.0));

        for (int y = 0; y < height; ++y) {
            k = y;
            pixel = pixels[y * width + t];
            amplitude = pixel * amp_factor;

            spectrum[k] = Complex(amplitude, 0.0);
            if (k != 0 && k != nfft / 2)
                spectrum[nfft - k] = Complex(amplitude, 0.0);
        }

        FFT(spectrum, true);

        for (int i = 0; i < nfft; ++i)
            output[t * hop + i] += spectrum[i].real() * window[i];
    }

    double max_val = 0.0;
    for (double v : output) max_val = std::max(max_val, std::fabs(v));
    if (max_val < 1e-9) max_val = 1.0;
    const double scale = (0.7 * 32767.0) / max_val;

    output_samples.resize(output_len);
    for (size_t i = 0; i < output.size(); ++i)
        output_samples[i] = static_cast<int16_t>(output[i] * scale);
}

std::vector<double> GenerateNoiseSegment(const int& num_samples,
                                         const double& amplitude = 0.5) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dist(-1.0, 1.0);

    std::vector<double> segment(num_samples);
    for (int i = 0; i < num_samples; ++i) segment[i] = amplitude * dist(gen);
    return segment;
}

std::vector<double> EmbedEchoSegment(const std::vector<double>& carrier,
                                     const int& bit, const double& delay0,
                                     const double& delay1, const double& alpha,
                                     const int& sample_rate) {
    int seg_len = (int)carrier.size();
    std::vector<double> result = carrier;

    double delay = (bit == 0) ? delay0 : delay1;
    int delay_samples = (int)std::round(delay * sample_rate);

    if (delay_samples >= seg_len) {
        std::cerr << "Warning: delay exceeds segment length, skipping echo.\n";
        return result;
    }

    for (int i = 0; i < seg_len - delay_samples; ++i)
        result[i + delay_samples] += alpha * carrier[i];

    return result;
}

std::vector<int> TextToBits(const std::string& text) {
    std::vector<int> bits;
    uint16_t len = static_cast<uint16_t>(text.size());

    for (int b = 0; b < 16; ++b) bits.push_back((len >> b) & 1);
    for (unsigned char ch : text) {
        for (int b = 0; b < 8; ++b) bits.push_back((ch >> b) & 1);
    }
    return bits;
}

std::vector<double> GenerateEchoTail(const std::string& text,
                                     const int& sample_rate,
                                     const double& seg_duration,
                                     const double& delay0, const double& delay1,
                                     const double& alpha) {
    std::vector<int> bits;

    for (int i = 0; i < 16; ++i) bits.push_back((SYNC_WORD >> i) & 1);

    std::vector<int> textBits = TextToBits(text);
    bits.insert(bits.end(), textBits.begin(), textBits.end());

    int seg_len = static_cast<int>(seg_duration * sample_rate);
    std::vector<double> tail;
    tail.reserve(bits.size() * seg_len);

    std::vector<double> carrier, seg;
    for (int bit : bits) {
        carrier = GenerateNoiseSegment(seg_len, 0.5);
        seg =
            EmbedEchoSegment(carrier, bit, delay0, delay1, alpha, sample_rate);
        tail.insert(tail.end(), seg.begin(), seg.end());
    }
    return tail;
}

std::vector<int16_t> MergeWithEcho(const std::vector<int16_t>& original,
                                   const std::vector<double>& tail,
                                   const int& sampleRate,
                                   const double& segDuration) {
    int segLen = static_cast<int>(segDuration * sampleRate);
    std::vector<int16_t> result = original;
    int pad = (segLen - (result.size() % segLen)) % segLen;
    for (int i = 0; i < pad; ++i) result.push_back(0);
    for (double s : tail) result.push_back(static_cast<int16_t>(s * 32767.0));
    return result;
}

std::string ReadTextFile(){
    std::ifstream file("alice.txt");
    file.seekg(0, std::ios::end);
    size_t size=file.tellg();
    std::string res(size,' ');
    file.seekg(0);
    file.read(&res[0],size);
    file.close();
    return res;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " input.bmp output.wav key [text]\n";
        return 4;
    }

    try {
        int temp = std::stoi(argv[3]);
        if (temp < 0 || temp > 255)
            throw std::runtime_error("Can't use this key.");
    } catch (std::exception& exp) {
        std::cerr << "Error: " << exp.what() << '\n';
        return 4;
    }

    int key = std::stoi(argv[3]);
    std::string bmp_file = argv[1], wav_file = argv[2];
    std::string secret_text =
                    (argc >= 5) ? argv[4] : ReadTextFile(),
                save_text = secret_text;
    for (int i = 0; i < secret_text.length(); ++i)
        secret_text[i] = secret_text[i] ^ key;

    try {
        std::vector<uint8_t> pixels;
        int width, height;
        ReadBmp(bmp_file, pixels, width, height);
        std::cout << "Image loaded: " << width << " x " << height << "\n";

        std::vector<int16_t> audio_image;
        GenerateSignal(pixels, width, height, audio_image, 44100, 1024);
        std::cout << "Audio for image generated, length: " << audio_image.size()
                  << " samples (" << audio_image.size() / 44100.0 << " sec)\n";

        std::vector<double> tail =
            GenerateEchoTail(secret_text, 44100, DEFAULT_SEG_DURATION,
                             DEFAULT_DELAY_0, DEFAULT_DELAY_1, DEFAULT_ALPHA);
        std::cout << "Echo tail generated, length: " << tail.size()
                  << " samples (" << tail.size() / 44100.0 << " sec)\n";

        std::vector<int16_t> final_audio =
            MergeWithEcho(audio_image, tail, 44100, DEFAULT_SEG_DURATION);

        WriteWav(wav_file, final_audio, 44100);
        std::cout << "WAV file written: " << wav_file << "\n";
        std::cout << "Embedded text: \"" << save_text << "\"\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 4;
    }

    return 0;
}