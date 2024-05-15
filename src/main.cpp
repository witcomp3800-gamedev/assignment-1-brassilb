#include <include/raylib.h>
#include <imgui/imgui.h>
#include <imgui/rlImGui.h>
#include <imgui/imgui_stdlib.h>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace a1 {
	/**
	 * Window caption and screen size settings.
	 */
	struct Window {
		std::string caption;
		int width = 1280;
		int height = 800;
	};

	/**
	 * 32-bit RGBA color.
	 */
	struct Color {
		std::uint8_t r = 0;
		std::uint8_t g = 0;
		std::uint8_t b = 0;
		std::uint8_t a = 0;

		/**
		 * Creates a default transparent color.
		 */
		Color() = default;

		/*
		 * Creates the specified color.
		 * @param r Red channel [0, 255]
		 * @param g Green channel [0, 255]
		 * @param b Blue channel [0, 255]
		 * @param a Alpha channel [0, 255], default 255 (opaque)
		 */
		Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
	};

	/**
	 * Font file path, size, and color.
	 */
	struct FontAsset {
		std::filesystem::path file;
		int size = 12;
		Color color ={ 255, 255, 255, 255 };
	};

	/**
	 *
	 */
	class Entity {
		// TODO
	};

	/**
	 * Game configuration for window settings, font information, and starting entity data.
	 */
	struct Config {
		Window window;
		FontAsset font_asset;
		std::vector<Entity> entity_templates;
	};

	/**
	 * Reads game config from an input stream.
	 * @details Each line may contain one of the following, in any order:
	 *     - Window [Caption] [Width] [Height]
	 *     - Font [File] [Size] [Red] [Green] [Blue]
	 *     - Rectangle [Name] [X] [Y] [X Velocity] [Y Velocity] [Red] [Green] [Blue] [Width] [Height]
	 *     - Circle [Name] [X] [Y] [X Velocity] [Y Velocity] [Red] [Green] [Blue] [Radius]
	 * @param input Input file stream
	 * @param obj Game config
	 */
	std::istream& operator >>(std::istream& input, Config& obj);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
	// Initialization
	//--------------------------------------------------------------------------------------
	const auto input_path = std::filesystem::path{ "assets/input.txt" };
	auto input_file = std::ifstream{ input_path };
	a1::Config config;
	input_file >> config;
	if( !input_file ) {
		std::cerr << "Failed to load config from path " << input_path << ". Exiting...\n";
		return -1;
	}
	auto& [window, font_asset, entity_templates] = config;

	SetConfigFlags(FLAG_WINDOW_HIGHDPI);
	InitWindow(window.width, window.height, window.caption.c_str());

	//initialize the raylib ImGui backend
	rlImGuiSetup(true);
	//increase ImGui item size to 2x
	ImGui::GetStyle().ScaleAllSizes(2);

	// Set raylib to target 60 frames-per-second (this does not mean it will actually RUN at 60 fps)
	SetTargetFPS(60);

	// General variables
	//--------------------------------------------------------------------------------------

	//shape properties to draw on the screen (circle for this example)
	//units of size and speed are in pixels
	float circRadius=50;
	float circSpeedX=1.0f;
	float circSpeedY=0.5f;
	bool drawCirc=true;
	float circX=50.0f;
	float circY=50.0f;
	float color[3] ={ 0.0f,0.0,1.0f }; //color is from 0-1

	//Let's draw some text to the screen too
	bool drawText=true;
	std::string strText= "Some Text";
	std::string newText= strText;

	//load a font (Raylib gives warning if font can't be found, then uses default as fallback)
	Font font = LoadFont(font_asset.file.string().c_str());

	// Main game loop
	//--------------------------------------------------------------------------------------
	while( !WindowShouldClose() )    // Detect window close button or ESC key
	{
		// Update
		//----------------------------------------------------------------------------------

		//move circle
		circX+=circSpeedX;
		circY+=circSpeedY;

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(BLACK);

		//********** Raylib Drawing Content **********

		//draw the cricle (center x, center y, radius, color(r,g,b,a))
		if( drawCirc ) {
			DrawCircle((int)circX, (int)circY, circRadius, ColorFromNormalized({ color[0],color[1],color[2],1.0f }));
		}

		//draw the text
		if( drawText ) {
			//get the size (x and y) of the text object
			//(font,c string, font size, font spaceing)
			Vector2 textSize= MeasureTextEx(font, strText.c_str(), font_asset.size, 1);

			//draw the text (using the text size to help draw it in the corner
			//(font,c string, vector2, font size, font spaceing, color)
			DrawTextEx(font, strText.c_str(), { 0.0f, window.height - textSize.y }, font_asset.size, 1, WHITE);
		}

		//********** ImGUI Content *********

		//Draw imgui stuff last so it is over the top of everything else
		rlImGuiBegin();

		//sets the next window to be at this position
		//also uses the imgui.ini that gets created at first run
		ImGui::SetNextWindowSize(ImVec2(350, 250));
		//creates a new window
		ImGui::Begin("My Window", NULL, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse);
		ImGui::Text("The Window Text!");
		//checkboxes, they directly modify the value (which is why we send a reference)
		ImGui::Checkbox("Draw Cricle", &drawCirc);
		ImGui::SameLine();
		ImGui::Checkbox("Draw Text", &drawText);

		//slider, again directly modifies the value and limites between 0 and 300 for this example
		ImGui::SliderFloat("Radius", &circRadius, 0.0f, 300.0f);

		//color picker button, directly modifies the color (3 element float array)
		ImGui::ColorEdit3("Circle Color", color);

		//text input field, directly modifies the string
		ImGui::InputText("Text", &newText);

		//buttons, returns true if clicked on this frame
		if( ImGui::Button("Set Text") ) {
			strText=newText;
		}

		//The next item will be on the same line as the previous one
		ImGui::SameLine();

		//Another button
		if( ImGui::Button("Reset Circle") ) {
			circX=50.0;
			circY=50.0;
			circRadius=50;
		}
		//ends this window
		ImGui::End();

		//show ImGui Demo Content if you want to see it
		//bool open = true;
		//ImGui::ShowDemoWindow(&open);

	// end ImGui Content
		rlImGuiEnd();

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// Clean Up
	//--------------------------------------------------------------------------------------
	rlImGuiShutdown();    // Shuts down the raylib ImGui backend
	UnloadFont(font);     // Remove font from memory
	CloseWindow();        // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}

namespace a1 {
	std::istream& operator >>(std::istream& input, Config& obj) {
		std::string s;
		while( input >> s ) {
			if( s == "Window" ) {
				input
					>> obj.window.caption
					>> obj.window.width
					>> obj.window.height;
			}
			else if( s == "Font" ) {
				input
					>> obj.font_asset.file
					>> obj.font_asset.size
					>> obj.font_asset.color.r >> obj.font_asset.color.g >> obj.font_asset.color.b;
				obj.font_asset.color.a = 255;
			}
			else if( s == "Circle" ) {
				// TODO
			}
			else if( s == "Rectangle" ) {
				// TODO
			}
			return input;
		}
	}
}