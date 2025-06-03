#include <chrono>
#include <fstream>
#include <iostream>
#include <boost/program_options.hpp>
#include "uni_nav_graph.h"

#include <thread>


#include <atomic>
#include <omp.h>
#include "fanns_survey_helpers.cpp"
#include "global_thread_counter.h"

using namespace std;

// Global atomic to store peak thread count
std::atomic<int> peak_threads(1);


int main(int argc, char** argv) {
    // Get number of WH threads and use that number of threads for the index construction
    unsigned int nthreads = std::thread::hardware_concurrency();
    omp_set_num_threads(nthreads);

    // Prepare thread monitoring
    std::atomic<bool> done(false);
    std::thread monitor(monitor_thread_count, std::ref(done));

    // Parameters 
    std::string path_database_vectors;
	std::string path_database_attributes;
    std::string path_index;
	std::string data_type;
	std::string distance_function;
	std::string index_type;
	std::string scenario;
	uint32_t num_cross_edges;
	uint32_t max_degree;
	uint32_t L_build;
	float alpha;
	
    // Parse arguments
    if (argc != 12) {
		cout << "Usage: " << argv[0] << " <path_database_vectors> <path_database_attributes> <path_index> <data_type> <distance_function> <index_type> <scenario> <num_cross_edges> <max_degree> <L_build> <alpha>" << endl;
	}

    // Store parameters
	path_database_vectors = argv[1];
	path_database_attributes = argv[2];
	path_index = argv[3];
	data_type = argv[4];
	distance_function = argv[5];
	index_type = argv[6];
	scenario = argv[7];
	num_cross_edges = atoi(argv[8]);
	max_degree = atoi(argv[9]);
	L_build = atoi(argv[10]);
	alpha = atof(argv[11]);

    // Check scenario
    if (scenario != "general" && scenario != "equality") {
        std::cerr << "Invalid scenario: " << scenario << std::endl;
        return -1;
    }

    // Load database
    std::shared_ptr<ANNS::IStorage> base_storage = ANNS::create_storage(data_type);
    base_storage->load_from_file(path_database_vectors, path_database_attributes);

    // Building the index (timed)
	auto start_time = std::chrono::high_resolution_clock::now();
    std::shared_ptr<ANNS::DistanceHandler> distance_handler = ANNS::get_distance_handler(data_type, distance_function);
    ANNS::UniNavGraph ung_index;
    ung_index.build(base_storage, distance_handler, scenario, index_type, nthreads, num_cross_edges, max_degree, L_build, alpha);
	auto end_time = std::chrono::high_resolution_clock::now();

    // Stop thread monitoring
    done = true;
    monitor.join();

    // Print statistics
    std::chrono::duration<double> time_diff = end_time - start_time;
    double duration = time_diff.count();
	printf("Maximum number of threads: %d\n", peak_threads.load()-1);   // Subtract 1 because of the monitoring thread
    printf("Index construction time: %.3f s\n", duration);
    peak_memory_footprint();

    // Write index to file
    ung_index.save(path_index);
    return 0;
}
