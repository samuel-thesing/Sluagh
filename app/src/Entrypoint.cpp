
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

	auto app = Application::create();
	app->addWindow(new Window("Sluagh", {800, 600}));
	//app->addWindow(new Window("Sluagh 2", {800, 600}));

	return app;
}
