
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

	Application::Config config = {
		.graphic_config = {
			.enable_multisampling = true,
			.multisample_count = SDL_GPU_SAMPLECOUNT_2
		}
	};

	auto app = Application::create(config);
	app->addWindow(new Window("Sluagh", {800, 600}));
	//app->addWindow(new Window("Sluagh 2", {800, 600}));

	return app;
}
