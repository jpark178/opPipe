#include "driver_state.h"
#include <cstring>

driver_state::driver_state()
{
}

driver_state::~driver_state()
{
	delete[] image_color;
	delete[] image_depth;
}

// This function should allocate and initialize the arrays that store color and
// depth.  This is not done during the constructor since the width and height
// are not known when this class is constructed.
void initialize_render(driver_state& state, int width, int height)
{
	state.image_width = width;
	state.image_height = height;
	state.image_color = 0;
	state.image_depth = 0;

	//Initialize the state color and depth arrays to the width*height of the image
	state.image_color = new pixel[width * height];
	state.image_depth = new float[width * height];

	for (int i = 0; i < (width * height); i++) {
		state.image_color[i] = make_pixel(0, 0, 0);
		state.image_depth[i] = 2;
	}

}

// This function will be called to render the data that has been stored in this class.
// Valid values of type are:
//   render_type::triangle - Each group of three vertices corresponds to a triangle.
//   render_type::indexed -  Each group of three indices in index_data corresponds
//                           to a triangle.  These numbers are indices into vertex_data.
//   render_type::fan -      The vertices are to be interpreted as a triangle fan.
//   render_type::strip -    The vertices are to be interpreted as a triangle strip.
void render(driver_state& state, render_type type)
{
	bool DEBUG = false;

	switch (type) {

	case(render_type::triangle): {

		//Initializing a data_vertex array setting the data pointing to the appropriate locations in state.vertex_data
		data_vertex* vertex_info[state.num_vertices];
		for (int i = 0, j = 0; i < state.num_vertices* state.floats_per_vertex; i += state.floats_per_vertex, j++) {
			vertex_info[j] = new data_vertex[MAX_FLOATS_PER_VERTEX];
			vertex_info[j]->data = &state.vertex_data[i];
		}


		//Initializing the data_geometry array and sending it to vertex shader to be populated
		data_geometry* vertex_holders[state.num_vertices];
		for (int i = 0; i < state.num_vertices; i++) {
			vertex_holders[i] = new data_geometry();
			vertex_holders[i]->data = vertex_info[i]->data;
			state.vertex_shader(*vertex_info[i], *vertex_holders[i], state.uniform_data);
		}


		//Sending each set of 3 vertices to clip_triangle
		for (int i = 0; i < state.num_vertices / 3; i++) {
			const data_geometry** triangle = const_cast<const data_geometry**>(vertex_holders + (3 * i));
			clip_triangle(state, triangle, 0);
		}


		//DEALLOCATION
		for (int i = 0; i < state.num_vertices; i++) {
			delete[] vertex_info[i];
			delete vertex_holders[i];
		}

		break;
	}
	case(render_type::indexed): {
		if (DEBUG) std::cout << "triangle_indexed" << std::endl;

		if (DEBUG) {
			for (int i = 0; i < state.num_vertices * state.floats_per_vertex; i++) {
				std::cout << "v[" << i << "]:" << state.vertex_data[i] << std::endl;
			}
			std::cout << std::endl;
		}

		//Initializing a data_vertex array setting the data pointing to the appropriate locations in state.vertex_data
		data_vertex* vertex_info[state.num_vertices];
		for (int i = 0, j = 0; i < state.num_vertices* state.floats_per_vertex; i += state.floats_per_vertex, j++) {
			vertex_info[j] = new data_vertex[MAX_FLOATS_PER_VERTEX];
			vertex_info[j]->data = &state.vertex_data[i];
		}

		//Initializing the data_geometry array and sending it to vertex shader to be populated
		data_geometry* vertex_holders[state.num_triangles * 3];
		for (int i = 0; i < state.num_triangles * 3; i++) {
			vertex_holders[i] = new data_geometry();
			vertex_holders[i]->data = vertex_info[state.index_data[i]]->data;
			state.vertex_shader(*vertex_info[state.index_data[i]], *vertex_holders[i], state.uniform_data);
		}

		//Sending each triangle to clip_triangle
		for (int i = 0; i < state.num_triangles; i++) {
			const data_geometry** triangle = const_cast<const data_geometry**>(vertex_holders + (3 * i));
			clip_triangle(state, triangle, 0);
		}

		//DEALLOCATION
		for (int i = 0; i < state.num_vertices; i++) {
			delete[] vertex_info[i];
		}
		for (int i = 0; i < state.num_triangles * 3; i++) {
			delete vertex_holders[i];
		}

		break;
	}
	case(render_type::fan): {
		if (DEBUG) std::cout << "triangle_fan" << std::endl;

		//Initializing a data_vertex array setting the data pointing to the appropriate locations in state.vertex_data
		data_vertex* vertex_info[state.num_vertices];
		for (int i = 0, j = 0; i < state.num_vertices* state.floats_per_vertex; i += state.floats_per_vertex, j++) {
			vertex_info[j] = new data_vertex[MAX_FLOATS_PER_VERTEX];
			vertex_info[j]->data = &state.vertex_data[i];
		}

		//Initializing the data_geometry array and sending it to vertex shader to be populated
		data_geometry* vertex_holders[state.num_vertices];
		for (int i = 0; i < state.num_vertices; i++) {
			vertex_holders[i] = new data_geometry();
			vertex_holders[i]->data = vertex_info[i]->data;
			state.vertex_shader(*vertex_info[i], *vertex_holders[i], state.uniform_data);
		}

		//Creating a triangle according to the triangle_fan scheme and sending each triangle to clip_triangle
		for (int i = 1, j = 2; i < state.num_vertices - 1; i++, j++) {
			data_geometry* tri_group[3];
			tri_group[0] = vertex_holders[0];
			tri_group[1] = vertex_holders[i];
			tri_group[2] = vertex_holders[j];

			const data_geometry** triangle = const_cast<const data_geometry**>(tri_group);
			clip_triangle(state, triangle, 0);
		}

		//DEALLOCATION
		for (int i = 0; i < state.num_vertices; i++) {
			delete[] vertex_info[i];
			delete vertex_holders[i];
		}

		break;
	}
	case(render_type::strip): {

		if (DEBUG) std::cout << "triangle_strip" << std::endl;

		//Initializing a data_vertex array setting the data pointing to the appropriate locations in state.vertex_data
		data_vertex* vertex_info[state.num_vertices];
		for (int i = 0, j = 0; i < state.num_vertices* state.floats_per_vertex; i += state.floats_per_vertex, j++) {
			vertex_info[j] = new data_vertex[MAX_FLOATS_PER_VERTEX];
			vertex_info[j]->data = &state.vertex_data[i];
		}

		//Initializing the data_geometry array and sending it to vertex shader to be populated
		data_geometry* vertex_holders[state.num_vertices];
		for (int i = 0; i < state.num_vertices; i++) {
			vertex_holders[i] = new data_geometry();
			vertex_holders[i]->data = vertex_info[i]->data;
			state.vertex_shader(*vertex_info[i], *vertex_holders[i], state.uniform_data);
		}

		//Creating a triangle according to the triangle_strip scheme and sending each triangle to clip_triangle
		for (int i = 0, j = 1, k = 2; i < state.num_vertices - 2; i++, j++, k++) {
			data_geometry* tri_group[3];
			tri_group[0] = vertex_holders[i];
			tri_group[1] = vertex_holders[j];
			tri_group[2] = vertex_holders[k];

			const data_geometry** triangle = const_cast<const data_geometry**>(tri_group);
			clip_triangle(state, triangle, 0);
		}

		//DEALLOCATION
		for (int i = 0; i < state.num_vertices; i++) {
			delete[] vertex_info[i];
			delete vertex_holders[i];
		}
		break;
	}
	default:;
	}

}

