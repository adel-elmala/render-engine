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

	void start_event_loop();
	void update_surface();
	void enable_window_resizing(bool enable);
	void update_window_title(const char *str);
	void bind_state(Engine_State *engine_state)
	{
		if (engine_state)
			state = engine_state;
	}
	void run();
	void init_imgui();

	SDL_Surface *m_window_surface;

private:
	bool init();
	bool resize();
	HWND native_win32_handle();

	unsigned int m_width;
	unsigned int m_height;

	SDL_Window *m_window_handle;
	Engine_State *state;
};
