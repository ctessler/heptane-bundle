#define MAX_THREADS 32
#include "../annot.h"

int
main(void) {
	//	int MAX_THREADS = 32;
	int threads[32]; 
	int bundles[] = {
		0x40000,
		0x40032,
		0x40544
	};
	int bundlep[] = {
		0,
		1,
		2
	};
	/* Select the active thread, from top of the priority queue */
	int ready_thread = threads[0];

	/* run the thread until it enters a new bundle */
	int priority = bundlep[2];

	/* Adjust the thread array */
	int i;
	for (i = 1; i < 4; i++) {
		ANNOT_MAXITER(4);
		if (threads[i] <= priority) {
			threads[i-1] = threads[i];
			continue;
		}
		threads[i] = ready_thread;
		break;
	}
	return 0;
}
