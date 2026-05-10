
#include <sluagh/sluagh.h>
#include <sluagh/sluagh_main.h>

Application *createApplication(Arguments args) {
	UNUSED(args)

#if APP_CONSOLE
	if (args.hasFlag("help")) {
		args.printHelp();
		return nullptr;
	}
	if (args.hasFlag("version")) {
		args.printVersion();
		return nullptr;
	}
#endif

	auto app = Application::create({"Sluagh"});

	return app;
}
