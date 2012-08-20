#include <pb_controller.h>
#include <pb_view.h>
#include <poddlthread.h>
#include <config.h>
#include <utils.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <fcntl.h>
#include <cerrno>

#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>

#include <keymap.h>
#include <configcontainer.h>
#include <colormanager.h>
#include <exceptions.h>
#include <queueloader.h>
#include <logger.h>

using namespace newsbeuter;

static std::string lock_file = "pb-lock.pid";

static void ctrl_c_action(int sig) {
	LOG(LOG_DEBUG,"caugh signal %d",sig);
	stfl::reset();
	utils::remove_fs_lock(lock_file);
	::exit(EXIT_FAILURE);
}

namespace podbeuter {

#define LOCK_SUFFIX ".lock"

/**
 * \brief Try to setup XDG style dirs.
 *
 * returns false, if that fails
 */
bool pb_controller::setup_dirs_xdg(const char *env_home) {
        const char *env_xdg_cache;
	const char *env_xdg_config;
	const char *env_xdg_data;
	std::string xdg_cache_dir;
	std::string xdg_config_dir;
	std::string xdg_data_dir;

	/* Test and create XDG base directories as needed */
	env_xdg_cache = ::getenv("XDG_CACHE_HOME");
	if (env_xdg_cache) {
		xdg_cache_dir = env_xdg_cache;
		create_xdg_base_dir(xdg_cache_dir);
	} else {
		xdg_cache_dir = env_home;
		xdg_cache_dir.append(NEWSBEUTER_PATH_SEP);
		xdg_cache_dir.append(".cache");
		create_xdg_base_dir(xdg_cache_dir);
	}

	env_xdg_config = ::getenv("XDG_CONFIG_HOME");
	if (env_xdg_config) {
		xdg_config_dir = env_xdg_config;
		create_xdg_base_dir(xdg_config_dir);
	} else {
		xdg_config_dir = env_home;
		xdg_config_dir.append(NEWSBEUTER_PATH_SEP);
		xdg_config_dir.append(".config");
		create_xdg_base_dir(xdg_config_dir);
	}

	env_xdg_data = ::getenv("XDG_DATA_HOME");
	if (env_xdg_data) {
		xdg_data_dir = env_xdg_data;
		create_xdg_base_dir(xdg_data_dir);
	} else {
		xdg_data_dir = env_home;
		xdg_data_dir.append(NEWSBEUTER_PATH_SEP);
		xdg_data_dir.append(".local");
		create_xdg_base_dir(xdg_data_dir);

		xdg_data_dir.append(NEWSBEUTER_PATH_SEP);
		xdg_data_dir.append("share");
		create_xdg_base_dir(xdg_data_dir);
	}

	/* Test and create project subfolders as needed */
	xdg_cache_dir.append(NEWSBEUTER_PATH_SEP);
	xdg_cache_dir.append(NEWSBEUTER_SUBDIR_XDG);
	create_xdg_base_dir(xdg_cache_dir);

	xdg_config_dir.append(NEWSBEUTER_PATH_SEP);
	xdg_config_dir.append(NEWSBEUTER_SUBDIR_XDG);
	create_xdg_base_dir(xdg_config_dir);

	xdg_data_dir.append(NEWSBEUTER_PATH_SEP);
	xdg_data_dir.append(NEWSBEUTER_SUBDIR_XDG);
	create_xdg_base_dir(xdg_data_dir);

	/* in cache */
	lock_file   = xdg_cache_dir + std::string(NEWSBEUTER_PATH_SEP) + lock_file;
	searchfile  = utils::strprintf("%s%shistory.search", xdg_cache_dir.c_str(), NEWSBEUTER_PATH_SEP);
	cmdlinefile = utils::strprintf("%s%shistory.cmdline", xdg_cache_dir.c_str(), NEWSBEUTER_PATH_SEP);

	/* in config */
	config_file = xdg_config_dir + std::string(NEWSBEUTER_PATH_SEP) + config_file;

	/* in data */
	cache_file  = xdg_data_dir + std::string(NEWSBEUTER_PATH_SEP) + cache_file;
	url_file    = xdg_data_dir + std::string(NEWSBEUTER_PATH_SEP) + url_file;
	queue_file  = xdg_data_dir + std::string(NEWSBEUTER_PATH_SEP) + queue_file;

	return true;
}

pb_controller::pb_controller() : v(0), config_file("config"), queue_file("queue"), cfg(0), view_update_(true),  max_dls(1), ql(0) { 
	const char * env_home;
	if (!(env_home = ::getenv("HOME"))) {
		struct passwd * spw = ::getpwuid(::getuid());
		if (spw) {
			env_home = spw->pw_dir;
		} else {
			std::cerr << _("Fatal error: couldn't determine home directory!") << std::endl;
			std::cerr << utils::strprintf(_("Please set the HOME environment variable or add a valid user for UID %u!"), ::getuid()) << std::endl;
			::exit(EXIT_FAILURE);
		}
	}

	/* Locate or create XDF standard directories. Cretes project subfolders in each directory. */
	if (!setup_dirs_xdg(env_home)) {
		std::cerr << _("Fatal error: could not setup user-data folders in standard XDG locations!") << std::endl;
		exit(EACCES);
	}

	/*
 	 * Upgrade user-data to XDG directories.
	 * Old directory is deprecated and new users will not get it. Old data is not deleted in case something goes wrong.
	 */
	std::string legacy_config_dir = env_home;
	legacy_config_dir.append(NEWSBEUTER_PATH_SEP);
	legacy_config_dir.append(NEWSBEUTER_CONFIG_SUBDIR);
	if (access(legacy_config_dir.c_str(), R_OK | X_OK | W_OK) == 0)
	{
		/* upgrades from any version prior to 2.6 if the directory does not contain "upgrade-v2.6" */
		std::string upgrade2_6_plock = legacy_config_dir;
		upgrade2_6_plock.append(NEWSBEUTER_PATH_SEP);
		upgrade2_6_plock.append("upgrade-v2.6");
		if (access(upgrade2_6_plock.c_str(), R_OK) != 0)
		{
			/* copy user-data and configuration to new XDG directory destinations */
			upgrade_user_data_file(utils::strprintf("%s%s.newsbeuter/config",   env_home, NEWSBEUTER_PATH_SEP), config_file); // ~/.config/newsbeuter/config
			upgrade_user_data_file(utils::strprintf("%s%s.newsbeuter/urls",     env_home, NEWSBEUTER_PATH_SEP), url_file);    // ~/.local/share/newsbeuter/urls
			upgrade_user_data_file(utils::strprintf("%s%s.newsbeuter/cache.db", env_home, NEWSBEUTER_PATH_SEP), cache_file);  // ~/.local/share/newsbeuter/news.db
			upgrade_user_data_file(utils::strprintf("%s%s.newsbeuter/queue",    env_home, NEWSBEUTER_PATH_SEP), queue_file);  // ~/.local/share/newsbeuter/queue

			/* leave behind a file to signal that an upgrade already has been performed */
			close(open(upgrade2_6_plock.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR));
		}
	}
}

bool pb_controller::create_xdg_base_dir(std::string xdg_dir) {
	if (access(xdg_dir.c_str(), R_OK | X_OK | W_OK) != 0)
	{
		mkdir(xdg_dir.c_str(),0700); // create directory if it does not exist
		if (access(xdg_dir.c_str(), R_OK | X_OK | W_OK) != 0)
		{
			std::cerr << utils::strprintf(_("Error: XDG directory '%s' not accessible."), xdg_dir.c_str()) << std::endl;
			return false;
		}
	}
	return true;
}

void pb_controller::upgrade_user_data_file(std::string src, std::string dst) {
	/* source exists, but destination does not. only safe condition to proceed with upgrade. */
	if (access(src.c_str(), R_OK | W_OK) == 0)
	{
		std::ifstream src_stream(src.c_str());
		if (access(dst.c_str(), R_OK | W_OK) != 0)
		{
			std::ofstream dst_stream(dst.c_str());
			dst_stream << src_stream.rdbuf();
		} else {
			std::cerr << utils::strprintf(_("Warning: User-data file %s not moved to new location %s on upgrade."), src.c_str(), dst.c_str()) << std::endl;
		}
	}
}

pb_controller::~pb_controller() { 
	delete cfg;
}

void pb_controller::run(int argc, char * argv[]) {
	int c;
	bool automatic_dl = false;

	::signal(SIGINT, ctrl_c_action);

	do {
		if ((c = ::getopt(argc, argv, "C:q:d:l:ha")) < 0)
			continue;

		switch (c) {
			case ':':
			case '?':
				usage(argv[0]);
				break;
			case 'C':
				config_file = optarg;
				break;
			case 'q':
				queue_file = optarg;
				break;
			case 'a':
				automatic_dl = true;
				break;
			case 'd': // this is an undocumented debug commandline option!
				GetLogger().set_logfile(optarg);
				break;
			case 'l': // this is an undocumented debug commandline option!
				{
					loglevel level = static_cast<loglevel>(atoi(optarg));
					if (level > LOG_NONE && level <= LOG_DEBUG)
						GetLogger().set_loglevel(level);
				}
				break;
			case 'h':
				usage(argv[0]);
				break;
			default:
				std::cout << utils::strprintf(_("%s: unknown option - %c"), argv[0], static_cast<char>(c)) << std::endl;
				usage(argv[0]);
				break;
		}
	} while (c != -1);

	std::cout << utils::strprintf(_("Starting %s %s..."), "podbeuter", PROGRAM_VERSION) << std::endl;

	pid_t pid;
	if (!utils::try_fs_lock(lock_file, pid)) {
		std::cout << utils::strprintf(_("Error: an instance of %s is already running (PID: %u)"), "podbeuter", pid) << std::endl;
		return;
	}

	std::cout << _("Loading configuration...");
	std::cout.flush();

	configparser cfgparser;
	cfg = new configcontainer();
	cfg->register_commands(cfgparser);
	colormanager * colorman = new colormanager();
	colorman->register_commands(cfgparser);

	keymap keys(KM_PODBEUTER);
	cfgparser.register_handler("bind-key", &keys);
	cfgparser.register_handler("unbind-key", &keys);

	null_config_action_handler null_cah;
	cfgparser.register_handler("macro", &null_cah);
	cfgparser.register_handler("ignore-article", &null_cah);
	cfgparser.register_handler("always-download", &null_cah);
	cfgparser.register_handler("define-filter", &null_cah);
	cfgparser.register_handler("highlight", &null_cah);
	cfgparser.register_handler("highlight-article", &null_cah);
	cfgparser.register_handler("reset-unread-on-update", &null_cah);

	try {
		cfgparser.parse("/etc/newsbeuter/config");
		cfgparser.parse(config_file);
	} catch (const configexception& ex) {
		std::cout << ex.what() << std::endl;
		delete colorman;
		return;	
	}

	if (colorman->colors_loaded())
		colorman->set_pb_colors(v);
	delete colorman;

	max_dls = cfg->get_configvalue_as_int("max-downloads");

	std::cout << _("done.") << std::endl;

	ql = new queueloader(queue_file, this);
	ql->reload(downloads_);

	v->set_keymap(&keys);
	
	v->run(automatic_dl);

	stfl::reset();

	std::cout <<  _("Cleaning up queue...");
	std::cout.flush();
	
	ql->reload(downloads_);
	delete ql;
	
	std::cout << _("done.") << std::endl;

	utils::remove_fs_lock(lock_file);
}

void pb_controller::usage(const char * argv0) {
	std::cout << utils::strprintf(_("%s %s\nusage %s [-C <file>] [-q <file>] [-h]\n"
				"-C <configfile> read configuration from <configfile>\n"
				"-q <queuefile>  use <queuefile> as queue file\n"
				"-a              start download on startup\n"
				"-h              this help\n"), "podbeuter", PROGRAM_VERSION, argv0);
	::exit(EXIT_FAILURE);
}

std::string pb_controller::get_dlpath() {
	return cfg->get_configvalue("download-path");
}

unsigned int pb_controller::downloads_in_progress() {
	unsigned int count = 0;
	if (downloads_.size() > 0) {
		for (std::vector<download>::iterator it=downloads_.begin();it!=downloads_.end();++it) {
			if (it->status() == DL_DOWNLOADING)
				++count;
		}
	}
	return count;
}

unsigned int pb_controller::get_maxdownloads() {
	return max_dls;
}

void pb_controller::reload_queue(bool remove_unplayed) {
	if (ql) {
		ql->reload(downloads_, remove_unplayed);
	}
}

double pb_controller::get_total_kbps() {
	double result = 0.0;
	if (downloads_.size() > 0) {
		for (std::vector<download>::iterator it=downloads_.begin();it!=downloads_.end();++it) {
			if (it->status() == DL_DOWNLOADING) {
				result += it->kbps();
			}
		}
	}
	return result;
}

void pb_controller::start_downloads() {
	int dl2start = get_maxdownloads() - downloads_in_progress();
	for (std::vector<download>::iterator it=downloads_.begin();dl2start > 0 && it!=downloads_.end();++it) {
		if (it->status() == DL_QUEUED) {
			poddlthread * thread = new poddlthread(&(*it), cfg);
			thread->start();
			--dl2start;
		}
	}
}

void pb_controller::increase_parallel_downloads() {
	++max_dls;
}

void pb_controller::decrease_parallel_downloads() {
	if (max_dls > 1)
		--max_dls;
}

void pb_controller::play_file(const std::string& file) {
	std::string cmdline;
	std::string player = cfg->get_configvalue("player");
	if (player == "")
		return;
	cmdline.append(player);
	cmdline.append(" \"");
	cmdline.append(utils::replace_all(file,"\"", "\\\""));
	cmdline.append("\"");
	stfl::reset();
	LOG(LOG_DEBUG, "pb_controller::play_file: running `%s'", cmdline.c_str());
	::system(cmdline.c_str());
}


} // namespace
