#include "../include/windowManager.h"

#include <iostream>
#include <SDL.h>

WindowManager::WindowManager() :m_width{ 800 }, m_height{ 600 }, state{}, draw_frame_callback{ nullptr }
{
	//ZoneScoped;

}

WindowManager::~WindowManager()
{
	//ZoneScoped;

	//event_loop.join();
	SDL_DestroyWindow(m_window_handle);
	SDL_Quit();
}

void WindowManager::run()
{
	//ZoneScoped;

	init();
	//event_loop = std::thread(&WindowManager::start_event_loop, this);
}

bool WindowManager::init()
{
	//ZoneScoped;

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

	// update engine state
	{
		std::unique_lock lock(state->m_window.m);
		state->running = true;
		state->m_window.win_resized = false;
		state->m_window.win_bytes_per_pixel = m_window_surface->format->BytesPerPixel;
		state->m_window.win_surface = m_window_surface->pixels;
		state->m_window.win_width = m_width;
		state->m_window.win_height = m_height;
	}

	// init ui state
	state->m_window.mouse_yaw = -90.0f;
	state->m_window.mouse_pitch = 0.0f;
	state->m_window.cursor_dx = 0.0f;
	state->m_window.cursor_dy = 0.0f;
	state->m_window.move_cam_back = false;
	state->m_window.move_cam_forward = false;
	state->m_window.move_cam_left = false;
	state->m_window.move_cam_right = false;
	state->m_window.enable_mouse_movement = false;

	return true;
}

bool WindowManager::resize(unsigned int width, unsigned int height)
{
	//ZoneScoped;

	return true;
}


void WindowManager::start_event_loop()
{
	//ZoneScoped;

	SDL_Event event;
	bool resized = false;
	while (state->running) {

		while (SDL_PollEvent(&event) != 0)
		{
			state->m_window.cursor_dx = 0.0f;
			state->m_window.cursor_dy = 0.0f;
			switch (event.type)
			{
			case SDL_QUIT:
				std::cout << "Exiting..\n";
				state->running = false;
				break;
			case SDL_MOUSEMOTION:
				state->m_window.cursor_dx = event.motion.xrel;
				state->m_window.cursor_dy = event.motion.yrel;
				//std::cout << "X: " << state->m_window.cursor_dx << " ,Y: " << state->m_window.cursor_dy << "\n";
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == 1)
					state->m_window.enable_mouse_movement = true;
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == 1)
					state->m_window.enable_mouse_movement = false;
				break;
			case SDL_KEYDOWN:
				std::cout << "KEYDOWN event\tkey: " << SDL_GetKeyName(event.key.keysym.sym) << "\n";
				switch (event.key.keysym.sym)
				{
				case SDLK_w:
					state->m_window.move_cam_forward = true;
					break;
				case SDLK_s:
					state->m_window.move_cam_back = true;
					break;
				case SDLK_d:
					state->m_window.move_cam_right = true;
					break;
				case SDLK_a:
					state->m_window.move_cam_left = true;
					break;
				case SDLK_ESCAPE:
					state->running = false;
					break;
				default:
					break;
				}
				break;
			case SDL_KEYUP:
				std::cout << "KEYUP event\tkey: " << SDL_GetKeyName(event.key.keysym.sym) << "\n";
				switch (event.key.keysym.sym)
				{
				case SDLK_w:
					state->m_window.move_cam_forward = false;
					break;
				case SDLK_s:
					state->m_window.move_cam_back = false;
					break;
				case SDLK_d:
					state->m_window.move_cam_right = false;
					break;
				case SDLK_a:
					state->m_window.move_cam_left = false;
					break;
				default:
					break;
				}
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
				break;
			default:
				break;
			}
		}
		if (resized) {
			resized = false;
			// update engine state
			{
				std::unique_lock lock(state->m_window.m);
				SDL_GetWindowSize(m_window_handle, (int*)&m_width, (int*)&m_height);
				m_window_surface = SDL_GetWindowSurface(m_window_handle);
				state->m_window.win_width = m_width;
				state->m_window.win_height = m_height;
				state->m_window.win_bytes_per_pixel = m_window_surface->format->BytesPerPixel;
				state->m_window.win_surface = m_window_surface->pixels;
				state->m_window.win_resized = true;
			}
		}
	}
}

void WindowManager::set_draw_frame_callback(void (*callback)(int w, int h, int bytes_per_pixel, void* framebuffer))
{
	//ZoneScoped;

	if (callback)
		draw_frame_callback = callback;
}

void WindowManager::update_surface()
{
	//ZoneScoped;

	if (state->running)
		SDL_UpdateWindowSurface(m_window_handle);
}

void WindowManager::enable_window_resizing(bool enable)
{
	//ZoneScoped;

	SDL_SetWindowResizable(m_window_handle, (SDL_bool)enable);
}

void WindowManager::update_window_title(const char* str)
{
	//ZoneScoped;

	if (state->running)
		SDL_SetWindowTitle(m_window_handle, str);
}