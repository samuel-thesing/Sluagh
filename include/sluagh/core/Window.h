#ifndef SLUAGH_WINDOW_H
#define SLUAGH_WINDOW_H
#include "SDL3/SDL.h"
#include "sluagh/export.h"
#include "sluagh/utils/Vector.h"

#include <string>

class RenderTarget;

class SLUAGH_EXPORT Window {
private:
	std::string title;
	Vec2i size;

	SDL_WindowID window_id;
	SDL_Window* sdl_window;

	RenderTarget* render_target;

public:
	Window(const std::string& title, Vec2i size);
	~Window();

	void setTitle(const std::string& new_title);
	void resize(Vec2i new_size);

	std::string getTitle() const { return title; }
	Vec2i getSize() const { return size; }
	RenderTarget* getRenderTarget() const { return render_target; }
	SDL_WindowID getWindowId() const { return window_id; }
	SDL_Window* getSDLWindow() const { return sdl_window; }

	void startRenderFrame(SDL_GPUCommandBuffer* command_buffer);
};

#endif // SLUAGH_WINDOW_H
