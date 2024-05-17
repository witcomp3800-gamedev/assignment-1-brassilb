#include <include/raylib.h>
#include <imgui/imgui.h>
#include <imgui/rlImGui.h>
#include <imgui/imgui_stdlib.h>
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
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
	 * Floating point RGBA color.
	 */
	struct Color {
		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;
		float a = 0.0f;

		/**
		 * Creates a default transparent color.
		 */
		Color() = default;

		/*
		 * Creates the specified color.
		 * @param r Red channel [0, 1]
		 * @param g Green channel [0, 1]
		 * @param b Blue channel [0, 1]
		 * @param a Alpha channel [0, 1], default 1 (opaque)
		 */
		Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
	};

	/**
	 * Font file path, size, and color.
	 */
	struct FontAsset {
		std::filesystem::path file;
		int size = 12;
		Color color ={ 1.0f, 1.0f, 1.0f, 1.0f };
	};

	/**
	 * 2-dimensional position vector.
	 */
	struct Position {
		float x = 0.0f;
		float y = 0.0f;

		/**
		 * Creates a default position with coordinates (0, 0).
		 */
		Position() = default;

		/**
		 * Creates a position with the specified coordinates.
		 * @param x X-coordinate
		 * @param y Y-coordinate
		 */
		Position(float x, float y) : x(x), y(y) {}
	};

	/**
	 * 2-dimensional velocity vector.
	 */
	struct Velocity {
		float x = 0.0f;
		float y = 0.0f;

		/**
		 * Creates default zero velocity.
		 */
		Velocity()  = default;

		/**
		 * Creates a velocity with the specified value.
		 * @param x X-component
		 * @param y Y-component
		 */
		Velocity(float x, float y) : x(x), y(y) {}
	};

	/**
	 * Represents a 2-dimensional shape which can be drawn to the screen with raylib.
	 */
	class Shape {
	public:
		/**
		 * Destructor
		 */
		virtual ~Shape() = default;

		/*
		 * Calculates how much a shape overflows a window given a position and scale.
		 * @param window Containing window
		 * @param position Coordinates in pixels
		 * @param scale Scale factor
		 * @return Overflow in pixels
		 */
		virtual Vector2 calc_overflow(const Window& window, Position position, float scale) const = 0;

		/**
		 * Draws a shape with specified position, scale, and color.
		 * @param position Coordinates in pixels
		 * @param scale Scale factor
		 * @param color Fill color
		 */
		virtual void draw(Position position, float scale, Color color) const = 0;

		/**
		 * Creates a new copy of this shape polymorphically.
		 * @return Owning pointer to new shape
		 */
		virtual Shape* clone() const = 0;
	};

	/**
	 * Represents a circle which can be drawn to the screen with raylib.
	 */
	class Circle : public Shape {
	public:
		float radius = 1.0f;

		/**
		 * Creates a default circle with radius 1.
		 */
		Circle() = default;

		/**
		 * Creates a circle with the specified radius.
		 * @param Radius
		 * @warning The effect of creating a circle with negative radius is undefined.
		 */
		Circle(float radius) : radius(radius) {}

		Vector2 calc_overflow(const Window& window, Position position, float scale) const override;
		void draw(Position position, float scale, Color color) const override;
		Shape* clone() const override { return new Circle{ *this }; }
	};

	/**
	 * Represents a rectangle which can be drawn to the screen with raylib.
	 */
	class Rectangle : public Shape {
	public:
		float width = 1.0f;
		float height = 1.0f;

		/**
		 * Creates a default rectangle with width and height 1.
		 */
		Rectangle() = default;

		/**
		 * Creates a rectangle with the specified width and height.
		 * @param width Width
		 * @param height Height
		 * @warning The effect of creating a rectangle with negative width or height is undefined.
		 */
		Rectangle(float width, float height) : width(width), height(height) {}

		Vector2 calc_overflow(const Window& window, Position position, float scale) const override;
		void draw(Position position, float scale, Color color) const override;
		Shape* clone() const override { return new Rectangle{ *this }; }
	};

	/**
	 * Game entity
	 */
	class Entity {
	public:
		std::string name;
		Position position;
		Velocity velocity;
		std::unique_ptr<Shape> shape;
		float scale = 1.0f;
		Color color;
		bool is_active = false;

		/**
		 * Creates a default deactivated entity.
		 */
		Entity() = default;

		/**
		 * Copy constructor (deep copy)
		 * @param other Entity object to copy
		 */
		Entity(const Entity& other);

		/**
		 * Move constructor
		 * @param other Entity object to move from
		 */
		Entity(Entity&& other) noexcept;

		/**
		 * Destructor
		 */
		~Entity() = default;

		/**
		 * Copy assignment operator (deep copy)
		 * @param right Entity object to copy
		 * @return self (for chaining)
		 */
		Entity& operator =(const Entity& right);

		/**
		 * Move assignment operator
		 * @param right Entity object to move from
		 * @return self (for chaining)
		 */
		Entity& operator =(Entity&& right) noexcept;
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
	 * @param input Input stream
	 * @param obj Game config
	 * @return Input stream (for chaining)
	 */
	std::istream& operator >>(std::istream& input, Config& obj);

	/**
	 * Draws an entity's name to the screen with raylib.
	 * @param entity Entity object to draw
	 * @param font raylib font data
	 * @param size Text font size
	 * @param color Text color
	 */
	void draw_name(const Entity& entity, const Font& font, int size, Color color);

	/**
	 * Draws an entity's shape to the screen with raylib.
	 * @param entity Entity object to draw
	 */
	void draw_shape(const Entity& entity);

	/**
	 * Loads game configuration from the specified file path.
	 * @param path Input file path
	 * @throws std::runtime_error if file not read
	 * @return Input stream (for chaining)
*
	 */
	Config load_config(const std::filesystem::path& path);

	/*
	 * Moves entity, adjusting position and velocity.
	 * @details If the entity shape collides with the window bounds, the velocity
				vector is adjusted in the x and/or y direction so the shape will
				bounce off the edge of the window.
	 * @param entity Entity object to move
	 * @param window Containing window
	 */
	void move(Entity& entity, const Window& window);

	/**
	 * Reads an entity with a circle from an input stream.
	 * @param input Input stream
	 * @param obj Game entity
	 * @return Input stream (for chaining)
	 */
	std::istream& read_circle_entity(std::istream& input, Entity& obj);

	/**
	 * Reads an entity's common components (i.e. not the shape) from an input stream.
	 * @param input Input stream
	 * @param obj Game entity
	 * @return Input stream (for chaining)
	 */
	std::istream& read_common_components(std::istream& input, Entity& obj);

	/**
	 * Reads an entity with a rectangle from an input stream.
	 * @param input Input stream
	 * @param obj Game entity
	 * @return Input stream (for chaining)
	 */
	std::istream& read_rectangle_entity(std::istream& input, Entity& obj);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
	// Initialization
	//--------------------------------------------------------------------------------------
	const auto input_path = std::filesystem::path{ "assets/input.txt" };
	const auto [window, font_asset, entity_templates] = a1::load_config(input_path);
	auto entities = std::vector<a1::Entity>{};
	entities.reserve(entity_templates.size());
	std::copy(entity_templates.begin(), entity_templates.end(), std::back_inserter(entities));

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
	bool draw_shapes_enabled = true;
	bool draw_names_enabled = true;
	bool move_enabled = true;
	const auto font = LoadFont(font_asset.file.string().c_str());

	// Main game loop
	//--------------------------------------------------------------------------------------
	while( !WindowShouldClose() )    // Detect window close button or ESC key
	{
		// Update
		//----------------------------------------------------------------------------------
		for( auto& entity : entities ) {
			if( !entity.is_active ) {
				continue;
			}
			if( move_enabled ) {
				move(entity, window);
			}
		}

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(BLACK);

		//********** Raylib Drawing Content **********

		for( const auto& entity : entities ) {
			if( !entity.is_active ) {
				continue;
			}
			if( draw_shapes_enabled ) {
				draw_shape(entity);
			}
			if( draw_names_enabled ) {
				draw_name(entity, font, font_asset.size, font_asset.color);
			}
		}

		//********** ImGUI Content *********

		rlImGuiBegin();
		ImGui::SetNextWindowSize(ImVec2(400, 400));
		ImGui::Begin("Assignment 1 Controls", NULL, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse);

		ImGui::SeparatorText("All Shape Controls");
		ImGui::Checkbox("Draw Shapes", &draw_shapes_enabled);
		ImGui::SameLine();
		ImGui::Checkbox("Draw Text", &draw_names_enabled);
		ImGui::SameLine();
		ImGui::Checkbox("Simulate", &move_enabled);

		ImGui::SeparatorText("Selected Shape Controls");
		if( entities.size() > 0 ) {
			static std::size_t current_index = 0;
			if( ImGui::BeginCombo("Shape", entities[current_index].name.c_str()) ) {
				for( std::size_t i = 0; i < entities.size(); ++i ) {
					const bool is_selected = current_index == i;
					if( ImGui::Selectable(entities[i].name.c_str(), is_selected) ) {
						current_index = i;
					}
					if( is_selected ) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			ImGui::Checkbox("Active", &entities[current_index].is_active);
		}

		/*
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
		*/

		ImGui::End();
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
	Vector2 Circle::calc_overflow(const Window& window, Position position, float scale) const {
		float min_x = position.x - scale * radius;
		float min_y = position.y - scale * radius;
		float max_x = position.x + scale * radius;
		float max_y = position.y + scale * radius;
		return {
			min_x < 0 ? min_x : max_x > window.width ? max_x - window.width : 0,
			min_y < 0 ? min_y : max_y > window.height ? max_y - window.height : 0
		};
	}

	void Circle::draw(Position position, float scale, Color color) const {
		DrawCircle(
			static_cast<int>(position.x),
			static_cast<int>(position.y),
			radius * scale,
			ColorFromNormalized({ color.r, color.g, color.b, color.a })
		);
	}

	Vector2 Rectangle::calc_overflow(const Window& window, Position position, float scale) const {
		float min_x = position.x - scale * width / 2;
		float min_y = position.y - scale * height / 2;
		float max_x = position.x + scale * width / 2;
		float max_y = position.y + scale * height / 2;
		return {
			min_x < 0 ? min_x : max_x > window.width ? max_x - window.width : 0,
			min_y < 0 ? min_y : max_y > window.height ? max_y - window.height : 0
		};
	}


	void Rectangle::draw(Position position, float scale, Color color) const {
		DrawRectangle(
			static_cast<int>(position.x - width / 2),
			static_cast<int>(position.y - height / 2),
			static_cast<int>(width),
			static_cast<int>(height),
			ColorFromNormalized({ color.r, color.g, color.b, color.a })
		);
	}

	Entity::Entity(const Entity& other) :
		name(other.name),
		position(other.position),
		velocity(other.velocity),
		shape(other.shape->clone()),
		color(other.color),
		is_active(other.is_active) {
	}

	Entity::Entity(Entity&& other) noexcept :
		name(std::move(other.name)),
		position(other.position),
		velocity(other.velocity),
		shape(std::move(other.shape)),
		color(other.color),
		is_active(other.is_active) {
	}

	Entity& Entity::operator =(const Entity& right) {
		if( &right != this ) {
			name = right.name;
			position = right.position;
			velocity = right.velocity;
			shape = std::unique_ptr<Shape>{ right.shape->clone() };
			color = right.color;
			is_active = right.is_active;
		}
		return *this;
	}

	Entity& Entity::operator =(Entity&& right) noexcept {
		if( &right != this ) {
			name = std::move(right.name);
			position = right.position;
			velocity = right.velocity;
			shape = std::move(right.shape);
			color = right.color;
			is_active = right.is_active;
		}
		return *this;
	}

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
				obj.font_asset.color.a = 1.0f;
			}
			else if( s == "Circle" ) {
				Entity circle_entity;
				if( read_circle_entity(input, circle_entity) ) {
					obj.entity_templates.push_back(std::move(circle_entity));
				}
			}
			else if( s == "Rectangle" ) {
				Entity rectangle_entity;
				if( read_rectangle_entity(input, rectangle_entity) ) {
					obj.entity_templates.push_back(std::move(rectangle_entity));
				}
			}
		}
		return input;
	}

	void draw_name(const Entity& entity, const Font& font, int size, Color color) {
		const auto text_size = MeasureTextEx(font, entity.name.c_str(), size, 1.0f);
		DrawTextEx(
			font,
			entity.name.c_str(),
			{ entity.position.x - text_size.x / 2, entity.position.y - text_size.y / 2 },
			size,
			1.0f,
			ColorFromNormalized({ color.r, color.g, color.b, color.a })
		);
	}

	void draw_shape(const Entity& entity) {
		entity.shape->draw(entity.position, entity.scale, entity.color);
	}

	Config load_config(const std::filesystem::path& path) {
		auto input_file = std::ifstream{ path };
		a1::Config config;
		input_file >> config;
		if( !input_file && !input_file.eof() ) {
			throw std::runtime_error("Failed to read configuration file.");
		}
		return config;
	}

	void move(Entity& entity, const Window& window) {
		const auto next_position = Position{
			entity.position.x + entity.velocity.x,
			entity.position.y + entity.velocity.y
		};
		const auto overflow = entity.shape->calc_overflow(window, next_position, entity.scale);
		if( overflow.x != 0 ) {
			entity.velocity.x = -entity.velocity.x;
		}
		if( overflow.y != 0 ) {
			entity.velocity.y = -entity.velocity.y;
		}
		entity.position ={
			entity.position.x + entity.velocity.x,
			entity.position.y + entity.velocity.y
		};
	}

	std::istream& read_circle_entity(std::istream& input, Entity& entity) {
		float radius;
		if( read_common_components(input, entity) && input >> radius ) {
			entity.shape = std::make_unique<Circle>(radius);
		}
		return input;
	}

	std::istream& read_common_components(std::istream& input, Entity& entity) {
		input
			>> entity.name
			>> entity.position.x >> entity.position.y
			>> entity.velocity.x >> entity.velocity.y
			>> entity.color.r >> entity.color.g >> entity.color.b;
		entity.color.a = 1.0f;
		entity.is_active = true;
		return input;
	}

	std::istream& read_rectangle_entity(std::istream& input, Entity& entity) {
		float width, height;
		if( read_common_components(input, entity) && input >> width >> height ) {
			entity.shape = std::make_unique<Rectangle>(width, height);
		}
		return input;
	}
}