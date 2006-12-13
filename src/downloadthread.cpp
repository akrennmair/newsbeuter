#include <downloadthread.h>

namespace noos
{

downloadthread::downloadthread(controller * c) : ctrl(c) {
}

downloadthread::~downloadthread() {
}

void downloadthread::run() {
	ctrl->reload_all();
	this->detached_exit();
}

}
