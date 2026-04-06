// Pipeline latency benchmark — 1000 iterations per stage.
// Saves identical results to benchmarks/results/benchmark_results.txt.

#include "../camera_pipeline/camera_capture.hpp"
#include "../camera_pipeline/ring_buffer/ring_buffer.hpp"
#include "../inference_engine/vision_encoder/vision_encoder.hpp"
#include "../action_decoder/action_decoder.hpp"
#include "../action_decoder/serial_publisher/serial_publisher.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// ── Timing helpers ────────────────────────────────────────────────────────────

using Clock = std::chrono::high_resolution_clock;
using Ms    = std::chrono::duration<double, std::milli>;

struct Stats {
    double mean;
    double min;
    double max;
    double p95;
};

static Stats compute_stats(std::vector<double> samples) {
    std::sort(samples.begin(), samples.end());
    double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
    Stats s{};
    s.mean = sum / static_cast<double>(samples.size());
    s.min  = samples.front();
    s.max  = samples.back();
    std::size_t p95_idx = static_cast<std::size_t>(
        std::ceil(0.95 * static_cast<double>(samples.size())) - 1);
    s.p95 = samples[std::min(p95_idx, samples.size() - 1)];
    return s;
}

// ── Table rendering ───────────────────────────────────────────────────────────

// UTF-8 box-drawing code points as raw byte strings
static const char* HL  = "\xe2\x94\x80";  // U+2500 ─
static const char* VL  = "\xe2\x94\x82";  // U+2502 │
static const char* TL  = "\xe2\x94\x8c";  // U+250C ┌
static const char* TR  = "\xe2\x94\x90";  // U+2510 ┐
static const char* BL  = "\xe2\x94\x94";  // U+2514 └
static const char* BR  = "\xe2\x94\x98";  // U+2518 ┘
static const char* LM  = "\xe2\x94\x9c";  // U+251C ├
static const char* RM  = "\xe2\x94\xa4";  // U+2524 ┤
static const char* TM  = "\xe2\x94\xac";  // U+252C ┬
static const char* BM  = "\xe2\x94\xb4";  // U+2534 ┴
static const char* XM  = "\xe2\x94\xbc";  // U+253C ┼

static constexpr int W_STAGE = 25;
static constexpr int W_COL   = 10;
static constexpr int W_THRU  = 14;

static std::string fmt_ms(double ms) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << ms << "ms";
    return oss.str();
}

static std::string fmt_fps(double mean_ms) {
    if (mean_ms <= 0.0) return "N/A";
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0)
        << (1000.0 / mean_ms) << " fps";
    return oss.str();
}

static std::string pad(const std::string& s, int w) {
    std::string r = s;
    if (static_cast<int>(r.size()) < w)
        r += std::string(w - r.size(), ' ');
    return " " + r + " ";
}

static std::string hline(const char* lc, const char* mc, const char* rc) {
    std::string s;
    s += lc;
    for (int i = 0; i < W_STAGE + 2; ++i) s += HL;
    s += mc;
    for (int col = 0; col < 4; ++col) {
        for (int i = 0; i < W_COL + 2; ++i) s += HL;
        s += mc;
    }
    for (int i = 0; i < W_THRU + 2; ++i) s += HL;
    s += rc;
    s += "\n";
    return s;
}

static std::string data_row(const std::string& stage,
                             const std::string& mean,
                             const std::string& mn,
                             const std::string& mx,
                             const std::string& p95,
                             const std::string& thr) {
    std::string s;
    s += VL; s += pad(stage, W_STAGE);
    s += VL; s += pad(mean,  W_COL);
    s += VL; s += pad(mn,    W_COL);
    s += VL; s += pad(mx,    W_COL);
    s += VL; s += pad(p95,   W_COL);
    s += VL; s += pad(thr,   W_THRU);
    s += VL; s += "\n";
    return s;
}

struct Row { std::string stage; Stats stats; };

