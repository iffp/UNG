#include <chrono>
#include <fstream>
#include <numeric>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "uni_nav_graph.h"
#include "utils.h"

#include <thread>
#include "fanns_survey_helpers.cpp"

using namespace std;

int main(int argc, char** argv) {
    // Get number of threads
    unsigned int nthreads = std::thread::hardware_concurrency();
    std::cout << "Number of threads: " << nthreads << std::endl;

    // Parameters 
    std::string path_query_vectors;
	std::string path_query_attributes;
	std::string path_groundtruth;
    std::string path_index;
	std::string data_type;
	std::string distance_function;
	std::string index_type;
	std::string scenario;
	uint32_t num_entry_points;
	uint32_t L_search;
	uint32_t k;
	
    // Parse arguments
    if (argc != 12) {
		cout << "Usage: " << argv[0] << " <path_query_vectors> <path_query_attributes> <path_groundtruth> <path_index> <data_type> <distance_function> <index_type> <scenario> <num_entry_points> <L_search> <k>" << endl;
	}

    // Store parameters
	path_query_vectors = argv[1];
	path_query_attributes = argv[2];
	path_groundtruth = argv[3];
	path_index = argv[4];
	data_type = argv[5];
	distance_function = argv[6];
	index_type = argv[7];
	scenario = argv[8];
	num_entry_points = std::stoi(argv[9]);
	L_search = std::stoi(argv[10]);
	k = std::stoi(argv[11]);

    // Check scenario
    if (scenario != "containment" && scenario != "equality" && scenario != "overlap") {
        std::cerr << "Invalid scenario: " << scenario << std::endl;
        return -1;
    }

    // Load Query data
    std::shared_ptr<ANNS::IStorage> query_storage = ANNS::create_storage(data_type);
    query_storage->load_from_file(path_query_vectors, path_query_attributes);
    auto n_queries = query_storage->get_num_points();

	// Load ground truth
	std::vector<std::vector<int>> groundtruth;
    groundtruth = read_ivecs(path_groundtruth);
    assert(n_queries == groundtruth.size() && "Number of queries in query vectors and groundtruth do not match");

    // Truncate ground-truth to at most k items
    for (std::vector<int>& vec : groundtruth) {
        if (vec.size() > k) {
            vec.resize(k);
        }
    }

    // Load Index
    ANNS::UniNavGraph ung_index;
    ung_index.load(path_index, data_type);

    // Preparation
    std::shared_ptr<ANNS::DistanceHandler> distance_handler = ANNS::get_distance_handler(data_type, distance_function);
    auto results = new std::pair<ANNS::IdxType, float>[n_queries * k];
    std::vector<float> num_cmps(n_queries);
    
    // Execute queries (timed)
	auto start_time = std::chrono::high_resolution_clock::now();
	ung_index.search(query_storage, distance_handler, nthreads, L_search, num_entry_points, scenario, k, results, num_cmps);
	auto end_time = std::chrono::high_resolution_clock::now();

	// Compute time
    std::chrono::duration<double> time_diff = end_time - start_time;
    double query_execution_time = time_diff.count();		

	// Compute recall
	// TODO: Check what happens if there are less than k matches
	size_t match_count = 0;
	size_t total_count = 0;
	for (size_t q = 0; q < n_queries; ++q) {
		int n_valid_neighbors = min((int)k, (int)groundtruth[q].size());
		vector<int> groundtruth_q = groundtruth[q];
		vector<int> nearest_neighbors_q;
		for (size_t i = 0; i < k; ++i) {
			nearest_neighbors_q.push_back(results[q * k + i].first);
		}
		sort(groundtruth_q.begin(), groundtruth_q.end());
		sort(nearest_neighbors_q.begin(), nearest_neighbors_q.end());
		vector<int> intersection;
		set_intersection(groundtruth_q.begin(), groundtruth_q.end(), nearest_neighbors_q.begin(), nearest_neighbors_q.end(), back_inserter(intersection));
		match_count += intersection.size();
		total_count += n_valid_neighbors;
	}
	
    // Report results
    double recall = (double)match_count / total_count;
    double qps = (double)n_queries / query_execution_time;
    peak_memory_footprint();
    printf("Queries per second: %.3f\n", qps);
    printf("Recall: %.3f\n", recall);
    return 0;
}
