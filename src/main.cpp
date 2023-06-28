#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cinttypes>
#include <vector>
#include <thread>
#include <mutex>

#include "Random.cuh"
#include "crr.h"

#define PANIC(...) { \
    std::fprintf(stderr, __VA_ARGS__); \
    std::abort(); \
}

struct Parameters {
    FILE *input_file;
    FILE *output_file;
    bool v113;
    int32_t salt;
    int32_t min_chunk_x;
    int32_t max_chunk_x;
    int32_t min_chunk_z;
    int32_t max_chunk_z;
    int32_t min_calls;
    int32_t max_calls;
};

void run_thread(const Parameters &parameters, std::mutex &input_mutex, std::mutex &output_mutex) {
    LCG min_skip = LCG::combine(parameters.min_calls == -1 ? 0 : -parameters.min_calls);

    std::vector<uint64_t> structure_seeds;

    while (true) {
        uint64_t internal_seed;
        {
            std::lock_guard<std::mutex> g(input_mutex);
            if (std::fscanf(parameters.input_file, "%" SCNu64 "\n", &internal_seed) != 1) {
                break;
            }
        }

        internal_seed = (internal_seed * min_skip.multiplier + min_skip.addend) & LCG::MASK;

        for (int32_t chunk_x = parameters.min_chunk_x; chunk_x <= parameters.max_chunk_x; chunk_x++) {
            for (int32_t chunk_z = parameters.min_chunk_z; chunk_z <= parameters.max_chunk_z; chunk_z++) {
                Random random = Random::withSeed(internal_seed);

                for (int32_t i = parameters.min_calls; i <= parameters.max_calls; i++) {
                    uint64_t population_seed = random.seed;
                    if (i != -1) {
                        population_seed ^= LCG::MULTIPLIER;
                        if (parameters.v113) {
                            population_seed -= static_cast<uint64_t>(parameters.salt);
                        }
                    }

                    if (parameters.v113) {
                        reverse_population_seed_113(population_seed, chunk_x  * 16, chunk_z  * 16, structure_seeds);
                    } else {
                        reverse_population_seed_112(population_seed, chunk_x, chunk_z, structure_seeds);
                    }

                    for (uint64_t structure_seed : structure_seeds) {
                        std::lock_guard<std::mutex> g(output_mutex);
                        if (parameters.min_chunk_x != parameters.max_chunk_x || parameters.min_chunk_z != parameters.max_chunk_z) {
                            std::fprintf(parameters.output_file, "%" PRIu64 " %" PRIi32 " %" PRIi32 "\n", structure_seed, chunk_x, chunk_z);
                        } else {
                            std::fprintf(parameters.output_file, "%" PRIu64 "\n", structure_seed);
                        }
                    }
                    structure_seeds.clear();

                    if (i != -1) {
                        random.skip<-1>();
                    }
                }        
            }
        }
    }
}

void print_usage(const char *program) {
    std::fprintf(stderr,
        "Usage:\n"
        "%s <input_file> <output_file> <thread_count> (1.12 | 1.13 <salt>) <min_chunk_x> <max_chunk_x> <min_chunk_z> <max_chunk_z> <min_calls> <max_calls>\n",
        program
    );
}

char *get_arg(int argc, char **argv, int arg) {
    if (arg >= argc) {
        print_usage(argv[0]);
        PANIC("Invalid args\n");
    }

    return argv[arg];
}

int main(int argc, char **argv) {
    int arg = 0;
    get_arg(argc, argv, arg++);
    const char *input_file_name = get_arg(argc, argv, arg++);
    const char *output_file_name = get_arg(argc, argv, arg++);
    int32_t thread_count = atoi(get_arg(argc, argv, arg++));
    if (thread_count < 1) thread_count = 1;
    const char *version_str = get_arg(argc, argv, arg++);
    if (strcmp(version_str, "1.12") != 0 && strcmp(version_str, "1.13") != 0) PANIC("Invalid version, must be 1.12 or 1.13\n");
    bool v113 = strcmp(version_str, "1.13") == 0;
    int32_t salt = v113 ? atoi(get_arg(argc, argv, arg++)) : -1;
    int32_t min_chunk_x = atoi(get_arg(argc, argv, arg++));
    int32_t max_chunk_x = atoi(get_arg(argc, argv, arg++));
    int32_t min_chunk_z = atoi(get_arg(argc, argv, arg++));
    int32_t max_chunk_z = atoi(get_arg(argc, argv, arg++));
    int32_t min_calls = atoi(get_arg(argc, argv, arg++));
    if (min_calls < -1) min_calls = -1;
    int32_t max_calls = atoi(get_arg(argc, argv, arg++));
    if (arg != argc) get_arg(argc, argv, argc);

    std::fprintf(stderr,
        "Using the following parameters:\n"
        "input_file = %s\n"
        "output_file = %s\n"
        "thread_count = %" PRIi32 "\n"
        "version = %s\n"
        "salt = %" PRIi32 "\n"
        "min_chunk_x = %" PRIi32 "\n"
        "max_chunk_x = %" PRIi32 "\n"
        "min_chunk_z = %" PRIi32 "\n"
        "max_chunk_z = %" PRIi32 "\n"
        "min_calls = %" PRIi32 "\n"
        "max_calls = %" PRIi32 "\n",
        input_file_name,
        output_file_name,
        thread_count,
        v113 ? "1.13+" : "1.12-",
        salt,
        min_chunk_x,
        max_chunk_x,
        min_chunk_z,
        max_chunk_z,
        min_calls,
        max_calls
    );

    FILE *input_file = std::fopen(input_file_name, "r");
    if (input_file == NULL) PANIC("Could not open input file\n");

    FILE *output_file = std::fopen(output_file_name, "w");
    if (output_file == NULL) PANIC("Could not open output file\n");

    Parameters parameters {
        input_file,
        output_file,
        v113,
        salt,
        min_chunk_x,
        max_chunk_x,
        min_chunk_z,
        max_chunk_z,
        min_calls,
        max_calls
    };
    std::mutex input_mutex;
    std::mutex output_mutex;

    std::vector<std::thread> threads;

    for (int32_t i = 0; i < thread_count; i++) {
        threads.emplace_back(run_thread, std::cref(parameters), std::ref(input_mutex), std::ref(output_mutex));
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::fclose(input_file);
    std::fclose(output_file);

    return 0;
}