#include "../include/windowManager.h"

#include <iostream>
#include <SDL.h>

WindowManager::WindowManager() :m_width{ 800 }, m_height{ 600 }, state{}, draw_frame_callback{nullptr}
{
}

WindowManager::~WindowManager()
{
	SDL_DestroyWindow(m_window_handle);
	SDL_Quit();
}

void WindowManager::run()
{
	init();
}
bool WindowManager::init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}
	else
	{
		m_window_handle = SDL_CreateWindow(
			"window title",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			m_width, m_height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
		);
		if (NULL == m_window_handle)
		{
			std::cerr << "SDL could not create a window! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}
	}
	m_window_surface = SDL_GetWindowSurface(m_window_handle);
	state->running = true;

	return true;
}
bool WindowManager::resize(unsigned int width, unsigned int height)
{
	return true;
}
// TODO[adel] : put this in a thread 
void WindowManager::start_event_loop()
{
	SDL_Event event;
	bool resized = false;

	while (state->running) {

		while (SDL_PollEvent(&event) != 0)
		{
			switch (event.type)
			{
			case SDL_QUIT:
				std::cout << "Exiting..\n";
				state->running = false;
				break;
			case SDL_MOUSEMOTION:
				break;
			case SDL_MOUSEBUTTONDOWN:
				break;
			case SDL_MOUSEBUTTONUP:
				break;
			case SDL_KEYDOWN:
				std::cout << "KEYDOWN event\tkey: " << SDL_GetKeyName(event.key.keysym.sym) << "\n";
				break;
			case SDL_KEYUP:
				std::cout << "KEYUP event\tkey: " << SDL_GetKeyName(event.key.keysym.sym) << "\n";
				break;
			case SDL_WINDOWEVENT: // resize event
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					std::cout << "resized\n";
					resized = true;
					break;
				default:
					break;
				}

			default:
				break;
			}
		}
		if (resized) {

			SDL_GetWindowSize(m_window_handle, (int*)&m_width, (int*)&m_height);
			// update engine state
			state->win_width = m_width;
			state->win_height = m_height;
			m_window_surface = SDL_GetWindowSurface(m_window_handle);
		}
		// call the renderer callback to fill the framebuffer with the new frame
		if (draw_frame_callback)
			draw_frame_callback(m_width, m_height, m_window_surface->format->BytesPerPixel, m_window_surface->pixels);
		SDL_UpdateWindowSurface(m_window_handle);
	}
}

void WindowManager::set_draw_frame_callback(void (*callback)(int w, int h, int bytes_per_pixel, void* framebuffer))
{
	if (callback)
		draw_frame_callback = callback;
}
