#include <thread.h>
#include <exception.h>

using namespace noos;

thread::thread() { }

thread::~thread() { }

void thread::start() {
	int rc = pthread_create(&pt, 0, thread::run_thread, this);
	if (rc != 0) {
		throw exception(rc);
	}
}

void thread::join() {
	pthread_join(pt, NULL);
}

void thread::exit() {
	pthread_exit(NULL);
}

void * thread::run_thread(void * p) {
	thread * t = static_cast<thread *>(p);
	t->run();
	return 0;
}