// This function clips a triangle (defined by the three vertices in the "in" array).
// It will be called recursively, once for each clipping face (face=0, 1, ..., 5) to
// clip against each of the clipping faces in turn.  When face=6, clip_triangle should
// simply pass the call on to rasterize_triangle.
void clip_triangle(driver_state& state, const data_geometry* in[3], int face)
{
	const bool DEBUG = false;

	bool is_positive = true, a_in = false, b_in = false, c_in = false;
	int plane = 0;

	//Base case: Face == 6, send the triangle rasterize_triangle
	if (face == 6)
	{
		if (DEBUG) std::cout << "RASTERIZING" << std::endl;

		if (DEBUG) {
			std::cout << "==== " << face << " ====" << std::endl;
			std::cout << "A " << in[0]->gl_Position << std::endl;
			for (int i = 3; i < state.floats_per_vertex; i++) std::cout << "   " << in[0]->data[i] << " ";
			std::cout << std::endl;
			std::cout << "B " << in[1]->gl_Position << std::endl;
			for (int i = 3; i < state.floats_per_vertex; i++) std::cout << "   " << in[1]->data[i] << " ";
			std::cout << std::endl;
			std::cout << "C " << in[2]->gl_Position << std::endl;
			for (int i = 3; i < state.floats_per_vertex; i++) std::cout << "   " << in[2]->data[i] << " ";
			std::cout << std::endl;
		}

		rasterize_triangle(state, in);
		return;
	}

	switch (face)
	{
	case 0: //Clipping with x = w
		is_positive = true;
		plane = 0;
		a_in = (in[0]->gl_Position[plane] <= in[0]->gl_Position[3]) ? true : false;
		b_in = (in[1]->gl_Position[plane] <= in[1]->gl_Position[3]) ? true : false;
		c_in = (in[2]->gl_Position[plane] <= in[2]->gl_Position[3]) ? true : false;
		if (DEBUG) std::cout << "Case 0: " << std::endl;
		break;
	case 1: //Clipping with x = -w
		is_positive = false;
		plane = 0;
		a_in = (in[0]->gl_Position[plane] >= -in[0]->gl_Position[3]) ? true : false;
		b_in = (in[1]->gl_Position[plane] >= -in[1]->gl_Position[3]) ? true : false;
		c_in = (in[2]->gl_Position[plane] >= -in[2]->gl_Position[3]) ? true : false;
		if (DEBUG) std::cout << "Case 1: " << std::endl;
		break;
	case 2: //Clipping with y = w
		is_positive = true;
		plane = 1;
		a_in = (in[0]->gl_Position[plane] <= in[0]->gl_Position[3]) ? true : false;
		b_in = (in[1]->gl_Position[plane] <= in[1]->gl_Position[3]) ? true : false;
		c_in = (in[2]->gl_Position[plane] <= in[2]->gl_Position[3]) ? true : false;
		if (DEBUG) std::cout << "Case 2: " << std::endl;
		break;
	case 3: //Clipping with y = -w
		is_positive = false;
		plane = 1;
		a_in = (in[0]->gl_Position[plane] >= -in[0]->gl_Position[3]) ? true : false;
		b_in = (in[1]->gl_Position[plane] >= -in[1]->gl_Position[3]) ? true : false;
		c_in = (in[2]->gl_Position[plane] >= -in[2]->gl_Position[3]) ? true : false;
		if (DEBUG) std::cout << "Case 3: " << std::endl;
		break;
	case 4: //Clipping with z = w
		is_positive = true;
		plane = 2;
		a_in = (in[0]->gl_Position[plane] <= in[0]->gl_Position[3]) ? true : false;
		b_in = (in[1]->gl_Position[plane] <= in[1]->gl_Position[3]) ? true : false;
		c_in = (in[2]->gl_Position[plane] <= in[2]->gl_Position[3]) ? true : false;
		if (DEBUG) std::cout << "Case 4: " << std::endl;
		break;
	case 5: //Clipping with z = -w
		is_positive = false;
		plane = 2;
		a_in = (in[0]->gl_Position[plane] >= -in[0]->gl_Position[3]) ? true : false;
		b_in = (in[1]->gl_Position[plane] >= -in[1]->gl_Position[3]) ? true : false;
		c_in = (in[2]->gl_Position[plane] >= -in[2]->gl_Position[3]) ? true : false;
		if (DEBUG) std::cout << "Case 5: " << std::endl;
		break;
	default:
		if (DEBUG) std::cout << "default" << std::endl;
		break;
	}

	//Setting up two data_geometry triangles to hold the possible newly created triangles
	data_geometry* triangle1[3];
	data_geometry* triangle2[3];
	for (int i = 0; i < 3; i++) {
		triangle1[i] = new data_geometry();
		triangle2[i] = new data_geometry();
	}
	//Creating two float* to hold the possible new vertex data
	float* data1 = new float[MAX_FLOATS_PER_VERTEX];
	float* data2 = new float[MAX_FLOATS_PER_VERTEX];

	if (a_in && b_in && c_in) { //Case 111
		if (DEBUG) std::cout << "All in" << std::endl;
		clip_triangle(state, in, face + 1);
	}
	else if (!a_in && b_in && c_in) {
		if (DEBUG) std::cout << "BC in" << std::endl;

		//New Triangle 1 (B,C,BA)
		triangle1[0]->gl_Position = in[1]->gl_Position;	//Set to vertex B position
		triangle1[0]->data = in[1]->data;				//Set to vertex B data

		triangle1[1]->gl_Position = in[2]->gl_Position; //Set to vertex C position
		triangle1[1]->data = in[2]->data;				//Set to vertex C data

		set_new_vertex(state, triangle1[2], in[1], in[0], plane, is_positive, data1); //Set to vertex BA

		//New Triangle 2 (C,CA,BA)
		triangle2[0]->gl_Position = in[2]->gl_Position; //Set to vertex C postion
		triangle2[0]->data = in[2]->data;				//Set to vertex C data

		set_new_vertex(state, triangle2[1], in[2], in[0], plane, is_positive, data2); //Set to vertex CA

		triangle2[2]->gl_Position = triangle1[2]->gl_Position;	//Set to vertex BA position
		triangle2[2]->data = triangle1[2]->data;				//Set to vertex BA data

		//Declaring consts for each triangle to pass to clip
		const data_geometry** t1 = const_cast<const data_geometry**>(triangle1);
		const data_geometry** t2 = const_cast<const data_geometry**>(triangle2);
		clip_triangle(state, t1, face + 1);
		clip_triangle(state, t2, face + 1);
	}
	else if (a_in && !b_in && c_in) {
		if (DEBUG) std::cout << "AC in" << std::endl;

		//New Triangle 1 (C,A,CB)
		triangle1[0]->gl_Position = in[2]->gl_Position;	//Set to vertex C position
		triangle1[0]->data = in[2]->data;				//Set to vertex C data

		triangle1[1]->gl_Position = in[0]->gl_Position; //Set to vertex A position
		triangle1[1]->data = in[0]->data;				//Set to vertex A position

		set_new_vertex(state, triangle1[2], in[2], in[1], plane, is_positive, data1); //Set vertex CB

		//New Triangle 2 (A,AB,CB)
		triangle2[0]->gl_Position = in[0]->gl_Position; //Set to vertex A position
		triangle2[0]->data = in[0]->data;				//Set to vertex A data

		set_new_vertex(state, triangle2[1], in[0], in[1], plane, is_positive, data2); //Set vertex AB

		triangle2[2]->gl_Position = triangle1[2]->gl_Position;  //Set to vertex CB position
		triangle2[2]->data = triangle1[2]->data;				//Set to vertex CB data

		//Declaring consts for each triangle to pass to clip
		const data_geometry** t1 = const_cast<const data_geometry**>(triangle1);
		const data_geometry** t2 = const_cast<const data_geometry**>(triangle2);
		clip_triangle(state, t1, face + 1);
		clip_triangle(state, t2, face + 1);
	}
	else if (a_in && b_in && !c_in) {
		if (DEBUG) std::cout << "AB in" << std::endl;

		//New Triangle 1 (A,B,AC)
		triangle1[0]->gl_Position = in[0]->gl_Position; //Set vertex to A position
		triangle1[0]->data = in[0]->data;				//Set vertex to A data

		triangle1[1]->gl_Position = in[1]->gl_Position; //Set vertex to B position
		triangle1[1]->data = in[1]->data;				//Set vertex to A data

		set_new_vertex(state, triangle1[2], in[0], in[2], plane, is_positive, data1); //Set vertex AC

		//New Triangle 2 (B,BC,AC)
		triangle2[0]->gl_Position = in[1]->gl_Position; //Set to vertex B position
		triangle2[0]->data = in[1]->data;				//Set to vertex B data

		set_new_vertex(state, triangle2[1], in[1], in[2], plane, is_positive, data2); //Set vertex BC

		triangle2[2]->gl_Position = triangle1[2]->gl_Position;  //Set to vertex AC position
		triangle2[2]->data = triangle1[2]->data;				//Set to vertex AC data

		//Declaring consts for each triangle to pass to clip
		const data_geometry** t1 = const_cast<const data_geometry**>(triangle1);
		const data_geometry** t2 = const_cast<const data_geometry**>(triangle2);
		clip_triangle(state, t1, face + 1);
		clip_triangle(state, t2, face + 1);
	}
	else if (!a_in && !b_in && c_in) {
		if (DEBUG) std::cout << "C in" << std::endl;

		//New Triangle 1 (C,CA,CB)
		triangle1[0]->gl_Position = in[2]->gl_Position; //Set to vertex C position
		triangle1[0]->data = in[2]->data;				//Set to vertex C data

		set_new_vertex(state, triangle1[1], in[2], in[0], plane, is_positive, data1); //Set vertex CA
		set_new_vertex(state, triangle1[2], in[2], in[1], plane, is_positive, data2); //Set vertex CB

		//Declaring a const the new triangle to pass to clip
		const data_geometry** t1 = const_cast<const data_geometry**>(triangle1);
		clip_triangle(state, t1, face + 1);
	}
	else if (a_in && !b_in && !c_in) {
		if (DEBUG) std::cout << "A in" << std::endl;

		//New Triangle 1 (A,AB,AC)
		triangle1[0]->gl_Position = in[0]->gl_Position; //Set to vertex A position
		triangle1[0]->data = in[0]->data;				//Set to vertex A data
		set_new_vertex(state, triangle1[1], in[0], in[1], plane, is_positive, data1); //Set vertex AB
		set_new_vertex(state, triangle1[2], in[0], in[2], plane, is_positive, data2); //Set vertex AC

		//Declaring a const the new triangle to pass to clip
		const data_geometry** t1 = const_cast<const data_geometry**>(triangle1);
		clip_triangle(state, t1, face + 1);
	}
	else if (!a_in && b_in && !c_in) {
		if (DEBUG) std::cout << "B in" << std::endl;

		//New Triangle 1 (B,BC,BA)
		triangle1[0]->gl_Position = in[1]->gl_Position; //Set to vertex B position
		triangle1[0]->data = in[1]->data;				//Set to vertex B position
		set_new_vertex(state, triangle1[1], in[1], in[2], plane, is_positive, data1); //Set vertex BC
		set_new_vertex(state, triangle1[2], in[1], in[0], plane, is_positive, data2); //Set vertex BA

		//Declaring a const the new triangle to pass to clip
		const data_geometry** t1 = const_cast<const data_geometry**>(triangle1);
		clip_triangle(state, t1, face + 1);
	}
	else {
		if (DEBUG) std::cout << "None" << std::endl;
	}


	//Deallocation
	delete[] data1;
	delete[] data2;

	for (int i = 0; i < 3; i++) {
		delete triangle1[i];
		delete triangle2[i];
	}

}

