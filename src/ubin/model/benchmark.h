#ifndef BENCHMARK_H_
#define BENCHMARK_H_

	/**
	 * @brief Pages interval struct for benchmark files
	 */
	struct interval
	{
		unsigned low;
		unsigned high;
	};

struct workload {
	const int size;
	const int *row;
	const int *col;
	struct interval **pages_interval;
	unsigned **work;
	unsigned **trials;
	unsigned **pages_strike;
};

#endif /* BENCHMARK_H_ */
