#pragma once

#include "common.h"

struct SDL_Window;
struct SDL_Surface;

struct Engine_State;


class WindowManager
{
public:
	WindowManager();
	~WindowManager();
	void set_draw_frame_callback(void (*callback)(int w, int h, int bytes_per_pixel, void* framebuffer));
	void start_event_loop();
	void bind_state(Engine_State* engine_state) { if(engine_state) state = engine_state; }
	void run();

	SDL_Surface* m_window_surface;
private:
	bool init();
	bool resize(unsigned int width, unsigned int height);

	unsigned int m_width;
	unsigned int m_height;
	void (*draw_frame_callback)(int w, int h, int bytes_per_pixel, void* framebuffer);

	SDL_Window* m_window_handle;
	Engine_State* state;
};