// Rasterize the triangle defined by the three vertices in the "in" array.  This
// function is responsible for rasterization, interpolation of data to
// fragments, calling the fragment shader, and z-buffering.
void rasterize_triangle(driver_state& state, const data_geometry* in[3])
{
	int width = state.image_width;
	int height = state.image_height;

	vec2 pixel_coords[3];
	for (int i = 0; i < 3; i++) {
		//Calculates the x pixel coordinate of each vertex
		pixel_coords[i][0] = ((width / 2) * in[i]->gl_Position[0] / in[i]->gl_Position[3]) + ((width / 2) - (0.5));
		//Calculates the y pixel coordinate of each vertex
		pixel_coords[i][1] = ((height / 2) * in[i]->gl_Position[1] / in[i]->gl_Position[3]) + ((height / 2) - (0.5));
	}
		
	//Calculating Bounding Box Coordinates of the triangle
	int min_x = std::min(std::min(pixel_coords[0][0], pixel_coords[1][0]), pixel_coords[2][0]);
	int max_x = std::max(std::max(pixel_coords[0][0], pixel_coords[1][0]), pixel_coords[2][0]);
	int min_y = std::min(std::min(pixel_coords[0][1], pixel_coords[1][1]), pixel_coords[2][1]);
	int max_y = std::max(std::max(pixel_coords[0][1], pixel_coords[1][1]), pixel_coords[2][1]);

	//Calculating the total area of the triangle
	float ABC_area = get_area(pixel_coords[0], pixel_coords[1], pixel_coords[2]);

	//Looping through the bounding box
	for (int i = min_x; i <= max_x; i++) {
		for (int j = min_y; j <= max_y; j++) {

			//Declaring the barycentric coordinates
			float alpha = 0, beta = 0, gamma = 0;

			//Defining the current pixel location as a vec2
			vec2 p(i, j);
			
			//Calculating each barycentric coordinate
			alpha = get_area(p, pixel_coords[1], pixel_coords[2]) / ABC_area;
			beta = get_area(p, pixel_coords[2], pixel_coords[0]) / ABC_area;
			gamma = get_area(p, pixel_coords[0], pixel_coords[1]) / ABC_area;

			//Checking if the current location is inside the triangle
			if ((alpha >= 0) && (beta >= 0) && (gamma >= 0)) {

				//Calculating the z value of the current point
				float point_z = (alpha * in[0]->gl_Position[2] / in[0]->gl_Position[3]) +(beta * in[1]->gl_Position[2] / in[1]->gl_Position[3]) + 
					(gamma * in[2]->gl_Position[2] / in[2]->gl_Position[3]);

				//Checking if the current z value is the closest we have seen
				if (point_z < state.image_depth[get_image_index(i, j, width)]) {

					//Set the current z as the min z so far
					state.image_depth[get_image_index(i, j, width)] = point_z;

					data_output* final_color = new data_output();
					data_fragment* color_data = new data_fragment();
					float* interp_color_data = new float[MAX_FLOATS_PER_VERTEX];

					//Iterating through each element in the triangle's data
					for (int k = 0; k < state.floats_per_vertex; k++) {

						switch (state.interp_rules[k]) {
						case(interp_type::flat): { //Case flat interpolation

							interp_color_data[k] = in[0]->data[k];
							break;
						}
						case(interp_type::smooth): { //Case smooth interpolation 
							float alpha_p = 0, beta_p = 0, gamma_p = 0, c = 0;

							c = (alpha / in[0]->gl_Position[3]) + (beta / in[1]->gl_Position[3]) + (gamma / in[2]->gl_Position[3]);

							//Calculate the smooth barycentric weights
							alpha_p = alpha / (in[0]->gl_Position[3] * c);
							beta_p = beta / (in[1]->gl_Position[3] * c);
							gamma_p = gamma / (in[2]->gl_Position[3] * c);

							interp_color_data[k] = (alpha_p * in[0]->data[k] + beta_p * in[1]->data[k] + gamma_p * in[2]->data[k]);
							break;
						}
						case(interp_type::noperspective): { //Case noperspective interpolation

							//Use the screen-space barycentric weights to interpolate
							interp_color_data[k] = (alpha * in[0]->data[k] + beta * in[1]->data[k] + gamma * in[2]->data[k]);
							break;
						}
						default:;
						}
					}

					color_data->data = interp_color_data;

					const data_fragment* c_color_data = const_cast<const data_fragment*>(color_data);
					//Send the interpolated color data to the fragment shader
					state.fragment_shader(*c_color_data, *final_color, state.uniform_data);

					//Set the pixel color to the final color
					state.image_color[get_image_index(i, j, width)] =
						make_pixel(final_color->output_color[0] * 255, final_color->output_color[1] * 255, final_color->output_color[2] * 255);

					//Deallocation
					delete[] interp_color_data;
					delete final_color;
					delete color_data;
				}
			}

		}
	}
}

