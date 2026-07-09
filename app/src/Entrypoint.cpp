
#include "sluagh/render/Renderer2d.h"

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
			.enable_multisampling = false,
			.multisample_count = SDL_GPU_SAMPLECOUNT_1
		}
	};

	auto app = Application::create(config);
	app->addWindow(new Window("Sluagh", {800, 600}));
	//app->addWindow(new Window("Sluagh 2", {800, 600}));
	app->getRenderManager()->addRenderer(new Renderer2d());

	return app;
}