static std::string render_table(const std::vector<Row>& rows) {
    std::string out;
    out += hline(TL, TM, TR);
    out += data_row(" Stage", " Mean", " Min", " Max", " P95", " Throughput");
    out += hline(LM, XM, RM);
    for (const auto& r : rows) {
        out += data_row(r.stage,
                        fmt_ms(r.stats.mean),
                        fmt_ms(r.stats.min),
                        fmt_ms(r.stats.max),
                        fmt_ms(r.stats.p95),
                        fmt_fps(r.stats.mean));
    }
    out += hline(BL, BM, BR);
    return out;
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    constexpr int ITER = 1000;

    CameraCapture  cap(0);
    DummyEncoder   enc;
    ActionDecoder  dec(Mode::CONTINUOUS);
    SerialPublisher pub("/tmp/bench_cmd.json");

    cv::Mat frame(480, 640, CV_8UC3);
    cv::randu(frame, cv::Scalar(0), cv::Scalar(255));

    std::vector<Row> rows;

    // ── Stage 1: Preprocessing ────────────────────────────────────────────────
    {
        std::vector<double> samples;
        for (int i = 0; i < ITER; ++i) {
            auto t0 = Clock::now();
            cap.preprocess(frame);
            samples.push_back(Ms(Clock::now() - t0).count());
        }
        rows.push_back({"1. Preprocessing", compute_stats(samples)});
    }

    // ── Stage 2: Ring Buffer push+pop ─────────────────────────────────────────
    {
        RingBuffer<std::vector<float>, 8> buf;
        std::vector<float> tensor = cap.preprocess(frame);
        std::vector<double> samples;
        for (int i = 0; i < ITER; ++i) {
            auto t0 = Clock::now();
            buf.push(tensor);
            std::vector<float> out;
            buf.pop(out);
            samples.push_back(Ms(Clock::now() - t0).count());
        }
        rows.push_back({"2. Ring Buffer", compute_stats(samples)});
    }

    // ── Stage 3: DummyEncoder.encode() ───────────────────────────────────────
    {
        std::vector<float> tensor = cap.preprocess(frame);
        std::vector<double> samples;
        for (int i = 0; i < ITER; ++i) {
            auto t0 = Clock::now();
            enc.encode(tensor);
            samples.push_back(Ms(Clock::now() - t0).count());
        }
        rows.push_back({"3. Vision Encoding", compute_stats(samples)});
    }

    // ── Stage 4: ActionDecoder.decode() ──────────────────────────────────────
    {
        std::vector<float> emb = enc.encode(cap.preprocess(frame));
        std::vector<double> samples;
        for (int i = 0; i < ITER; ++i) {
            auto t0 = Clock::now();
            dec.decode(emb);
            samples.push_back(Ms(Clock::now() - t0).count());
        }
        rows.push_back({"4. Action Decoding", compute_stats(samples)});
    }

    // ── Stage 5: SerialPublisher.write() ─────────────────────────────────────
    {
        RobotCommand cmd = dec.decode(enc.encode(cap.preprocess(frame)));
        std::vector<double> samples;
        for (int i = 0; i < ITER; ++i) {
            auto t0 = Clock::now();
            pub.write(cmd);
            samples.push_back(Ms(Clock::now() - t0).count());
        }
        rows.push_back({"5. Serial Publisher", compute_stats(samples)});
    }

    // ── Stage 6: Full pipeline (1→5 chained) ─────────────────────────────────
    {
        RingBuffer<std::vector<float>, 8> buf;
        std::vector<double> samples;
        for (int i = 0; i < ITER; ++i) {
            auto t0 = Clock::now();
            auto tensor = cap.preprocess(frame);
            buf.push(tensor);
            std::vector<float> retrieved;
            buf.pop(retrieved);
            auto emb = enc.encode(retrieved);
            auto cmd = dec.decode(emb);
            pub.write(cmd);
            samples.push_back(Ms(Clock::now() - t0).count());
        }
        rows.push_back({"6. Full Pipeline", compute_stats(samples)});
    }

    // ── Output ────────────────────────────────────────────────────────────────
    std::string table = render_table(rows);

    constexpr double RT_BUDGET_MS = 33.33;
    double full_mean = rows.back().stats.mean;
    std::string trailer = "Target for real-time at 30fps: 33.33ms per frame\n";
    if (full_mean > RT_BUDGET_MS) {
        trailer += "\xe2\x9a\xa0\xef\xb8\x8f  WARNING: Full pipeline exceeds real-time budget\n";
    } else {
        trailer += "\xe2\x9c\x85 Real-time budget met\n";
    }

    std::cout << table << trailer;

    std::filesystem::create_directories("benchmarks/results");
    std::ofstream ofs("benchmarks/results/benchmark_results.txt");
    if (ofs.is_open()) {
        ofs << table << trailer;
    }

    return 0;
}