//Calculate the area of a triangle with vertices ABC
float get_area(vec2 a, vec2 b, vec2 c) {
	return 0.5 * (((b[0] * c[1]) - (c[0] * b[1])) - ((a[0] * c[1]) - (c[0] * a[1])) + ((a[0] * b[1]) - (b[0] * a[1])));
}

//Get the image index location of a point x,y based off of screen width 
int get_image_index(int i, int j, int width) {
	return (width * j) + i;
}

//Calculates the intersection point of a plane and a line segment defined by two points v_in and v_out and the interpolated data for that point. 
//v_in is the point inside the plane and v_out is the point outside of the plane.
//Interpolates the data for the intersection point in based off of that interp_rule for each data element
void set_new_vertex(driver_state& state, data_geometry* triangle, const data_geometry* v_in, const data_geometry* v_out, int plane, bool is_pos, float* new_data) {

	float smooth_alpha = 0, nopersp_alpha = 0;

	//Calculate the interection point between v_in and v_out
	if (is_pos)
		smooth_alpha = (v_out->gl_Position[3] - v_out->gl_Position[plane]) / (v_in->gl_Position[plane] - v_in->gl_Position[3] + v_out->gl_Position[3] - v_out->gl_Position[plane]);
	else
		smooth_alpha = (-v_out->gl_Position[3] - v_out->gl_Position[plane]) / (v_in->gl_Position[plane] + v_in->gl_Position[3] - v_out->gl_Position[3] - v_out->gl_Position[plane]);

	//Sets the output vertex's gl_Position to the intersection point
	triangle->gl_Position = smooth_alpha * v_in->gl_Position + (1 - smooth_alpha) * v_out->gl_Position;

	//Calculate the noperspect_alpha for non perspective interpolation
	nopersp_alpha = smooth_alpha * v_in->gl_Position[3] / (smooth_alpha * v_in->gl_Position[3] + (1 - smooth_alpha) * v_out->gl_Position[3]);

	//Interpolates based off of the interpolation rule for the given data element.
	for (int i = 0; i < state.floats_per_vertex; i++) {
		switch (state.interp_rules[i]) {
		case(interp_type::flat): {
			new_data[i] = v_in->data[i];
			break;
		}
		case(interp_type::smooth): {
			new_data[i] = smooth_alpha * v_in->data[i] + (1 - smooth_alpha) * v_out->data[i];

			break;
		}
		case(interp_type::noperspective): {
			new_data[i] = nopersp_alpha * v_in->data[i] + (1 - nopersp_alpha) * v_out->data[i];
			break;
		}
		default:;
		}

	}

	//Sets the output vertex's data to the interpolated data
	triangle->data = new_data;

}
