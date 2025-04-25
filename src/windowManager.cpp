#include "../include/windowManager.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_dx11.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include <iostream>

WindowManager::WindowManager() : m_width{800}, m_height{600}, state{}
{
	// ZoneScoped;
}

WindowManager::~WindowManager()
{
	// ZoneScoped;
	SDL_DestroyWindow(m_window_handle);
	SDL_Quit();
}

void WindowManager::run()
{
	// ZoneScoped;
	init();
}

void WindowManager::init_imgui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoKeyboard; // disable Keyboard Controls

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForD3D(m_window_handle);
	ImGui_ImplDX11_Init((ID3D11Device *)state->gpu.device, (ID3D11DeviceContext *)state->gpu.context);
}

bool WindowManager::init()
{
	// ZoneScoped;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}
	else
	{
		// From 2.0.18: Enable native IME.
	#ifdef SDL_HINT_IME_SHOW_UI
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
	#endif
		m_window_handle = SDL_CreateWindow(
			"window title",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			m_width, m_height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		if (NULL == m_window_handle)
		{
			std::cerr << "SDL could not create a window! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}
	}
	SDL_DisplayMode md{};
	SDL_GetCurrentDisplayMode(0, &md);
	state->window.screen_width = md.w;
	state->window.screen_height = md.h;
	state->window.window_origin_x = md.w / 2 - m_width / 2;
	state->window.window_origin_y = md.h / 2 - m_height / 2;
	m_window_surface = SDL_GetWindowSurface(m_window_handle);
	if (NULL == m_window_surface)
	{
		std::cerr << "SDL could not acquire window surface! SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}

	// update engine state
	{
		std::unique_lock lock(state->window.m);
		state->running = true;
		state->window.resized = false;
		state->window.bytes_per_pixel = m_window_surface->format->BytesPerPixel;
		state->window.surface = m_window_surface->pixels;
		state->window.width = m_width;
		state->window.height = m_height;
		state->window.win32_win = native_win32_handle();
	}

	// init ui state
	state->window.mouse_yaw = state->backend == BACKEND_D3D11 ? 90.0f : -90.0f;
	state->window.mouse_pitch = 0.0f;
	state->window.cursor_dx = 0.0f;
	state->window.cursor_dy = 0.0f;
	state->window.move_cam_back = false;
	state->window.move_cam_forward = false;
	state->window.move_cam_left = false;
	state->window.move_cam_right = false;
	state->window.enable_mouse_movement = false;

	return true;
}

bool WindowManager::resize()
{
	// ZoneScoped;
	std::unique_lock lock(state->window.m);
	SDL_GetWindowSize(m_window_handle, (int *)&m_width, (int *)&m_height);
	m_window_surface = SDL_GetWindowSurface(m_window_handle);
	state->window.width = m_width;
	state->window.height = m_height;
	state->window.bytes_per_pixel = m_window_surface->format->BytesPerPixel;
	state->window.surface = m_window_surface->pixels;
	state->window.resized = true;

	return true;
}

HWND WindowManager::native_win32_handle()
{
	SDL_SysWMinfo systemInfo;
	SDL_VERSION(&systemInfo.version);
	SDL_GetWindowWMInfo(m_window_handle, &systemInfo);

	return systemInfo.info.win.window;
}

void WindowManager::start_event_loop()
{
	// ZoneScoped;
	SDL_Event event;
	ImGuiIO &io = ImGui::GetIO();
	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
		{
			state->running = false;
			break;
		}
		case SDL_MOUSEMOTION:
		{
			if (io.WantCaptureMouse)
			{
				ImGui_ImplSDL2_ProcessEvent(&event);
				break;
			}
			state->window.cursor_dx = event.motion.xrel;
			state->window.cursor_dy = event.motion.yrel;
			state->window.cursor_x = event.motion.x;
			state->window.cursor_y = event.motion.y;
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if (io.WantCaptureMouse)
			{
				ImGui_ImplSDL2_ProcessEvent(&event);
				break;
			}
			if (event.button.button == 1)
				state->window.enable_mouse_movement = true;
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			if (io.WantCaptureMouse)
			{
				ImGui_ImplSDL2_ProcessEvent(&event);
				break;
			}
			if (event.button.button == 1)
				state->window.enable_mouse_movement = false;
			break;
		}
		case SDL_KEYDOWN:
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_w:
				state->window.move_cam_forward = true;
				break;
			case SDLK_s:
				state->window.move_cam_back = true;
				break;
			case SDLK_d:
				state->window.move_cam_right = true;
				break;
			case SDLK_a:
				state->window.move_cam_left = true;
				break;
			case SDLK_ESCAPE:
				state->running = false;
				break;
			default:
				break;
			}
			break;
		}
		case SDL_KEYUP:
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_w:
				state->window.move_cam_forward = false;
				break;
			case SDLK_s:
				state->window.move_cam_back = false;
				break;
			case SDLK_d:
				state->window.move_cam_right = false;
				break;
			case SDLK_a:
				state->window.move_cam_left = false;
				break;
			default:
				break;
			}
			break;
		}
		case SDL_WINDOWEVENT: // resize event
		{

			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				resize();
				break;
			default:
				break;
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}
}

void WindowManager::update_surface()
{
	// ZoneScoped;
	if (state->running)
		SDL_UpdateWindowSurface(m_window_handle);
}

void WindowManager::enable_window_resizing(bool enable)
{
	// ZoneScoped;
	SDL_SetWindowResizable(m_window_handle, (SDL_bool)enable);
}

void WindowManager::update_window_title(const char *str)
{
	// ZoneScoped;
	if (state->running)
		SDL_SetWindowTitle(m_window_handle, str);
}